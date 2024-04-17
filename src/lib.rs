use nix::sys::eventfd::EventFd;
use snafu::prelude::*;
use std::{
    mem::MaybeUninit,
    ops::Deref,
    os::{
        fd::AsRawFd,
        raw::{c_longlong, c_void},
    },
    pin::Pin,
    sync::{Mutex, OnceLock},
    time::Duration,
};

pub mod sys {
    #![allow(
        clippy::all,
        non_upper_case_globals,
        non_snake_case,
        non_camel_case_types,
        unused
    )]
    include!(concat!(env!("OUT_DIR"), "/bindings.rs"));
}

mod fourcc;
pub use fourcc::*;

// Contains simple wrappers around the Magewell SDK types.
mod types;
pub use types::*;

// The Magewell SDK doesn't really give us error codes, so we just use "whatever".
pub type Result<T, E = snafu::Whatever> = std::result::Result<T, E>;

static INIT_ONCE: OnceLock<bool> = OnceLock::new();

/// Initializes the MW API. Typically this doesn't need to be called directly, as it will be called
/// automatically when needed.
pub fn init() -> bool {
    *INIT_ONCE.get_or_init(|| unsafe { sys::MWCaptureInitInstance() != 0 })
}

/// You'll get unpredictable results if multiple callers are simultaneously using
/// `MWRefreshDevice`, `MWGetChannelCount`, and other functions that operate on the SDK's global
/// device list. This mutex synchronizes access to those functions.
pub static DEVICE_LIST_MUTEX: Mutex<()> = Mutex::new(());

/// Returns info for all available capture channels.
pub fn get_channel_info() -> Result<Vec<ChannelInfo>> {
    if !init() {
        whatever!("unable to initialize magewell api");
    }

    let _lock = DEVICE_LIST_MUTEX
        .lock()
        .whatever_context("unable to lock device list")?;

    unsafe {
        if sys::MWRefreshDevice() != sys::_MW_RESULT__MW_SUCCEEDED {
            whatever!("unable to refresh device list");
        }
    }

    let channel_count = unsafe { sys::MWGetChannelCount() };

    (0..channel_count)
        .map(|i| -> Result<ChannelInfo> {
            let mut info = MaybeUninit::uninit();
            unsafe {
                if sys::MWGetChannelInfoByIndex(i, info.as_mut_ptr())
                    != sys::_MW_RESULT__MW_SUCCEEDED
                {
                    whatever!("unable to get channel info");
                }
                Ok(info.assume_init().into())
            }
        })
        .collect::<Result<Vec<_>>>()
}

/// Magewell capture channels have slightly different APIs depending on whether they're from the
/// Eco or Pro device family.
pub enum Channel {
    Eco(EcoChannel),
    Pro(ProChannel),
}

struct ChannelHandle(*mut c_void);

impl Deref for ChannelHandle {
    type Target = *mut c_void;

    fn deref(&self) -> &Self::Target {
        &self.0
    }
}

impl Drop for ChannelHandle {
    fn drop(&mut self) {
        unsafe { sys::MWCloseChannel(self.0) };
    }
}

impl Channel {
    /// Opens an Eco or Pro device based on the board and channel index.
    pub fn open(board_index: u8, channel_index: u8) -> Result<Self> {
        if !init() {
            whatever!("unable to initialize magewell api");
        }

        let handle = {
            let _lock = DEVICE_LIST_MUTEX
                .lock()
                .whatever_context("unable to lock device list")?;

            unsafe {
                if sys::MWRefreshDevice() != sys::_MW_RESULT__MW_SUCCEEDED {
                    whatever!("unable to refresh device list");
                }
            }

            let handle = unsafe { sys::MWOpenChannel(board_index as _, channel_index as _) };
            if handle.is_null() {
                whatever!("unable to open channel");
            }
            ChannelHandle(handle)
        };

        let info: ChannelInfo = {
            let mut info = MaybeUninit::uninit();
            unsafe {
                if sys::MWGetChannelInfo(*handle, info.as_mut_ptr())
                    != sys::_MW_RESULT__MW_SUCCEEDED
                {
                    whatever!("unable to get channel info");
                }
                info.assume_init().into()
            }
        };

        Ok(if info.family_name().to_str() == Ok("Eco Capture") {
            let event_fd = EventFd::new().whatever_context("unable to create eventfd")?;
            Channel::Eco(EcoChannel {
                handle,
                info,
                event_fd,
                video_capture_frame: None,
            })
        } else {
            Channel::Pro(ProChannel { handle, info })
        })
    }
}

pub trait UniversalCaptureFamily {
    fn handle(&self) -> *mut c_void;

    fn info(&self) -> &ChannelInfo;

    fn get_video_signal_status(&self) -> Result<VideoSignalStatus> {
        let mut status = MaybeUninit::uninit();
        unsafe {
            if sys::MWGetVideoSignalStatus(self.handle(), status.as_mut_ptr())
                != sys::_MW_RESULT__MW_SUCCEEDED
            {
                whatever!("unable to get video signal status");
            }
            Ok(status.assume_init().into())
        }
    }
}

pub trait ProEcoCaptureFamily: UniversalCaptureFamily {
    fn get_device_time(&self) -> Result<Duration> {
        let mut time: c_longlong = 0;
        unsafe {
            if sys::MWGetDeviceTime(self.handle(), &mut time as *mut _)
                != sys::_MW_RESULT__MW_SUCCEEDED
            {
                whatever!("unable to get device time");
            }
            Ok(Duration::from_nanos(100 * time as u64))
        }
    }
}

