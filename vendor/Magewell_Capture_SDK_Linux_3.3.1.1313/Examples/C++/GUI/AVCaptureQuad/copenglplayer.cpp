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

#include "copenglplayer.h"

pthread_mutex_t g_p_mutex;
unsigned char g_buffer[1920*1080*2];

static const GLfloat g_vertex_vertices[]={
    -1.0f,-1.0f,
    1.0f,-1.0f,
    -1.0f,1.0f,
    1.0f,1.0f,
} ;

static const GLfloat g_texture_vertices[]={
    0.0f,1.0f,
    1.0f,1.0f,
    0.0f,0.0f,
    1.0f,0.0f,
};

COpenGLPlayer::COpenGLPlayer(QWidget *parent):QOpenGLWidget(parent)
{
    //init yuy2 shaders
    m_yuy2_vs=NULL;
    m_yuy2_fs=NULL;
    m_yuy2_sp=NULL;

    //init texture data
    m_p_uc_data=NULL;

    //init texture size
    m_n_width=1920;
    m_n_height=1080;

    m_p_uc_data=new unsigned char[1920*1080*2];

    m_b_opengl_functions=false;
    m_b_opengl_program=false;

    m_glui_vertex=0;
    m_glui_texture=1;

    m_csp = MW_CSP_BT_709;
    m_csp_lvl_in = MW_CSP_LEVELS_TV;
    m_csp_lvl_out = MW_CSP_LEVELS_PC;

    pthread_mutex_init(&g_p_mutex,NULL);
}

COpenGLPlayer::~COpenGLPlayer()
{
     pthread_mutex_destroy(&g_p_mutex);
     if(m_p_uc_data!=NULL)
         delete m_p_uc_data;
}

void COpenGLPlayer::put_frame(unsigned char *buf, int wid, int hei, int csp, int level)
{

    m_n_width=wid;
    m_n_height=hei;
    pthread_mutex_lock(&g_p_mutex);
    memcpy(m_p_uc_data,g_buffer,1920*1080*2);
    pthread_mutex_unlock(&g_p_mutex);
    makeCurrent();
    update();

    if(0 == csp)
    {
       m_csp = MW_CSP_BT_601;
    }
    else if(1 == csp)
    {
       m_csp = MW_CSP_BT_709;
    }
    else if(2 == csp)
    {
       m_csp =  MW_CSP_BT_2020;
    }


    if(0 == level)
    {
        m_csp_lvl_in = MW_CSP_LEVELS_TV;
    }
    else if(1 == level)
    {
        m_csp_lvl_in = MW_CSP_LEVELS_PC;
    }

}

void COpenGLPlayer::initializeGL()
{
    m_b_opengl_functions=initializeOpenGLFunctions();
    if(!m_b_opengl_functions){
        emit opengl_state(false);
        return;
    }

    glEnable(GL_DEPTH_TEST);

    m_b_opengl_program=create_yuy2_program();
    if(!m_b_opengl_program)
        return;

    glVertexAttribPointer(m_glui_vertex,2,GL_FLOAT,0,0,g_vertex_vertices);
    glVertexAttribPointer(m_glui_texture,2,GL_FLOAT,0,0,g_texture_vertices);

    glEnableVertexAttribArray(m_glui_vertex);
    glEnableVertexAttribArray(m_glui_texture);

    m_yuy2_txeture=new QOpenGLTexture(QOpenGLTexture::Target2D);
    m_yuy2_txeture->create();
    m_glui_yuy2_texture_id=m_yuy2_txeture->textureId();

    glGenFramebuffers(1,&m_glui_frame_buffer);
    glBindFramebuffer(GL_FRAMEBUFFER,m_glui_frame_buffer);
    glGenRenderbuffers(1,&m_glui_render_buffer);
    glBindRenderbuffer(GL_RENDERBUFFER,m_glui_render_buffer);
    glRenderbufferStorage(GL_RENDERBUFFER,GL_RGBA8,1920,1080);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_RENDERBUFFER,m_glui_render_buffer);

    glBindFramebuffer(GL_FRAMEBUFFER,0);
}

