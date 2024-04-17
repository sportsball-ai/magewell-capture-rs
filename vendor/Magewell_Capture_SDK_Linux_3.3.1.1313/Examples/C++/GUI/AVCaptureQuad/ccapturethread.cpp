////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2011-2018 Magewell Electronics Co., Ltd. (Nanjing)
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
////////////////////////////////////////////////////////////////////////////////

#include "ccapturethread.h"



CCaptureThread::CCaptureThread(QObject *parent) : QObject(parent)
{
    m_d_fps=0.0;
    m_b_running=false;

    memset(m_arr_channel,0,sizeof(channel_item)*4);

    m_pt_video_thread=0;
    m_p_exit_video_thread=0;

    m_p_buffer=NULL;
    m_p_buffer_1=NULL;

    m_p_param=NULL;
    m_p_callback=NULL;

    m_p_buffer_1=new unsigned char[1920*1080*2];
}

CCaptureThread::~CCaptureThread()
{
    if(m_p_buffer_1!=NULL)
        delete m_p_buffer_1;
}

bool CCaptureThread::create(QVector<QVector<channel_item> > *p_vec_items, int n_cx, int n_cy, unsigned int ui_fourcc, unsigned int ui_frameduration, ICaptureCallBack *p_callback, void *param)
{
    MW_RESULT mr=MW_FAILED;
    bool b_ret=false;

    for(int i=0;i<m_vec_items.size();i++){
        m_vec_items[i].clear();
    }
    m_vec_items.clear();

    for(int i=0;i<p_vec_items->size();i++){
        m_vec_items.push_back((*p_vec_items)[i]);
    }

    do{
        for(int i=0;i<m_vec_items.size();i++){
            for(int j=0;j<m_vec_items[i].size();j++){
                if(m_vec_items[i][j].m_n_index==-1){
                    m_vec_items[i][j].m_h_channel=NULL;
                    continue;
                }

                char sz_device_path[256];
                MWGetDevicePath(m_vec_items[i][j].m_n_index,sz_device_path);
                m_vec_items[i][j].m_h_channel=MWOpenChannelByPath(sz_device_path);
                if(m_vec_items[i][j].m_h_channel==NULL)
                    continue;
                b_ret=false;

                do{
                   m_vec_items[i][j].m_h_capture_event=MWCreateEvent();
                    if(m_vec_items[i][j].m_h_capture_event==0)
                        break;

                    m_vec_items[i][j].m_h_notify_event=MWCreateEvent();
                    if(m_vec_items[i][j].m_h_notify_event==0)
                        break;

                    mr=MWStartVideoCapture(m_vec_items[i][j].m_h_channel,m_vec_items[i][j].m_h_capture_event);
                    if(mr!=MW_SUCCEEDED)
                        break;

                    unsigned int ui_notify=MWCAP_NOTIFY_VIDEO_FRAME_BUFFERED|MWCAP_NOTIFY_VIDEO_SIGNAL_CHANGE;
                    m_vec_items[i][j].m_h_notify=MWRegisterNotify(m_vec_items[i][j].m_h_channel,m_vec_items[i][j].m_h_notify_event,ui_notify);
                    if(m_vec_items[i][j].m_h_notify==0)
                        break;

                    b_ret=true;
                }while(false);

                if(!b_ret){
                    MWStopVideoCapture(m_vec_items[i][j].m_h_channel);
                    MWCloseChannel(m_vec_items[i][j].m_h_channel);
                    m_vec_items[i][j].m_h_channel=NULL;

                    if(m_vec_items[i][j].m_h_capture_event!=0){
                        MWCloseEvent(m_vec_items[i][j].m_h_capture_event);
                        m_vec_items[i][j].m_h_capture_event=0;
                    }
                    if(m_vec_items[i][j].m_h_notify_event!=0){
                        MWCloseEvent(m_vec_items[i][j].m_h_notify_event);
                        m_vec_items[i][j].m_h_notify_event=0;
                    }
                }

                m_vec_items[i][j].m_rec_pos.left=n_cx*m_vec_items[i][j].m_rec_pos.left/100;
                m_vec_items[i][j].m_rec_pos.right=n_cx*m_vec_items[i][j].m_rec_pos.right/100;
                m_vec_items[i][j].m_rec_pos.top=n_cy*m_vec_items[i][j].m_rec_pos.top/100;
                m_vec_items[i][j].m_rec_pos.bottom=n_cy*m_vec_items[i][j].m_rec_pos.bottom/100;
            }
        }

        unsigned int ui_stride=FOURCC_CalcMinStride(ui_fourcc,n_cx,4);
        m_cb_buffer=FOURCC_CalcImageSize(ui_fourcc,n_cx,n_cy,ui_stride);
        m_p_buffer=new unsigned char[m_cb_buffer];

        memset(m_p_buffer,0,m_cb_buffer);

        m_p_param=param;
        m_p_callback=p_callback;

        m_n_cx=n_cx;
        m_n_cy=n_cy;
        m_dw_fourcc=ui_fourcc;
        m_dw_frameduration=ui_frameduration;

        m_p_exit_video_thread=MWCreateEvent();
        if(m_p_exit_video_thread==0)
            break;

        m_b_running=true;

        int err=pthread_create(&m_pt_video_thread,NULL,video_layer_thread_proc,this);
        if(err!=0){
            m_b_running=false;
            break;
        }
        b_ret=true;

    }while(false);

    if(!b_ret)
        destory_layer();
}

