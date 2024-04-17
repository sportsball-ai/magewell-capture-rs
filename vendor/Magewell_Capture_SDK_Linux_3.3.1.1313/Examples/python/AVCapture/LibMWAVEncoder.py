import ctypes
from ctypes import *
AVEncoder = CDLL('./c_libs/libMWAVEncoderPy.so')
'''
typedef struct mw_venc_frame_info
{
    mw_venc_frame_type_t frame_type;					
    int delay;										
    long long pts;										
}mw_venc_frame_info_t;
'''

class mw_venc_frame_info_t(Structure):
    _pack_ = 1
    _fields_ = [('frame_type',c_uint32),
               ('delay',c_int),
               ('pts',c_int64)]


'''
typedef void(*MW_ENCODER_CALLBACK)(void * user_ptr, unsigned char * p_frame, unsigned int frame_len, mw_venc_frame_info_t *p_frame_info);
mw_video_encoder_handle mw_video_encoder_open(int width, int height, unsigned int mwfourcc, int bitrate, int fps, 
    int idr_interval, int is_h265, int use_gpu, MW_ENCODER_CALLBACK frame_callback, void *user_ptr);
int mw_video_encode_frame(mw_video_encoder_handle handle, unsigned char *p_data, long long ts);
int mw_video_encoder_close(mw_video_encoder_handle handle)

mw_audio_encoder_handle mw_audio_encoder_open(unsigned int channels, unsigned int sample_rate, unsigned int bits_per_sample,
    unsigned int bit_rate, MW_ENCODER_CALLBACK frame_callback, void *user_ptr);
int mw_audio_encode_frame(mw_audio_encoder_handle handle, unsigned char *p_data, unsigned int data_len, long long ts);
int mw_audio_encoder_close(mw_audio_encoder_handle handle)
'''

MW_ENCODER_CALLBACK = CFUNCTYPE(None,py_object, c_void_p, c_uint32, POINTER(mw_venc_frame_info_t))

AVEncoder.mw_video_encoder_open.restype = c_void_p
AVEncoder.mw_video_encoder_open.argtypes=[c_int, c_int, c_uint, c_int, c_int, c_int, c_int, c_int, MW_ENCODER_CALLBACK, py_object]
AVEncoder.mw_video_encoder_close.argtypes=[c_void_p]

AVEncoder.mw_video_encode_frame.argtypes=[c_void_p, c_void_p, c_int64]

AVEncoder.mw_audio_encoder_open.restype = c_void_p
AVEncoder.mw_audio_encoder_open.argtypes=[c_uint, c_uint, c_uint, c_uint, MW_ENCODER_CALLBACK, py_object]
AVEncoder.mw_audio_encoder_close.argtypes=[c_void_p]

AVEncoder.mw_audio_encode_frame.argtypes=[c_void_p, c_void_p, c_uint, c_int64]