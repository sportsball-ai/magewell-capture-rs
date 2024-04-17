#ifndef COMMON_H
#define COMMON_H
#include "pthread.h"

extern pthread_mutex_t g_p_mutex;
extern unsigned char g_buffer[1920*1080*2];

#endif // COMMON_H
