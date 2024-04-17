////////////////////////////////////////////////////////////////////////////////
// CONFIDENTIAL and PROPRIETARY software of Magewell Electronics Co., Ltd.
// Copyright (c) 2011-2014 Magewell Electronics Co., Ltd. (Nanjing)
// All rights reserved.
// This copyright notice MUST be reproduced on all authorized copies.
////////////////////////////////////////////////////////////////////////////////
#include "videorenderxcb.h"

#include <stdlib.h>
#include <string.h>
#include <errno.h>

VideoRenderXCB::VideoRenderXCB() :
    m_pConnection(NULL),
    m_xvPort(0xFFFFFFFF),
    m_xvid(0),
    m_lastImageBuf(NULL),
    m_lastBuffer(NULL),
    m_lastBufSize(0),
    m_nSupportFourccCount(0)
{
    pthread_mutex_init(&m_mutex, NULL);
}

VideoRenderXCB::~VideoRenderXCB()
{
    pthread_mutex_lock(&m_mutex);
    m_lastImageBuf = NULL;
    m_lastBuffer = NULL;
    m_lastBufSize = 0;
    pthread_mutex_unlock(&m_mutex);

    if (m_pConnection != NULL) {
        xcb_unmap_window(m_pConnection, m_xWindow);
        xcb_disconnect(m_pConnection);
        m_pConnection = NULL;
    }

    pthread_mutex_destroy(&m_mutex);
}

bool VideoRenderXCB::Init(int nWindowWidth, int nWindowHeight, xcb_drawable_t xWindowParent)
{
    xcb_drawable_t       win;
    //xcb_gcontext_t       foreground;
    uint32_t             mask = 0;
    uint32_t             values[2];

    m_pConnection = xcb_connect(NULL, NULL);
    if (m_pConnection == NULL) {
        //mw_debug(MW_DEBUG_ERR, "Connect to X server failed!\n");
        return false;
    }

    /* Get the first screen */
    m_pScreen = xcb_setup_roots_iterator(xcb_get_setup(m_pConnection)).data;

    if (!CheckXVideo(m_pConnection))
        return false;

    /* Create black (foreground) graphic context */
    win = m_pScreen->root;

    //foreground = xcb_generate_id(m_pConnection);
    mask = XCB_GC_FOREGROUND | XCB_GC_GRAPHICS_EXPOSURES;
    values[0] = m_pScreen->black_pixel;
    values[1] = 0;
    //xcb_create_gc(m_pConnection, foreground, win, mask, values);

    /* Ask for our window's Id */
    win = xcb_generate_id(m_pConnection);

    /* Create the window */
    mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
    values[0] = m_pScreen->white_pixel;
    values[1] = XCB_EVENT_MASK_EXPOSURE;
    xcb_create_window(m_pConnection,                 /* Connection          */
                      XCB_COPY_FROM_PARENT,          /* depth               */
                      win,                           /* window Id           */
                      m_pScreen->root,               /* parent window       */
                      0, 0,                          /* x, y                */
                      nWindowWidth, nWindowHeight,   /* width, height       */
                      10,                            /* border_width        */
                      XCB_WINDOW_CLASS_INPUT_OUTPUT, /* class               */
                      m_pScreen->root_visual,        /* visual              */
                      mask, values);                 /* masks */

    /* Set the title of the window */
    xcb_change_property (m_pConnection,
                         XCB_PROP_MODE_REPLACE,
                         win,
                         XCB_ATOM_WM_NAME,
                         XCB_ATOM_STRING,
                         8,
                         strlen("mwcap-v4l2"),
                         "mwcap-v4l2");

    m_xWindow = win;

    /* Map the window on the screen */
    xcb_map_window(m_pConnection, win);

    m_xGC = xcb_generate_id(m_pConnection);
    xcb_create_gc (m_pConnection, m_xGC, m_xWindow, 0, NULL);

    AutoDetectAdaptor();
    if (m_xvPort == 0xFFFFFFFF)
        return false;

    return true;
}

uint32_t *VideoRenderXCB::GetSupportedFourcc(int &nCount)
{
    uint32_t *newData = NULL;
    nCount = 0;

    if (m_arSupportFourcc.size() <= 0)
        return NULL;

    newData = new uint32_t[m_arSupportFourcc.size()];
    if (newData == NULL)
        return NULL;

    nCount = m_arSupportFourcc.size();
    for (int i = 0; i < nCount; i++) {
        newData[i] = m_arSupportFourcc[i].mwFourcc;
    }
    return newData;
}

