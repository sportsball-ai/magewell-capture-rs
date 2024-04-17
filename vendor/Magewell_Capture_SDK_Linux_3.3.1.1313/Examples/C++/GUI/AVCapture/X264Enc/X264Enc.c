#include<stdio.h>
#include <stdlib.h>
#include <string.h>
#include "X264Enc.h"
#ifdef WIN32
#include <windows.h>
#define inline __inline
#endif

#ifdef __cplusplus
extern "C"{
#endif
#include "stdint.h"
#include "x264_config.h"
#include "x264.h"
#ifdef __cplusplus
}
#endif

#ifdef WIN32
#pragma comment(lib,"libx264.dll.a")
#endif

typedef struct st_x264_encoder{
    void *check_cover;

    x264_param_t parm;
    x264_nal_t *nal_info;
    x264_picture_t pic_out;
    x264_picture_t pic_in;
    x264_t *x264_handle;

    int key_frame_interval;
    int bit_rate;
    int frame_count;
    unsigned int in_pix_format;
    unsigned int max_nal;
    int need_reset;
}x264_encoder_t;

x264_param_t g_parm = {0};
unsigned char g_parm_init_flag = 0;

int x264_get_csp(unsigned int in_pix_format){

    switch(in_pix_format){
        case MW_X264_ENC_FORMAT_I420:
            return X264_CSP_I420;
        case MW_X264_ENC_FORMAT_YV12:
            return X264_CSP_YV12;
        case MW_X264_ENC_FORMAT_NV12:
            return X264_CSP_NV12;
        case MW_X264_ENC_FORMAT_NV21:
            return X264_CSP_NV21;
        default:
            printf("not support in_pix_format[%u]", in_pix_format);
            return 0;
    }
    return 0;
}

int x264_set_img_info(unsigned int in_pix_format, int width, x264_image_t *p_img){

    switch(in_pix_format){
        case MW_X264_ENC_FORMAT_I420:
            p_img->i_plane = 3; 
            p_img->i_stride[0] = width;
            p_img->i_stride[1] = width/2;
            p_img->i_stride[2] = width/2;
            break;
        case MW_X264_ENC_FORMAT_YV12:
            p_img->i_plane = 3; 
            p_img->i_stride[0] = width;
            p_img->i_stride[1] = width/2;
            p_img->i_stride[2] = width/2;
            break;
        case MW_X264_ENC_FORMAT_NV12:
            p_img->i_plane = 2; 
            p_img->i_stride[0] = width;
            p_img->i_stride[1] = width;
            break;
        case MW_X264_ENC_FORMAT_NV21:
            p_img->i_plane = 2; 
            p_img->i_stride[0] = width;
            p_img->i_stride[1] = width;
            break;
        default:
            printf("not support in_pix_format[%u]", in_pix_format);
            return -1;
    }

    p_img->i_csp = x264_get_csp(in_pix_format); 
    
    return 1;
}

