package com.magewell;

import java.nio.ByteBuffer;
import java.nio.FloatBuffer;

//import static org.lwjgl.opengl.GL20.*;
//import static org.lwjgl.opengl.ARBFramebufferObject.*;

import static org.lwjgl.opengl.GL30.*;
import static org.lwjgl.opengl.GL30.glGetUniformLocation;

import org.lwjgl.BufferUtils;

public class VideoRender {
    public static final int RENDER_RGB24 = 1;
    public static final int RENDER_BGR24 = 2;
    public static final int RENDER_RGBA = 3;
    public static final int RENDER_BGRA = 4;
    public static final int RENDER_ARGB = 5;
    public static final int RENDER_ABGR = 6;

    public static final int RENDER_NV12 = 101;
    public static final int RENDER_NV21 = 102;
    public static final int RENDER_I420 = 103;
    public static final int RENDER_YV12 = 104;
    public static final int RENDER_YUY2 = 105;

    public static final int RENDER_P010 = 201;

    int m_width;
    int m_height;
    int m_program;
    int[] m_tex = new int[4];
    //int m_fbo;
    //int m_rbo;
    int m_yuy2_buffer;
    long m_fourcc;
    int m_verLocation;
    int m_texLocation;

    float[] vertexVertices = {

            -1.0f, -1.0f,
            1.0f, -1.0f,
            -1.0f,  1.0f,
            1.0f,  1.0f,
    };

    float[] textureVertices = {
            0.0f,  1.0f,
            1.0f,  1.0f,
            0.0f,  0.0f,
            1.0f,  0.0f,
    };

    String vertex_shader=
            "#version 130 \n" +
            "in vec2 vertexIn; \n" +
            "in vec2 textureIn; \n" +
            "out vec2 textureOut; \n" +
            "void main(void) \n" +
            "{ \n" +
            "  gl_Position = vec4(vertexIn, 0.0, 1); \n" +
            "  textureOut = textureIn; \n" +
            "} \n";

    String nv12_fragment_shader=
            "#version  130 \n" +
            "in vec2 textureOut; \n" +
            "uniform sampler2D tex_y; \n" +
            "uniform sampler2D tex_uv; \n" +
            "void main(void) \n" +
            "{ \n" +
            "  float r, g, b, my, mu, mv; \n" +
            "  my = texture2D(tex_y, textureOut).r; \n" +
            "  mu = texture2D(tex_uv, textureOut).r; \n" +
            "  mv = texture2D(tex_uv, textureOut).g; \n" +
            "  my = 1.1643*(my-0.0625); \n" +
            "  mu = mu - 0.5; \n" +
            "  mv = mv - 0.5; \n" +
            "  r = my+1.5958*mv; \n" +
            "  g = my-0.39173*mu-0.81290*mv; \n" +
            "  b = my+2.017*mu; \n" +
            "  gl_FragColor=vec4(r,g,b,1.0); \n" +
            "} \n";
    String nv21_fragment_shader=
            "#version  130 \n" +
            "in vec2 textureOut; \n" +
            "uniform sampler2D tex_y; \n" +
            "uniform sampler2D tex_uv; \n" +
            "void main(void) \n" +
            "{ \n" +
            "  float r, g, b, my, mu, mv; \n" +
            "  my = texture2D(tex_y, textureOut).r; \n" +
            "  mu = texture2D(tex_uv, textureOut).g; \n" +
            "  mv = texture2D(tex_uv, textureOut).r; \n" +
            "  my = 1.1643*(my-0.0625); \n" +
            "  mu = mu - 0.5; \n" +
            "  mv = mv - 0.5; \n" +
            "  r = my+1.5958*mv; \n" +
            "  g = my-0.39173*mu-0.81290*mv; \n" +
            "  b = my+2.017*mu; \n" +
            "  gl_FragColor=vec4(r,g,b,1.0); \n" +
            "} \n";
    String nv12_fragment_shader2_0=
            "#version  130 \n" +
            "in vec2 textureOut; \n" +
            "uniform sampler2D tex_y; \n" +
            "uniform sampler2D tex_uv; \n" +
            "uniform int cx; \n" +
            "uniform int cy; \n" +
            "void main(void) \n" +
            "{ \n" +
            "  float r, g, b, my, mu, mv; \n" +
            "  my = texture2D(tex_y, textureOut).r; \n" +
            "  highp float fxDD,fyDD; \n" +
            "  int t_cx = cx; \n" +
            "  int t_cy = cy; \n" +
            "  highp float width = float(t_cx); \n" +
            "  highp float height = float(t_cy); \n" +
            "  int x = int(floor(width*textureOut.x)); \n" +
            "  int imod = int(x/2); \n" +
            "  int t_imod = imod; \n" +
            "  int i = x - (t_imod*2); \n" +
            "  if(i==0) \n" +
            "  { \n" +
            "    fxDD = textureOut.x + (1.0f/width); \n" +
            "    mu = texture2D(tex_uv, vec2(textureOut.x,textureOut.y)).r; \n" +
            "    mv = texture2D(tex_uv, vec2(fxDD,textureOut.y)).r; \n" +
            "  } \n" +
            "  else \n" +
            "  { \n" +
            "    fxDD = textureOut.x - (1.0f/width); \n" +
            "    mu = texture2D(tex_uv, vec2(fxDD,textureOut.y)).r; \n" +
            "    mv = texture2D(tex_uv, vec2(textureOut.x,textureOut.y)).r; \n" +
            "  } \n" +
            "  my = 1.1643*(my-0.0625); \n" +
            "  mu = mu - 0.5; \n" +
            "  mv = mv - 0.5; \n" +
            "  r = my+1.5958*mv; \n" +
            "  g = my-0.39173*mu-0.81290*mv; \n" +
            "  b = my+2.017*mu; \n" +
            "  gl_FragColor=vec4(r,g,b,1.0); \n" +
            "} \n";

