#define  GLFW_IMGUI
#include <stdint.h>

#define RENDER_RGB24 1
#define RENDER_BGR24 2
#define RENDER_RGBA  3
#define RENDER_BGRA  4
#define RENDER_ARGB  5
#define RENDER_ABGR  6

#define RENDER_NV12  101
#define RENDER_NV21  102
#define RENDER_I420  103
#define RENDER_YV12  104
#define RENDER_YUY2  105

#define RENDER_P010  201

#ifdef GLFW_IMGUI  
  #include <GL/glew.h> 
  #include <GLFW/glfw3.h>
#else  
  #include <GL/glew.h> 
  #if defined(_MSC_VER)
    //#define _AFXDLL
    //#include <afxwin.h>
  #elif defined(__GNUC__) && !defined(__arm__)	
	  #include<X11/X.h>
	  #include<X11/Xlib.h>
	  #include<GL/gl.h>
	  #include<GL/glx.h>
	  #include<GL/glu.h>
  #endif
#endif	

#define RENDER_MAX_TEX 4 
#define RENDER_MAX_BUFFERS 16

typedef enum mw_render_color_space {
	MW_RENDER_CSP_AUTO,
	MW_RENDER_CSP_BT_601,
	MW_RENDER_CSP_BT_709,
	MW_RENDER_CSP_BT_2020,
	MW_RENDER_CSP_CNT
}mw_render_color_space_t;

typedef struct mw_render_ctrl
{
    uint32_t display_w;
    uint32_t display_h;
    uint8_t  hdr_on;
    uint8_t  val_ctrl;
    uint8_t  threshold;

}mw_render_ctrl_t;

typedef struct mw_render_init
{
    uint32_t render_fourcc;
    int32_t width;
    int32_t height;
    int32_t buffers_num;
    int32_t is_in_limited;
    int32_t is_out_limited;
    mw_render_color_space_t color;
}mw_render_init_t;

class MWOpenGLRender
{
  public:
    MWOpenGLRender();
    ~MWOpenGLRender();

  public:
    int32_t open(mw_render_init_t *rinit);
    void close();
    int32_t render(uint8_t *data, mw_render_ctrl_t *rctrl, int32_t buffers_index);

    int32_t m_tex_num;
    int32_t m_tex_offset[RENDER_MAX_TEX];
    int32_t m_tex_size[RENDER_MAX_TEX];
    uint8_t *m_p_mapbuf[RENDER_MAX_BUFFERS][RENDER_MAX_TEX];

  private:
    void clean_up();
    int32_t gen_textures();
    int32_t build_program();
    int32_t build_program_hdr();
    void render_tex(int tex_index, GLenum internalformat, int32_t tex_w, int tex_h, GLenum format, GLenum tex_type, char *p_tex_name, uint8_t *p_data, GLuint glbuffer);
    void calc_rgb_trans(int is_in_limited, int is_out_limited);
    void calc_yuv_trans(int is_in_limited, int is_out_limited, mw_render_color_space_t m_csp_in);

  private:
    GLfloat					m_rgb_trans[3];

    // yuv
    float					m_csp_mat[3][3];
    float					m_csp_const[3];

    GLuint m_tex[RENDER_MAX_TEX];
    GLuint m_glbuffers[RENDER_MAX_BUFFERS][RENDER_MAX_TEX];
    uint32_t m_render_fourcc;
    int32_t m_width;
    int32_t m_height;
    GLuint m_program;
    GLuint m_program_hdr;
    GLuint m_program_sdr;
    int32_t m_buffers_num;
    GLuint m_fragment_shader;
    GLuint m_fragment_shader_hdr;
    GLuint m_vertex_shader;
    GLuint m_vertex_shader_hdr;
    int32_t m_is_init;
};
