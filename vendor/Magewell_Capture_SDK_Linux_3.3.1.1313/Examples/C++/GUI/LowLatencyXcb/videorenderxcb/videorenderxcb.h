////////////////////////////////////////////////////////////////////////////////
// CONFIDENTIAL and PROPRIETARY software of Magewell Electronics Co., Ltd.
// Copyright (c) 2011-2014 Magewell Electronics Co., Ltd. (Nanjing)
// All rights reserved.
// This copyright notice MUST be reproduced on all authorized copies.
////////////////////////////////////////////////////////////////////////////////
#ifndef VIDEORENDERXCB_H
#define VIDEORENDERXCB_H

#ifdef __cplusplus
extern "C" {
#endif
#include <stdio.h>
#include <string.h>

#include <sys/ipc.h>
#include <sys/shm.h>

#include <xcb/xcb.h>
#include <xcb/xproto.h>
#include <xcb/xv.h>
#ifdef __cplusplus
}
#endif

#include "videorenderbase.h"

class VideoRenderXCB : public VideoRenderBase
{
public:
    VideoRenderXCB();
    virtual ~VideoRenderXCB();

    typedef struct _VideoRenderImageBufXCB : public VideoRenderImageBuf {
        bool                bIsShmMem;
        xcb_shm_seg_t       xSegment;
    } VideoRenderImageBufXCB;

    bool Init(int nWindowWidth, int nWindowHeight, xcb_drawable_t xWindowParent = -1);

    char *GetName() { return (char *)"xv"; }

    virtual uint32_t *GetSupportedFourcc(int &nCount);

    virtual bool SetFrameFormat(int nWidth, int nHeight, uint32_t dwColorSpace, bool bTry = false);

    virtual bool IsSupportImageBufType(VideoRenderImageBufType type) {
        if (type == VIDEO_RENDER_IMAGEBUF_VIRTUAL)
            return true;

        return false;
    }
    virtual VideoRenderImageBuf *AllocImageBuffer(VideoRenderImageBufType type);
    virtual void FreeImageBuffer(VideoRenderImageBuf *imageBuf);

    virtual void RunLoop(ExitCleanCallback exit_call, void *data);

    virtual void DisplayFrame(VideoRenderImageBuf *imageBuf);
    virtual int DisplayFrame(void *buffer, size_t size);

    virtual void DisplayFps(unsigned long long frameCount, double fps);

protected:

private:
    static bool CheckXVideo(xcb_connection_t *conn);

    void DisplayLastFrame();

    void AutoDetectAdaptor();
    bool IsSupportShm();
    bool CreateXImage(VideoRenderImageBufXCB *pImageBuf);
    void DestroyXImage(VideoRenderImageBufXCB *pImageBuf);
    void ScanSupportedFourcc();
    uint32_t XCBGetFourcc(const xcb_xv_image_format_info_t *f);

    uint32_t FindXVFormatIDFromMWFourcc(uint32_t fourcc);

    xcb_connection_t        *m_pConnection;
    xcb_screen_t            *m_pScreen;
    xcb_drawable_t          m_xWindow;
    xcb_gcontext_t          m_xGC;
    xcb_xv_port_t           m_xvPort;
    uint32_t                m_xvid;

    /* last dispaly frame */
    VideoRenderImageBuf     *m_lastImageBuf;
    void                    *m_lastBuffer;
    size_t                  m_lastBufSize;

    /* device */
    typedef struct _FourccMap {
        uint32_t            mwFourcc;
        uint32_t            xvId;
    } FourccMap;

    static bool fourcc_is_equal(FourccMap &item1, FourccMap &item2) {
        return item1.mwFourcc == item2.mwFourcc;
    }

    MWArray<FourccMap, 16>  m_arSupportFourcc;
    int                     m_nSupportFourccCount;

    pthread_mutex_t         m_mutex;
};

#endif // VIDEORENDERXCB_H
