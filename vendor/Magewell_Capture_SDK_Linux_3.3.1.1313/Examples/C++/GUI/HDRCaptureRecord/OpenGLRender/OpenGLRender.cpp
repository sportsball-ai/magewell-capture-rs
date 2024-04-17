#include<stdio.h>
#include<string.h>
#include "OpenGLRender.h"
#include "OpenGLShader.h"

typedef struct mw_rgb_rng_s {
	double			m_d_min;
	double			m_d_max;
}mw_rgb_rng_t;

typedef struct mw_yuv_rng_s {
	double			m_d_ymin;
	double			m_d_ymax;
	double			m_d_cmax;
	double			m_d_cmid;
}mw_yuv_rng_t;

static const mw_rgb_rng_t g_rgb_rng_limited = {
	16.0/255.0,
	235.0/255.0
};
static const mw_rgb_rng_t g_rgb_rng_full = {
	0.0/255.0,
	255.0/255.0
};

static const mw_yuv_rng_t g_yuv_rng_limited = { 
	16.0 / 255.0, 
	235.0 / 255.0, 
	240.0 / 255.0, 
	128.0 / 255.0 
};

static const mw_yuv_rng_t g_yuv_rng_full = {
	0.0 , 
	255.0 / 255.0, 
	255.0 / 255.0, 
	128.0 / 255.0 
};

static const GLfloat vertexVertices[] = {
	-1.0f, -1.0f,
	1.0f, -1.0f,
	-1.0f,  1.0f,
	1.0f,  1.0f,
};

static const GLfloat textureVertices[] = {
    0.0f,  1.0f,
    1.0f,  1.0f,
    0.0f,  0.0f,
    1.0f,  0.0f,
};

MWOpenGLRender::MWOpenGLRender() 
{
    m_tex_num = 0;
    for(int32_t i = 0; i < RENDER_MAX_TEX; i++){
        m_tex[i] = 0;
        m_tex_offset[i] = 0;
        m_tex_size[i] = 0;
    }
    /*for(int32_t i = 0; i < RENDER_MAX_BUFFERS; i++){
        for(int j = 0; j < RENDER_MAX_TEX; j++){
            m_glbuffers[i][j] = 0;
            m_p_mapbuf[i][j] = 0;
        }
    }*/
    m_yuy2_buffer = 0;
    m_render_fourcc = 0;
    m_width = 0;
    m_height = 0;
    m_program = 0;
    m_buffers_num = 0;
    m_fragment_shader = 0;
    m_vertex_shader = 0;
    m_is_init = 0;
}

MWOpenGLRender::~MWOpenGLRender() 
{
    clean_up();
    m_is_init = 0;
}

void MWOpenGLRender::clean_up()
{
    if (m_fragment_shader){
        glDeleteShader(m_fragment_shader);
        m_fragment_shader = 0;
    }

    if (m_vertex_shader){
        glDeleteShader(m_vertex_shader);
        m_vertex_shader = 0;
    }

    if (m_program){
        glDeleteProgram(m_program);
        m_program = 0;
    }


    if (m_fragment_shader_hdr){
        glDeleteShader(m_fragment_shader_hdr);
        m_fragment_shader_hdr = 0;
    }

    if (m_vertex_shader_hdr){
        glDeleteShader(m_vertex_shader_hdr);
        m_vertex_shader_hdr = 0;
    }

    if (m_program_hdr){
        glDeleteProgram(m_program_hdr);
        m_program_hdr = 0;
    }

    /*for(int32_t i = 0; i < RENDER_MAX_BUFFERS; i++){
        for(int32_t j = 0; j < RENDER_MAX_TEX; j++){
            if(m_glbuffers[i][j]){
                glDeleteBuffers(1,&m_glbuffers[i][j]);
                m_glbuffers[i][j] = 0;
            }
            m_p_mapbuf[i][j] = 0;
        }
    }*/
    if(m_yuy2_buffer){
        glDeleteBuffers(1,&m_yuy2_buffer);
        m_yuy2_buffer = 0;
    }
    for(int32_t i = 0; i < RENDER_MAX_TEX; i++){
        glDeleteTextures(1,&m_tex[i]);
        m_tex[i] = 0;
        m_tex_offset[i] = 0;
        m_tex_size[i] = 0;
    }
} 	