void CCaptureThread::destory_layer()
{
    if(m_pt_video_thread!=0){
        if(m_p_exit_video_thread!=0){
            MWSetEvent(m_p_exit_video_thread);
            m_b_running=false;

            void *p_thread_return;
            pthread_join(m_pt_video_thread,&p_thread_return);
        }
        m_pt_video_thread=0;
    }

    for(int i=0;i<m_vec_items.size();i++){
        for(int j=0;j<m_vec_items[i].size();j++){
            if(m_vec_items[i][j].m_h_notify!=0){
                MWUnregisterNotify(m_vec_items[i][j].m_h_channel,m_vec_items[i][j].m_h_notify);
                m_vec_items[i][j].m_h_notify=0;
            }
            if(m_vec_items[i][j].m_h_channel!=NULL){
                MWStopVideoCapture(m_vec_items[i][j].m_h_channel);
                MWCloseChannel(m_vec_items[i][j].m_h_channel);
                m_vec_items[i][j].m_h_channel=NULL;
            }
            if(m_vec_items[i][j].m_h_capture_event!=0){
                MWCloseEvent(m_vec_items[i][j].m_h_capture_event);
                m_vec_items[i][j].m_h_capture_event=0;
            }
            if(m_vec_items[i][j].m_h_notify_event!=0){
                MWCloseEvent(m_vec_items[i][j].m_h_notify_event);
                m_vec_items[i][j].m_h_notify_event=0;
            }
        }

        if(m_p_buffer!=NULL){
            delete m_p_buffer;
            m_p_buffer=NULL;
        }
    }
}

