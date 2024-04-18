use super::{sys, AudioSignalStatus, ChannelInfo, Result, VideoSignalStatus};
use snafu::prelude::*;
use std::{ffi::c_void, mem::MaybeUninit};

/// # Safety
/// The pointers returned by implementations of this trait must be valid.
pub unsafe trait UniversalCaptureFamilyChannel {
    fn handle(&self) -> *mut c_void;

    fn info(&self) -> &ChannelInfo;

    fn get_audio_signal_status(&self) -> Result<AudioSignalStatus> {
        let mut status = MaybeUninit::uninit();
        unsafe {
            if sys::MWGetAudioSignalStatus(self.handle(), status.as_mut_ptr())
                != sys::_MW_RESULT__MW_SUCCEEDED
            {
                whatever!("unable to get audio signal status");
            }
            Ok(status.assume_init().into())
        }
    }

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