bool VideoRenderXCB::SetFrameFormat(int nWidth, int nHeight, uint32_t dwColorSpace, bool bTry)
{
    xcb_xv_query_image_attributes_cookie_t query_attributes_cookie;
    xcb_xv_query_image_attributes_reply_t *query_attributes_reply;
    uint32_t xvid;

    xvid = FindXVFormatIDFromMWFourcc(dwColorSpace);
    if (xvid == 0)
        return false;

    query_attributes_cookie = xcb_xv_query_image_attributes(
                m_pConnection, m_xvPort, xvid,
                nWidth, nHeight);
    query_attributes_reply = xcb_xv_query_image_attributes_reply(
                m_pConnection, query_attributes_cookie, NULL);
    if (query_attributes_reply == NULL)
        return false;

    if (query_attributes_reply->width != nWidth
            || query_attributes_reply->height != nHeight) {
        free(query_attributes_reply);
        return false;
    }

    if (!bTry) {
        m_nWidth = nWidth;
        m_nHeight = nHeight;
        m_dwColorSpace = dwColorSpace;
        m_BufferSize = FOURCC_CalcImageSize(
                    dwColorSpace, nWidth, nHeight,
                    FOURCC_CalcMinStride(dwColorSpace, nWidth, 4));
        m_xvid = xvid;
    }

    return true;
}

VideoRenderImageBuf *VideoRenderXCB::AllocImageBuffer(VideoRenderImageBufType type)
{
    if (type != VIDEO_RENDER_IMAGEBUF_VIRTUAL)
        return NULL;

    if (m_BufferSize == 0) {
        //mw_debug(MW_DEBUG_ERR, "Set frame format first!\n");
        return NULL;
    }

    VideoRenderImageBufXCB *pImageBufXCB = new VideoRenderImageBufXCB;
    if (pImageBufXCB == NULL)
        return NULL;

    if (!CreateXImage(pImageBufXCB)) {
        delete pImageBufXCB;
        return NULL;
    }

    return pImageBufXCB;
}


void VideoRenderXCB::FreeImageBuffer(VideoRenderImageBuf *pBaseImage)
{
    VideoRenderImageBufXCB *pImageBuf;
    pImageBuf = reinterpret_cast<VideoRenderImageBufXCB *>(pBaseImage);

    DestroyXImage(pImageBuf);
    delete pImageBuf;
}

void VideoRenderXCB::DisplayFrame(VideoRenderImageBuf *pBaseImage)
{
    VideoRenderImageBufXCB *pImageBuf;

    if (pBaseImage == NULL)
        return;

    pthread_mutex_lock(&m_mutex);
    m_lastImageBuf = pBaseImage;
    m_lastBuffer = NULL;
    m_lastBufSize = 0;
    do {
        xcb_void_cookie_t ck;
        if (m_pConnection == NULL)
            break;

        xcb_get_geometry_cookie_t  geomCookie = xcb_get_geometry(m_pConnection, m_xWindow);  // window is a xcb_drawable_t
        xcb_get_geometry_reply_t  *geom       = xcb_get_geometry_reply(m_pConnection, geomCookie, NULL);

        pImageBuf = reinterpret_cast<VideoRenderImageBufXCB *>(pBaseImage);

        if (pImageBuf->xSegment) {
            ck = xcb_xv_shm_put_image(m_pConnection, m_xvPort, m_xWindow, m_xGC,
                                 pImageBuf->xSegment, m_xvid, 0,
                                 0, 0, m_nWidth, m_nHeight,
                                 0, 0, geom->width, geom->height,
                                 m_nWidth, m_nHeight,
                                 0);
        } else {
            ck = xcb_xv_put_image(m_pConnection, m_xvPort, m_xWindow, m_xGC,
                             m_xvid,
                             0, 0, m_nWidth, m_nHeight,
                             0, 0, geom->width, geom->height,
                             m_nWidth, m_nHeight,
                             pImageBuf->size, (const uint8_t *)pImageBuf->buf
                             );
        }
        xcb_generic_error_t *e = xcb_request_check(m_pConnection, ck);
        if (e != NULL) {
            //mw_debug(MW_DEBUG_ERR, "%s: X11 error %d", "cannot put image", e->error_code);
            free (e);
        }

        free (geom);

        xcb_flush(m_pConnection);
    } while (0);
    pthread_mutex_unlock(&m_mutex);
}

