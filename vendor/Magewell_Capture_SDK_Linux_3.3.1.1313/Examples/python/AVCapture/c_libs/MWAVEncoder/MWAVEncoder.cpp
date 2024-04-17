#include<stdio.h>
#include<stdlib.h>
#include"VoAacEnc/VoAacEnc.h"
#include"X264Enc/X264Enc.h"
#include"MWFOURCC.h"
#ifdef WITH_GPU_VENC
#include"mw_venc/mw_venc.h"
#else
typedef enum mw_venc_frame_type
{
    MW_VENC_FRAME_TYPE_UNKNOWN,							///<Unknown frame
    MW_VENC_FRAME_TYPE_IDR,								///<IDR frame
    MW_VENC_FRAME_TYPE_I,								///<I-frame
    MW_VENC_FRAME_TYPE_P,								///<P-frame
    MW_VENC_FRAME_TYPE_B,								///<B-frame
    MW_VENC_FRAME_TYPE_COUNT							///<Number of frame types enumerated
}mw_venc_frame_type_t;
typedef struct mw_venc_frame_info
{
    mw_venc_frame_type_t frame_type;					
    int delay;										
    long long pts;										
}mw_venc_frame_info_t;
typedef void(*MW_ENCODER_CALLBACK)(void * user_ptr, unsigned char * p_frame, unsigned int frame_len, mw_venc_frame_info_t *p_frame_info);
#endif
typedef  struct st_video_enc_handle { int unused; }*mw_video_encoder_handle;
typedef  struct st_audio_enc_handle { int unused; }*mw_audio_encoder_handle;
typedef struct st_mw_video_encoder{
    unsigned int is_gpu;
    unsigned int x264_max_nal;
    MW_ENCODER_CALLBACK callback;
    void * user_ptr;
    void * handle;
}mw_video_encoder_t;

#define MAX_AAC_AUDIO_BUF 8192
typedef struct st_mw_audio_encoder{
    mw_voaac_enc_t handle;
    MW_ENCODER_CALLBACK callback;
    long long ts;
    void *user_ptr;
    unsigned char aac_buf[MAX_AAC_AUDIO_BUF];
}mw_audio_encoder_t;