    String rgb_fragment_shader=
            "#version  130 \n" +
            "in vec2 textureOut; \n" +
            "uniform sampler2D tex_rgb; \n" +
            "void main(void) \n" +
            "{ \n" +
            "  gl_FragColor=vec4(texture2D(tex_rgb, textureOut)); \n" +
            "} \n";
    String i420_fragment_shader=
            "#version  130 \n" +
            "in vec2 textureOut; \n" +
            "uniform sampler2D tex_y; \n" +
            "uniform sampler2D tex_u; \n" +
            "uniform sampler2D tex_v; \n" +
            "void main(void) \n" +
            "{ \n" +
            "  float r, g, b, my, mu, mv; \n" +
            "  my = texture2D(tex_y, textureOut).r; \n" +
            "  mu = texture2D(tex_u, textureOut).r; \n" +
            "  mv = texture2D(tex_v, textureOut).r; \n" +
            "  my = 1.1643*(my-0.0625); \n" +
            "  mu = mu - 0.5; \n" +
            "  mv = mv - 0.5; \n" +
            "  r = my+1.5958*mv; \n" +
            "  g = my-0.39173*mu-0.81290*mv; \n" +
            "  b = my+2.017*mu; \n" +
            "  gl_FragColor=vec4(r,g,b,1.0); \n" +
            "} \n";
//                    "  highp float widthf = float(width); \n" +
    String yuy2_fragment_shader_2=
            "#version  130 \n" +
            "in vec2 textureOut; \n" +
            "uniform sampler2D tex_yuv; \n" +
            "uniform int width; \n" +
            "void main(void) \n" +
            "{ \n" +
            "  float r, g, b, my, mu, mv; \n" +
            "  my = texture2D(tex_yuv, textureOut).r; \n" +
            "  int x = int(floor(width*textureOut.x)); \n" +
            "  highp float fxDD,fyDD; \n" +
            "  int imod = int(x/2); \n" +
            "  int t_imod = imod; \n" +
            "  int i = x - (t_imod*2); \n" +
            "  if(i==0) \n" +
            "  { \n" +
            "    fxDD = textureOut.x + (1.0f/width); \n" +
            "    mu = texture2D(tex_yuv, vec2(textureOut.x,textureOut.y)).g; \n" +
            "    mv = texture2D(tex_yuv, vec2(fxDD,textureOut.y)).g; \n" +
            "  } \n" +
            "  else \n" +
            "  { \n" +
            "    fxDD = textureOut.x - (1.0f/width); \n" +
            "    mu = texture2D(tex_yuv, vec2(fxDD,textureOut.y)).g; \n" +
            "    mv = texture2D(tex_yuv, vec2(textureOut.x,textureOut.y)).g; \n" +
            "  } \n" +
            "  my = 1.1643*(my-0.0625); \n" +
            "  mu = mu - 0.5; \n" +
            "  mv = mv - 0.5; \n" +
            "  r = my+1.5958*mv; \n" +
            "  g = my-0.39173*mu-0.81290*mv; \n" +
            "  b = my+2.017*mu; \n" +
            "  gl_FragColor=vec4(r,g,b,1.0); \n" +
            "} \n";
    String yuy2_fragment_shader=
            "#version  130 \n" +
            "in vec2 textureOut; \n" +
            "uniform sampler2D tex_yuv_rg; \n" +
            "uniform sampler2D tex_yuv_rgba; \n" +
            "void main(void) \n" +
            "{ \n" +
            "  float r, g, b, my, mu, mv; \n" +
            "  my = texture2D(tex_yuv_rg, textureOut).r; \n" +
            "  mu = texture2D(tex_yuv_rgba, textureOut).g; \n" +
            "  mv = texture2D(tex_yuv_rgba, textureOut).a; \n" +
            "  my = 1.1643*(my-0.0625); \n" +
            "  mu = mu - 0.5; \n" +
            "  mv = mv - 0.5; \n" +
            "  r = my+1.5958*mv; \n" +
            "  g = my-0.39173*mu-0.81290*mv; \n" +
            "  b = my+2.017*mu; \n" +
            "  gl_FragColor=vec4(r,g,b,1.0); \n" +
            "} \n";