int VideoRenderXCB::DisplayFrame(void *buffer, size_t size)
{
    int ret = -1;
    if (buffer == NULL)
        return -1;

    pthread_mutex_lock(&m_mutex);
    m_lastImageBuf = NULL;
    m_lastBuffer = buffer;
    m_lastBufSize = size;
    do {
        if (m_pConnection == NULL){
            break;
        }
        xcb_get_geometry_cookie_t  geomCookie = xcb_get_geometry(m_pConnection, m_xWindow);  // window is a xcb_drawable_t
        xcb_get_geometry_reply_t  *geom       = xcb_get_geometry_reply(m_pConnection, geomCookie, NULL);
        if(NULL == geom){
            break;
        }
        xcb_xv_put_image(m_pConnection, m_xvPort, m_xWindow, m_xGC,
                         m_xvid,
                         0, 0, m_nWidth, m_nHeight,
                         0, 0, geom->width, geom->height,
                         m_nWidth, m_nHeight,
                         size, (const uint8_t *)buffer
                         );

        free (geom);
        ret = 1;
        xcb_flush(m_pConnection);
    } while (0);
    pthread_mutex_unlock(&m_mutex);
    return ret;
}

void VideoRenderXCB::DisplayLastFrame()
{
    VideoRenderImageBufXCB *pImageBuf;

    pthread_mutex_lock(&m_mutex);
    do {
        if (m_lastImageBuf == NULL
                && (m_lastBuffer == NULL || m_lastBufSize == 0)) {
            break;
        }

        xcb_void_cookie_t ck;
        if (m_pConnection == NULL)
            break;

        xcb_get_geometry_cookie_t  geomCookie = xcb_get_geometry(m_pConnection, m_xWindow);  // window is a xcb_drawable_t
        xcb_get_geometry_reply_t  *geom       = xcb_get_geometry_reply(m_pConnection, geomCookie, NULL);

        if (m_lastImageBuf != NULL) {
            pImageBuf = reinterpret_cast<VideoRenderImageBufXCB *>(m_lastImageBuf);

            if (pImageBuf->xSegment) {
                ck = xcb_xv_shm_put_image(m_pConnection, m_xvPort, m_xWindow, m_xGC,
                                          pImageBuf->xSegment, m_xvid, 0,
                                          0, 0, m_nWidth, m_nHeight,
                                          0, 0, geom->width, geom->height,
                                          m_nWidth, m_nHeight,
                                          0);
            } else {
                ck = xcb_xv_put_image(m_pConnection, m_xvPort, m_xWindow, m_xGC,
                                      m_xvid,
                                      0, 0, m_nWidth, m_nHeight,
                                      0, 0, geom->width, geom->height,
                                      m_nWidth, m_nHeight,
                                      pImageBuf->size, (const uint8_t *)pImageBuf->buf
                                      );
            }
            xcb_generic_error_t *e = xcb_request_check(m_pConnection, ck);
            if (e != NULL) {
                //mw_debug(MW_DEBUG_ERR, "%s: X11 error %d", "cannot put image", e->error_code);
                free (e);
            }
        } else {
            xcb_xv_put_image(m_pConnection, m_xvPort, m_xWindow, m_xGC,
                             m_xvid,
                             0, 0, m_nWidth, m_nHeight,
                             0, 0, geom->width, geom->height,
                             m_nWidth, m_nHeight,
                             m_lastBufSize, (const uint8_t *)m_lastBuffer
                             );
        }

        free (geom);

        xcb_flush(m_pConnection);
    } while (0);
    pthread_mutex_unlock(&m_mutex);
}

