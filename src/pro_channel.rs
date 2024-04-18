use super::{sys, ChannelHandle, ChannelInfo, ProEcoCaptureFamily, Result, UniversalCaptureFamily};
use snafu::prelude::*;
use std::ffi::c_void;

pub struct ProChannel {
    handle: ChannelHandle,
    info: ChannelInfo,
    event: sys::MWCAP_PTR,
}

impl Drop for ProChannel {
    fn drop(&mut self) {
        unsafe { sys::MWCloseEvent(self.event) };
    }
}

impl ProChannel {
    pub(crate) fn new(handle: ChannelHandle, info: ChannelInfo) -> Result<Self> {
        let event = unsafe { sys::MWCreateEvent() };
        if event == 0 {
            whatever!("unable to create event");
        }
        Ok(Self {
            handle,
            info,
            event,
        })
    }
}

impl UniversalCaptureFamily for ProChannel {
    fn handle(&self) -> *mut c_void {
        *self.handle
    }

    fn info(&self) -> &ChannelInfo {
        &self.info
    }
}

impl ProEcoCaptureFamily for ProChannel {
    fn event(&self) -> sys::MWCAP_PTR {
        self.event
    }
}
