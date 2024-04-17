use super::sys;

/// A four character code representing a pixel format. See
/// vendor/Magewell_Capture_SDK_Linux_3.3.1.1313/Include/MWFOURCC.h for detailed information.
pub struct FourCC(u32);

impl FourCC {
    pub const fn new(a: char, b: char, c: char, d: char) -> Self {
        Self(a as u32 | (b as u32) << 8 | (c as u32) << 16 | (d as u32) << 24)
    }

    pub fn as_u32(&self) -> u32 {
        self.0
    }

    pub fn min_stride(&self, width: u16, alignment: usize) -> usize {
        unsafe { sys::MWRsLibFourCCCalcMinStride(self.0, width as _, alignment as _) as _ }
    }

    pub fn image_size(&self, width: u16, height: u16, stride: usize) -> usize {
        unsafe {
            sys::MWRsLibFourCCCalcImageSize(self.0, width as _, height as _, stride as _) as _
        }
    }
}