void VideoRenderXCB::RunLoop(ExitCleanCallback exit_call, void *data)
{
    xcb_generic_event_t *e;

    xcb_intern_atom_cookie_t cookie = xcb_intern_atom(
                m_pConnection, 1, sizeof("WM_PROTOCOLS")-1, "WM_PROTOCOLS");
    xcb_intern_atom_reply_t* reply = xcb_intern_atom_reply(m_pConnection, cookie, 0);

    xcb_intern_atom_cookie_t cookie2 = xcb_intern_atom(
                m_pConnection, 0, sizeof("WM_DELETE_WINDOW")-1, "WM_DELETE_WINDOW");
    xcb_intern_atom_reply_t* reply2 = xcb_intern_atom_reply(m_pConnection, cookie2, 0);

    xcb_change_property(m_pConnection, XCB_PROP_MODE_REPLACE,
                        m_xWindow, reply->atom, 4, 32, 1, &reply2->atom);

    /* We flush the request */
    xcb_flush(m_pConnection);

    while ((e = xcb_wait_for_event(m_pConnection))) {
        switch (e->response_type & ~0x80) {
        case XCB_EXPOSE: {
            DisplayLastFrame();
            break;
        }
        case XCB_CLIENT_MESSAGE: {
            //mw_debug(0, "\n");
            if((*(xcb_client_message_event_t*)e).data.data32[0] == reply2->atom) {
                //mw_debug(0, "\n");
                if (exit_call != NULL)
                    exit_call(data);

                pthread_mutex_lock(&m_mutex);
                m_lastImageBuf = NULL;
                m_lastBuffer = NULL;
                m_lastBufSize = 0;
                pthread_mutex_unlock(&m_mutex);

                xcb_unmap_window(m_pConnection, m_xWindow);
                xcb_destroy_window(m_pConnection, m_xWindow);
                xcb_disconnect(m_pConnection);
                goto out;
            }
            break;
        }
        default: {
            /* Unknown event type, ignore it */
            break;
        }
        }
        /* Free the Generic Event */
        free(e);
    }

out:
    free(reply);
    free(reply2);

    pthread_mutex_lock(&m_mutex);
    m_pConnection = NULL;
    pthread_mutex_unlock(&m_mutex);
}

void VideoRenderXCB::DisplayFps(unsigned long long frameCount, double fps)
{
    char titleStr[64];

    pthread_mutex_lock(&m_mutex);
    do {
        if (m_pConnection == NULL)
            break;

        sprintf(titleStr, "mwcap-v4l2  [total frame]: %lld [average fps]: %.2f", frameCount, fps);
        xcb_change_property (m_pConnection,
                             XCB_PROP_MODE_REPLACE,
                             m_xWindow,
                             XCB_ATOM_WM_NAME,
                             XCB_ATOM_STRING,
                             8,
                             strlen(titleStr),
                             titleStr);
        xcb_flush (m_pConnection);
    } while (0);
    pthread_mutex_unlock(&m_mutex);
}


