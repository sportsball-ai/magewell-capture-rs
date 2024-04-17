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

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QTimer>
#include <QImage>



const set_val g_set_val[][4]={
    {
        {1,0,0,50,50},{2,50,0,50,50},{3,0,50,50,50},{4,50,50,50,50}
    },
    {
        {1,0,0,100,100},{4,12,12,25,25},{2,62,12,25,25},{3,37,62,25,25}
    },
    {
        {4,12,12,25,25},{1,0,0,100,100},{2,62,12,25,25},{3,37,62,25,25}
    },
    {
        {4,12,12,25,25},{2,62,12,25,25},{1,0,0,100,100},{3,37,62,25,25}
    }
};

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    memset(m_n_valid_channel,-1,sizeof(int)*36);
    m_n_num_valid_channel=0;

    memset(m_arr_layer,0,sizeof(int)*4);
    memset(m_arr_sel,0,sizeof(int)*4);
    m_n_sel_num=0;

    memset(m_arr_items,0,sizeof(channel_item)*4);

    m_n_cx=CAP_CX;
    m_n_cy=CAP_CY;
    m_n_frameduration=CAP_DURA;
    m_n_fourcc=MWFOURCC_YUY2;

    m_csp = 1;
    m_level = 0;

    MWCaptureInitInstance();
    MWRefreshDevice();
    int t_n_channel_count=MWGetChannelCount();
    MW_RESULT t_mr=MW_FAILED;
    MWCAP_CHANNEL_INFO t_channel_info;
    for(int i=0;i<t_n_channel_count;i++){
        t_mr=MWGetChannelInfoByIndex(i,&t_channel_info);
        if(t_mr!=MW_SUCCEEDED)
            continue;
        QString t_qs_name;
        QString t_qs_valid_name;
        t_qs_name.append(t_channel_info.szFamilyName);
        if(t_qs_name.contains(QString("Pro Capture"))){
            m_n_valid_channel[m_n_num_valid_channel]=i;
            m_n_num_valid_channel++;
            t_qs_valid_name.sprintf("%s %02d-%d",t_channel_info.szProductName,t_channel_info.byBoardIndex,t_channel_info.byChannelIndex);
            valid_channel t_v_channel;
            t_v_channel.m_n_index=i;
            t_v_channel.m_qs_name=t_qs_valid_name;
            q_vec_channel.push_back(t_v_channel);
        }
        char sz_device_path[256];
        MWGetDevicePath(i,sz_device_path);
        HCHANNEL channel;
        channel=MWOpenChannelByPath(sz_device_path);
        if(NULL == channel)
            break;
        MWCAP_VIDEO_SIGNAL_STATUS status;
        if(MW_SUCCEEDED != MWGetVideoSignalStatus(channel,&status))
            break;
        if(MWCAP_VIDEO_SIGNAL_LOCKED == status.state)
        {
            if(MWCAP_VIDEO_COLOR_FORMAT_YUV601 == status.colorFormat)
            {
                m_csp = 0;
            }
            else if(MWCAP_VIDEO_COLOR_FORMAT_YUV709 == status.colorFormat)
            {
                m_csp = 1;
            }
            else if(MWCAP_VIDEO_COLOR_FORMAT_YUV2020 == status.colorFormat)
            {
                m_csp = 2;
            }

            if(MWCAP_VIDEO_QUANTIZATION_LIMITED == status.quantRange)
            {
                m_level = 0;
            }
            else if(MWCAP_VIDEO_QUANTIZATION_FULL == status.quantRange)
            {
                m_level = 1;
            }
        }
        MWCloseChannel(channel);

    }
    if(m_n_num_valid_channel==0){
        QMessageBox::information(NULL,"Warning","Can't find any pro capture cards.");
        QTimer::singleShot(0,qApp,SLOT(quit()));
    }

    connect(this,SIGNAL(get_frame(unsigned char*,int,int,int,int)),
            this,SLOT(put_frame(unsigned char*,int,int,int,int)));
    connect(ui->opengl_player,SIGNAL(opengl_state(bool)),this,SLOT(on_opengl_version(bool)));

    m_p_timer=new QTimer(this);
    connect(m_p_timer,SIGNAL(timeout()),this,SLOT(fresh_fps()));

    init_channel_set();
    for(int i=0;i<4;i++){
        m_cfg.generate_file(i,m_channel_set[i],q_vec_channel,q_vec_channel.size());
    }

    m_p_timer->start(1000);

}