mw_x264_encoder_t mw_x264_encoder_create(int width, int height, unsigned int in_pix_format, int frame_rate, int bit_rate, int key_frame_interval, unsigned int *p_max_nal_num)
{
    x264_encoder_t *p_x264_encoder = (x264_encoder_t*)malloc(sizeof(x264_encoder_t));
    if(p_x264_encoder == NULL){
        printf("x264encoder malloc fail");
        return NULL;
    }
    memset(p_x264_encoder, 0, sizeof(x264_encoder_t));
    p_x264_encoder->check_cover = p_x264_encoder;

    x264_picture_init(&(p_x264_encoder->pic_in));
    x264_picture_init(&(p_x264_encoder->pic_out));
    if(x264_set_img_info(in_pix_format, width, &(p_x264_encoder->pic_in.img)) < 0){
        printf("x264encoder[%p] x264_set_img_info failed",p_x264_encoder);
        free(p_x264_encoder);
        return NULL;
    }
    p_x264_encoder->pic_in.i_type = X264_TYPE_AUTO;

    //x264_param_default(&(p_x264_encoder->parm));
    if(g_parm_init_flag == 0){
        x264_param_default_preset(&g_parm, "veryfast", "zerolatency");
        g_parm_init_flag = 1;
    }
    memcpy(&(p_x264_encoder->parm), &g_parm, sizeof(g_parm));
    p_x264_encoder->parm.i_threads = 2;
    p_x264_encoder->parm.i_width = width;
    p_x264_encoder->parm.i_height = height;
    p_x264_encoder->parm.b_cabac = 0;
    p_x264_encoder->parm.i_bframe = 0;
    p_x264_encoder->parm.b_interlaced = 0;
     //p_x264_encoder->parm.i_slice_count = 0;
    p_x264_encoder->parm.rc.i_rc_method = X264_RC_ABR;
    p_x264_encoder->parm.i_level_idc = 41;
    p_x264_encoder->parm.rc.i_bitrate = bit_rate / 1000;
    p_x264_encoder->parm.rc.i_vbv_max_bitrate = p_x264_encoder->parm.rc.i_bitrate;
    p_x264_encoder->parm.rc.i_vbv_buffer_size = p_x264_encoder->parm.rc.i_bitrate;

    p_x264_encoder->parm.b_intra_refresh = 1;
    p_x264_encoder->parm.b_annexb = 1;
    p_x264_encoder->parm.i_keyint_min = 50;
    p_x264_encoder->parm.i_keyint_max = 255;
    p_x264_encoder->parm.i_fps_num = frame_rate;
    p_x264_encoder->parm.i_csp = x264_get_csp(in_pix_format); 
    p_x264_encoder->parm.i_fps_den = 1;
    p_x264_encoder->parm.b_annexb = 1;
    p_x264_encoder->parm.b_sliced_threads = 0;

    x264_param_apply_profile(&(p_x264_encoder->parm), "baseline");
    if ((p_x264_encoder->x264_handle = x264_encoder_open(&(p_x264_encoder->parm))) == NULL)
    {
        printf("x264encoder[%p] x264_encoder_open failed",p_x264_encoder);
        free(p_x264_encoder);
        return NULL;
    }

    p_x264_encoder->key_frame_interval = key_frame_interval;
    p_x264_encoder->bit_rate = bit_rate;
    p_x264_encoder->frame_count = 0;
    p_x264_encoder->in_pix_format = in_pix_format;
    p_x264_encoder->need_reset = 0;
    p_x264_encoder->max_nal = 8;
    if(p_max_nal_num)
        *p_max_nal_num = p_x264_encoder->max_nal;
    printf("x264encoder[%p] create ok, width[%d], height[%d], in_pix_format[%d], bit_rate[%d], key_frame_interval[%d]", p_x264_encoder, width, height, in_pix_format, bit_rate, key_frame_interval);
    return (mw_x264_encoder_t)p_x264_encoder;
}
int x264_set_yuv_pic(x264_encoder_t *p_x264_encoder, unsigned char *p_inframe){

    switch(p_x264_encoder->in_pix_format){
        case MW_X264_ENC_FORMAT_I420:
            p_x264_encoder->pic_in.img.plane[0] = p_inframe;
            p_x264_encoder->pic_in.img.plane[1] = p_inframe + p_x264_encoder->parm.i_width * p_x264_encoder->parm.i_height;
            p_x264_encoder->pic_in.img.plane[2] = p_inframe + p_x264_encoder->parm.i_width * p_x264_encoder->parm.i_height * 5 / 4;
            break;
        case MW_X264_ENC_FORMAT_YV12:
            p_x264_encoder->pic_in.img.plane[0] = p_inframe;
            p_x264_encoder->pic_in.img.plane[1] = p_inframe + p_x264_encoder->parm.i_width * p_x264_encoder->parm.i_height;
            p_x264_encoder->pic_in.img.plane[2] = p_inframe + p_x264_encoder->parm.i_width * p_x264_encoder->parm.i_height * 5 / 4;
            break;
        case MW_X264_ENC_FORMAT_NV12:
            p_x264_encoder->pic_in.img.plane[0] = p_inframe;
            p_x264_encoder->pic_in.img.plane[1] = p_inframe + p_x264_encoder->parm.i_width * p_x264_encoder->parm.i_height;
            break;
        case MW_X264_ENC_FORMAT_NV21:
            p_x264_encoder->pic_in.img.plane[0] = p_inframe;
            p_x264_encoder->pic_in.img.plane[1] = p_inframe + p_x264_encoder->parm.i_width * p_x264_encoder->parm.i_height;
            break;
        default:
            printf("x264encoder[%p], not support in_pix_format[%u]", p_x264_encoder, p_x264_encoder->in_pix_format);
            return -1;
    }

    return 1;
}

