////////////////////////////////////////////////////////////////////////////////
// CONFIDENTIAL and PROPRIETARY software of Magewell Electronics Co., Ltd.
// Copyright (c) 2011-2014 Magewell Electronics Co., Ltd. (Nanjing)
// All rights reserved.
// This copyright notice MUST be reproduced on all authorized copies.
////////////////////////////////////////////////////////////////////////////////
#ifndef VIDEORENDERBASE_H
#define VIDEORENDERBASE_H

#ifdef __cplusplus
extern "C" {
#endif
#include <stdint.h>
#include <unistd.h>
#ifdef __cplusplus
}
#endif
#include "MWFOURCC.h"
//#include "win-types.h"
//#include "mw-fourcc.h"
#include "mw-utils.h"

#define VIDEO_RENDER_PIXEL_RGB32

enum VideoRenderImageBufType {
    VIDEO_RENDER_IMAGEBUF_VIRTUAL = 0,
    VIDEO_RENDER_IMAGEBUF_PHYSICAL,
};

typedef void (*ExitCleanCallback)(void *data);

typedef struct _VideoRenderImageBuf {
    int                 type; /* VideoRenderImageBufType */
    void                *buf;
    size_t              size;
} VideoRenderImageBuf;

class VideoRenderBase
{
public:
    VideoRenderBase();

    virtual void RunLoop(ExitCleanCallback exit_call, void *data) = 0;

    virtual char *GetName() = 0;

    /* the return value should be release with delete */
    virtual uint32_t *GetSupportedFourcc(int &nCount) = 0;

    virtual bool SetFrameFormat(int nWidth, int nHeight, uint32_t dwColorSpace, bool bTry = false) = 0;

    /* image buffers */
    virtual bool IsSupportImageBufType(VideoRenderImageBufType type) {
        return false;
    }
    virtual VideoRenderImageBuf *AllocImageBuffer(VideoRenderImageBufType type) {
        return NULL;
    }
    virtual void FreeImageBuffer(VideoRenderImageBuf *imageBuf) {
    }

    virtual void DisplayFrame(VideoRenderImageBuf *imageBuf) {
    }
    virtual int DisplayFrame(void *buffer, size_t size) {
    }

    virtual void DisplayFps(unsigned long long frameCount, double fps) {
    }

protected:
    int                                 m_nWidth;
    int                                 m_nHeight;
    uint32_t                            m_dwColorSpace;
    size_t                              m_BufferSize;
};

#endif // VIDEORENDERBASE_H