MainWindow::~MainWindow()
{
    delete ui;
    MWCaptureExitInstance();
    m_p_timer->stop();
}

void MainWindow::on_video_signal_changed(int cx, int cy, void *p_param)
{

}

void MainWindow::on_capture_callback(const unsigned char *pby_data, int cb_size, void *p_param)
{
    emit get_frame((unsigned char *)pby_data,m_n_cx,m_n_cy,m_csp,m_level);
}

bool MainWindow::load_setting_file(int index)
{
    m_cfg.load_file(index);
    return true;
}

bool MainWindow::parse_setting_file(int index)
{
    return true;
}

bool MainWindow::set_capture(int index)
{
    cfg *p_cfg=m_cfg.get_channelset(index);
    m_n_sel_num=0;
    for(int i=0;i<4;i++){
        int t_n_layer=p_cfg->m_set[i].m_n_layer;
        t_n_layer--;
        m_arr_items[t_n_layer].m_n_index=p_cfg->m_set[i].m_n_index;
        m_arr_items[t_n_layer].m_rec_pos.left=p_cfg->m_set[i].m_n_posx;
        m_arr_items[t_n_layer].m_rec_pos.right=p_cfg->m_set[i].m_n_poswid;//temp wid
        m_arr_items[t_n_layer].m_rec_pos.top=p_cfg->m_set[i].m_n_posy;
        m_arr_items[t_n_layer].m_rec_pos.bottom=p_cfg->m_set[i].m_n_poshei;//temp hei
    }

    for(int i=0;i<4;i++){
        //check index valid
        bool b_find=false;
        for(int j=0;j<q_vec_channel.size();j++){
            if(m_arr_items[i].m_n_index==q_vec_channel[j].m_n_index){
                b_find=true;
                break;
            }
        }
        if(b_find){
            m_n_sel_num++;
        }
        else{
            m_arr_items[i].m_n_index=-1;
        }

    }

    //relayer recheck
    re_layer();

    return true;
}

bool MainWindow::open_preview()
{
    m_thread.create(&m_real_layer,m_n_cx,m_n_cy,m_n_fourcc,m_n_frameduration,this,this);
    return true;
}

void MainWindow::close_preview()
{
    m_thread.destory_layer();
}

void MainWindow::init_channel_set()
{
    int t_size=q_vec_channel.size();
    int t_num=0;
    if(t_size>=4)
        t_num=4;
    else
        t_num=t_size;

    for(int i=0;i<4;i++){
        for(int j=0;j<4;j++){
            m_channel_set[i][j].m_n_index=-1;
            m_channel_set[i][j].m_n_posx=g_set_val[i][j].m_n_posx;
            m_channel_set[i][j].m_n_posy=g_set_val[i][j].m_n_posy;
            m_channel_set[i][j].m_n_poswid=g_set_val[i][j].m_n_poswid;
            m_channel_set[i][j].m_n_poshei=g_set_val[i][j].m_n_poshei;
            m_channel_set[i][j].m_n_layer=g_set_val[i][j].m_n_layer;
        }
    }

    for(int i=0;i<4;i++){
        for(int j=0;j<t_num;j++){
            m_channel_set[i][j].m_n_index=q_vec_channel[j].m_n_index;
        }
    }
}

void MainWindow::re_layer()
{
    check_item();
    re_vec_layer();
}

