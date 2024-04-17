import ctypes
from ctypes import *
import threading

from LibMWMp4 import *
from LibMWAVEncoder import *

def audio_encoder_callback(avrecord, frame, frame_size, frame_info):
    Mp4.mw_mp4_write_audio(avrecord.m_mp4_handle, frame, frame_size, frame_info.contents.pts)
    pass

def video_encoder_callback(avrecord, frame, frame_size, frame_info):
    Mp4.mw_mp4_write_video(avrecord.m_mp4_handle, frame, frame_size, frame_info.contents.pts)
    pass

class AVRecord(object):
    def __init__(self, filename):
        self.m_width = 0
        self.m_height = 0
        self.m_foucc = 0
        self.m_channels = 0
        self.m_sample_rate = 0
        self.v_callback = MW_ENCODER_CALLBACK(video_encoder_callback)
        self.a_callback = MW_ENCODER_CALLBACK(audio_encoder_callback)
        self.m_video_enc_handle = 0
        self.m_audio_enc_handle = 0
        self.lock = threading.Lock()
        self.m_mp4_handle = Mp4.mw_mp4_open(filename)

    def __del__(self):
        AVEncoder.mw_video_encoder_close(self.m_video_enc_handle)
        AVEncoder.mw_audio_encoder_close(self.m_audio_enc_handle)
        Mp4.mw_mp4_close(self.m_mp4_handle)

    def set_video(self, width, height, foucc):
        self.m_width = width
        self.m_height = height
        self.m_foucc = foucc
        h264_info = mw_mp4_h264_info_t()
        h264_info.codec_type = MW_MP4_VIDEO_TYPE_H264
        h264_info.width = width
        h264_info.height = height
        h264_info.timescale = 1000
        h264_info.h264.sps = 0
        h264_info.h264.sps_size = 0
        h264_info.h264.pps = 0
        h264_info.h264.pps_size = 0
        Mp4.mw_mp4_set_video(self.m_mp4_handle, byref(h264_info))
        self.m_video_enc_handle = AVEncoder.mw_video_encoder_open(width, height, foucc, 4096, 60, 60, 0, 0, self.v_callback, py_object(self))

    def set_audio(self, channels, sample_rate):
        self.m_channels = channels
        self.m_sample_rate = sample_rate
        audio_info = mw_mp4_audio_info_t()
        audio_info.codec_type = MW_MP4_AUDIO_TYPE_ADTS_AAC
        audio_info.channels = channels
        audio_info.sample_rate = sample_rate
        audio_info.timescale = 1000
        audio_info.profile = 0
        Mp4.mw_mp4_set_audio(self.m_mp4_handle, byref(audio_info))
        self.m_audio_enc_handle = AVEncoder.mw_audio_encoder_open(channels, sample_rate, 16, 32, self.a_callback, py_object(self))

    def write_video(self, data, len, timestamp):
        AVEncoder.mw_video_encode_frame(self.m_video_enc_handle, data, timestamp)

        
    def write_audio(self, data, len, timestamp):
        AVEncoder.mw_audio_encode_frame(self.m_audio_enc_handle, data, len, timestamp)

