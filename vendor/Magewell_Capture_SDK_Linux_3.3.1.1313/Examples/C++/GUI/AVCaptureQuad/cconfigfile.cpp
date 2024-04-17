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

#include "cconfigfile.h"

CConfigFile::CConfigFile(QObject *parent) : QObject(parent)
{
    m_qs_name_1=QString("setting1.cfg");
    m_qs_name_2=QString("setting2.cfg");
    m_qs_name_3=QString("setting3.cfg");
    m_qs_name_4=QString("setting4.cfg");

    m_file=new QFile(this);

    m_arr_cfg[0].m_qs_name=m_qs_name_1;

    m_arr_cfg[1].m_qs_name=m_qs_name_2;

    m_arr_cfg[2].m_qs_name=m_qs_name_3;

    m_arr_cfg[3].m_qs_name=m_qs_name_4;
}

bool CConfigFile::detect_file()
{
   for(int i=0;i<4;i++){
       m_arr_cfg[i].m_b_exist=m_file->exists(m_arr_cfg[i].m_qs_name);
       if(m_arr_cfg[i].m_b_exist){
       }
       else{
       }
   }
   bool t_b_ret=true;
   for(int i=0;i<4;i++){
       if(m_arr_cfg[i].m_b_exist==false){
           t_b_ret=false;
           break;
       }
   }
   return t_b_ret;
}

bool CConfigFile::generate_file(int index, channel_set *p_set, QVector<valid_channel> p_valid, int n_num)
{
    m_file->setFileName(m_arr_cfg[index].m_qs_name);
    m_file->open(QIODevice::WriteOnly);
    QByteArray t_bta;

    //write setted channel
    QString t_qs_con;
    for(int i=0;i<4;i++){
        QString t_qs_set;
        t_qs_set.sprintf("index:%d posx:%d posy:%d poswid:%d poshei:%d layer:%d\n",
                         p_set[i].m_n_index,p_set[i].m_n_posx,p_set[i].m_n_posy,
                         p_set[i].m_n_poswid,p_set[i].m_n_poshei,p_set[i].m_n_layer);
        t_qs_con.append(t_qs_set);
    }

    t_qs_con.append("********************************************************\n");

    //write valid channels
    for(int i=0;i<n_num;i++){
        QString t_qs_va;
        t_qs_va.sprintf("index:%d name:%s\n",p_valid[i].m_n_index,p_valid[i].m_qs_name.toStdString().c_str());
        t_qs_con.append(t_qs_va);
    }
    t_qs_con.append("********************************************************\n");
    t_qs_con.append("posx 0-99\nposy 0-99\nposywid 1-100\nposhei 1-100\nlayer 1-4\n");

    t_bta.append(t_qs_con);
    m_file->write(t_bta);
    m_file->close();
}

bool CConfigFile::load_file(int index)
{
    m_file->setFileName(m_arr_cfg[index].m_qs_name);
    m_file->open(QIODevice::ReadOnly);
    QByteArray line1=m_file->readLine(128);
    QString qs_line1;
    qs_line1.append(line1);
    parse_line(qs_line1,index,0);

    QByteArray line2=m_file->readLine(128);
    QString qs_line2;
    qs_line2.append(line2);
    parse_line(qs_line2,index,1);

    QByteArray line3=m_file->readLine(128);
    QString qs_line3;
    qs_line3.append(line3);
    parse_line(qs_line3,index,2);

    QByteArray line4=m_file->readLine(128);
    QString qs_line4;
    qs_line4.append(line4);
    parse_line(qs_line4,index,3);

    m_file->close();
}

cfg *CConfigFile::get_channelset(int index)
{
    return &m_arr_cfg[index];
}

bool CConfigFile::parse_line(QString line, int cfg_i, int cfg_ii)
{
    bool b_ret=false;

    int t_n_pos=line.indexOf(":",0);//pos index:
    int t_n_sp=line.indexOf(" ",0);
    QString t_qs_index=line.mid(t_n_pos+1,t_n_sp-t_n_pos);
    m_arr_cfg[cfg_i].m_set[cfg_ii].m_n_index=t_qs_index.toInt();

    t_n_pos=line.indexOf(":",t_n_sp);
    t_n_sp=line.indexOf(" ",t_n_pos);
    QString qs_x=line.mid(t_n_pos+1,t_n_sp-t_n_pos);
    m_arr_cfg[cfg_i].m_set[cfg_ii].m_n_posx=qs_x.toInt();

    t_n_pos=line.indexOf(":",t_n_sp);
    t_n_sp=line.indexOf(" ",t_n_pos);
    QString qs_y=line.mid(t_n_pos+1,t_n_sp-t_n_pos);
    m_arr_cfg[cfg_i].m_set[cfg_ii].m_n_posy=qs_y.toInt();

    t_n_pos=line.indexOf(":",t_n_sp);
    t_n_sp=line.indexOf(" ",t_n_pos);
    QString qs_wid=line.mid(t_n_pos+1,t_n_sp-t_n_pos);
    m_arr_cfg[cfg_i].m_set[cfg_ii].m_n_poswid=qs_wid.toInt();

    t_n_pos=line.indexOf(":",t_n_sp);
    t_n_sp=line.indexOf(" ",t_n_pos);
    QString qs_hei=line.mid(t_n_pos+1,t_n_sp-t_n_pos);
    m_arr_cfg[cfg_i].m_set[cfg_ii].m_n_poshei=qs_hei.toInt();

    t_n_pos=line.indexOf(":",t_n_sp);
    t_n_sp=line.indexOf("\n",t_n_pos);
    QString qs_layer=line.mid(t_n_pos+1,t_n_sp-t_n_pos);
    m_arr_cfg[cfg_i].m_set[cfg_ii].m_n_layer=qs_layer.toInt();

    return b_ret;
}