void MainWindow::check_item()
{
    for(int i=0;i<4;i++){
        if(!fine_index(m_arr_items[i].m_n_index)){
            m_arr_items[i].m_n_index=-1;
        }
        re_fix_pos(&m_arr_items[i]);

    }
}

bool MainWindow::fine_index(int n_index)
{
    bool b_ret=false;
    for(int i=0;i<q_vec_channel.size();i++){
        if(n_index==q_vec_channel[i].m_n_index){
            b_ret=true;
            break;
        }
    }
    return b_ret;
}

void MainWindow::re_fix_pos(channel_item *pitem)
{
    //posx 0-99
    //posy 0-99
    //poswid 1- 100-posx
    //poshei 1- 100-posy

    if(!(pitem->m_rec_pos.left>=0&&pitem->m_rec_pos.left<=99)){
        pitem->m_rec_pos.left=0;
    }
    int t_n_right=pitem->m_rec_pos.left+pitem->m_rec_pos.right;
    if(!(t_n_right>=1&&t_n_right<=100)){
        pitem->m_rec_pos.right=100;
    }
    else{
        pitem->m_rec_pos.right=t_n_right;
    }
    if(!(pitem->m_rec_pos.top>=0&&pitem->m_rec_pos.top<=99)){
        pitem->m_rec_pos.top=0;
    }
    int t_n_bottom=pitem->m_rec_pos.top+pitem->m_rec_pos.bottom;
    if(!(t_n_bottom>=1&&t_n_bottom<=100)){
        pitem->m_rec_pos.bottom=100;
    }
    else{
        pitem->m_rec_pos.bottom=t_n_bottom;
    }
}

void MainWindow::re_vec_layer()
{
    m_real_layer.clear();
    QVector<channel_item> t_vec_1;
    QVector<channel_item> t_vec_2;
    for(int i=0;i<4;i++){
        if(m_arr_items[i].m_n_index==-1)
            t_vec_1.push_back(m_arr_items[i]);
        else
            t_vec_2.push_back(m_arr_items[i]);
    }
    if(t_vec_2.empty()){
        return;
    }

    QVector<channel_item> t_vec_3;
    t_vec_3.push_back(t_vec_2[0]);
    m_real_layer.push_back(t_vec_3);

    for(int i=1;i<t_vec_2.size();i++){
        channel_item t_it=t_vec_2[i];
        bool t_reg=false;
        for(int j=0;j<m_real_layer.size();j++){
            for(int k=0;k<m_real_layer[j].size();k++){
                channel_item t_it_1=m_real_layer[j][k];
                if(check_registration(&t_it,&t_it_1)){
                    t_reg=true;
                    break;
                }
            }

        }
        if(t_reg){
            QVector<channel_item> t_vec_4;
            t_vec_4.push_back(t_it);
            m_real_layer.push_back(t_vec_4);
        }else{
            m_real_layer.last().push_back(t_it);
        }
    }

}

bool MainWindow::check_registration(channel_item *p_item1, channel_item *p_item2)
{
    bool b_ret=true;
    //horizontal
    line_point point[4];
    point[0].m_n_p=p_item1->m_rec_pos.left;
    point[0].m_n_index=1;
    point[1].m_n_p=p_item1->m_rec_pos.right;
    point[1].m_n_index=1;
    point[2].m_n_p=p_item2->m_rec_pos.left;
    point[2].m_n_index=2;
    point[3].m_n_p=p_item2->m_rec_pos.right;
    point[3].m_n_index=2;

    bool b_ret_h=check_line_registration(point);

    //vertical
    line_point point_2[4];
    point_2[0].m_n_p=p_item1->m_rec_pos.top;
    point_2[0].m_n_index=1;
    point_2[1].m_n_p=p_item1->m_rec_pos.bottom;
    point_2[1].m_n_index=1;
    point_2[2].m_n_p=p_item2->m_rec_pos.top;
    point_2[2].m_n_index=2;
    point_2[3].m_n_p=p_item2->m_rec_pos.bottom;
    point_2[3].m_n_index=2;

    bool b_ret_v=check_line_registration(point_2);

    if(b_ret_h==false||b_ret_v==false)
        b_ret=false;
    else
        b_ret=true;

    return b_ret;
}