void *CCaptureThread::video_layer_thread_proc()
{
    int t_n_width=m_n_cx;
    int t_n_height=m_n_cy;

    int t_n_stride=FOURCC_CalcMinStride(m_dw_fourcc,t_n_width,4);

    int t_n_framecount=0;
    int t_n_fpscount=0;

    HCHANNEL t_h_channel=NULL;
    //get the first valid channel and
    //capture in time mode in order to
    //splicing
    for(int i=0;i<m_vec_items.size();i++){
        for(int j=0;j<m_vec_items[i].size();j++){
            if(m_vec_items[i][j].m_h_channel!=NULL){
                t_h_channel=m_vec_items[i][j].m_h_channel;
                break;
            }
        }
        if(t_h_channel!=NULL)
            break;
    }

    if(t_h_channel==NULL)
        return NULL;

    MWCAP_PTR t_h_timer_event=MWCreateEvent();
    if(t_h_timer_event==0)
        return NULL;

    HTIMER t_h_timer=MWRegisterTimer(t_h_channel,t_h_timer_event);

    MW_RESULT t_mr=MW_FAILED;
    long long t_ll_begin=0;
    t_mr=MWGetDeviceTime(t_h_channel,&t_ll_begin);

    long long t_ll_expire_time=t_ll_begin;
    long long t_ll_last=t_ll_begin;

    while (m_b_running) {
        t_ll_expire_time+=m_dw_frameduration;

        t_mr=MWScheduleTimer(t_h_channel,t_h_timer,t_ll_expire_time);
        if(t_mr!=MW_SUCCEEDED)
            continue;

        MWCAP_PTR t_arr_event_notify[2]={m_p_exit_video_thread,t_h_timer_event};
        unsigned int t_ui_ret=MWMultiWaitEvent(t_arr_event_notify,2,-1);
        //0 means wait_failed
        //1 means m_p_exit_video_thread
        if(t_ui_ret==0||t_ui_ret==1){
            int k=0;
            k++;
            break;
        }
        if(false){

        }
        else{

            memset(m_p_buffer,0,m_cb_buffer);

            for(int i=0;i<m_vec_items.size();i++){
                int t_n_num_event=0;
                MWCAP_PTR t_arr_event[4];
                for(int j=0;j<m_vec_items[i].size();j++){
                    if(m_vec_items[i][j].m_h_channel==NULL)
                        continue;

                    t_mr=MWCaptureVideoFrameToVirtualAddressEx(
                                m_vec_items[i][j].m_h_channel,
                                MWCAP_VIDEO_FRAME_ID_NEWEST_BUFFERED,
                                m_p_buffer,
                                m_cb_buffer,
                                t_n_stride,
                                FALSE,
                                NULL,
                                m_dw_fourcc,
                                t_n_width,
                                t_n_height,
                                0,
                                0,
                                NULL,
                                NULL,
                                0,
                                100,
                                0,
                                100,
                                0,
                                MWCAP_VIDEO_DEINTERLACE_BLEND,
                                MWCAP_VIDEO_ASPECT_RATIO_IGNORE,
                                NULL,
                                &(m_vec_items[i][j].m_rec_pos),
                                0,
                                0,
                                MWCAP_VIDEO_COLOR_FORMAT_UNKNOWN,
                                MWCAP_VIDEO_QUANTIZATION_UNKNOWN,
                                MWCAP_VIDEO_SATURATION_UNKNOWN
                                );

                    if(t_mr!=MW_SUCCEEDED)
                        continue;

                    t_arr_event[t_n_num_event++]=m_vec_items[i][j].m_h_capture_event;

                }

                for(int i=0;i<t_n_num_event;i++){
                    MWWaitEvent(t_arr_event[i],-1);
                }

                for(int i=0;i<m_vec_items.size();i++){
                    for(int j=0;j<m_vec_items[i].size();j++){
                        MWCAP_VIDEO_CAPTURE_STATUS captureStatus;
                        t_mr =MWGetVideoCaptureStatus(m_vec_items[i][j].m_h_channel, &captureStatus);//must call this function to clean mem
                    }
                }

            }

            pthread_mutex_lock(&g_p_mutex);
            memcpy(g_buffer,m_p_buffer,m_cb_buffer);
            pthread_mutex_unlock(&g_p_mutex);
            if(m_p_callback!=NULL)
                m_p_callback->on_capture_callback(g_buffer,m_cb_buffer,m_p_param);

            t_n_framecount++;

            long long t_ll_current=0;
            MWGetDeviceTime(t_h_channel,&t_ll_current);

            t_n_fpscount++;
            if(t_n_fpscount%10==0){
                m_d_fps=(double)t_n_fpscount*10000000LL/(t_ll_current-t_ll_last);
                if(t_ll_current-t_ll_last>30000000LL){
                    t_ll_last=t_ll_current;
                    t_n_fpscount=0;
                }
            }
        }
    }

    if(t_h_timer!=0){
        MWUnregisterTimer(t_h_channel,t_h_timer);
    }
    if(t_h_timer_event!=0)
        MWCloseEvent(t_h_timer_event);

    return NULL;
}

double CCaptureThread::get_fps()
{
    return m_d_fps;
}