void COpenGLPlayer::paintGL()
{
    if(!m_b_opengl_program){
        return;
    }
    glViewport(0,0,1920,1080);
    glClearColor(1.0,1.0,0.0,1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glBindFramebuffer(GL_FRAMEBUFFER,m_glui_frame_buffer);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    glClearColor(0.0,0.0,0.0,0.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D,m_glui_yuy2_texture_id);
    pthread_mutex_lock(&g_p_mutex);
    glTexImage2D(GL_TEXTURE_2D,0,GL_RG,1920,1080,0,GL_RG,GL_UNSIGNED_BYTE,m_p_uc_data);
    pthread_mutex_unlock(&g_p_mutex);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glUniform1i(m_n_uniform_yuy2_texture0,0);


    glDrawArrays(GL_TRIANGLE_STRIP,0,4);
    glFinish();

    glBindFramebuffer(GL_READ_FRAMEBUFFER,m_glui_frame_buffer);

    glViewport(0,0,this->width(),this->height());

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER,defaultFramebufferObject());
    glClearColor(0.0,0.0,0.0,0.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    int t_cx,t_cy;
    t_cx=this->width();
    t_cy=this->height();
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    glBlitFramebuffer(0,0,1920,1080,0,0,t_cx,t_cy,GL_COLOR_BUFFER_BIT,GL_NEAREST);

    glFinish();
}

void COpenGLPlayer::resizeGL(int w, int h)
{
    if(!m_b_opengl_functions){
        return;
    }
    if(h==0)
    {
        h=1;
    }
    glViewport(0,0,w,h);
}

bool COpenGLPlayer::create_yuy2_program()
{
    bool b_ret=false;
    if(m_yuy2_vs!=NULL){
        delete m_yuy2_vs;
        m_yuy2_vs=NULL;
    }

    if(m_yuy2_fs!=NULL){
        delete m_yuy2_fs;
        m_yuy2_fs=NULL;
    }

    if(m_yuy2_sp!=NULL){
        delete m_yuy2_sp;
        m_yuy2_sp=NULL;
    }

    m_yuy2_vs=new QOpenGLShader(QOpenGLShader::Vertex,this);
    m_yuy2_fs=new QOpenGLShader(QOpenGLShader::Fragment,this);

    QString path_vs,path_fs;
    path_vs.append(":/gl/gl_vs.vert");
    path_fs.append(":/gl/gl_fs.frag");

    b_ret=m_yuy2_vs->compileSourceFile(path_vs);
    if(!b_ret)
        return b_ret;
    b_ret=m_yuy2_fs->compileSourceFile(path_fs);
    if(!b_ret)
        return b_ret;

    m_yuy2_sp=new QOpenGLShaderProgram(this);
    b_ret=m_yuy2_sp->addShader(m_yuy2_vs);
    if(!b_ret)
        return b_ret;
    b_ret=m_yuy2_sp->addShader(m_yuy2_fs);
    if(!b_ret)
        return b_ret;

    //bind attribute location
    m_yuy2_sp->bindAttributeLocation("qt_Vertex",m_glui_vertex);
    m_yuy2_sp->bindAttributeLocation("qt_TextureIn",m_glui_texture);

    b_ret=m_yuy2_sp->link();
    if(!b_ret)
        return b_ret;
    b_ret=m_yuy2_sp->bind();
    if(!b_ret)
        return b_ret;

    m_n_uniform_yuy2_texture0=m_yuy2_sp->uniformLocation("qt_Texture0");

    //set uniform value
    m_yuy2_sp->setUniformValue("cx",1920);
    m_yuy2_sp->setUniformValue("cy",1080);


      float csp_coeff[3][3] = {0.0};
      float csp_const[3] = {0.0};
      float lr = 0.0;
      float lg = 0.0;
      float lb = 0.0;




      {
          switch (m_csp)
          {
          case MW_CSP_BT_601:
              lr = 0.299, lg = 0.587, lb = 0.114;
              break;
          case MW_CSP_BT_709:
              lr = 0.2126, lg = 0.7152, lb = 0.0722;
              break;
          case MW_CSP_BT_2020:
              lr = 0.2627, lg = 0.6780, lb = 0.0593;
              break;
          }
      }

      csp_coeff[0][0] = 1;
      csp_coeff[0][1] = 0;
      csp_coeff[0][2] = 2 * (1 - lr);
      csp_coeff[1][0] = 1;
      csp_coeff[1][1] = -2 * (1 - lb) * lb / lg;
      csp_coeff[1][2] = -2 * (1 - lr) * lr / lg;
      csp_coeff[2][0] = 1;
      csp_coeff[2][1] = 2 * (1 - lb);
      csp_coeff[2][2] = 0;


      yuvlevels yuvlev = {0.0};
      rgblevels rgblev = {0.0};
      rgblevels t_rgblev_tv={ 16.0 / 255.0, 235.0 / 255.0 };
      rgblevels t_rgblev_pc={ 0.0, 255.0 / 255.0 };
      yuvlevels t_lev_tv={ 16.0 / 255.0, 235.0 / 255.0, 240.0 / 255.0, 128.0 / 255.0 };
      yuvlevels t_lev_pc={ 0.0 , 255.0 / 255.0, 255.0 / 255.0, 128.0 / 255.0 };








      {
          switch (m_csp_lvl_in)
          {
          case MW_CSP_LEVELS_TV:
              yuvlev = t_lev_tv;
              break;
          case MW_CSP_LEVELS_PC:
              yuvlev = t_lev_pc;
              break;
          }
          switch (m_csp_lvl_out)
          {
          case MW_CSP_LEVELS_TV:
              rgblev = t_rgblev_tv;
              break;
          case MW_CSP_LEVELS_PC: rgblev = t_rgblev_pc;
              break;
          }
      }


      double ymul = (rgblev.max - rgblev.min) / (yuvlev.ymax - yuvlev.ymin);
      double cmul = (rgblev.max - rgblev.min) / (yuvlev.cmax - yuvlev.cmid) / 2.0;

      for (int i = 0; i < 3; i++) {
          csp_coeff[i][0] *= ymul;
          csp_coeff[i][1] *= cmul;
          csp_coeff[i][2] *= cmul;

          csp_const[i] = rgblev.min - csp_coeff[i][0] * yuvlev.ymin
              - (csp_coeff[i][1] + csp_coeff[i][2]) * yuvlev.cmid;

      }


    m_yuy2_sp->setUniformValue("csp_const",csp_const[0],csp_const[1],csp_const[2]);
    m_yuy2_sp->setUniformValue("csp_coeff_0",csp_coeff[0][0],csp_coeff[1][0],csp_coeff[2][0]);
    m_yuy2_sp->setUniformValue("csp_coeff_1",csp_coeff[0][1],csp_coeff[1][1],csp_coeff[2][1]);
    m_yuy2_sp->setUniformValue("csp_coeff_2",csp_coeff[0][2],csp_coeff[1][2],csp_coeff[2][2]);

    return true;
}
