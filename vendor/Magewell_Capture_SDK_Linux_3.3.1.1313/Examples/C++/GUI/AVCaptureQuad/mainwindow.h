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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVector>
#include <QTimer>
#include "ccapturethread.h"
#include "cconfigfile.h"

namespace Ui {
class MainWindow;
}

#define CAP_CX      1920
#define CAP_CY      1080
#define CAP_DURA    166667//1000000//400000

typedef struct _set_val{
    int m_n_layer;
    int m_n_posx;
    int m_n_posy;
    int m_n_poswid;
    int m_n_poshei;
}set_val;

typedef struct _line_point{
    int m_n_p;
    int m_n_index;
}line_point;

class MainWindow : public QMainWindow,public ICaptureCallBack
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public:
    void on_video_signal_changed(int cx,int cy,void *p_param);
    void on_capture_callback(const unsigned char* pby_data,int cb_size,void *p_param);

protected:
    int m_n_valid_channel[36];
    int m_n_num_valid_channel;

    QVector<valid_channel> q_vec_channel;

    int m_arr_layer[4];
    int m_arr_sel[4];
    int m_n_sel_num;

    CCaptureThread m_thread;
    channel_item m_arr_items[4];
    int m_n_item_num;

    channel_set m_channel_set[4][4];

    int m_n_cx;
    int m_n_cy;
    unsigned int m_n_frameduration;
    unsigned int m_n_fourcc;

    int m_csp;
    int m_level;

    CConfigFile m_cfg;

    QVector<QVector<channel_item> > m_real_layer;

    QTimer *m_p_timer;

protected:
    bool load_setting_file(int index);
    bool parse_setting_file(int index);
    bool set_capture(int index);

    bool open_preview();
    void close_preview();

    void init_channel_set();

    void re_layer();
    void check_item();
    bool fine_index(int n_index);
    void re_fix_pos(channel_item *pitem);
    void re_vec_layer();
    bool check_registration(channel_item *p_item1,channel_item *p_item2);
    bool check_line_registration(line_point *point);

protected:
    void closeEvent(QCloseEvent *event);

signals:
    void get_frame(unsigned char *pbuffer,int wid,int hei, int csp, int level);

public slots:
    void put_frame(unsigned char *pbuffer,int wid,int hei, int csp, int level);

    void fresh_fps();

    void on_opengl_version(bool t_b_opengl);

private slots:
    void on_load_3_clicked();

    void on_load_2_clicked();

    void on_load_4_clicked();

    void on_load_1_clicked();

    void on_pushButton_clicked();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