    public VideoRender(){

    }
    public int bulid_program()
    {
        m_program = glCreateProgram();
        int vs = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vs,vertex_shader);
        glCompileShader(vs);
        String vsLog = glGetShaderInfoLog(vs);
        if (vsLog.trim().length() > 0) {
            System.err.println(vsLog);
            return -1;
        }
        glAttachShader(m_program, vs);

        int fs = glCreateShader(GL_FRAGMENT_SHADER);
        if((RENDER_RGB24 == m_fourcc) || (RENDER_BGR24 == m_fourcc) || (RENDER_BGRA == m_fourcc) || (RENDER_RGBA == m_fourcc) || (RENDER_ABGR == m_fourcc) || (RENDER_ARGB == m_fourcc)){
            glShaderSource(fs,rgb_fragment_shader);
        }else if(RENDER_NV12 == m_fourcc){
            glShaderSource(fs,nv12_fragment_shader);
        }else if(RENDER_NV21 == m_fourcc){
            glShaderSource(fs,nv21_fragment_shader);
        }else if((RENDER_I420 == m_fourcc) || (RENDER_YV12 == m_fourcc)){
            glShaderSource(fs,i420_fragment_shader);
        }else if(RENDER_YUY2 == m_fourcc){
            glShaderSource(fs,yuy2_fragment_shader);
        }else if(RENDER_P010 == m_fourcc){
            glShaderSource(fs,nv12_fragment_shader);
        }
        glCompileShader(fs);
        String fsLog = glGetShaderInfoLog(fs);
        if (fsLog.trim().length() > 0) {
            System.err.println(fsLog);
            return -2;
        }
        glAttachShader(m_program, fs);