void MWOpenGLRender::close()
{
    clean_up();
    m_is_init = 0;
}

int32_t MWOpenGLRender::gen_textures(){
    int32_t gen_tex_num = 0;
    //int32_t gen_buffers_num = 0;
    if((RENDER_RGB24 == m_render_fourcc) || (RENDER_BGR24 == m_render_fourcc)){
        m_tex_num = 1;
        m_tex_offset[0] = 0;
        m_tex_size[0] = m_width*m_height*3;
    }else if((RENDER_BGRA == m_render_fourcc) || (RENDER_RGBA == m_render_fourcc) ||
      (RENDER_ABGR == m_render_fourcc) || (RENDER_ARGB == m_render_fourcc)){
        m_tex_num = 1;
        m_tex_offset[0] = 0;
        m_tex_size[0] = m_width*m_height*4;
    }else if(RENDER_YUY2 == m_render_fourcc){
        m_tex_num = 1;
        m_tex_offset[0] = 0;
        m_tex_size[0] = m_width*m_height*2;
    }else if((RENDER_NV12 == m_render_fourcc) || (RENDER_NV21 == m_render_fourcc)){
        m_tex_num = 2;
        m_tex_offset[0] = 0;
        m_tex_offset[1] = m_width*m_height;
        m_tex_size[0] = m_width*m_height;
        m_tex_size[1] = m_width*m_height/2;
    }else if(RENDER_P010 == m_render_fourcc){
        m_tex_num = 2;
        m_tex_offset[0] = 0;
        m_tex_offset[1] = m_width*m_height*2;
        m_tex_size[0] = m_width*m_height*2;
        m_tex_size[1] = m_width*m_height;
    }else if( (RENDER_I420 == m_render_fourcc) || (RENDER_YV12 == m_render_fourcc)){
        m_tex_num = 3;
        m_tex_offset[0] = 0;
        m_tex_offset[1] = m_width*m_height;
        m_tex_offset[2] = m_width*m_height * 5 / 4;
        m_tex_size[0] = m_width*m_height;
        m_tex_size[1] = m_width*m_height/4;
        m_tex_size[2] = m_width*m_height/4;
    }
    gen_tex_num = m_tex_num;
    //gen_buffers_num = m_buffers_num;
    if(RENDER_YUY2 == m_render_fourcc){
        gen_tex_num = 2;
        /*if(0 == m_buffers_num){
            gen_buffers_num = 1;
        }*/
        glGenBuffers(1, &m_yuy2_buffer);
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, m_yuy2_buffer);
        glBufferData(GL_PIXEL_UNPACK_BUFFER, m_tex_size[0], 0, GL_STREAM_DRAW);
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
    }
    glGenTextures(gen_tex_num, &m_tex[0]);
    for(int32_t i = 0; i < gen_tex_num; i++){
        if (0 == m_tex[i]) {
            printf("glGenTextures\n");
            return -1;
        }
        glBindTexture(GL_TEXTURE_2D, m_tex[i]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		//glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, (height * 2), 0, GL_RED, GL_UNSIGNED_BYTE, 0);
		//glBindTexture(GL_TEXTURE_2D, 0);
    }
    /*if(!gen_buffers_num){
        return 0;
    }
    GLbitfield flags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
    for(int i = 0; i < gen_buffers_num; i++){
        for(int j = 0; j < m_tex_num; j++){
            glGenBuffers(1, &m_glbuffers[i][j]);
            if(!m_glbuffers[i][j]){
                printf("glGenBuffers\n");
                return -2;
            }
            glBindBuffer(GL_PIXEL_UNPACK_BUFFER, m_glbuffers[i][j]);
            glBufferStorage(GL_PIXEL_UNPACK_BUFFER, m_tex_size[j], 0, flags);
            m_p_mapbuf[i][j] = (uint8_t *)glMapBufferRange(GL_PIXEL_UNPACK_BUFFER, 0, m_tex_size[j], flags);
            if(!m_p_mapbuf[i][j]){
                printf("glMapBufferRange\n");
                return -3;
            }
        }
    }*/
    return 0;
}