impl UniversalCaptureFamily for Channel {
    fn handle(&self) -> *mut c_void {
        match self {
            Channel::Eco(ch) => ch.handle(),
            Channel::Pro(ch) => ch.handle(),
        }
    }

    fn info(&self) -> &ChannelInfo {
        match self {
            Channel::Eco(ch) => ch.info(),
            Channel::Pro(ch) => ch.info(),
        }
    }
}

pub struct EcoChannel {
    handle: ChannelHandle,
    info: ChannelInfo,
    event_fd: EventFd,
    // hold onto a reference of the currently set capture frame
    video_capture_frame: Option<Pin<Box<EcoVideoCaptureFrame>>>,
}

impl UniversalCaptureFamily for EcoChannel {
    fn handle(&self) -> *mut c_void {
        *self.handle
    }

    fn info(&self) -> &ChannelInfo {
        &self.info
    }
}

impl ProEcoCaptureFamily for EcoChannel {}

impl EcoChannel {
    pub fn start_video_capture(&mut self, width: u16, height: u16, format: FourCC) -> Result<()> {
        let mut params = sys::_MWCAP_VIDEO_ECO_CAPTURE_OPEN {
            cx: width as _,
            cy: height as _,
            dwFOURCC: format.as_u32(),
            llFrameDuration: -1,
            hEvent: self.event_fd.as_raw_fd() as _,
        };
        unsafe {
            if sys::MWStartVideoEcoCapture(self.handle(), &mut params as *mut _)
                != sys::_MW_RESULT__MW_SUCCEEDED
            {
                whatever!("unable to start video capture");
            }
        }
        Ok(())
    }

    pub fn stop_video_capture(&mut self) -> Result<()> {
        unsafe {
            if sys::MWStopVideoEcoCapture(self.handle()) != sys::_MW_RESULT__MW_SUCCEEDED {
                whatever!("unable to stop video capture");
            }
        }
        Ok(())
    }

    pub fn set_video_capture_frame(&mut self, frame: EcoVideoCaptureFrame) -> Result<()> {
        if self.video_capture_frame.is_some() {
            whatever!("video frame already set");
        }
        let mut frame = Box::pin(frame);
        unsafe {
            if sys::MWCaptureSetVideoEcoFrame(self.handle(), frame.as_mut_ptr())
                != sys::_MW_RESULT__MW_SUCCEEDED
            {
                whatever!("unable to set video frame");
            }
        }
        self.video_capture_frame = Some(frame);
        Ok(())
    }

    pub fn get_video_capture_status(&mut self) -> Result<EcoVideoCaptureStatus> {
        let status = unsafe {
            let mut status = MaybeUninit::uninit();
            if sys::MWGetVideoEcoCaptureStatus(self.handle(), status.as_mut_ptr())
                != sys::_MW_RESULT__MW_SUCCEEDED
            {
                whatever!("unable to get video capture status");
            }
            status.assume_init()
        };
        Ok(EcoVideoCaptureStatus::new(
            *Pin::into_inner(
                self.video_capture_frame
                    .take()
                    .whatever_context("no video frame set")?,
            ),
            status,
        ))
    }

    pub fn wait(&self) -> Result<()> {
        if self
            .event_fd
            .read()
            .whatever_context("unable to read eventfd")?
            == 0
        {
            whatever!("error event received");
        }
        Ok(())
    }
}

pub struct ProChannel {
    handle: ChannelHandle,
    info: ChannelInfo,
}

impl UniversalCaptureFamily for ProChannel {
    fn handle(&self) -> *mut c_void {
        *self.handle
    }

    fn info(&self) -> &ChannelInfo {
        &self.info
    }
}

impl ProEcoCaptureFamily for ProChannel {}

// Theses tests will pass if there are no devices present, but to really get their full value, an
// Eco device should be present and channel 0:0 should be connected to a video source.
#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_channel_info() {
        for ch in get_channel_info().unwrap() {
            println!(
                "[{}:{}] {} ({}), driver = {:?}, fw = {:?}, hw = {}",
                ch.board_index(),
                ch.channel_index(),
                ch.product_name().to_str().unwrap(),
                ch.board_serial_number().to_str().unwrap(),
                ch.driver_version(),
                ch.firmware_version(),
                ch.hardware_version(),
            );
        }
    }

    #[test]
    fn test_channel() {
        if get_channel_info().unwrap().is_empty() {
            println!("skipping test, no devices found");
            return;
        }

        let ch = Channel::open(0, 0).unwrap();

        let video_status = ch.get_video_signal_status().unwrap();
        println!(
            "video state = {:?}, width = {}, height = {}, frame duration = {:?}",
            video_status.state(),
            video_status.image_width(),
            video_status.image_height(),
            video_status.frame_duration(),
        );

        match ch {
            Channel::Eco(mut ch) => {
                let start_time = ch.get_device_time().unwrap();
                let format = FourCC::new('B', 'G', 'R', ' ');
                let stride = format.min_stride(video_status.image_width(), 4);
                let image_size = format.image_size(
                    video_status.image_width(),
                    video_status.image_height(),
                    stride,
                );

                ch.start_video_capture(
                    video_status.image_width(),
                    video_status.image_height(),
                    format,
                )
                .unwrap();

                let mut frame = EcoVideoCaptureFrame::new(image_size, stride);

                for _ in 0..5 {
                    ch.set_video_capture_frame(frame).unwrap();
                    ch.wait().unwrap();
                    let status = ch.get_video_capture_status().unwrap();
                    assert!(status.timestamp() > start_time);
                    frame = status.into_frame();
                }

                ch.stop_video_capture().unwrap();
            }
            _ => {
                // TODO: support pro devices better?
                panic!("expected channel type");
            }
        }
    }
}