/* private */
void VideoRenderXCB::AutoDetectAdaptor()
{
    xcb_xv_query_adaptors_cookie_t ck;
    xcb_xv_query_adaptors_reply_t *adaptors;
    xcb_xv_adaptor_info_iterator_t it;

    ck = xcb_xv_query_adaptors(m_pConnection, m_xWindow);
    adaptors = xcb_xv_query_adaptors_reply(m_pConnection, ck, NULL);

    if (adaptors == NULL)
        return;

    for (it = xcb_xv_query_adaptors_info_iterator(adaptors);
         it.rem > 0;
         xcb_xv_adaptor_info_next(&it)) {
        const xcb_xv_adaptor_info_t *a = it.data;

        printf("xcb_xv_adaptor_info_name=%s\n", xcb_xv_adaptor_info_name(a));
        if (!(a->type & XCB_XV_TYPE_IMAGE_MASK))
            continue;

        xcb_xv_list_image_formats_reply_t *r;
        r = xcb_xv_list_image_formats_reply(
                    m_pConnection,
                    xcb_xv_list_image_formats(m_pConnection, a->base_id),
                    NULL
                    );
        if (r == NULL)
            continue;

        const xcb_xv_image_format_info_t *f, *end;

        f = xcb_xv_list_image_formats_format(r);
        end = f + xcb_xv_list_image_formats_format_length(r);

        for (; f < end; f++) {
#if 0
            printf("id=0x%x type=%d format=%d depth=%d bpp=%d\n",
                   f->id, f->type, f->format, f->depth, f->bpp);
            printf("fourcc: %c%c%c%c\n",
                   ((f->id) >> 24) & 0xFF, ((f->id) >> 16) & 0xFF,
                   ((f->id) >> 8) & 0xFF, (f->id & 0xFF));
            printf("y_sample_bits=%d u_sample_bits=%d v_sample_bits=%d\n"
                   "vhorz_y_period=%d vhorz_u_period=%d vhorz_v_period=%d\n"
                   "vvert_y_period=%d vvert_u_period=%d vvert_v_period=%d\n"
                   "",
                   f->y_sample_bits, f->u_sample_bits, f->v_sample_bits,
                   f->vhorz_y_period, f->vhorz_u_period, f->vhorz_v_period,
                   f->vvert_y_period, f->vvert_u_period, f->vvert_v_period);
            /*
            for (size_t k=0; k < sizeof(f->vcomp_order); k++) {
                printf("%02x ", f->vcomp_order[k]);
            }
            */
            printf("%s\n", f->vcomp_order);
#endif
            /*
            uint32_t mw_fourcc = XCBGetFourcc(f);

            printf("fourcc: %c%c%c%c\n",
                   mw_fourcc & 0xFF, (mw_fourcc >> 8) & 0xFF,
                   (mw_fourcc >> 16) & 0xFF, (mw_fourcc >> 24) & 0xFF);
                   */
            uint32_t mw_fourcc = XCBGetFourcc(f);
            if (mw_fourcc != 0) {
                FourccMap fourccMap = {
                    mw_fourcc, f->id
                };

                if (!m_arSupportFourcc.has(fourccMap, fourcc_is_equal)) {
                    m_arSupportFourcc.append(fourccMap);
                }
            }
        }

        free (r);

        if (m_arSupportFourcc.size() > 0) {

            /* Grab a port */
            for (unsigned int i = 0; i < a->num_ports; i++) {
                xcb_xv_port_t port = a->base_id + i;
                xcb_xv_grab_port_reply_t *gr =
                        xcb_xv_grab_port_reply(m_pConnection,
                                               xcb_xv_grab_port(m_pConnection, port, XCB_CURRENT_TIME), NULL);
                uint8_t result = gr ? gr->result : 0xff;

                free (gr);
                if (result == 0) {
                    m_xvPort = port;
                    goto grabbed_port;
                }
                //mw_debug(MW_DEBUG_INFO, "cannot grab port %d: Xv error %d\n", port, result);
            }
            continue;
        }

        continue;
    }

grabbed_port:
    free(adaptors);
}

/**
 * Check that the X server supports the XVideo extension.
 */
bool VideoRenderXCB::CheckXVideo(xcb_connection_t *conn)
{
    xcb_xv_query_extension_reply_t *r;
    xcb_xv_query_extension_cookie_t ck = xcb_xv_query_extension (conn);
    bool ok = false;

    r = xcb_xv_query_extension_reply (conn, ck, NULL);
    if (r != NULL)
    {   /* We need XVideo 2.2 for PutImage */
        if ((r->major > 2) || (r->major == 2 && r->minor >= 2))
        {
            //mw_debug(MW_DEBUG_INFO, "using XVideo extension v%d.%d\n",
             //        r->major, r->minor);
            ok = true;
        }
        //else
         //   mw_debug(MW_DEBUG_ERR, "XVideo extension too old (v%d.%d)\n",
         //            r->major, r->minor);
        free (r);
    }
    //else
    //    mw_debug(MW_DEBUG_ERR, "XVideo extension not available\n");

    return ok;
}

uint32_t VideoRenderXCB::XCBGetFourcc(const xcb_xv_image_format_info_t *f)
{
    if (f->byte_order != XCB_IMAGE_ORDER_LSB_FIRST && f->bpp != 8)
        return 0;

    switch (f->type) {
    case XCB_XV_IMAGE_FORMAT_INFO_TYPE_RGB:
#if 0
        if (f->num_planes == 1) {
            switch (f->bpp) {
            case 32:
                if (f->depth == 24)
                    return MWFOURCC_ARGB;
                break;
            case 24:
                if (f->depth == 24)
                    return MWFOURCC_RGB24;
                break;
            default:
                break;
            }
        }
#endif
        break;

    case XCB_XV_IMAGE_FORMAT_INFO_TYPE_YUV:
        switch (f->num_planes) {
        case 1:
            switch (f->bpp)
            {
            case 16:
                if (f->vhorz_u_period == 2 && f->vvert_u_period == 1) {
                    if (!strcmp ((const char *)f->vcomp_order, "YUYV"))
                        return MWFOURCC_YUY2;
                    if (!strcmp ((const char *)f->vcomp_order, "UYVY"))
                        return MWFOURCC_UYVY;
                }
                break;
            }
            break;
        case 3:
            switch (f->bpp)
            {
            case 12:
                if (f->vhorz_u_period == 2 && f->vvert_u_period == 2)
                {
                    if (!strcmp ((const char *)f->vcomp_order, "YVU"))
                        return MWFOURCC_YV12;
                    if (!strcmp ((const char *)f->vcomp_order, "YUV"))
                        return MWFOURCC_I420;
                }
            }
            break;
        }
    default:
        break;
    }

    return 0;
}

