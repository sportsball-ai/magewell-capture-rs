from OpenGL.GL import *
from PyQt5.QtWidgets import QOpenGLWidget
from PyQt5 import QtCore
import numpy as np
import copy
from LibMWCapture import *

g_str_v="""
    #version 130
    in vec2 vertexIn;
    in vec2 textureIn;
    out vec2 textureOut;
    void main(void){
        gl_Position = vec4(vertexIn,0.0,1);
        textureOut = textureIn;
    }
    """

g_str_f_yuyv="""
    #version 130
    uniform sampler2D tex_yuv_rg;
    uniform sampler2D tex_yuv_rgba;
    in vec2 textureOut;
    void main(void){
        float r,g,b,y,u,v;
        y = texture2D(tex_yuv_rg,textureOut).r;
        u = texture2D(tex_yuv_rgba,textureOut).g;
        v = texture2D(tex_yuv_rgba,textureOut).a;
        y = 1.1643*(y-0.0625);
        u = u - 0.5;
        v = v - 0.5;
        r = y + 1.5958*v;
        g = y - 0.39173*u - 0.81290*v;
        b = y + 2.017*u;
        gl_FragColor=vec4(r,g,b,1.0);
    }
    """

g_str_f_nv12 = """
	#version  130
	in vec2 textureOut;
	uniform sampler2D tex_y;
	uniform sampler2D tex_uv;
	void main(void)
	{
	  float r, g, b, my, mu, mv;
	  my = texture2D(tex_y, textureOut).r;
	  mu = texture2D(tex_uv, textureOut).r;
	  mv = texture2D(tex_uv, textureOut).g;

	  my = 1.1643*(my-0.0625);
	  mu = mu - 0.5;
	  mv = mv - 0.5;
	  r = my+1.5958*mv;
	  g = my-0.39173*mu-0.81290*mv;
	  b = my+2.017*mu;
	  gl_FragColor=vec4(r,g,b,1.0);
	}
"""

g_ver_vertices = np.array([-1.0,-1.0,1.0,-1.0,-1.0,1.0,1.0,1.0],dtype=np.float32)
g_ver_textures = np.array([0.0,1.0,1.0,1.0,0.0,0.0,1.0,0.0],dtype=np.float32)

