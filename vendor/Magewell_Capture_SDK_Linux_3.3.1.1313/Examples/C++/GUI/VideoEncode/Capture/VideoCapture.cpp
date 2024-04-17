#include <stdio.h>
#include <string.h>
#include <time.h>
#include "ProductVer.h"
#include "LibMWCapture/MWCapture.h"
#include "MWFOURCC.h"
#include "VideoCapture.h"
#define VIDEO_FRAME_NUM 4
static char g_device_name[MAX_CAPTURE][80];
static int32_t g_capture_num = 0;
static uint8_t *g_frames[VIDEO_FRAME_NUM] = {NULL, NULL, NULL, NULL};
static HCHANNEL g_channel_handle = NULL;
static HANDLE g_video_handle = NULL;

int GetTickCount() 
{
    struct timespec tp;
    clock_gettime(CLOCK_MONOTONIC, &tp);
    return (tp.tv_sec * 1000 + tp.tv_nsec / 1000000);
}

void print_version()
{
    BYTE maj, min;
    WORD bulid;
    MWGetVersion(&maj, &min, &bulid);
    printf("Magewell MWCapture SDK V%d.%d.1.%d - AVCapture2\n", maj, min, bulid);

}
void list_device()
{
    
    MWCAP_CHANNEL_INFO mci;
    MWRefreshDevice();
    g_capture_num = MWGetChannelCount();
    for (int i = 0; i < g_capture_num; i++) {
        MWGetChannelInfoByIndex(i, &mci);    
        sprintf(g_device_name[i], "%d %s", i, mci.szProductName);
    }
}

void init_capture()
{
    MWCaptureInitInstance();
    print_version();
    list_device();
}
int32_t get_device_list(char **pp_device_name)
{
    int i;
    for(i = 0; i < g_capture_num; i++){
        pp_device_name[i] = g_device_name[i];
    }
    return g_capture_num;
}
/********************************************************************************************/
static char *g_resolution[MAX_RESOLUTION_NUM] = {"720x480","1920x1080","3840x2160"};
int32_t get_support_resolution(int32_t device_index, char **pp_resolution)
{
    pp_resolution[0] = g_resolution[0];
    pp_resolution[1] = g_resolution[1];
    pp_resolution[2] = g_resolution[2];
    if(strstr(g_device_name[device_index], "4K")){
        return MAX_RESOLUTION_NUM;
    }
    else{
        return 2;
    }
    
}
int32_t get_resolution(int32_t resolution_index, int32_t *p_width, int32_t *p_height)
{
    if(0 == resolution_index){
        *p_width = 720;
        *p_height = 480;
    }
    else if(1 == resolution_index){
        *p_width = 1920;
        *p_height = 1080;
    }
    else{
        *p_width = 3840;
        *p_height = 2160;
    }
    return 1;
}
/********************************************************************************************/
static char *g_fps[MAX_FPS_NUM] = {"30.00","60.00"};
int32_t get_support_fps(int32_t device_index, char **pp_fps)
{
    pp_fps[0] = g_fps[0];
    pp_fps[1] = g_fps[1];
    return MAX_FPS_NUM;
}

int32_t get_frame_duration(int32_t fps_index, int32_t *p_duration)
{
    if(0 == fps_index){
        *p_duration = 333333;
    }else{
        *p_duration = 166667;
    }
    return 1;
}
/********************************************************************************************/

static char *g_fourcc[MAX_FOURCC_NUM] = {"YUY2", "NV12"};
int32_t get_support_fourcc(int32_t device_index, char **pp_fourcc)
{

    if(strstr(g_device_name[device_index], "USB")){
        pp_fourcc[0] = g_fourcc[0];
    }else{
        pp_fourcc[0] = g_fourcc[1];
    }
    return 1;
}
int32_t get_fourcc(int32_t fourcc_index, int32_t *p_fourcc)
{
    if(strstr(g_device_name[fourcc_index], "USB")){
        *p_fourcc = MWFOURCC_YUY2;
    }else{
        *p_fourcc = MWFOURCC_NV12;
    }
    return 1;
}
/********************************************************************************************/
int32_t g_picture_size = 0;
int32_t g_capture_frame_num = 0;
int32_t g_capture_width = 1920;
int32_t g_capture_height = 1080;
int32_t g_capture_fourcc = MWFOURCC_NV12;
int32_t g_duration = 333333;

void video_frame_callback(BYTE *p_buffer, long buffer_Len, void* param)
{
    memcpy(g_frames[g_capture_frame_num % VIDEO_FRAME_NUM], p_buffer, (buffer_Len > g_picture_size)?g_picture_size:buffer_Len);
    g_capture_frame_num++;
    
}

void stop_capture()
{
    int32_t i;
    if(g_video_handle){
        MWDestoryVideoCapture(g_video_handle);
        g_video_handle = NULL;
    }
    if(g_channel_handle){
        MWCloseChannel(g_channel_handle);
        g_channel_handle = NULL;
    }
    g_capture_frame_num = 0;
}

