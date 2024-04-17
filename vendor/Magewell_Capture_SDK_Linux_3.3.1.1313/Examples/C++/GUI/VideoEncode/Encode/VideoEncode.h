#ifndef VIDEO_ENCODE_H
#define VIDEO_ENCODE_H
#include <stdio.h>
#include "mw_venc.h"
#define MAX_PLATFROM_NUM 8
int32_t get_support_platfrom(char **pp_platfrom, int width, int height, mw_venc_fourcc fourcc);

int32_t start_encode(int32_t platfrom_index, mw_venc_param_t *p_enc_param, char *p_file_name);

int32_t stop_encode();

float get_now_encode_fps();

int32_t get_now_encode_frames();

long long get_now_encode_size();

void encode_init();

void encode_deinit();

#endif