class CRenderWid(QOpenGLWidget):
    def __init__(self, parent=None, flags = QtCore.Qt.WindowFlags()):
        super().__init__(parent=parent, flags=flags)
        self.m_f_r = 1.0
        self.m_f_g = 1.0
        self.m_f_b = 1.0
        self.m_glsl_text = 0
        self.m_glsl_shader_v = 0
        self.m_glint_len_shader_v = GLint(0)
        self.m_glsl_shader_f = 0
        self.m_glint_len_shader_f = GLint(0)
        self.m_glsl_program_yuyv = 0
        self.m_glsl_fbo = 0
        self.m_glsl_rbo = 0
        self.m_data = 0
        self.m_fourcc = 0
        self.m_cx = 0
        self.m_cy = 0

    def __del__(self):
        pass

    def initializeGL(self):
        print(glGetString(GL_VERSION))
        #self.setup_render_for_yuy2()

    def paintGL(self):
        #print('paintgl')
        if self.m_data!= 0:
            if self.m_fourcc == MWFOURCC_YUY2:
                self.render_yuy2(1)
            elif self.m_fourcc == MWFOURCC_NV12:
                self.render_nv12(1)
            else:
                glClear(GL_COLOR_BUFFER_BIT)
                glClearColor(0.0,0.0,0.0,1.0)    
        else:
            glClear(GL_COLOR_BUFFER_BIT)
            glClearColor(0.0,0.0,0.0,1.0)

    def resizeGL(self, x, y):
        self.makeCurrent()
        glViewport(0,0,x,y)

    def open_render(self,fourcc,cx,cy):
        
        self.abolish_render()
        t_b_ret = False
        if fourcc == MWFOURCC_YUY2:
            t_b_ret = self.setup_render_for_yuy2(cx,cy)
        elif fourcc == MWFOURCC_NV12:
            t_b_ret = self.setup_render_for_nv12(cx,cy)
        if t_b_ret == True:
            self.m_fourcc = fourcc
            self.m_cx = cx
            self.m_cy = cy
        else:
            self.m_fourcc = 0
            self.m_cx = 0
            self.m_cy = 0
        return t_b_ret 

    def setup_render_for_yuy2(self,cx,cy):
        self.makeCurrent()
        self.m_glsl_text1 = glGenTextures(1)
        if self.m_glsl_text1 == 0:
            self.abolish_render()
            return False
            
        glBindTexture(GL_TEXTURE_2D,self.m_glsl_text1)
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR)
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR)
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE)
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE)

        self.m_glsl_text2 = glGenTextures(1)
        if self.m_glsl_text2 == 0:
            self.abolish_render()
            return False
            
        glBindTexture(GL_TEXTURE_2D,self.m_glsl_text2)
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR)
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR)
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE)
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE)

        self.m_glsl_shader_v = glCreateShader(GL_VERTEX_SHADER)
        if self.m_glsl_shader_v == 0:
            self.abolish_render()
            return False
        self.m_glsl_shader_f = glCreateShader(GL_FRAGMENT_SHADER)
        if self.m_glsl_shader_f == 0:
            self.abolish_render()
            return False
        glShaderSource(self.m_glsl_shader_v,g_str_v)
        glShaderSource(self.m_glsl_shader_f,g_str_f_yuyv)

        glCompileShader(self.m_glsl_shader_v)
        t_status = glGetShaderiv(self.m_glsl_shader_v,GL_COMPILE_STATUS)
        if not(t_status):
            print("v----------\n\n%s\n\n________\n" % (glGetShaderInfoLog(self.m_glsl_shader_v)))
            self.abolish_render()
            return False
        
        glCompileShader(self.m_glsl_shader_f)
        t_status = glGetShaderiv(self.m_glsl_shader_f,GL_COMPILE_STATUS)
        if not(t_status):
            print("f----------\n\n%s\n\n________\n" % (glGetShaderInfoLog(self.m_glsl_shader_f)))
            self.abolish_render()
            return False

        self.m_glsl_program_yuyv = glCreateProgram()
        glAttachShader(self.m_glsl_program_yuyv,self.m_glsl_shader_v)
        glAttachShader(self.m_glsl_program_yuyv,self.m_glsl_shader_f)
        glLinkProgram(self.m_glsl_program_yuyv)
        t_status = glGetProgramiv(self.m_glsl_program_yuyv,GL_LINK_STATUS)
        if not(t_status):
            self.abolish_render()
            return False
        
        self.m_glsl_yuyv_ver_loc = glGetAttribLocation(self.m_glsl_program_yuyv,"vertexIn");
        self.m_glsl_yuyv_tex_loc = glGetAttribLocation(self.m_glsl_program_yuyv,"textureIn");

        glVertexAttribPointer(self.m_glsl_yuyv_ver_loc,2,GL_FLOAT,0,0,g_ver_vertices)
        glEnableVertexAttribArray(self.m_glsl_yuyv_ver_loc)
        glVertexAttribPointer(self.m_glsl_yuyv_tex_loc,2,GL_FLOAT,0,0,g_ver_textures)
        glEnableVertexAttribArray(self.m_glsl_yuyv_tex_loc)

        return True

    def setup_render_for_nv12(self,cx,cy):
        self.makeCurrent()
        self.m_glsl_text1 = glGenTextures(1)
        if self.m_glsl_text1 == 0:
            self.abolish_render()
            return False
        glBindTexture(GL_TEXTURE_2D,self.m_glsl_text1)
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR)
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR)
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE)
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE)

        self.m_glsl_text2 = glGenTextures(1)
        if self.m_glsl_text2 == 0:
            self.abolish_render()
            return False
        glBindTexture(GL_TEXTURE_2D,self.m_glsl_text2)
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR)
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR)
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE)
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE)

        self.m_glsl_shader_v = glCreateShader(GL_VERTEX_SHADER)
        if self.m_glsl_shader_v == 0:
            self.abolish_render()
            return False
        self.m_glsl_shader_f = glCreateShader(GL_FRAGMENT_SHADER)
        if self.m_glsl_shader_f == 0:
            self.abolish_render()
            return False
        glShaderSource(self.m_glsl_shader_v,g_str_v)
        glShaderSource(self.m_glsl_shader_f,g_str_f_nv12)

        glCompileShader(self.m_glsl_shader_v)
        t_status = glGetShaderiv(self.m_glsl_shader_v,GL_COMPILE_STATUS)
        if not(t_status):
            print("v----------\n\n%s\n\n________\n" % (glGetShaderInfoLog(self.m_glsl_shader_v)))
            self.abolish_render()
            return False
        
        glCompileShader(self.m_glsl_shader_f)
        t_status = glGetShaderiv(self.m_glsl_shader_f,GL_COMPILE_STATUS)
        if not(t_status):
            print("f----------\n\n%s\n\n________\n" % (glGetShaderInfoLog(self.m_glsl_shader_f)))
            self.abolish_render()
            return False

        self.m_glsl_program_yuyv = glCreateProgram()
        glAttachShader(self.m_glsl_program_yuyv,self.m_glsl_shader_v)
        glAttachShader(self.m_glsl_program_yuyv,self.m_glsl_shader_f)
        glLinkProgram(self.m_glsl_program_yuyv)
        t_status = glGetProgramiv(self.m_glsl_program_yuyv,GL_LINK_STATUS)
        if not(t_status):
            self.abolish_render()
            return False
        
        self.m_glsl_yuyv_ver_loc = glGetAttribLocation(self.m_glsl_program_yuyv,"vertexIn");
        self.m_glsl_yuyv_tex_loc = glGetAttribLocation(self.m_glsl_program_yuyv,"textureIn");

        glVertexAttribPointer(self.m_glsl_yuyv_ver_loc,2,GL_FLOAT,0,0,g_ver_vertices)
        glEnableVertexAttribArray(self.m_glsl_yuyv_ver_loc)
        glVertexAttribPointer(self.m_glsl_yuyv_tex_loc,2,GL_FLOAT,0,0,g_ver_textures)
        glEnableVertexAttribArray(self.m_glsl_yuyv_tex_loc)

        return True

    def abolish_render(self):
        self.makeCurrent()

        if self.m_glsl_shader_v != 0:
            glDeleteShader(self.m_glsl_shader_v)
            self.m_glsl_shader_v = 0

        if self.m_glsl_shader_f != 0:
            glDeleteShader(self.m_glsl_shader_f)
            self.m_glsl_shader_f = 0

        if self.m_glsl_program_yuyv != 0:
            glDeleteProgram(self.m_glsl_program_yuyv)
            self.m_glsl_program_yuyv = 0

        if self.m_glsl_text != 0:
            glDeleteTextures([self.m_glsl_text])

    def render_yuy2(self,pbframe):
        self.makeCurrent()
        if pbframe == 0:
            glClear(GL_COLOR_BUFFER_BIT)
            glClearColor(0.0,0.0,0.0,1.0)
            return True
        glUseProgram(self.m_glsl_program_yuyv)

        glClear(GL_COLOR_BUFFER_BIT)
        glClearColor(1.0,0.0,0.0,1.0)
        
        glActiveTexture(GL_TEXTURE0)
        glBindTexture(GL_TEXTURE_2D,self.m_glsl_text1)
        if pbframe!=0:
            glTexImage2D(GL_TEXTURE_2D,0,GL_RG,self.m_cx,self.m_cy,0,GL_RG,GL_UNSIGNED_BYTE,self.m_data)
        glUniform1i(glGetUniformLocation(self.m_glsl_program_yuyv,"tex_yuv_rg"),0)

        glActiveTexture(GL_TEXTURE1)
        glBindTexture(GL_TEXTURE_2D,self.m_glsl_text2)
        if pbframe!=0:
            glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,self.m_cx/2,self.m_cy,0,GL_RGBA,GL_UNSIGNED_BYTE,self.m_data)
        glUniform1i(glGetUniformLocation(self.m_glsl_program_yuyv,"tex_yuv_rgba"),1)

        glDrawArrays(GL_TRIANGLE_STRIP,0,4)
        glViewport(0,0,self.width(),self.width())

        return True

    def render_nv12(self,pbframe):
        self.makeCurrent()
        if pbframe == 0:
            glClear(GL_COLOR_BUFFER_BIT)
            glClearColor(0.0,0.0,0.0,1.0)
            return True
        glUseProgram(self.m_glsl_program_yuyv)

        glClear(GL_COLOR_BUFFER_BIT)
        glClearColor(1.0,0.0,0.0,1.0)
        
        glActiveTexture(GL_TEXTURE0)
        glBindTexture(GL_TEXTURE_2D,self.m_glsl_text1)
        if pbframe!=0:
            glTexImage2D(GL_TEXTURE_2D,0,GL_RED,self.m_cx,self.m_cy,0,GL_RED,GL_UNSIGNED_BYTE,self.m_data)
        glUniform1i(glGetUniformLocation(self.m_glsl_program_yuyv,"tex_y"),0)

        glActiveTexture(GL_TEXTURE1)
        glBindTexture(GL_TEXTURE_2D,self.m_glsl_text2)
        if pbframe!=0:
            glTexImage2D(GL_TEXTURE_2D,0,GL_RG,self.m_cx/2,self.m_cy/2,0,GL_RG,GL_UNSIGNED_BYTE,self.m_data[self.m_cx*self.m_cy:])#
        glUniform1i(glGetUniformLocation(self.m_glsl_program_yuyv,"tex_uv"),1)
        #self.m_cx*self.m_cy

        glDrawArrays(GL_TRIANGLE_STRIP,0,4)
        #glViewport(0,0,self.width(),self.width())
        return True

    def put_frame(self,pbframe):
        self.m_data = pbframe#copy.deepcopy(pbframe)
        self.update()
        
