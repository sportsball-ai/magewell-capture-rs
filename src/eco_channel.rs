use super::{
    sys, ChannelHandle, ChannelInfo, EcoVideoCaptureFrame, EcoVideoCaptureStatus, FourCC,
    ProEcoCaptureFamilyChannel, Result, UniversalCaptureFamilyChannel,
};
use nix::sys::eventfd::EventFd;
use snafu::prelude::*;
use std::{boxed::Box, ffi::c_void, mem::MaybeUninit, os::fd::AsRawFd, pin::Pin};

pub struct EcoChannel {
    handle: ChannelHandle,
    info: ChannelInfo,
    event_fd: EventFd,
    // hold onto a reference of the currently set capture frame
    video_capture_frame: Option<Pin<Box<EcoVideoCaptureFrame>>>,
}

unsafe impl UniversalCaptureFamilyChannel for EcoChannel {
    fn handle(&self) -> *mut c_void {
        *self.handle
    }

    fn info(&self) -> &ChannelInfo {
        &self.info
    }
}

unsafe impl ProEcoCaptureFamilyChannel for EcoChannel {
    fn event(&self) -> sys::MWCAP_PTR {
        self.event_fd.as_raw_fd() as _
    }
}

impl EcoChannel {
    pub(crate) fn new(handle: ChannelHandle, info: ChannelInfo) -> Result<Self> {
        let event_fd = EventFd::new().whatever_context("unable to create eventfd")?;
        Ok(Self {
            handle,
            info,
            event_fd,
            video_capture_frame: None,
        })
    }

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

    /// Returns the next video capture frame, if available. If no frame is available, returns
    /// `None`. Invoke `wait` to block until a frame may be available.
    pub fn get_video_capture_status(&mut self) -> Result<Option<EcoVideoCaptureStatus>> {
        let status = unsafe {
            let mut status = MaybeUninit::uninit();
            if sys::MWGetVideoEcoCaptureStatus(self.handle(), status.as_mut_ptr())
                != sys::_MW_RESULT__MW_SUCCEEDED
            {
                whatever!("unable to get video capture status");
            }
            status.assume_init()
        };
        if status.pvFrame == 0 {
            return Ok(None);
        }
        Ok(Some(EcoVideoCaptureStatus::new(
            *Pin::into_inner(
                self.video_capture_frame
                    .take()
                    .whatever_context("no video frame set")?,
            ),
            status,
        )))
    }

    /// Blocks until the next video frame is available or until an event registered via
    /// `register_notify`.
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