int32_t start_capture(int32_t device_index, int32_t resolution_index, int32_t fps_index, int32_t fourcc_index)
{
    char path[256] = {0};
    int32_t i, min_stride, picture_size;
    
    if(MW_SUCCEEDED != MWGetDevicePath(device_index, path)){
        printf("%s get device path fail\n", g_device_name[device_index]);
        return -1;
    }
    
    g_channel_handle = MWOpenChannelByPath(path);
    if(NULL == g_channel_handle){
        printf("%s open channel fail\n", g_device_name[device_index]);
        return -1;
    }
    get_resolution(resolution_index, &g_capture_width, &g_capture_height);
    get_frame_duration(fps_index, &g_duration);
    get_fourcc(device_index, &g_capture_fourcc);
    min_stride = FOURCC_CalcMinStride(g_capture_fourcc, g_capture_width, 4);
    picture_size = FOURCC_CalcImageSize(g_capture_fourcc, g_capture_width, g_capture_height, min_stride);
    
    if(g_picture_size < picture_size){
        for(i = 0; i < VIDEO_FRAME_NUM; i++){
            if(g_frames[i]){
                free(g_frames[i]);
            }
            g_frames[i] = (uint8_t*)malloc(picture_size*2);//render
        }
        g_picture_size = picture_size;
    }

    g_video_handle = MWCreateVideoCapture(g_channel_handle, g_capture_width, g_capture_height, 
        g_capture_fourcc, g_duration, video_frame_callback, NULL);
    if (NULL == g_video_handle){
        printf("%s capture video fail\n", g_device_name[device_index]);
        goto end_and_free;
    }
    return 0;
end_and_free:
    stop_capture();
    return -1;
}
static int32_t g_render_index = 0;
int32_t get_render_data(uint8_t **pp_render_data)
{
    if(NULL == g_video_handle){
        return -1;
    }
    if(g_capture_frame_num <= 0){
        return 1;
    }
    if((g_capture_frame_num < g_render_index) && (g_capture_frame_num)){
        g_render_index = g_capture_frame_num -1;
    }
    if(g_capture_frame_num <= g_render_index){
        return 1;
    }
    else if((g_capture_frame_num - g_render_index) > (VIDEO_FRAME_NUM / 2)){
        g_render_index = g_capture_frame_num - 1;
    }
    *pp_render_data = g_frames[g_render_index % VIDEO_FRAME_NUM];
    g_render_index++;
    return 0;
}
static int32_t g_encode_index = 0;
int32_t get_encode_data(uint8_t **pp_encode_data)
{
    if(NULL == g_video_handle){
        return -1;
    }
    if((g_capture_frame_num < g_encode_index) && (g_capture_frame_num)){
        g_encode_index = g_capture_frame_num -1;
    }
    if(g_capture_frame_num <= g_encode_index){
        return 1;
    }
    else if((g_capture_frame_num - g_encode_index) > 2){
        g_encode_index = g_capture_frame_num - 1;
    }

    *pp_encode_data = g_frames[g_encode_index % VIDEO_FRAME_NUM];
    g_encode_index++;
    return 0;
}

int32_t get_capture_param(int32_t *p_width, int32_t *p_height, int32_t *p_fourcc, int32_t *p_fps_d, int32_t*p_fps_n)
{
    *p_width = g_capture_width;
    *p_height = g_capture_height;
    if(MWFOURCC_NV12 == g_capture_fourcc){
        *p_fourcc = CAPTURE_FOURCC_NV12;
    }else{
        *p_fourcc = CAPTURE_FOURCC_YUYV;
    }
    if(333333 == g_duration){
        *p_fps_d = 1;
        *p_fps_n = 30;
    }
    else{
        *p_fps_d = 1;
        *p_fps_n = 60;
    }
}

float get_now_capture_fps()
{
    static float fps = 0.0;
    static int prev_time = 0;
    int now_time;
    static int prev_frame_num = 0;
    int step = 100;
    if(NULL == g_channel_handle){
        fps = 0.0;
        return fps;
    }
    if(g_capture_frame_num < prev_frame_num){
        prev_frame_num = g_capture_frame_num;
    }
    if(g_capture_frame_num < 50){
        step = 5;
    }
    if((g_capture_frame_num - prev_frame_num) < step){
        return fps;
    }
    now_time = GetTickCount();
    if(prev_time == 0){
        prev_time = now_time;
        prev_frame_num = g_capture_frame_num;
        return fps;
    }
    if(0 == (now_time - prev_time)){
        prev_frame_num = g_capture_frame_num;
        return fps;
    }
    fps = (g_capture_frame_num - prev_frame_num) *1000.0 / (now_time - prev_time);
    prev_time = now_time;
    prev_frame_num = g_capture_frame_num;
    return fps;
}
int32_t get_now_capture_frames()
{
    return g_capture_frame_num;
}

void deinit_capture()
{
    int32_t i;
    stop_capture();
    MWCaptureExitInstance();
    for(i = 0; i < VIDEO_FRAME_NUM; i++){
        if(g_frames[i]){
            free(g_frames[i]);
            g_frames[i] = NULL;
        }
    }
}


