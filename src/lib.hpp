#include "MWFOURCC.h"
#include "LibMWCapture/MWCapture.h"
#include "LibMWCapture/MWEcoCapture.h"

DWORD MWRsLibFourCCCalcImageSize(DWORD dwFOURCC, int cx, int cy, DWORD cbStride);
DWORD MWRsLibFourCCCalcMinStride(DWORD dwFOURCC, int cx, DWORD dwAlign);
