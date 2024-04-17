#ifndef VIDEO_CAPTURE_H
#define VIDEO_CAPTURE_H
#define CAPTURE_FOURCC_YUYV 1
#define CAPTURE_FOURCC_NV12 2

#define MAX_CAPTURE 16
#define MAX_RESOLUTION_NUM 3
#define MAX_FPS_NUM 2
#define MAX_FOURCC_NUM 2


#include <stdio.h>
int GetTickCount();
void init_capture();
int32_t get_device_list(char **pp_device_name);
int32_t get_support_resolution(int32_t device_index, char **pp_resolution);
int32_t get_support_fps(int32_t device_index, char **pp_fps);
int32_t get_support_fourcc(int32_t device_index, char **pp_fourcc);
int32_t start_capture(int32_t device_index, int32_t resolution_index, int32_t fps_index, int32_t fourcc_index);
int32_t get_render_data(uint8_t **pp_render_data);
int32_t get_encode_data(uint8_t **pp_encode_data);
int32_t get_capture_param(int32_t *p_width, int32_t *p_height, int32_t *p_fourcc, int32_t *p_fps_d, int32_t *p_fps_n);
float get_now_capture_fps();
int32_t get_now_capture_frames();
void stop_capture();
void deinit_capture();
#endif