uint32_t VideoRenderXCB::FindXVFormatIDFromMWFourcc(uint32_t fourcc)
{
    for (size_t i = 0; i < m_arSupportFourcc.size(); i++) {
        if (fourcc == m_arSupportFourcc[i].mwFourcc)
            return m_arSupportFourcc[i].xvId;
    }

    return 0;
}

bool VideoRenderXCB::IsSupportShm()
{
    xcb_shm_query_version_cookie_t ck;
    xcb_shm_query_version_reply_t *r;
    bool shm = true;

    ck = xcb_shm_query_version(m_pConnection);
    r = xcb_shm_query_version_reply(m_pConnection, ck, NULL);
    if (!r) {
        //mw_debug(MW_DEBUG_INFO, "shared memory (MIT-SHM) not available\n");
        //mw_debug(MW_DEBUG_WARNING, "display will be slow\n");
        shm = false;
    }
    free (r);

    return shm;
}

bool VideoRenderXCB::CreateXImage(VideoRenderImageBufXCB *pImageBuf)
{
    pImageBuf->type = VIDEO_RENDER_IMAGEBUF_VIRTUAL;
    pImageBuf->buf = NULL;
    pImageBuf->bIsShmMem = false;
    do {
        if (IsSupportShm()) {
            xcb_shm_seg_t segment;
            void *ximage;
            int shmid;
            xcb_void_cookie_t shm_attach_cookie;
            xcb_generic_error_t *generic_error;

            shmid = shmget(IPC_PRIVATE, m_BufferSize, IPC_CREAT | 0777);
            if (shmid < 0 ) {
                //mw_debug(MW_DEBUG_ERR,
                //         "shared memory error in shmget: %s\n", strerror(errno));
                break;
            }

            ximage = shmat(shmid, NULL, 0/* read/write */);
            if (ximage == ((void *) -1)) {
                //mw_debug(MW_DEBUG_ERR,
                //         "shared memory error (address error)\n");
                shmctl(shmid, IPC_RMID, 0);
                break;
            }

            /* Attach the segment to X */
            segment = xcb_generate_id(m_pConnection);
            shm_attach_cookie = xcb_shm_attach_checked(m_pConnection, segment, shmid, 0);
            generic_error = xcb_request_check(m_pConnection, shm_attach_cookie);
            if (generic_error != NULL) {
                //mw_debug(MW_DEBUG_ERR,
                //         "x11 error during shared memory XImage creation\n");
                free(generic_error);
                shmdt(ximage);
                shmctl(shmid, IPC_RMID, 0);
                break;
            }

            pImageBuf->bIsShmMem = true;
            pImageBuf->buf = ximage;
            pImageBuf->xSegment = segment;
            pImageBuf->size = m_BufferSize;
            /*
             * Now that the Xserver has learned about and attached to the
             * shared memory segment,  delete it.  It's actually deleted by
             * the kernel when all users of that segment have detached from
             * it.  Gives an automatic shared memory cleanup in case we crash.
             */
            shmctl(shmid, IPC_RMID, 0);
        }
    } while (0);

    if (pImageBuf->buf == NULL) {
        pImageBuf->buf = malloc(m_BufferSize);
        pImageBuf->bIsShmMem = false;
        pImageBuf->xSegment = 0;
        pImageBuf->size = m_BufferSize;
    }

    return (pImageBuf->buf != NULL);
}

void VideoRenderXCB::DestroyXImage(VideoRenderImageBufXCB *pImageBuf)
{
    if (pImageBuf->buf != NULL) {
        if (pImageBuf->bIsShmMem) {
            xcb_shm_detach(m_pConnection, pImageBuf->xSegment);
            pImageBuf->xSegment = 0;
            shmdt(pImageBuf->buf);
        } else {
            free(pImageBuf->buf);
        }
        pImageBuf->buf = NULL;
    }
}
