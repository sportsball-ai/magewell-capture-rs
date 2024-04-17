#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include "VideoEncode.h"
#include "VideoCapture.h"
//static char g_support_platfrom[MAX_PLATFROM_NUM][8];
static char g_support_platfrom[MAX_PLATFROM_NUM][128];
static int g_gpu_index[MAX_PLATFROM_NUM];
static int32_t g_platfrom_num = 0;
static int32_t g_runing = 0;
static pthread_t g_encode_tid = 0;
static mw_venc_handle_t g_encode_handle = NULL;
static int32_t g_encod_flag = 0; //0 init  1enc 2 stop 3 stoped
static long long g_write_size = 0;
static int32_t g_encode_frames = 0;
int32_t get_support_platfrom(char **pp_platfrom, int width, int height, mw_venc_fourcc fourcc)
{
    int32_t i;
    int gpu_num = mw_venc_get_gpu_num();
    for(i = 0; (i < gpu_num)&&(g_platfrom_num < MAX_PLATFROM_NUM); i++){
        mw_venc_gpu_info_t info;
        if(mw_venc_get_gpu_info_by_index(i, &info)){
            continue;
        }
        memcpy(g_support_platfrom[g_platfrom_num], info.gpu_name, 128);
        if((MW_VENC_FOURCC_YUY2 == fourcc) && ((MW_VENC_PLATFORM_INTEL == info.platform) || (MW_VENC_PLATFORM_NVIDIA == info.platform))){
            printf("%s not support YUY2\n",info.gpu_name);
            continue;
        }
        g_gpu_index[g_platfrom_num] = i;
        pp_platfrom[g_platfrom_num] = g_support_platfrom[g_platfrom_num];
        g_platfrom_num++;
    }
    return g_platfrom_num;
}
void frame_callback (void * user_ptr, const uint8_t * p_frame, uint32_t frame_len, mw_venc_frame_info_t *p_frame_info)
{
    if(user_ptr){
        fwrite(p_frame, 1, frame_len, (FILE*)user_ptr);
        g_write_size += frame_len;
    }

}


int32_t start_encode(int32_t platfrom_index, mw_venc_param_t *p_enc_param, char *p_file_name)
{
    mw_venc_platform_t platform;
    FILE *fpout = fopen(p_file_name, "wb");
    if((MW_VENC_PLATFORM_NVIDIA == platform) && (MW_VENC_FOURCC_YUY2 == p_enc_param->fourcc)){
        if((g_platfrom_num > 1) && (0 == platfrom_index)){
            return start_encode(platfrom_index + 1, p_enc_param, p_file_name);
        }else{
            return -2;
        }
    }
    g_encode_handle = mw_venc_create_by_index(g_gpu_index[platfrom_index], p_enc_param, frame_callback, fpout);
    if(NULL == g_encode_handle){
        return -3;
    }
    g_encod_flag = 1;
    return 0;
}

int32_t stop_encode()
{
    g_encod_flag = 2;
    while(2 == g_encod_flag){
        usleep(1000);
    }
    g_encode_frames = 0;
    g_write_size = 0;
    return 0;
}


float get_now_encode_fps()
{
    static float fps = 0.0;
    static int prev_time = 0;
    int now_time;
    static int prev_frame_num = 0;
    int step = 100;
    if(NULL == g_encode_handle){
        fps = 0.0;
        return fps;
    }
    if(g_encode_frames < prev_frame_num){
        prev_frame_num = g_encode_frames;
    }
    if(g_encode_frames < 50){
        step = 5;
    }
    if((g_encode_frames - prev_frame_num) < step){
        return fps;
    }
    now_time = GetTickCount();
    if(prev_time == 0){
        prev_time = now_time;
        prev_frame_num = g_encode_frames;
        return fps;
    }
    if(0 == (now_time - prev_time)){
        prev_frame_num = g_encode_frames;
        return fps;
    }
    fps = (g_encode_frames - prev_frame_num) *1000.0 / (now_time - prev_time);
    prev_time = now_time;
    prev_frame_num = g_encode_frames;
    return fps;

}
int32_t get_now_encode_frames()
{
    return g_encode_frames;
}
long long get_now_encode_size()
{
    return g_write_size;
}

void *encode_thread(void *){
    uint8_t *p_enc_data;
    while(g_runing){
        if(g_encod_flag != 1){
            if(g_encod_flag == 2){
                if(g_encode_handle){
                    mw_venc_destory(g_encode_handle);
                    g_encode_handle = 0;
                }
                g_encod_flag = 0;
            }
            usleep(10000);
            continue;
        }
        if(!get_encode_data(&p_enc_data)){
            mw_venc_put_frame(g_encode_handle, p_enc_data);
            g_encode_frames++;
        }else{
            usleep(10000);
        }
    }
    return NULL;
}
void encode_init()
{
    g_runing = 1;
    //mw_venc_init();//amd crash, must call after glfwCreateWindow
    pthread_create(&g_encode_tid, NULL, encode_thread, NULL);
}

void encode_deinit()
{
    void *status;
    g_runing = 0;
    pthread_join(g_encode_tid, &status);
    mw_venc_deinit();
}