        glLinkProgram(m_program);
        String programLog = glGetProgramInfoLog(m_program);
        if (programLog.trim().length() > 0) {
            System.err.println(programLog);
            return -3;
        }
        return 0;
    }

    public int GenTextures(int num){
        for(int i = 0; i < num;i++){
            m_tex[i] = glGenTextures();
            if (0 == m_tex[i]) {
                System.out.println("glGenTextures");
                return -1;
            }
            glBindTexture(GL_TEXTURE_2D, m_tex[i]);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        }

        return 0;
    }

    public int init(long fourcc, int width, int height) {
        int ret;
        m_width =width;
        m_height = height;
        m_fourcc = fourcc;
        ret = bulid_program();
        if(ret < 0){
            return ret;
        }
        if((RENDER_RGB24 == m_fourcc) || (RENDER_BGR24 == m_fourcc) ||
          (RENDER_BGRA == m_fourcc) || (RENDER_RGBA == m_fourcc) ||
          (RENDER_ABGR == m_fourcc) || (RENDER_ARGB == m_fourcc)){
            GenTextures(1);
        }else if(RENDER_YUY2 == m_fourcc){
            GenTextures(2);
            m_yuy2_buffer = glGenBuffers();
            //glBindBuffer(GL_PIXEL_UNPACK_BUFFER, m_yuy2_buffer);
            //glBufferData(GL_PIXEL_UNPACK_BUFFER, width*height*2, GL_STREAM_DRAW);
            //glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
        }else if((RENDER_NV12 == m_fourcc) || (RENDER_NV21 == m_fourcc) || (RENDER_P010 == m_fourcc)){
            GenTextures(2);
        }else if( (RENDER_I420 == m_fourcc) || (RENDER_YV12 == m_fourcc)){
            GenTextures(3);
        }
        if(RENDER_YUY2 == m_fourcc)

/*
        m_fbo = glGenFramebuffers();
        if (0 == m_fbo) {
            System.out.println("glGenFramebuffers");
            return 9;
        }
        glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
        m_rbo = glGenRenderbuffers();
        if (0 == m_rbo) {
            System.out.println("glGenRenderbuffers");
            return 10;
        }
        glBindRenderbuffer(GL_RENDERBUFFER, m_rbo);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, m_width, m_height);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, m_rbo);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);*/

        glUseProgram(m_program);
        m_verLocation = glGetAttribLocation(m_program, "vertexIn");
        m_texLocation = glGetAttribLocation(m_program, "textureIn");
        glUseProgram(0);
        return 0;
    }

    public void render(ByteBuffer data ,int display_w, int display_h) {
        //glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
        //glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(m_program);
        //glViewport(0, 0, m_width, m_height);
        //glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
        if((RENDER_RGB24 == m_fourcc) || (RENDER_BGR24 == m_fourcc) ||
                (RENDER_BGRA == m_fourcc) || (RENDER_RGBA == m_fourcc) ||
                (RENDER_ABGR == m_fourcc) || (RENDER_ARGB == m_fourcc)) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, m_tex[0]);
            if(RENDER_RGB24 == m_fourcc){
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_width, m_height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            }else if(RENDER_BGR24 == m_fourcc){
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_width, m_height, 0, GL_BGR, GL_UNSIGNED_BYTE, data);
            }else if(RENDER_BGRA == m_fourcc){
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_width, m_height, 0, GL_BGRA, GL_UNSIGNED_BYTE, data);
            }else if(RENDER_RGBA == m_fourcc){
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
            }else if(RENDER_ABGR == m_fourcc){
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_width, m_height, 0, GL_BGRA, GL_UNSIGNED_BYTE, data.position(1));//fix
            }else if(RENDER_ARGB == m_fourcc){
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data.position(1));//fix
            }

            glUniform1i(glGetUniformLocation(m_program, "tex_rgb"), 0);
        }else if(RENDER_YUY2 == m_fourcc){
            glBindBuffer(GL_PIXEL_UNPACK_BUFFER, m_yuy2_buffer);
            glBufferData(GL_PIXEL_UNPACK_BUFFER, data, GL_STREAM_DRAW);


            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, m_tex[0]);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RG, m_width, m_height, 0, GL_RG, GL_UNSIGNED_BYTE, 0);
            glUniform1i(glGetUniformLocation(m_program, "tex_yuv_rg"), 0);

            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, m_tex[1]);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_width/2, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
            glUniform1i(glGetUniformLocation(m_program, "tex_yuv_rgba"), 1);
            glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
        }else if((RENDER_NV12 == m_fourcc) || (RENDER_NV21 == m_fourcc)){
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, m_tex[0]);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, m_width, m_height, 0, GL_RED, GL_UNSIGNED_BYTE, data.position(0));
            glUniform1i(glGetUniformLocation(m_program, "tex_y"), 0);

            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, m_tex[1]);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RG, m_width/2, m_height/2, 0, GL_RG, GL_UNSIGNED_BYTE, data.position(m_width*m_height));
            glUniform1i(glGetUniformLocation(m_program, "tex_uv"), 1);