int32_t MWOpenGLRender::build_program_hdr()
{
    GLint vertCompiled = 0, fragCompiled = 0, linked = 0;
    m_vertex_shader_hdr = glCreateShader(GL_VERTEX_SHADER);
    if (!m_vertex_shader_hdr){
        printf("GL_VERTEX_SHADER\n");
        return -1;
    }
    m_fragment_shader_hdr = glCreateShader(GL_FRAGMENT_SHADER);
    if (!m_fragment_shader_hdr){
        printf("GL_FRAGMENT_SHADER\n");
        return -2;
    }

    glShaderSource(m_vertex_shader_hdr, 1, (const GLchar **)&g_vertex_shader_str, NULL);

    glShaderSource(m_fragment_shader_hdr, 1, (const GLchar **)&g_p010_fragment_shader_str, NULL);

    glCompileShader(m_vertex_shader_hdr);
    glGetShaderiv(m_vertex_shader_hdr, GL_COMPILE_STATUS, &vertCompiled);
    if (!vertCompiled){
        printf("vertCompiled %d\n", vertCompiled);
        return -3;
    }
    glCompileShader(m_fragment_shader_hdr);
    glGetShaderiv(m_fragment_shader_hdr, GL_COMPILE_STATUS, &fragCompiled);
    if (!fragCompiled){
        printf("fragCompiled %d\n", fragCompiled);
        return -4;
    }

    m_program_hdr = glCreateProgram();
    if (!m_program_hdr){
        printf("glCreateProgram\n");
        return -5;
    }
    glAttachShader(m_program_hdr, m_vertex_shader_hdr);
    glAttachShader(m_program_hdr, m_fragment_shader_hdr);
    glLinkProgram(m_program_hdr);
    glGetProgramiv(m_program_hdr, GL_LINK_STATUS, &linked);
    if (!linked){
        printf("linked %d\n", linked);
        return -6;
    }printf("0000000\n");
    return 0;
}

int32_t MWOpenGLRender::build_program()
{
    GLint vertCompiled = 0, fragCompiled = 0, linked = 0;
    m_vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    if (!m_vertex_shader){
        printf("GL_VERTEX_SHADER\n");
        return -1;
    }
    m_fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    if (!m_fragment_shader){
        printf("GL_FRAGMENT_SHADER\n");
        return -2;
    }

    glShaderSource(m_vertex_shader, 1, (const GLchar **)&g_vertex_shader_str, NULL);

    if((RENDER_RGB24 == m_render_fourcc) || (RENDER_RGBA == m_render_fourcc)){
        glShaderSource(m_fragment_shader, 1, (const GLchar **)&g_rgb_fragment_shader_str, NULL);
    }else if((RENDER_BGR24 == m_render_fourcc) || (RENDER_BGRA == m_render_fourcc)){
        glShaderSource(m_fragment_shader, 1, (const GLchar **)&g_bgr_fragment_shader_str, NULL);
    }else if(RENDER_ABGR == m_render_fourcc){
        glShaderSource(m_fragment_shader, 1, (const GLchar **)&g_abgr_fragment_shader_str, NULL);
    }else if(RENDER_ARGB == m_render_fourcc){
        glShaderSource(m_fragment_shader, 1, (const GLchar **)&g_argb_fragment_shader_str, NULL);
    }else if(RENDER_YUY2 == m_render_fourcc){
        glShaderSource(m_fragment_shader, 1, (const GLchar **)&g_yuy2_fragment_shader_str, NULL);
    }else if(RENDER_NV12 == m_render_fourcc){
        glShaderSource(m_fragment_shader, 1, (const GLchar **)&g_nv12_fragment_shader_str, NULL);
    }else if(RENDER_NV21 == m_render_fourcc){
        glShaderSource(m_fragment_shader, 1, (const GLchar **)&g_nv21_fragment_shader_str, NULL);
    }else if(RENDER_P010 == m_render_fourcc){
	    glShaderSource(m_fragment_shader, 1, (const GLchar **)&g_nv12_fragment_shader_str, NULL);
        build_program_hdr();
    }else if(RENDER_I420 == m_render_fourcc){
        glShaderSource(m_fragment_shader, 1, (const GLchar **)&g_i420_fragment_shader_str, NULL);
    }else if(RENDER_YV12 == m_render_fourcc){
        glShaderSource(m_fragment_shader, 1, (const GLchar **)&g_yv12_fragment_shader_str, NULL);
    }

    glCompileShader(m_vertex_shader);
    glGetShaderiv(m_vertex_shader, GL_COMPILE_STATUS, &vertCompiled);
    if (!vertCompiled){
        printf("vertCompiled %d\n", vertCompiled);
        return -3;
    }
    glCompileShader(m_fragment_shader);
    glGetShaderiv(m_fragment_shader, GL_COMPILE_STATUS, &fragCompiled);
    if (!fragCompiled){
        printf("fragCompiled %d\n", fragCompiled);
        return -4;
    }

    m_program_sdr = glCreateProgram();
    if (!m_program_sdr){
        printf("glCreateProgram\n");
        return -5;
    }
    glAttachShader(m_program_sdr, m_vertex_shader);
    glAttachShader(m_program_sdr, m_fragment_shader);
    glLinkProgram(m_program_sdr);
    glGetProgramiv(m_program_sdr, GL_LINK_STATUS, &linked);
    if (!linked){
        printf("linked %d\n", linked);
        return -6;
    }
    return 0;
}

