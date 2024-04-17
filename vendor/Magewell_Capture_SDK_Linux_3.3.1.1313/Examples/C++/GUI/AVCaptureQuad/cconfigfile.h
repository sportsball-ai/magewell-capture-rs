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

#ifndef CCONFIGFILE_H
#define CCONFIGFILE_H

#include <QObject>
#include <QFile>
#include <QVector>

/*
 * file extended format .cfg
*/
typedef struct _valid_chanel{
    int m_n_index;
    QString m_qs_name;
}valid_channel;

typedef struct _channel_set{
    int m_n_index;
    int m_n_posx;
    int m_n_posy;
    int m_n_poswid;
    int m_n_poshei;
    int m_n_layer;
}channel_set;

typedef struct _cfg{
    bool m_b_exist;
    QString m_qs_name;

    channel_set m_set[4];
}cfg;



class CConfigFile : public QObject
{
    Q_OBJECT
public:
    explicit CConfigFile(QObject *parent = 0);

protected:
    QString m_qs_name_1;
    QString m_qs_name_2;
    QString m_qs_name_3;
    QString m_qs_name_4;

    cfg m_arr_cfg[4];

public:
    bool detect_file();
    bool generate_file(int index, channel_set *p_set, QVector<valid_channel> p_valid, int n_num);
    bool load_file(int index);

    cfg *get_channelset(int index);

protected:
    bool parse_line(QString line,int cfg_i,int cfg_ii);

protected:
    QFile *m_file;

signals:

public slots:
};

#endif // CCONFIGFILE_H