unsigned int get_fourcc_x264(unsigned int mwfourcc)
{
    if(MWFOURCC_I420 == mwfourcc){
        return MW_X264_ENC_FORMAT_I420;
    }
    else if(MWFOURCC_YV12 == mwfourcc){
        return MW_X264_ENC_FORMAT_YV12;
    }
    else if(MWFOURCC_NV12 == mwfourcc){
        return MW_X264_ENC_FORMAT_NV12;
    }
    else if(MWFOURCC_NV21 == mwfourcc){
        return MW_X264_ENC_FORMAT_NV21;
    }else{
        return MW_X264_ENC_FORMAT_NV12;///////////
    }
    return 0;
}
#ifdef WITH_GPU_VENC
mw_venc_fourcc_t get_fourcc_gpu(unsigned int mwfourcc)
{
    if(MWFOURCC_I420 == mwfourcc){
        return MW_VENC_FOURCC_I420;
    }
    else if(MWFOURCC_YV12 == mwfourcc){
        return MW_VENC_FOURCC_YV12;
    }
    else if(MWFOURCC_NV12 == mwfourcc){
        return MW_VENC_FOURCC_NV12;
    }
    else if(MWFOURCC_NV21 == mwfourcc){
        return MW_VENC_FOURCC_NV21;
    }
    else if(MWFOURCC_YUY2 == mwfourcc){
        return MW_VENC_FOURCC_YUY2;
    }
    else if(MWFOURCC_P010 == mwfourcc){
        return MW_VENC_FOURCC_P010;
    }
    else if(MWFOURCC_BGRA == mwfourcc){
        return MW_VENC_FOURCC_BGRA;
    }
    else if(MWFOURCC_RGBA == mwfourcc){
        return MW_VENC_FOURCC_RGBA;
    }
    else if(MWFOURCC_ARGB == mwfourcc){
        return MW_VENC_FOURCC_ARGB;
    }
    else if(MWFOURCC_ABGR == mwfourcc){
        return MW_VENC_FOURCC_ABGR;
    }
    else{
        return MW_VENC_FOURCC_UNKNOWN;
    }
    return MW_VENC_FOURCC_UNKNOWN;
}
#endif
#ifdef __cplusplus
extern "C"
{
#endif
mw_video_encoder_handle mw_video_encoder_open(int width, int height, unsigned int mwfourcc, int bitrate, int fps, int idr_interval, int is_h265, int use_gpu, MW_ENCODER_CALLBACK frame_callback, void *user_ptr)
{
    mw_video_encoder_t *p_vencoder=(mw_video_encoder_t *)malloc(sizeof(mw_video_encoder_t));
    if(NULL == p_vencoder){
        printf("malloc mw_video_encoder_t fail\n");
        return NULL;
    }
    if(is_h265 || use_gpu){//gpu
#ifdef WITH_GPU_VENC
        int32_t gpu_num = mw_venc_get_gpu_num();
        mw_venc_param_t param;
        if(0 == gpu_num){
            printf("can not find gpu, need to install gpu driver\n");
        }
        mw_venc_get_default_param(&param);
        param.code_type = is_h265?MW_VENC_CODE_TYPE_H265:MW_VENC_CODE_TYPE_H264;
        param.fourcc = get_fourcc_gpu(mwfourcc);
        param.width = width;
        param.height = height;
        param.fps.den = 1;
        param.fps.num = fps;
        param.gop_pic_size = idr_interval;
        param.rate_control.mode = MW_VENC_RATECONTROL_CBR;
        param.rate_control.target_bitrate = bitrate;
        p_vencoder->is_gpu = 1;
        for(int index = 0; index < gpu_num; index++){
            p_vencoder->handle = (void *)mw_venc_create_by_index(index, &param, frame_callback, user_ptr);
            if(p_vencoder->handle){
                break;
            }
        }

#else
        free(p_vencoder);
        printf("not define WITH_GPU_VENC, please to check Makefile,");
        return NULL;
#endif
    }else{//x264
        p_vencoder->is_gpu = 0;
        p_vencoder->callback = frame_callback;
        p_vencoder->user_ptr = user_ptr;
        p_vencoder->handle = (void *)mw_x264_encoder_create(width, height, get_fourcc_x264(mwfourcc), fps, bitrate*1000, idr_interval, &(p_vencoder->x264_max_nal));
    }
    if(NULL == p_vencoder->handle){
        free(p_vencoder);
        return NULL;
    }
    return (mw_video_encoder_handle)p_vencoder;
}

int mw_video_encode_frame(mw_video_encoder_handle handle, unsigned char *p_data, long long ts)
{
    mw_video_encoder_t *p_vencoder=(mw_video_encoder_t *)handle;
    if(NULL == p_vencoder){
        return -1;
    }
    if(p_vencoder->is_gpu){
#ifdef WITH_GPU_VENC
        if(MW_VENC_STATUS_SUCCESS != mw_venc_put_frame_ex((mw_venc_handle_t)p_vencoder->handle,p_data,ts)){
            return -1;
        }
#else
        printf("not define WITH_GPU_VENC, please to check Makefile,");
        return -1;
#endif
    }else{
        int ret;
        unsigned char is_idr;
        unsigned char *p_nal_buf[16];
        unsigned int nal_len[16];
        unsigned int nal_num;
        unsigned int frame_len = 0;
        ret = mw_x264_encode_frame((mw_x264_encoder_t)p_vencoder->handle, 0, p_data, p_nal_buf, nal_len, &nal_num, &is_idr);
        if(ret < 0){
            printf("error enc h264\n");
            return -1;
        }
        if(0 == ret){
            return 0;
        }
        if(p_vencoder->callback){
            mw_venc_frame_info_t frame_info;
            frame_info.delay = 0;
            frame_info.frame_type = is_idr?MW_VENC_FRAME_TYPE_IDR:MW_VENC_FRAME_TYPE_P;
            frame_info.pts = ts;
            for(int i = 0; i < nal_num; i++){
                frame_len += nal_len[i];
            }
            p_vencoder->callback(p_vencoder->user_ptr, p_nal_buf[0], frame_len, &frame_info);
        }
    }
    return 0;
}
int mw_video_encoder_close(mw_video_encoder_handle handle)
{
    mw_video_encoder_t *p_vencoder=(mw_video_encoder_t *)handle;
    if(NULL == p_vencoder){
        return -1;
    }
    if(p_vencoder->is_gpu){
#ifdef WITH_GPU_VENC
        if(MW_VENC_STATUS_SUCCESS != mw_venc_destory((mw_venc_handle_t)p_vencoder->handle)){
            return -1;
        }
#else
        printf("not define WITH_GPU_VENC, please to check Makefile,");
        return -1;
#endif
    }else{
        if(mw_x264_encoder_destory((mw_x264_encoder_t)p_vencoder->handle) < 0){
            return -1;
        }
    }
    free(p_vencoder);
    return 0;
}

mw_audio_encoder_handle mw_audio_encoder_open(unsigned int channels, unsigned int sample_rate, unsigned int bits_per_sample, unsigned int bit_rate, MW_ENCODER_CALLBACK frame_callback, void *user_ptr)
{
    mw_audio_encoder_t *p_aencoder = (mw_audio_encoder_t*)malloc(sizeof(mw_audio_encoder_t));
    if(NULL == p_aencoder){
        printf("malloc mw_video_encoder_t fail\n");
        return NULL;
    }
    p_aencoder->ts = -1;
    p_aencoder->callback = frame_callback;
    p_aencoder->user_ptr = user_ptr;
    p_aencoder->handle = mw_voaac_enc_create(channels, sample_rate, bits_per_sample, bit_rate*1000);
    if(NULL == p_aencoder->handle){
        printf("voaac create fail\n");
        return NULL;
    }
    return (mw_audio_encoder_handle)p_aencoder;
}

int mw_audio_encode_frame(mw_audio_encoder_handle handle, unsigned char *p_data, unsigned int data_len, long long ts)
{
    mw_audio_encoder_t *p_aencoder = (mw_audio_encoder_t*)handle;
    unsigned int aac_frame_len[128];
    int out_len;
    if(NULL == p_aencoder){
        return -1;
    }
    if(-1 == p_aencoder->ts){
        p_aencoder->ts = ts;
    }
    out_len = mw_voaac_enc_frame(p_aencoder->handle, p_data, data_len, p_aencoder->aac_buf, MAX_AAC_AUDIO_BUF, aac_frame_len);
    if(out_len < 0){
        return -1;
    }
    if(out_len == 0){
        return 0;
    }
    if(p_aencoder->callback){
        int callback_len;
        mw_venc_frame_info_t frame_info;
        int index = 0;
        frame_info.delay = 0;
        //frame_info.frame_type = 1;
        frame_info.pts = p_aencoder->ts;
        while((callback_len < out_len) && (index < 128)){
            p_aencoder->callback(p_aencoder->user_ptr, p_aencoder->aac_buf + callback_len, aac_frame_len[index], &frame_info);
            callback_len += aac_frame_len[index];
            index++;
        }
        p_aencoder->ts = -1;
    }
    return 0;
}
int mw_audio_encoder_close(mw_audio_encoder_handle handle)
{
    mw_audio_encoder_t *p_aencoder = (mw_audio_encoder_t*)handle;
    if(NULL == p_aencoder){
        return -1;
    }
    return mw_voaac_enc_destory(p_aencoder->handle);
}

#ifdef __cplusplus
}
#endif