void 
MWOpenGLRender::calc_rgb_trans(int is_in_limited, int is_out_limited)
{
	int32_t in_min = 0;
	int32_t in_max = 255;
	int32_t out_min = 0;
	int32_t out_max = 255;
	if (is_in_limited) {//m_render_init.m_clr_rng_in == MW_CLR_RNG_LIMITED
		in_min = 16;
		in_max = 255;
	}
	if (is_out_limited) {//m_render_init.m_clr_rng_out == MW_CLR_RNG_LIMITED
		out_min = 16;
		out_max = 255;
	}
	m_rgb_trans[0] = in_min*1.0 / (in_max - in_min);
	m_rgb_trans[1] = (out_max - out_min)*1.0 / (in_max - in_min);
	m_rgb_trans[2] = out_min*1.0 / 255;
}

void 
MWOpenGLRender::calc_yuv_trans(int is_in_limited, int is_out_limited, mw_render_color_space_t m_csp_in)
{
	memset(m_csp_mat, 0, sizeof(float) * 9);
	memset(m_csp_const, 0, sizeof(float) * 3);

	float f_r = 0.0;
	float f_g = 0.0;
	float f_b = 0.0;

	switch (m_csp_in)
	{
	case MW_RENDER_CSP_BT_601:
		f_r = 0.299;
		f_g = 0.587;
		f_b = 0.114;
		break;
	case MW_RENDER_CSP_BT_709:
	case MW_RENDER_CSP_AUTO:
		f_r = 0.2126;
		f_g = 0.7152;
		f_b = 0.0722;
		break;
	case MW_RENDER_CSP_BT_2020:
		f_r = 0.2627;
		f_g = 0.6780;
		f_b = 0.0593;
		break;
	default:
		f_r = 0.2126;
		f_g = 0.7152;
		f_b = 0.0722;
		break;
	}

	m_csp_mat[0][0] = 1;
	m_csp_mat[0][1] = 0;
	m_csp_mat[0][2] = 2*(1-f_r);
	m_csp_mat[1][0] = 1;
	m_csp_mat[1][1] = -2 * (1 - f_b)*f_b / f_g;
	m_csp_mat[1][2] = -2 * (1 - f_r)*f_r / f_g;
	m_csp_mat[2][0] = 1;
	m_csp_mat[2][1] = 2 * (1 - f_b);
	m_csp_mat[2][2] = 0;

	mw_yuv_rng_t	yuv_rng = { 0.0 };
	mw_rgb_rng_t	rgb_rng = { 0.0 };

	if (is_in_limited)
		yuv_rng = g_yuv_rng_limited;
	else
		yuv_rng = g_yuv_rng_full;

	if (is_out_limited)
		rgb_rng = g_rgb_rng_limited;
	else
		rgb_rng = g_rgb_rng_full;

	double d_ymul = 0.0;
	double d_cmul = 0.0;
	d_ymul = (rgb_rng.m_d_max - rgb_rng.m_d_min) / (yuv_rng.m_d_ymax - yuv_rng.m_d_ymin);
	d_cmul = (rgb_rng.m_d_max - rgb_rng.m_d_min) / (yuv_rng.m_d_cmax - yuv_rng.m_d_cmid) / 2.0;

	for (int i = 0;i < 3;i++) {
		m_csp_mat[i][0] *= d_ymul;
		m_csp_mat[i][1] *= d_cmul;
		m_csp_mat[i][2] *= d_cmul;
		m_csp_const[i] = rgb_rng.m_d_min - m_csp_mat[i][0] * yuv_rng.m_d_ymin
			- (m_csp_mat[i][1] + m_csp_mat[i][2])*yuv_rng.m_d_cmid;
	}
}