int x264_process_reset(x264_encoder_t *p_x264_encoder)
{
    int err;
    x264_encoder_parameters(p_x264_encoder->x264_handle, &(p_x264_encoder->parm));
    p_x264_encoder->parm.rc.i_bitrate = p_x264_encoder->bit_rate / 1000;
    p_x264_encoder->parm.rc.i_rc_method = X264_RC_ABR;
    p_x264_encoder->parm.rc.i_vbv_max_bitrate = p_x264_encoder->parm.rc.i_bitrate;
    p_x264_encoder->parm.rc.i_vbv_buffer_size = p_x264_encoder->parm.rc.i_bitrate;
    err = x264_encoder_reconfig(p_x264_encoder->x264_handle, &(p_x264_encoder->parm));
    if (err){
        printf("x264encoder[%p], reset error err[%d]", p_x264_encoder, err);
        return -1;
    }else{
        printf("x264encoder[%p], reset success", p_x264_encoder);
    }
    return 1;
}

int mw_x264_encode_frame(mw_x264_encoder_t handle,unsigned char need_key, unsigned char *p_inframe, unsigned char *p_nal_buf[], unsigned int nal_len[], unsigned int *p_nal_num, unsigned char *p_out_frame_type)
{
    int i;
    int nal_num;
    int temp_size = 0;
    x264_encoder_t *p_x264_encoder = (x264_encoder_t*)handle;
    if ((p_x264_encoder == NULL) || (p_x264_encoder->check_cover != p_x264_encoder)){
        printf("x264encoder[%p] check", p_x264_encoder);
        return -1;
    }
    if(p_x264_encoder->need_reset){
        x264_process_reset(p_x264_encoder);
        p_x264_encoder->need_reset = 0;
    }
    if(need_key || p_x264_encoder->frame_count++ >= p_x264_encoder->key_frame_interval){
        p_x264_encoder->frame_count = 0;
        p_x264_encoder->pic_in.i_type = X264_TYPE_IDR;
    }else{
        p_x264_encoder->pic_in.i_type = X264_TYPE_AUTO;
    }
    if(x264_set_yuv_pic(p_x264_encoder, p_inframe) < 0){
        return -2;
    }
    temp_size = x264_encoder_encode(p_x264_encoder->x264_handle, &(p_x264_encoder->nal_info), &nal_num, &(p_x264_encoder->pic_in), &(p_x264_encoder->pic_out)); 
    if((temp_size <= 0) || (nal_num <= 0) || (nal_num > p_x264_encoder->max_nal)){
        printf("x264encoder[%p] not out frame, return[%d], nal_num[%d]", p_x264_encoder, temp_size, nal_num);
        return temp_size;
    }
    if(p_nal_num)
        *p_nal_num = nal_num;
	if(p_out_frame_type){
		*p_out_frame_type = 0;
	}
    for(i = 0; i < nal_num; i++){
        p_nal_buf[i] = p_x264_encoder->nal_info[i].p_payload;
        nal_len[i] = p_x264_encoder->nal_info[i].i_payload;
		if(nal_len[i] < 5){
			continue;
		}
		if(((p_nal_buf[i][3] == 1) && ((p_nal_buf[i][4] & 0x1f) == 5)) || ((p_nal_buf[i][2] == 1) && ((p_nal_buf[i][3] & 0x1f) == 5))){
			if(p_out_frame_type){
				*p_out_frame_type = 1;
			}
		}
    }
    return temp_size;
}


int mw_x264_encoder_reset(mw_x264_encoder_t handle, int bit_rate, int key_frame_interval)
{
    x264_encoder_t *p_x264_encoder = (x264_encoder_t*)handle;
    if ((p_x264_encoder == NULL) || (p_x264_encoder->check_cover != p_x264_encoder)){
        printf("x264encoder[%p] check", p_x264_encoder);
        return -1;
    }
    if(p_x264_encoder->need_reset){
        return 0;
    }
    p_x264_encoder->bit_rate = bit_rate;
    p_x264_encoder->key_frame_interval = key_frame_interval;
    p_x264_encoder->need_reset = 1;
    return 1;
}

int mw_x264_encoder_destory(mw_x264_encoder_t handle)
{
    x264_encoder_t *p_x264_encoder = (x264_encoder_t*)handle;
    if ((p_x264_encoder == NULL) || (p_x264_encoder->check_cover != p_x264_encoder)){
        printf("x264encoder[%p] check", p_x264_encoder);
        return -1;
    }
    p_x264_encoder->check_cover = 0;
    if(p_x264_encoder->x264_handle){
        x264_encoder_close(p_x264_encoder->x264_handle);
    }
    p_x264_encoder->x264_handle = NULL;

    free(p_x264_encoder);
    return 1;
}
