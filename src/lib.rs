use snafu::prelude::*;
use std::{
    mem::MaybeUninit,
    ops::Deref,
    os::raw::c_void,
    sync::{Mutex, OnceLock},
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

#[cfg(feature = "dep-stubs")]
mod dep_stubs;

mod fourcc;
pub use fourcc::*;

mod eco_channel;
pub use eco_channel::*;

mod pro_channel;
pub use pro_channel::*;

mod universal_capture_family;
pub use universal_capture_family::*;

mod pro_eco_capture_family;
pub use pro_eco_capture_family::*;

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
            Channel::Eco(EcoChannel::new(handle, info)?)
        } else {
            Channel::Pro(ProChannel::new(handle, info)?)
        })
    }
}

unsafe impl UniversalCaptureFamilyChannel for Channel {
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

unsafe impl ProEcoCaptureFamilyChannel for Channel {
    fn event(&self) -> sys::MWCAP_PTR {
        match self {
            Channel::Eco(ch) => ch.event(),
            Channel::Pro(ch) => ch.event(),
        }
    }
}

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

        let mut ch = Channel::open(0, 0).unwrap();

        let start_time = ch.get_device_time().unwrap();

        let audio_status = ch.get_audio_signal_status().unwrap();
        println!(
            "audio channels = {}, bit depth = {}, sample rate = {}",
            audio_status.channel_count(),
            audio_status.bits_per_sample(),
            audio_status.sample_rate(),
        );

        let video_status = ch.get_video_signal_status().unwrap();
        println!(
            "video state = {:?}, width = {}, height = {}, frame duration = {:?}",
            video_status.state(),
            video_status.image_width(),
            video_status.image_height(),
            video_status.frame_duration(),
        );

        // Try capturing some audio.
        let mut audio_frame = AudioCaptureFrame::default();
        {
            ch.start_audio_capture().unwrap();
            let notify_handle = ch
                .register_notify(NotifyEvents::AUDIO_FRAME_BUFFERED)
                .unwrap();

            match &mut ch {
                Channel::Eco(ch) => {
                    for _ in 0..5 {
                        ch.wait().unwrap();
                        let mut count = 0;
                        while ch.capture_audio_frame(&mut audio_frame).unwrap() {
                            count += 1;
                            assert!(audio_frame.timestamp() > start_time);
                        }
                        assert!(count > 0);
                    }
                }
                _ => {
                    // TODO: support pro devices better?
                    panic!("expected channel type");
                }
            }

            ch.unregister_notify(notify_handle).unwrap();
            ch.stop_audio_capture().unwrap();
        }

        // Try capturing some video.
        match ch {
            Channel::Eco(mut ch) => {
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
                    frame = loop {
                        ch.wait().unwrap();
                        if let Some(status) = ch.get_video_capture_status().unwrap() {
                            assert!(status.timestamp() > start_time);
                            break status.into_frame();
                        }
                    }
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