int32_t MWOpenGLRender::open(mw_render_init_t *rinit)
{	    
    m_render_fourcc = rinit->render_fourcc;
    m_width = rinit->width;
    m_height = rinit->height;
    /*m_buffers_num = rinit->buffers_num;
    if(rinit->buffers_num > RENDER_MAX_BUFFERS){
        rinit->buffers_num = RENDER_MAX_BUFFERS;
        m_buffers_num = RENDER_MAX_BUFFERS;
    }*/
    calc_rgb_trans(rinit->is_in_limited, rinit->is_out_limited);
    calc_yuv_trans(rinit->is_in_limited, rinit->is_out_limited, rinit->color);
    if(gen_textures() < 0){
        clean_up();
        return -1;
    }

    if(build_program() < 0){
        clean_up();
        return -2;
    }
    GLuint verLocation = glGetAttribLocation(m_program_sdr, "vertexIn");
    GLuint texLocation = glGetAttribLocation(m_program_sdr, "textureIn");
    glVertexAttribPointer(verLocation, 2, GL_FLOAT, 0, 0, vertexVertices);
    glEnableVertexAttribArray(verLocation);

    glVertexAttribPointer(texLocation, 2, GL_FLOAT, 0, 0, textureVertices);
    glEnableVertexAttribArray(texLocation);

    if(m_program_hdr){
        GLuint verLocation = glGetAttribLocation(m_program_hdr, "vertexIn");
        GLuint texLocation = glGetAttribLocation(m_program_hdr, "textureIn");
        glVertexAttribPointer(verLocation, 2, GL_FLOAT, 0, 0, vertexVertices);
        glEnableVertexAttribArray(verLocation);

        glVertexAttribPointer(texLocation, 2, GL_FLOAT, 0, 0, textureVertices);
        glEnableVertexAttribArray(texLocation);
    }


    m_is_init = 1;
    glClearColor(0.9, 0.0, 0.0, 1.0);
    return 0;		 	    
}

void MWOpenGLRender::render_tex(int tex_index, GLenum internalformat, int32_t tex_w, int tex_h, GLenum format, GLenum tex_type, char *p_tex_name, uint8_t *p_data/*, GLuint glbuffer*/)
{
    /*if(glbuffer){
        glActiveTexture(GL_TEXTURE0 + tex_index);
        glBindTexture(GL_TEXTURE_2D, m_tex[tex_index]);
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, glbuffer);
        glTexImage2D(GL_TEXTURE_2D, 0, internalformat, tex_w, tex_h, 0, format, tex_type, 0);
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
        glUniform1i(glGetUniformLocation(m_program, p_tex_name), tex_index);
    }else if(p_data){*/
        glActiveTexture(GL_TEXTURE0 + tex_index);
        glBindTexture(GL_TEXTURE_2D, m_tex[tex_index]);
        glTexImage2D(GL_TEXTURE_2D, 0, internalformat, tex_w, tex_h, 0, format, tex_type, p_data?(p_data + m_tex_offset[tex_index]):0);
        glUniform1i(glGetUniformLocation(m_program, p_tex_name), tex_index);
    //}
}

