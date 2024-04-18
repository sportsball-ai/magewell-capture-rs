use super::{sys, AudioCaptureFrame, Result, UniversalCaptureFamily};
use snafu::prelude::*;
use std::{os::raw::c_longlong, time::Duration};

pub trait ProEcoCaptureFamily: UniversalCaptureFamily {
    fn event(&self) -> sys::MWCAP_PTR;

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

    /// Causes `wait` to return any time the specified events (e.g.
    /// `MWCAP_NOTIFY_AUDIO_FRAME_BUFFERED`) occur. Returns a handle that can be used to
    /// unregister.
    fn register_notify(&self, events: u32) -> Result<sys::MWCAP_PTR> {
        Ok(unsafe {
            let handle = sys::MWRegisterNotify(self.handle(), self.event(), events);
            if handle == 0 {
                whatever!("unable to register notify");
            }
            handle
        })
    }

    fn unregister_notify(&self, handle: sys::MWCAP_PTR) -> Result<()> {
        unsafe {
            if sys::MWUnregisterNotify(self.handle(), handle) != sys::_MW_RESULT__MW_SUCCEEDED {
                whatever!("unable to unregister notify");
            }
        }
        Ok(())
    }

    fn start_audio_capture(&mut self) -> Result<()> {
        unsafe {
            if sys::MWStartAudioCapture(self.handle()) != sys::_MW_RESULT__MW_SUCCEEDED {
                whatever!("unable to start audio capture");
            }
        }
        Ok(())
    }

    fn stop_audio_capture(&mut self) -> Result<()> {
        unsafe {
            if sys::MWStopAudioCapture(self.handle()) != sys::_MW_RESULT__MW_SUCCEEDED {
                whatever!("unable to start audio capture");
            }
        }
        Ok(())
    }

    /// Fills the given frame, if audio is available. Returns false if no audio is available.
    ///
    /// You can wait for `MWCAP_NOTIFY_AUDIO_FRAME_BUFFERED` to ensure audio is available.
    fn capture_audio_frame(&mut self, frame: &mut AudioCaptureFrame) -> Result<bool> {
        frame.inner.dwSyncCode = 0;
        unsafe {
            match sys::MWCaptureAudioFrame(self.handle(), &mut frame.inner as _) {
                sys::_MW_RESULT__MW_SUCCEEDED => Ok(frame.inner.dwSyncCode != 0),
                sys::_MW_RESULT__MW_ENODATA => Ok(false),
                _ => whatever!("unable to capture audio frame"),
            }
        }
    }
}