/*
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, m_tex[1]);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, m_width, m_height/2, 0, GL_RED, GL_UNSIGNED_BYTE, data.position(m_width*m_height));
            glUniform1i(glGetUniformLocation(m_program, "tex_uv"), 1);
            glUniform1i(glGetUniformLocation(m_program, "cx"), m_width);
*/
        }else if(RENDER_P010 == m_fourcc){
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, m_tex[0]);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, m_width, m_height, 0, GL_RED, GL_UNSIGNED_SHORT, data.position(0));
            glUniform1i(glGetUniformLocation(m_program, "tex_y"), 0);

            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, m_tex[1]);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RG, m_width/2, m_height/2, 0, GL_RG, GL_UNSIGNED_SHORT, data.position(m_width*m_height*2));
            glUniform1i(glGetUniformLocation(m_program, "tex_uv"), 1);

            /*
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, m_tex[1]);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, m_width, m_height/2, 0, GL_RED, GL_UNSIGNED_SHORT, data.position(m_width*m_height));
            glUniform1i(glGetUniformLocation(m_program, "tex_uv"), 1);
            glUniform1i(glGetUniformLocation(m_program, "cx"), m_width);
            */
        }else if((RENDER_I420 == m_fourcc) || (RENDER_YV12 == m_fourcc)){
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, m_tex[0]);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, m_width, m_height, 0, GL_RED, GL_UNSIGNED_BYTE, data.position(0));
            glUniform1i(glGetUniformLocation(m_program, "tex_y"), 0);

            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, m_tex[1]);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, m_width/2, m_height/2, 0, GL_RED, GL_UNSIGNED_BYTE, data.position(m_width*m_height));
            if(RENDER_I420 == m_fourcc){
                glUniform1i(glGetUniformLocation(m_program, "tex_u"), 1);
            }else{
                glUniform1i(glGetUniformLocation(m_program, "tex_v"), 1);
            }
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, m_tex[2]);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, m_width/2, m_height/2, 0, GL_RED, GL_UNSIGNED_BYTE, data.position(m_width*m_height*5/4));
            if(RENDER_I420 == m_fourcc){
                glUniform1i(glGetUniformLocation(m_program, "tex_v"), 2);
            }else{
                glUniform1i(glGetUniformLocation(m_program, "tex_u"), 2);
            }
        }

        glVertexAttribPointer(m_verLocation, 2, GL_FLOAT, false, 0,  (FloatBuffer) BufferUtils
                .createFloatBuffer(vertexVertices.length).put(vertexVertices).flip());
        glEnableVertexAttribArray(m_verLocation);
        glVertexAttribPointer(m_texLocation, 2, GL_FLOAT, false, 0, (FloatBuffer) BufferUtils
                .createFloatBuffer(textureVertices.length).put(textureVertices).flip());
        glEnableVertexAttribArray(m_texLocation);

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
       // glBindFramebuffer(GL_FRAMEBUFFER, 0);
        //glBindFramebuffer(GL_READ_FRAMEBUFFER, m_fbo);
        glViewport(0, 0, display_w, display_h);
        //glBlitFramebuffer(0, 0, m_width, m_height, 0, 0, display_w, display_h, GL_COLOR_BUFFER_BIT, GL_NEAREST);
        glUseProgram(0);

    }
}
