#ifndef X264_ENC_H
#define X264_ENC_H
#include <stdio.h>

typedef  struct st_x264_enc_handle { int unused; }*mw_x264_encoder_t;


typedef enum en_x264_enc_format
{
	MW_X264_ENC_FORMAT_I420 = 1,
	MW_X264_ENC_FORMAT_YV12 = 2,
    MW_X264_ENC_FORMAT_NV12 = 3, //          0x0003  /* yuv 4:2:0, with one y plane and one packed u+v */
    MW_X264_ENC_FORMAT_NV21 = 4,//           0x0004  /* yuv 4:2:0, with one y plane and one packed v+u */
    MW_X264_ENC_FORMAT_I422 = 5
}mw_x264_enc_format_t;
#ifdef __cplusplus
extern "C" {
#endif

mw_x264_encoder_t mw_x264_encoder_create(int width, int height, unsigned int in_pix_format, int frame_rate, int bit_rate, int key_frame_interval, unsigned int *p_max_nal_num);

int mw_x264_encode_frame(mw_x264_encoder_t handle,unsigned char need_key, unsigned char *p_inframe, unsigned char *p_nal_buf[], unsigned int nal_len[], unsigned int *p_nal_num, unsigned char *p_out_frame_type);

int mw_x264_encoder_reset(mw_x264_encoder_t handle, int bit_rate, int key_frame_interval);

int mw_x264_encoder_destory(mw_x264_encoder_t handle);

#ifdef __cplusplus
}
#endif
#endif