int32_t MWOpenGLRender::render(uint8_t *p_data, mw_render_ctrl_t *rctrl)//, int32_t buffers_index
{
    int32_t set_rgb = 0;
    int32_t set_csp = 0;
    /*if(m_p_mapbuf[0][0] && p_data && ((buffers_index < 0) || (buffers_index > m_buffers_num))){
        for(int i = 0; i< m_tex_num;i++){
            memcpy(m_p_mapbuf[0][i], p_data+m_tex_offset[i], m_tex_size[i]);
        }
        buffers_index = 0;
        p_data = NULL;
    }
    if((buffers_index < 0) || (buffers_index > m_buffers_num)){
        buffers_index = 0;
    }*/
    if(rctrl->hdr_on && m_program_hdr){
        m_program = m_program_hdr;
    }else{
        m_program = m_program_sdr;
    }
    glUseProgram(m_program);
    if((RENDER_RGB24 == m_render_fourcc) || (RENDER_BGR24 == m_render_fourcc)){
        render_tex(0, GL_RGB, m_width, m_height, GL_RGB, GL_UNSIGNED_BYTE, "tex_rgb", p_data);//, m_glbuffers[buffers_index][0]
        set_rgb = 1;
    }else if((RENDER_BGRA == m_render_fourcc) || (RENDER_RGBA == m_render_fourcc) ||
      (RENDER_ABGR == m_render_fourcc) || (RENDER_ARGB == m_render_fourcc)){
        render_tex(0, GL_BGRA, m_width, m_height, GL_BGRA, GL_UNSIGNED_BYTE, "tex_rgb", p_data);//, m_glbuffers[buffers_index][0]
        set_rgb = 1;
    }else if(RENDER_YUY2 == m_render_fourcc){
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, m_yuy2_buffer);
        uint8_t *p_map_data = (uint8_t *)glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY);
        if(p_map_data){
            memcpy(p_map_data, p_data, m_tex_size[0]);
            glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
        }
        render_tex(0, GL_RG, m_width, m_height, GL_RG, GL_UNSIGNED_BYTE, "tex_yuv_rg", 0);//, m_glbuffers[buffers_index][0]
        render_tex(1, GL_RGBA, m_width/2, m_height, GL_RGBA, GL_UNSIGNED_BYTE, "tex_yuv_rgba", 0);//, m_glbuffers[buffers_index][0]
        //}
        set_csp = 1;
    }else if((RENDER_NV12 == m_render_fourcc) || (RENDER_NV21 == m_render_fourcc)){
        render_tex(0, GL_RED, m_width, m_height, GL_RED, GL_UNSIGNED_BYTE, "tex_y", p_data);//, m_glbuffers[buffers_index][0]
        render_tex(1, GL_RG, m_width/2, m_height/2, GL_RG, GL_UNSIGNED_BYTE, "tex_uv", p_data);//, m_glbuffers[buffers_index][1]
        set_csp = 1;
    }else if(RENDER_P010 == m_render_fourcc){
        render_tex(0, GL_RED, m_width, m_height, GL_RED, GL_UNSIGNED_SHORT, "tex_y", p_data);//, m_glbuffers[buffers_index][0]
        render_tex(1, GL_RG, m_width/2, m_height/2, GL_RG, GL_UNSIGNED_SHORT, "tex_uv", p_data);//, m_glbuffers[buffers_index][1]
        glUniform1i(glGetUniformLocation(m_program, "threshold"), rctrl->threshold);
        glUniform1i(glGetUniformLocation(m_program, "val_ctrl"), rctrl->val_ctrl);
        set_csp = 1;
    }else if((RENDER_I420 == m_render_fourcc) || (RENDER_YV12 == m_render_fourcc)){
        render_tex(0, GL_RED, m_width, m_height, GL_RED, GL_UNSIGNED_BYTE, "tex_y", p_data);//, m_glbuffers[buffers_index][0]
        render_tex(1, GL_RED, m_width/2, m_height/2, GL_RED, GL_UNSIGNED_BYTE, "tex_u", p_data);//, m_glbuffers[buffers_index][1]
        render_tex(2, GL_RED, m_width/2, m_height/2, GL_RED, GL_UNSIGNED_BYTE, "tex_v", p_data);//, m_glbuffers[buffers_index][2]
        set_csp = 1;
    }

    if(set_rgb){
        glUniform3f(glGetUniformLocation(m_program, "rgb_rng"),
		m_rgb_trans[0],
		m_rgb_trans[1],
		m_rgb_trans[2]);
    }
    if(set_csp){
        glUniformMatrix3fv(glGetUniformLocation(m_program, "cspmat"), 1, GL_FALSE, &m_csp_mat[0][0]);
        glUniform3f(glGetUniformLocation(m_program, "cspconst"), m_csp_const[0], m_csp_const[1], m_csp_const[2]);
    }

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glViewport(0, 0, rctrl->display_w, rctrl->display_h);
    glUseProgram(0);
    return 0;	
}





