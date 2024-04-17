use super::sys;
use std::{ffi::CStr, time::Duration};

pub struct ChannelInfo {
    inner: sys::MWCAP_CHANNEL_INFO,
}

impl ChannelInfo {
    pub fn board_index(&self) -> u8 {
        self.inner.byBoardIndex
    }

    pub fn channel_index(&self) -> u8 {
        self.inner.byChannelIndex
    }

    pub fn board_serial_number(&self) -> &CStr {
        CStr::from_bytes_until_nul(&self.inner.szBoardSerialNo).expect("invalid serial number")
    }

    pub fn firmware_name(&self) -> &CStr {
        CStr::from_bytes_until_nul(&self.inner.szFirmwareName).expect("invalid firmware name")
    }

    pub fn product_name(&self) -> &CStr {
        CStr::from_bytes_until_nul(&self.inner.szProductName).expect("invalid product name")
    }

    pub fn family_name(&self) -> &CStr {
        CStr::from_bytes_until_nul(&self.inner.szFamilyName).expect("invalid family name")
    }

    pub fn firmware_version(&self) -> (u16, u16) {
        let v = self.inner.dwFirmwareVersion;
        ((v >> 16) as _, v as _)
    }

    pub fn driver_version(&self) -> (u8, u8, u16) {
        let v = self.inner.dwDriverVersion;
        ((v >> 24) as _, (v >> 16) as _, v as _)
    }

    pub fn hardware_version(&self) -> char {
        self.inner.chHardwareVersion as _
    }
}

impl From<sys::MWCAP_CHANNEL_INFO> for ChannelInfo {
    fn from(mut info: sys::MWCAP_CHANNEL_INFO) -> Self {
        // For some reason serial numbers have a bunch of spaces at the end. We'll go ahead and
        // trim them here.
        for c in info.szBoardSerialNo.iter_mut().rev() {
            if *c != 0 && *c != 0x20 {
                break;
            }
            *c = 0;
        }

        ChannelInfo { inner: info }
    }
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum VideoSignalState {
    None,
    Locked,
    Locking,
    Unsupported,
    Other,
}

impl From<sys::MWCAP_VIDEO_SIGNAL_STATE> for VideoSignalState {
    fn from(state: sys::MWCAP_VIDEO_SIGNAL_STATE) -> Self {
        match state {
            sys::_MWCAP_VIDEO_SIGNAL_STATE_MWCAP_VIDEO_SIGNAL_NONE => Self::None,
            sys::_MWCAP_VIDEO_SIGNAL_STATE_MWCAP_VIDEO_SIGNAL_LOCKED => Self::Locked,
            sys::_MWCAP_VIDEO_SIGNAL_STATE_MWCAP_VIDEO_SIGNAL_LOCKING => Self::Locking,
            sys::_MWCAP_VIDEO_SIGNAL_STATE_MWCAP_VIDEO_SIGNAL_UNSUPPORTED => Self::Unsupported,
            _ => Self::Other,
        }
    }
}

pub struct VideoSignalStatus {
    inner: sys::MWCAP_VIDEO_SIGNAL_STATUS,
}

impl VideoSignalStatus {
    pub fn state(&self) -> VideoSignalState {
        self.inner.state.into()
    }

    pub fn image_width(&self) -> u16 {
        self.inner.cx as _
    }

    pub fn image_height(&self) -> u16 {
        self.inner.cy as _
    }

    pub fn frame_duration(&self) -> Duration {
        Duration::from_nanos(100 * self.inner.dwFrameDuration as u64)
    }
}

impl From<sys::MWCAP_VIDEO_SIGNAL_STATUS> for VideoSignalStatus {
    fn from(status: sys::MWCAP_VIDEO_SIGNAL_STATUS) -> Self {
        VideoSignalStatus { inner: status }
    }
}

pub struct EcoVideoCaptureFrame {
    // XXX: `_buf` is referenced by `inner`!
    buf: Vec<u8>,
    inner: sys::_MWCAP_VIDEO_ECO_CAPTURE_FRAME,
}

impl EcoVideoCaptureFrame {
    pub fn new(size: usize, stride: usize) -> Self {
        let mut buf = vec![0; size];
        Self {
            inner: sys::_MWCAP_VIDEO_ECO_CAPTURE_FRAME {
                pvFrame: buf.as_mut_ptr() as _,
                cbFrame: size as _,
                cbStride: stride as _,
                bBottomUp: 0,
                deinterlaceMode: sys::_MWCAP_VIDEO_DEINTERLACE_MODE_MWCAP_VIDEO_DEINTERLACE_BLEND,
                pvContext: 0,
            },
            buf,
        }
    }

    pub fn as_slice(&self) -> &[u8] {
        &self.buf
    }

    pub fn as_mut_ptr(&mut self) -> *mut sys::_MWCAP_VIDEO_ECO_CAPTURE_FRAME {
        &mut self.inner
    }
}

pub struct EcoVideoCaptureStatus {
    frame: EcoVideoCaptureFrame,
    status: sys::_MWCAP_VIDEO_ECO_CAPTURE_STATUS,
}

impl EcoVideoCaptureStatus {
    pub(crate) fn new(
        frame: EcoVideoCaptureFrame,
        status: sys::_MWCAP_VIDEO_ECO_CAPTURE_STATUS,
    ) -> Self {
        Self { frame, status }
    }

    pub fn frame(&self) -> &EcoVideoCaptureFrame {
        &self.frame
    }

    pub fn into_frame(self) -> EcoVideoCaptureFrame {
        self.frame
    }

    pub fn timestamp(&self) -> Duration {
        Duration::from_nanos(100 * self.status.llTimestamp as u64)
    }
}
