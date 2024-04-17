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

#ifndef COPENGLPLAYER_H
#define COPENGLPLAYER_H

#include <QObject>
#include <QOpenGLWidget>
#include <QOpenGLFunctions_3_0>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QOpenGLContext>
#include "common.h"



typedef enum _color_space {
    MW_CSP_AUTO = 0,
    MW_CSP_BT_601,
    MW_CSP_BT_709,
    MW_CSP_BT_2020,
    MW_CSP_COUNT,
}mw_color_space_e;

typedef enum _color_space_level {
    MW_CSP_LEVELS_AUTO = 0,
    MW_CSP_LEVELS_TV,
    MW_CSP_LEVELS_PC,
    MW_CSP_LEVELS_COUNT,
}mw_color_space_level_e;


typedef struct yuvlevels { double ymin, ymax, cmax, cmid; }yuvlevels;

typedef struct rgblevels { double min, max; }rgblevels;




class COpenGLPlayer : public QOpenGLWidget,protected QOpenGLFunctions_3_0
{
    Q_OBJECT
public:
    COpenGLPlayer(QWidget *parent=NULL);
    ~COpenGLPlayer();

public:
    void put_frame(unsigned char* buf,int wid,int hei, int csp, int level);

public:
    bool m_b_opengl_functions;

protected:
    void initializeGL();
    void paintGL();
    void resizeGL(int w,int h);

protected:
    bool create_yuy2_program();

private:
    QOpenGLShader           *m_yuy2_vs;         //vertex shader
    QOpenGLShader           *m_yuy2_fs;         //fragment shader
    QOpenGLShaderProgram    *m_yuy2_sp;         //shder program
    QOpenGLTexture          *m_yuy2_txeture;    //yuy2 texture

    GLuint m_glui_vertex;
    GLuint m_glui_texture;

    GLuint m_glui_frame_buffer;
    GLuint m_glui_render_buffer;

    GLuint m_glui_yuy2_texture_id;

    int m_n_uniform_yuy2_texture0;


    int m_n_width;
    int m_n_height;

    unsigned char*m_p_uc_data;
    unsigned char *m_pic;

    mw_color_space_e m_csp;
        mw_color_space_level_e m_csp_lvl_in;
        mw_color_space_level_e m_csp_lvl_out;



    bool m_b_opengl_program;

signals:
    void opengl_state(bool t_b_opengl3_0);
};

#endif // COPENGLPLAYER_H