bool MainWindow::check_line_registration(line_point *point)
{
    bool b_ret=true;

    QVector<line_point> t_vec;
    for(int i=0;i<4;i++){
        int t_min=0;
        int index=-1;
        for(int k=0;k<4;k++){
            if(point[k].m_n_index==3)
                continue;
            else{
                t_min=point[k].m_n_p;
                index=k;
                break;
            }

        }
        for(int j=0;j<4;j++){
            if(i==j)
                continue;
            else if(point[j].m_n_index==3)
                continue;
            else{
                if(t_min>point[j].m_n_p){
                    t_min=point[j].m_n_p;
                    index=j;
                }
                else if(t_min==point[j].m_n_p){
                    if(t_vec.empty()){
                        continue;
                    }
                    else{
                        if(t_vec.last().m_n_index==point[j].m_n_index){
                            t_min=point[j].m_n_p;
                            index=j;
                        }
                    }
                }
            }
        }
        if(index!=-1){
            t_vec.push_back(point[index]);
            point[index].m_n_index=3;
        }
    }

    QString str;
    for(int i=0;i<t_vec.size();i++){
        if(t_vec[i].m_n_index==1)
            str.append("1");
        else
            str.append("2");
    }
    if(str.contains("11")&&str.contains("22"))
        b_ret=false;
    else
        b_ret=true;

    return b_ret;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    m_thread.destory_layer();
    QMainWindow::closeEvent(event);
}

void MainWindow::put_frame(unsigned char *pbuffer, int wid, int hei, int csp, int level)
{

    static int i=0;
    i++;
    ui->opengl_player->put_frame((unsigned char *)pbuffer,wid,hei, csp, level);

}

void MainWindow::fresh_fps()
{
    QString str;
    str.sprintf("FPS:%.02lf",m_thread.get_fps());
    ui->statusBar->showMessage(str);
}

void MainWindow::on_opengl_version(bool t_b_opengl)
{
    if(!t_b_opengl){
        QMessageBox::information(NULL,"Warning","This program need OpenGL 3.0 support,please upgrade OpenGL on this machine.");
        exit(0);
    }
}

void MainWindow::on_load_1_clicked()
{
    m_thread.destory_layer();

    bool b_ret=false;

    //load setting file
    b_ret=load_setting_file(0);
    if(!b_ret)
        return;

    //set channel and layer
    b_ret=set_capture(0);
    if(!b_ret)
        return;

    //strat capture
    open_preview();
}

void MainWindow::on_load_2_clicked()
{
    m_thread.destory_layer();

    bool b_ret=false;

    //load setting file
    b_ret=load_setting_file(1);
    if(!b_ret)
        return;

    //set channel and layer
    b_ret=set_capture(1);
    if(!b_ret)
        return;

    //strat capture
    open_preview();
}

void MainWindow::on_load_3_clicked()
{
    m_thread.destory_layer();

    bool b_ret=false;

    //load setting file
    b_ret=load_setting_file(2);
    if(!b_ret)
        return;

    //set channel and layer
    b_ret=set_capture(2);
    if(!b_ret)
        return;

    //strat capture
    open_preview();
}

void MainWindow::on_load_4_clicked()
{
    m_thread.destory_layer();

    bool b_ret=false;

    //load setting file
    b_ret=load_setting_file(3);
    if(!b_ret)
        return;

    //set channel and layer
    b_ret=set_capture(3);
    if(!b_ret)
        return;

    //strat capture
    open_preview();
}



void MainWindow::on_pushButton_clicked()
{
    m_thread.destory_layer();
}
