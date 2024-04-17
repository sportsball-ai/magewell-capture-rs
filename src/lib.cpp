#include "lib.hpp"

DWORD MWRsLibFourCCCalcImageSize(DWORD dwFOURCC, int cx, int cy, DWORD cbStride) {
    // This function is inlined, so we need to wrap it to make it accessible to Rust.
    return FOURCC_CalcImageSize(dwFOURCC, cx, cy, cbStride);
}

DWORD MWRsLibFourCCCalcMinStride(DWORD dwFOURCC, int cx, DWORD dwAlign) {
    // This function is inlined, so we need to wrap it to make it accessible to Rust.
    return FOURCC_CalcMinStride(dwFOURCC, cx, dwAlign);
}
