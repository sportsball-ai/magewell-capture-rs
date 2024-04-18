use super::sys;
use bitflags::bitflags;
use std::{ffi::CStr, os::raw::c_char, time::Duration};

fn bytes_to_cstr(bytes: &[c_char]) -> &CStr {
    unsafe { CStr::from_ptr(bytes.as_ptr()) }
}

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
        bytes_to_cstr(&self.inner.szBoardSerialNo)
    }

    pub fn firmware_name(&self) -> &CStr {
        bytes_to_cstr(&self.inner.szFirmwareName)
    }

    pub fn product_name(&self) -> &CStr {
        bytes_to_cstr(&self.inner.szProductName)
    }

    pub fn family_name(&self) -> &CStr {
        bytes_to_cstr(&self.inner.szFamilyName)
    }

    pub fn firmware_version(&self) -> (u16, u16) {
        let v = self.inner.dwFirmwareVersion;
        ((v >> 16) as _, v as _)
    }

    pub fn driver_version(&self) -> (u8, u8, u16) {
        let v = self.inner.dwDriverVersion;
        ((v >> 24) as _, (v >> 16) as _, v as _)
    }

    // The cast is necessary on some targets (e.g. x86_64-unknown-linux-gnu), but not others.
    #[allow(clippy::unnecessary_cast)]
    pub fn hardware_version(&self) -> char {
        self.inner.chHardwareVersion as u8 as _
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

pub struct AudioSignalStatus {
    inner: sys::MWCAP_AUDIO_SIGNAL_STATUS,
}

impl AudioSignalStatus {
    pub fn is_lpcm(&self) -> bool {
        self.inner.bLPCM != 0
    }

    pub fn channel_count(&self) -> u32 {
        self.inner.wChannelValid.count_ones() * 2
    }

    pub fn bits_per_sample(&self) -> u8 {
        self.inner.cBitsPerSample
    }

    pub fn sample_rate(&self) -> u32 {
        self.inner.dwSampleRate
    }
}

impl From<sys::MWCAP_AUDIO_SIGNAL_STATUS> for AudioSignalStatus {
    fn from(status: sys::MWCAP_AUDIO_SIGNAL_STATUS) -> Self {
        AudioSignalStatus { inner: status }
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
    buf: Box<[u8]>,
    inner: sys::_MWCAP_VIDEO_ECO_CAPTURE_FRAME,
}

impl EcoVideoCaptureFrame {
    pub fn new(size: usize, stride: usize) -> Self {
        let mut buf = vec![0; size].into_boxed_slice();
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

pub struct AudioCaptureFrame {
    pub(crate) inner: sys::_MWCAP_AUDIO_CAPTURE_FRAME,
}

impl Default for AudioCaptureFrame {
    fn default() -> Self {
        Self {
            inner: sys::_MWCAP_AUDIO_CAPTURE_FRAME {
                iFrame: 0,
                dwReserved: 0,
                dwSyncCode: 0,
                cFrameCount: 0,
                llTimestamp: 0,
                adwSamples: [0; (sys::MWCAP_AUDIO_SAMPLES_PER_FRAME
                    * sys::MWCAP_AUDIO_MAX_NUM_CHANNELS) as _],
            },
        }
    }
}

// In the future, this can be replaced with `ptr.is_aligned()`.
fn is_aligned<T>(ptr: *const T) -> bool {
    ptr.align_offset(std::mem::align_of::<T>()) == 0
}

impl AudioCaptureFrame {
    /// For LPCM, the channel order is 0L, 1L, 2L, 3L, 0R, 1R, 2R, 3R.
    pub fn samples(&self) -> &[u32] {
        // We have to do this in a slightly round-about way to appease the compiler, which
        // otherwise complains about the possibility of `adwSamples` being unaligned (even though
        // it always is).
        let ptr = std::ptr::addr_of!(self.inner.adwSamples);
        // Proof that our alignment is fine:
        assert!(is_aligned(ptr));
        unsafe { (*ptr).as_slice() }
    }

    pub fn timestamp(&self) -> Duration {
        Duration::from_nanos(100 * self.inner.llTimestamp as u64)
    }
}

impl From<sys::_MWCAP_AUDIO_CAPTURE_FRAME> for AudioCaptureFrame {
    fn from(frame: sys::_MWCAP_AUDIO_CAPTURE_FRAME) -> Self {
        AudioCaptureFrame { inner: frame }
    }
}

bitflags! {
    pub struct NotifyEvents: u32 {
        const INPUT_SORUCE_START_SCAN = 1;
        const INPUT_SORUCE_STOP_SCAN = 2;
        const INPUT_SORUCE_SCAN_CHANGE = 3;
        const VIDEO_INPUT_SOURCE_CHANGE = 4;
        const AUDIO_INPUT_SOURCE_CHANGE = 8;
        const INPUT_SPECIFIC_CHANGE = 16;
        const VIDEO_SIGNAL_CHANGE = 32;
        const AUDIO_SIGNAL_CHANGE = 64;
        const VIDEO_FIELD_BUFFERING = 128;
        const VIDEO_FRAME_BUFFERING = 256;
        const VIDEO_FIELD_BUFFERED = 512;
        const VIDEO_FRAME_BUFFERED = 1024;
        const VIDEO_SMPTE_TIME_CODE = 2048;
        const AUDIO_FRAME_BUFFERED = 4096;
        const AUDIO_INPUT_RESET = 8192;
        const VIDEO_SAMPLING_PHASE_CHANGE = 16384;
        const LOOP_THROUGH_CHANGED = 32768;
        const LOOP_THROUGH_EDID_CHANGED = 65536;
        const NEW_SDI_ANC_PACKET = 131072;
    }
}
