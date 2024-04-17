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

#ifndef CCAPTURETHREAD_H
#define CCAPTURETHREAD_H

#include <QObject>
#include <QVector>
#include "LibMWCapture/MWCapture.h"
#include "MWFOURCC.h"
#include "common.h"

typedef struct _channel_item{
    int m_n_index;
    int m_n_board_index;
    int m_n_channel_index;

    HCHANNEL m_h_channel;
    MWCAP_PTR m_h_capture_event;
    MWCAP_PTR m_h_notify_event;
    HNOTIFY m_h_notify;
    RECT m_rec_pos;
}channel_item,*p_channel_item;

class ICaptureCallBack
{
public:
    virtual void on_video_signal_changed(int cx,int cy,void *p_param)=0;
    virtual void on_capture_callback(const unsigned char* pby_data,int cb_size,void *p_param)=0;
};

class CCaptureThread : public QObject
{
    Q_OBJECT
public:
    explicit CCaptureThread(QObject *parent = 0);
    ~CCaptureThread();

public:
    bool create(QVector<QVector<channel_item> >*p_vec_items,int n_cx,int n_cy,unsigned int ui_fourcc,unsigned int ui_frameduration,ICaptureCallBack *p_callback,void* param);

    void destory_layer();

    static void *video_layer_thread_proc(void *p_param){
        CCaptureThread *p_this=(CCaptureThread*)p_param;
        return p_this->video_layer_thread_proc();
    }
    void *video_layer_thread_proc();

    double get_fps();

protected:
    double m_d_fps;

    channel_item m_arr_channel[4];
    QVector<QVector<channel_item> > m_vec_items;

    pthread_t m_pt_video_thread;

    bool m_b_running;
    MWCAP_PTR m_p_exit_video_thread;

    unsigned char* m_p_buffer;
    unsigned char* m_p_buffer_1;
    int m_cb_buffer;

    //callback and caller
    void* m_p_param;
    ICaptureCallBack *m_p_callback;

    //cx,cy,format,...
    int m_n_cx;
    int m_n_cy;
    int m_n_num_channel;
    int m_dw_fourcc;
    int m_dw_frameduration;

signals:

public slots:
};

#endif // CCAPTURETHREAD_H
