#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <time.h>
#include <pthread.h>
#include <sys/eventfd.h>
#include <sys/select.h>

#include "ProductVer.h"
#include "LibMWCapture/MWCapture.h"
#include "LibMWCapture/MWEcoCapture.h"
#include "MWFOURCC.h"

#include "OpenGLRender/OpenGLRender.h"
#include "imgui.h"
#include "imgui_impl_glfw_gl3.h"

#include "AlsaPlayer/AlsaPlayerDevice.h"
#include "mw_mp4/mw_mp4.h"
#include "X264Enc/X264Enc.h"
#include "VoAacEnc/VoAacEnc.h"
#define MAX_CAPTURE 16
#define VIDEO_FRAME_NUM 4
#define AUDIO_FRAME_NUM 500



#define CAPTURE_CHANNEL 2
#define CAPTURE_BIT_PER_SAMPLE 16
#define CAPTURE_SAMPLE_PER_FRAME 192

#define MAX_ECO_BUFFER_COUNT 4



#define CAPTURE_FRAME_RATE 25

static int CAPTURE_SAMPLE_RATE = 48000;
static int CAPTURE_WIDTH = 1920;
static int CAPTURE_HEIGHT = 1080;

static unsigned int CAPTURE_FOURCC = MWFOURCC_NV12;
static unsigned int RENDER_FOURCC = RENDER_NV12;
static unsigned int ENC_FOURCC = MW_X264_ENC_FORMAT_NV12;

static int g_image_size = 0;
static int g_min_stride = 0;

static unsigned char *g_video_frame[VIDEO_FRAME_NUM];
static long long g_video_time[VIDEO_FRAME_NUM];
static int g_vfw_index = 0;
static int g_vfr_index_rec = 0;


static unsigned char *g_audio_frame[AUDIO_FRAME_NUM];
static long long g_audio_time[AUDIO_FRAME_NUM];

//static int g_audio_cache_size = (CAPTURE_SAMPLE_PER_FRAME * CAPTURE_CHANNEL * CAPTURE_BIT_PER_SAMPLE / 2 * 500);
static int g_afw_index = 0;
static int g_afr_index_play = 0;
static int g_afr_index_rec = 0;


static int g_video_capture = 0;
static int g_audio_capture = 0;
static int g_recording = 0;
static int g_running = 0;

static int g_audio_frame_len = 0;


static int g_capture_num = 0;
static char g_capture_name[MAX_CAPTURE][80];
static int g_capture_index[MAX_CAPTURE];
static HCHANNEL g_channel_handle = NULL;

static int g_audio_capture_frames = 0;
static int g_audio_play_frames = 0;
static int g_audio_encode_frames = 0;
static int g_video_capture_frames = 0;
static int g_video_render_frames = 0;
static int g_video_encode_frames = 0;
void print_version()
{
    BYTE maj, min;
    WORD build;
    MWGetVersion(&maj, &min, &build);
    printf("Magewell MWCapture SDK V%d.%d.1.%d - AVCapture\n", maj, min, build);
    printf("Usage:\n./AVCapture [-fourcc nv12] [-width 1920] [-height 1080] [-sample_rate 48000]\n");
    printf("USB Devices are not supported\n");
}


void *audio_player(void *)
{
    int frame_len = 2 * CAPTURE_CHANNEL * CAPTURE_SAMPLE_PER_FRAME;
    int samples;
    int offset;
    ALSAPlayerDevice audio_player;
    audio_player.create_device("default");
    audio_player.SetFormat(CAPTURE_SAMPLE_RATE, CAPTURE_CHANNEL, CAPTURE_SAMPLE_PER_FRAME);
    audio_player.SetVolume(99, 0, 100);
    while(g_running&g_audio_capture){
        if(g_afw_index == g_afr_index_play){
            usleep(10000);
            continue;
        }
        if(((g_afw_index + AUDIO_FRAME_NUM - g_afr_index_play) % AUDIO_FRAME_NUM) > (AUDIO_FRAME_NUM/2)){
            if(g_afw_index){
                g_afr_index_play = g_afw_index - 1;
            }
            else{
                g_afr_index_play = AUDIO_FRAME_NUM - 1;
            }
        }
        audio_player.write_data(g_audio_frame[g_afr_index_play], CAPTURE_SAMPLE_PER_FRAME);
        g_afr_index_play++;
        g_audio_play_frames++;
        if(AUDIO_FRAME_NUM == g_afr_index_play){
            g_afr_index_play = 0;
        }
    }
    audio_player.destory_device();
    return NULL;
}

int eco_event_wait(int event, int timeout/*ms*/)
{
	fd_set rfds;
	struct timeval tv;
	struct timeval *ptv = NULL;
	eventfd_t value = 0;
	int retval;

	FD_ZERO(&rfds);
	FD_SET(event, &rfds);

	if (timeout < 0) {
		ptv = NULL;
	} else if (timeout == 0) {
		tv.tv_sec = timeout / 1000;
		tv.tv_usec = (timeout % 1000) * 1000;
		ptv = &tv;
	} else {
		tv.tv_sec = timeout / 1000;
		tv.tv_usec = (timeout % 1000) * 1000;
		ptv = &tv;
	}

	retval = select(event + 1, &rfds, NULL, NULL, ptv);
	if (retval == -1)
		return retval;
	else if (retval > 0) {
		retval = eventfd_read(event, &value);
		if (value > 0) {
			return value;
		} else {
			return retval < 0 ? retval : -1;
		}
	}

	// timeout
	return 0;
}

void *audio_capture(void * parm)
{
    int i;
    int eco_event = 0;
    char is_eco = ((char)(long long)parm);
    MWCAP_PTR notify_event = 0;
    HNOTIFY notify = 0;
    bool capture_start = 0;
    DWORD input_count = 0;
    int now_channels=0;
    int depth = CAPTURE_BIT_PER_SAMPLE / 8;
    int channel_offset;
    DWORD *capture_buf;
    //unsigned char *audio_cache_end = g_audio_cache + g_audio_cache_size;
    MWCAP_AUDIO_SIGNAL_STATUS audio_signal_status;
    //HCHANNEL g_channel_handle = (HCHANNEL)parm;
        if(is_eco){
        eco_event = eventfd(0, EFD_NONBLOCK);
        if (0 == eco_event) {
            printf("create eco event failed\n");
            goto audio_capture_stoped;
        }
    }else{
        notify_event = MWCreateEvent();
        if(NULL == notify_event){
            printf("create notify_event fail\n");
            goto audio_capture_stoped;
        }
    }
    MWGetAudioInputSourceArray(g_channel_handle, NULL, &input_count);
    if(input_count == 0){
        printf("can't find audio input %d\n", input_count);
        goto audio_capture_stoped;
    }

    if (MW_SUCCEEDED != MWStartAudioCapture(g_channel_handle)) {
        printf("start audio capture fail!\n");
        goto audio_capture_stoped;
    }
    
    capture_start = 1;
    if(is_eco){
        notify= MWRegisterNotify(g_channel_handle, eco_event, MWCAP_NOTIFY_AUDIO_FRAME_BUFFERED);
    }else{
        notify= MWRegisterNotify(g_channel_handle, notify_event, MWCAP_NOTIFY_AUDIO_FRAME_BUFFERED);
    }
    if (notify == 0) {
        printf("Register Notify fail\n");
        goto audio_capture_stoped;
    }
    
    if(MW_SUCCEEDED != MWGetAudioSignalStatus(g_channel_handle, &audio_signal_status)){
        printf("can't get audio signal status\n");
        goto audio_capture_stoped;
    }
    
    if (FALSE == audio_signal_status.wChannelValid){
        printf("audio signal is invalid\n");
        goto audio_capture_stoped;
    }
    for (i = 0; i < (MWCAP_AUDIO_MAX_NUM_CHANNELS / 2); i++){
        now_channels += (audio_signal_status.wChannelValid & (0x01 << i)) ? 2 : 0;
    }
    if(now_channels < CAPTURE_CHANNEL){
        printf("audio channel %d error\n", now_channels);
        goto audio_capture_stoped;
    }
    channel_offset = now_channels / 2;

    while(g_audio_capture && g_running){
        ULONGLONG notify_status = 0;
        unsigned char *audio_frame;
        MWCAP_AUDIO_CAPTURE_FRAME macf;
        if(is_eco){
            if (eco_event_wait(eco_event, 1000) <= 0){
                printf("wait notify error or timeout\n");
                break;
            }
        }else{
            if (MWWaitEvent(notify_event, 1000) <= 0){
                printf("wait notify error or timeout\n");
                break;
            }
        }
        if (MW_SUCCEEDED != MWGetNotifyStatus(g_channel_handle, notify, &notify_status)){
            continue;
        }
        if (0 == (notify_status & MWCAP_NOTIFY_AUDIO_FRAME_BUFFERED)){
            continue;
        }
        while(1){         
            if (MW_ENODATA == MWCaptureAudioFrame(g_channel_handle, &macf)){
                break;
            }
    /*
    L1L2L3L4R1R2R3R4L5L6L7L8R5R6R7R8(4byte)
    to 2channel 16bit
    L1R1L5R5(2byte)
    */
            capture_buf = macf.adwSamples;
            audio_frame = g_audio_frame[g_afw_index];// + g_afcw_byte % g_audio_cache_size;
            for (i = 0 ; i < MWCAP_AUDIO_SAMPLES_PER_FRAME; i++){
                WORD temp = capture_buf[0] >> (32 - CAPTURE_BIT_PER_SAMPLE);
                memcpy(audio_frame, &temp, depth);
                audio_frame += depth;
                
                temp = capture_buf[MWCAP_AUDIO_MAX_NUM_CHANNELS / 2] >> (32 - CAPTURE_BIT_PER_SAMPLE);
                memcpy(audio_frame, &temp, depth);
                audio_frame += depth;
                capture_buf += MWCAP_AUDIO_MAX_NUM_CHANNELS;
            }
            MWGetDeviceTime(g_channel_handle, &g_audio_time[g_afw_index]);
            g_audio_time[g_afw_index] /= 10000;
            g_audio_capture_frames++;
            g_afw_index++;// += CAPTURE_CHANNEL*MWCAP_AUDIO_SAMPLES_PER_FRAME*depth;
            if(AUDIO_FRAME_NUM == g_afw_index){
                g_afw_index = 0;
            }
        }
    }
audio_capture_stoped:
    g_audio_capture = 0;
    if(notify){
        MWUnregisterNotify(g_channel_handle, notify);
        notify = 0;
    }
    if(eco_event){
        eventfd_write(eco_event, 1);
        close(eco_event);
    }

    if(capture_start){
        MWStopAudioCapture(g_channel_handle);
        capture_start = 0;
    }

    if(notify_event!= 0){
        MWCloseEvent(notify_event);
        notify_event = 0;
    }

    return NULL;
}



void eco_video_capture()
{
    int ret, i;
    int event;
    MWCAP_VIDEO_ECO_CAPTURE_OPEN eco_open;
    MWCAP_VIDEO_ECO_CAPTURE_FRAME eco_frame[MAX_ECO_BUFFER_COUNT];
    MWCAP_VIDEO_ECO_CAPTURE_STATUS eco_status;
    memset(eco_frame, 0, sizeof(MWCAP_VIDEO_ECO_CAPTURE_FRAME) * MAX_ECO_BUFFER_COUNT);
    event = eventfd(0, EFD_NONBLOCK);
    if (0 == event) {
        printf("create event failed\n");
        return;
    }

    eco_open.cx = CAPTURE_WIDTH;
    eco_open.cy = CAPTURE_HEIGHT;
    eco_open.dwFOURCC = CAPTURE_FOURCC;
    eco_open.hEvent = event;
    eco_open.llFrameDuration = 10000000 / CAPTURE_FRAME_RATE;//25p/s; -1 for input frame rate 
    ret = MWStartVideoEcoCapture(g_channel_handle, &eco_open);
    if (0 != ret) {
        printf("MWStartVideoEcoCapture failed! iRet=%d\n", ret);
        goto end;
    }
    for (i = 0; i < MAX_ECO_BUFFER_COUNT; i++) {
        eco_frame[i].deinterlaceMode = MWCAP_VIDEO_DEINTERLACE_BLEND;
        eco_frame[i].cbFrame  = g_image_size;
        eco_frame[i].pvFrame  = (unsigned long)malloc(eco_frame[i].cbFrame);
        eco_frame[i].cbStride = g_min_stride;
        eco_frame[i].bBottomUp = FALSE;
        if (eco_frame[i].pvFrame == 0) {
            printf("malloc memory error\n");
            MWStopVideoEcoCapture(g_channel_handle);
            goto end;
        }
        eco_frame[i].pvContext = (MWCAP_PTR)&eco_frame[i];
        memset((void *)eco_frame[i].pvFrame, 0, eco_frame[i].cbFrame);
        ret = MWCaptureSetVideoEcoFrame(g_channel_handle, &eco_frame[i]);
        if (0 != ret) {
            printf("MWCaptureSetVideoEcoFrame failed! iRet=%d\n", ret);
            MWStopVideoEcoCapture(g_channel_handle);
            goto end;
        }
    }
    while(g_video_capture && g_running){
        ret = eco_event_wait(event, 1000);
        if(ret <= 0){
            printf("event timeout or error! iRet=%d\n", ret);
            continue;
        }
        memset(&eco_status, 0, sizeof(eco_status));
        ret = MWGetVideoEcoCaptureStatus(g_channel_handle, &eco_status);
        if(0 != ret){
            printf("MWGetVideoEcoCaptureStatus failed! iRet=%d\n", ret);
            break;
        }
        memcpy(g_video_frame[g_vfw_index], (unsigned char*)eco_status.pvFrame, g_image_size);
        MWGetDeviceTime(g_channel_handle, &g_video_time[g_vfw_index]);
        g_video_time[g_vfw_index] /= 10000;
        g_vfw_index++;
        g_video_capture_frames++;
        if(g_vfw_index >= VIDEO_FRAME_NUM){
            g_vfw_index = 0;
        }

        ret = MWCaptureSetVideoEcoFrame(g_channel_handle, (MWCAP_VIDEO_ECO_CAPTURE_FRAME *)eco_status.pvContext);
        if(0 != ret){
            printf("MWCaptureSetVideoEcoFrame failed! iRet=%d\n", ret);
            break;
        }
    }

    MWStopVideoEcoCapture(g_channel_handle);
    eventfd_write(event, 1);
end:
    for (i = 0; i < MAX_ECO_BUFFER_COUNT; i++) {
        if(eco_frame[i].pvFrame){
            free((void*)eco_frame[i].pvFrame);
        }
    }
    close(event);
}


void pro_video_capture()
{
    HNOTIFY notify = 0;
    int i;
    bool capture_start = 0;
    MWCAP_PTR notify_event = 0;
    MWCAP_PTR capture_event = 0;
    MWCAP_VIDEO_BUFFER_INFO video_buffer_info;
    MWCAP_VIDEO_FRAME_INFO video_frame_info;
    MWCAP_VIDEO_SIGNAL_STATUS video_signal_status;
    DWORD notify_buffer_mode = MWCAP_NOTIFY_VIDEO_FRAME_BUFFERED;
    
    //HCHANNEL g_channel_handle = (HCHANNEL)parm;
    notify_event = MWCreateEvent();
    if(NULL == notify_event){
        printf("create notify_event fail\n");
        goto video_capture_stoped;
    }
    capture_event = MWCreateEvent();
    if(NULL == capture_event){
        printf("create capture_event fail\n");
        goto video_capture_stoped;
    }
    if (MW_SUCCEEDED != MWStartVideoCapture(g_channel_handle, capture_event)) {
        printf("start video capture fail!\n");
        goto video_capture_stoped;
    }
    capture_start = 1;
    if(MW_SUCCEEDED != MWGetVideoSignalStatus(g_channel_handle, &video_signal_status)){
        printf("can't get video signal status\n");
        goto video_capture_stoped;
    }
    switch (video_signal_status.state) {
    case MWCAP_VIDEO_SIGNAL_NONE:
        printf("input signal status: NONE\n");
        break;
    case MWCAP_VIDEO_SIGNAL_UNSUPPORTED:
        printf("input signal status: Unsupported\n");
        break;
    case MWCAP_VIDEO_SIGNAL_LOCKING:
        printf("input signal status: Locking\n");
        break;
    case MWCAP_VIDEO_SIGNAL_LOCKED:
        printf("input signal status: Locked\n");
        break;
    default:
        printf("input signal status: unknow %d\n",video_signal_status.state);
    }
    if (video_signal_status.state != MWCAP_VIDEO_SIGNAL_LOCKED) {
        printf("input signal status error\n");
        goto video_capture_stoped;
    }
    if(video_signal_status.bInterlaced){
        notify_buffer_mode = MWCAP_NOTIFY_VIDEO_FIELD_BUFFERED;
    }
    notify= MWRegisterNotify(g_channel_handle, notify_event, notify_buffer_mode);
    if (notify == 0) {
        printf("Register Notify fail\n");
        goto video_capture_stoped;
    }
    for(i = 0; i < VIDEO_FRAME_NUM; i++){
        MWPinVideoBuffer(g_channel_handle, (MWCAP_PTR)g_video_frame[i], g_image_size);
    }
    while(g_video_capture && g_running){
        MW_RESULT ret;
        ULONGLONG notify_status = 0;
        MWCAP_VIDEO_DEINTERLACE_MODE mode;
        if(MWWaitEvent(notify_event, 1000) <= 0) {
            printf("wait notify error or timeout\n");
            break;
        }
        if (MWGetNotifyStatus(g_channel_handle, notify, &notify_status) != MW_SUCCEEDED){
            continue;
        }
        if (MWGetVideoBufferInfo(g_channel_handle, &video_buffer_info) != MW_SUCCEEDED){
            continue;
        }
        if (MWGetVideoFrameInfo(g_channel_handle, video_buffer_info.iNewestBufferedFullFrame, &video_frame_info) != MW_SUCCEEDED){
            continue;
        }
        if (0 == (notify_status & notify_buffer_mode)) {
            continue;
        }
        if(video_signal_status.bInterlaced){
            if(0 == video_buffer_info.iBufferedFieldIndex){
                mode = MWCAP_VIDEO_DEINTERLACE_TOP_FIELD;
            }else{
                mode = MWCAP_VIDEO_DEINTERLACE_BOTTOM_FIELD;
            }
        }else{
            mode = MWCAP_VIDEO_DEINTERLACE_BLEND;
        }
        ret = MWCaptureVideoFrameToVirtualAddressEx(g_channel_handle,
            video_buffer_info.iNewestBufferedFullFrame,
            (unsigned char *)g_video_frame[g_vfw_index], 
            g_image_size, 
            g_min_stride,
            0,
            0,
            CAPTURE_FOURCC,
            CAPTURE_WIDTH,
            CAPTURE_HEIGHT,
            0,
            0,
            0,
            0,
            0,
            100,
            0,
            100,
            0,
            mode,
            MWCAP_VIDEO_ASPECT_RATIO_CROPPING,
            0,
            0,
            0,
            0,
            MWCAP_VIDEO_COLOR_FORMAT_UNKNOWN,
            MWCAP_VIDEO_QUANTIZATION_UNKNOWN,
            MWCAP_VIDEO_SATURATION_UNKNOWN);
        if(MW_SUCCEEDED != ret){
            continue;
        }
        if(MWWaitEvent(capture_event, 1000) <= 0) {
            printf("wait capture event error or timeout\n");
            break;
        }
        MWCAP_VIDEO_CAPTURE_STATUS captureStatus;
        MWGetVideoCaptureStatus(g_channel_handle, &captureStatus);
        MWGetDeviceTime(g_channel_handle, &g_video_time[g_vfw_index]);
        g_video_time[g_vfw_index] /= 10000;
        g_vfw_index++;
        g_video_capture_frames++;
        if(g_vfw_index >= VIDEO_FRAME_NUM){
            g_vfw_index = 0;
        }
    }
    for(i = 0; i < VIDEO_FRAME_NUM; i++){
        MWUnpinVideoBuffer(g_channel_handle, (LPBYTE)g_video_frame[i]);
    }
video_capture_stoped:
    g_video_capture = 0;
    if(notify){
        MWUnregisterNotify(g_channel_handle, notify);
        notify = 0;
    }
    if(capture_start){
        MWStopVideoCapture(g_channel_handle);
    }
    if(notify_event!= 0){
        MWCloseEvent(notify_event);
        notify_event = 0;
    }

    if(capture_event!= 0){
        MWCloseEvent(capture_event);
        capture_event = 0;
    }
}
void *video_capture(void *parm){
    char is_eco = (char)((long long)parm);
    if(is_eco){
        eco_video_capture();
    }else{
        pro_video_capture();
    }
    return NULL;
}
char g_out_file_name[256];
void generate_file_name()
{
    time_t timep;   
    struct tm *p; 
    time(&timep);   
    p = localtime(&timep);
    sprintf(g_out_file_name, "%d-%d-%d_%d-%d-%d.mp4", 1900 + p->tm_year,
        1 + p->tm_mon,  p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec);
    printf("mp4 file name is: %s\n", g_out_file_name);
}

void *av_record(void *parm)
{
    int ret;
    unsigned int i;
    unsigned int nalu_num;
    unsigned int aac_len[2];
    unsigned char aac[1024];
    unsigned char *p_nal_buf[16];
    unsigned int nal_len[16];
    unsigned int nal_num;
    unsigned int frame_len = 0;
    unsigned char is_set_video = 0;
    unsigned char is_set_audio = 0;
    mw_x264_encoder_t venc = NULL;
    mw_voaac_enc_t aenc = NULL;
    mw_mp4_handle_t mp4 = NULL;

    generate_file_name();
    mp4 = mw_mp4_open(g_out_file_name);
    if(g_video_capture){
        venc = mw_x264_encoder_create(CAPTURE_WIDTH, CAPTURE_HEIGHT, ENC_FOURCC, CAPTURE_FRAME_RATE, 2048000, 60, &nalu_num);
        //Mp4SetVideoParm(mp4, EN_VIDEO_CODEC_TYPE_H264, CAPTURE_WIDTH, CAPTURE_HEIGHT); 
    }
    if(g_audio_capture){
        aenc = mw_voaac_enc_create(CAPTURE_CHANNEL, CAPTURE_SAMPLE_RATE, CAPTURE_BIT_PER_SAMPLE, 32000);
        //Mp4SetAudioParm(mp4, EN_AUDIO_CODEC_TYPE_AAC, CAPTURE_SAMPLE_RATE);
    }
    g_afr_index_rec = g_afw_index;
    g_vfr_index_rec = g_vfw_index;
    while(g_recording && g_running){
        int av_write = 0;
        if(aenc && (g_afr_index_rec != g_afw_index)){//audio
            ret = mw_voaac_enc_frame(aenc, g_audio_frame[g_afr_index_rec], g_audio_frame_len, aac, 1024, aac_len);
            g_audio_encode_frames++;
            if(ret < 0){
                printf("error aac\n");
                break;
            }

            if(ret){
               if(0 == is_set_audio){
                   mw_mp4_audio_info_t audio_info;
                   audio_info.codec_type = MW_MP4_AUDIO_TYPE_ADTS_AAC;
                   audio_info.channels = CAPTURE_CHANNEL;
                   audio_info.sample_rate = CAPTURE_SAMPLE_RATE;
                   audio_info.timescale = 1000;
                   audio_info.profile = 0;
                   mw_mp4_set_audio(mp4, &audio_info);
                   is_set_audio = 1;
                }
                mw_mp4_write_audio(mp4, aac, aac_len[0], g_audio_time[g_afr_index_rec]);
                //Mp4WriteAudio(mp4, aac, aac_len[0], g_audio_time[g_afr_index_rec]);
            }
            g_afr_index_rec++;
            if(g_afr_index_rec >= AUDIO_FRAME_NUM){
                g_afr_index_rec = 0;
            }
        }
        else if(venc && (g_vfr_index_rec != g_vfw_index)){//video
            ret = mw_x264_encode_frame(venc, 0, g_video_frame[g_vfr_index_rec], p_nal_buf, nal_len, &nal_num, NULL);
            g_video_encode_frames++;
            if(ret < 0){
                printf("error enc h264\n");
                break;
            }
            if(ret){
                if(0 == is_set_video){
                    mw_mp4_video_info_t video_info;
                    video_info.codec_type = MW_MP4_VIDEO_TYPE_H264;
                    video_info.timescale = 1000;
                    video_info.width = CAPTURE_WIDTH;
                    video_info.height = CAPTURE_HEIGHT;
                    video_info.h264.sps = NULL;
                    video_info.h264.sps_size = 0;
                    video_info.h264.pps = NULL;
                    video_info.h264.pps_size = 0;
                    mw_mp4_set_video(mp4, &video_info);
                    is_set_video = 1;
                }
                frame_len = 0;
                for(i = 0; i < nal_num; i++){
                    frame_len += nal_len[i];
                }
		//printf("vvv %d, %lld %lld %lld %lld\n",g_afr_index_rec,g_audio_time[0],g_audio_time[1],g_audio_time[2],g_audio_time[3]);
                mw_mp4_write_video(mp4, p_nal_buf[0], frame_len, g_video_time[g_vfr_index_rec]);
                //Mp4WriteVideo(mp4, p_nal_buf[0], frame_len, g_video_time[g_vfr_index_rec]);
            }
            g_vfr_index_rec++;
            if(g_vfr_index_rec >= VIDEO_FRAME_NUM){
                g_vfr_index_rec = 0;
            }
        }else{
            usleep(10000);
        }
    }
    mw_mp4_close(mp4);
    if(venc){
        mw_x264_encoder_destory(venc);
    }
    if(aenc){
        mw_voaac_enc_destory(aenc);
    }
    g_audio_encode_frames = 0;
    g_video_encode_frames = 0;
    g_out_file_name[0]=0;
}

void list_device()
{
    int capture_num;
    MWCAP_CHANNEL_INFO mci;
    MWRefreshDevice();
    capture_num = MWGetChannelCount();
    for (int i = 0; i < capture_num; i++) {
        MWGetChannelInfoByIndex(i, &mci);
        if((NULL == strstr(mci.szProductName, "Pro Capture")) && (NULL == strstr(mci.szProductName, "Eco Capture"))){
            continue;
        }
        sprintf(g_capture_name[g_capture_num], "%d %s", i, mci.szProductName);
        g_capture_index[g_capture_num] = i;
        g_capture_num++;
        if(g_capture_num >= MAX_CAPTURE){
            printf("too many capture, just list %d\n", MAX_CAPTURE);
            break;
        }
    }
}



void imgui_clieck_device(int index, bool is_choose)
{
    //static HCHANNEL g_channel_handle = NULL;
    char is_eco = 0;
    MWCAP_CHANNEL_INFO channel_info;
    static HANDLE video_handle = NULL;
    static HANDLE audio_handle = NULL;
    void *status;
    static pthread_t audio_player_tid = 0;
    static pthread_t audio_capture_tid = 0;
    static pthread_t video_capture_tid = 0;
    g_video_capture = 0;
    g_audio_capture = 0;
    if(audio_capture_tid){
        pthread_join(audio_capture_tid, &status);
        audio_capture_tid = 0;
    }
    if(video_capture_tid){
        pthread_join(video_capture_tid, &status);
        video_capture_tid = 0;
    }
    if(g_channel_handle){
        MWCloseChannel(g_channel_handle);
        g_channel_handle = NULL;
    }
    if(audio_player_tid){
        pthread_join(audio_player_tid, &status);
        audio_player_tid = 0;
        g_afw_index= g_afr_index_play = g_afr_index_rec = 0;
    }
    g_video_capture_frames = 0;
    g_audio_capture_frames = 0;
    g_audio_play_frames = 0;
    g_video_render_frames = 0;
    if(0 == is_choose){
        return;
    }
    char path[256] = {0};
    if(MW_SUCCEEDED != MWGetDevicePath(index, path)){
        printf("%s get device path fail\n", g_capture_name[index]);
        return;
    }
    
    g_channel_handle = MWOpenChannelByPath(path);
    if(NULL == g_channel_handle){
        printf("%s open channel fail\n", g_capture_name[index]);
        return;
    }
    if (MW_SUCCEEDED != MWGetChannelInfo(g_channel_handle, &channel_info)) {
        printf("Can't get channel info!\n");
        return;
    }
    if(0 == strcmp(channel_info.szFamilyName, "Eco Capture")){
        is_eco = 1;
    }
    g_video_capture = 1;
    g_audio_capture = 1;
    pthread_create(&audio_capture_tid, NULL, audio_capture, (void*)is_eco);
    pthread_create(&video_capture_tid, NULL, video_capture, (void*)is_eco);
    pthread_create(&audio_player_tid, NULL, audio_player, NULL);
}

void imgui_init(GLFWwindow* window)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui_ImplGlfwGL3_Init(window, true);
    ImGui::StyleColorsClassic();

}


void imgui_record()
{
    static pthread_t rec_tid;
    if(0 == g_recording){
        if (ImGui::Button("start record")){
            g_recording = 1;
            printf("start record\n");
            pthread_create(&rec_tid, NULL, av_record, NULL);
        }
    }
    else{
        if (ImGui::Button("stop  record")){
            void *status;
            printf("stop record\n");
            g_recording = 0;
            pthread_join(rec_tid, &status);
        }
    }
}

void imgui_choose_device()
{
    int i;
    bool refresh = 0;
    static bool choose[MAX_CAPTURE] = {0};
    static bool is_first = 1;
    if(is_first){
        imgui_clieck_device(g_capture_index[0], 1);
        memset(choose, 0, sizeof(choose));
        choose[0] = 1;
        is_first = 0;
    }
    ImGui_ImplGlfwGL3_NewFrame();
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("Devices")) {
            for (i = 0; i < g_capture_num; i++) {
                if (ImGui::MenuItem(g_capture_name[i], NULL, &choose[i])) {
                    imgui_clieck_device(g_capture_index[i], choose[i]);
                    if(choose[i]){
                        memset(choose, 0, sizeof(choose));
                        choose[i] = 1;
                    }
                }
            }
            /*if(ImGui::MenuItem("refresh device", NULL, refresh)){
                memset(choose, 0, sizeof(choose));
                imgui_clieck_device(0, 0);
                list_device();
            }*/
            ImGui::EndMenu();
        }
        imgui_record();
        ImGui::EndMainMenuBar();
    }
}

int GetTickCount()
{
    struct timespec tp;
    clock_gettime(CLOCK_MONOTONIC, &tp);
    return (tp.tv_sec * 1000 + tp.tv_nsec / 1000000);
}
void get_fps_text(char *p_text)
{
    static int prev_time = 0;
    static int count = 0;
    static int prev_audio_capture_frames = 0;
    static int audio_play_frames = 0;
    static int prev_audio_encode_frames = 0;
    static int prev_video_capture_frames = 0;
    static int prev_video_render_frames = 0;
    static int prev_video_encode_frames = 0;

    static float audio_capture_fps = 0.0;
    static float audio_play_fps = 0.0;
    static float audio_encode_fps = 0.0;
    static float video_capture_fps = 0.0;
    static float video_render_fps = 0.0;
    static float video_encode_fps = 0.0;

    if(count == 0){
        prev_time = GetTickCount();
    }else if(count < 100){
        int time=GetTickCount()-prev_time;
        if(time){
            audio_capture_fps = (g_audio_capture_frames - prev_audio_capture_frames)*1000.0 / time;
            audio_play_fps = (g_audio_play_frames - audio_play_frames)*1000.0 / time;
            audio_encode_fps = (g_audio_encode_frames - prev_audio_encode_frames)*1000.0 / time;
            video_capture_fps = (g_video_capture_frames - prev_video_capture_frames)*1000.0 / time;
            video_render_fps = (g_video_render_frames - prev_video_render_frames)*1000.0 / time;
            video_encode_fps = (g_video_encode_frames - prev_video_encode_frames)*1000.0 / time;
        }
    }else if((count % 100) == 0){
        int now_time = GetTickCount();
        if(now_time != prev_time){
            int time = now_time - prev_time;
            audio_capture_fps = (g_audio_capture_frames - prev_audio_capture_frames)*1000.0 / time;
            audio_play_fps = (g_audio_play_frames - audio_play_frames)*1000.0 / time;
            audio_encode_fps = (g_audio_encode_frames - prev_audio_encode_frames)*1000.0 / time;
            video_capture_fps = (g_video_capture_frames - prev_video_capture_frames)*1000.0 / time;
            video_render_fps = (g_video_render_frames - prev_video_render_frames)*1000.0 / time;
            video_encode_fps = (g_video_encode_frames - prev_video_encode_frames)*1000.0 / time;
            prev_audio_capture_frames = g_audio_capture_frames;
            audio_play_frames = g_audio_play_frames;
            prev_audio_encode_frames = g_audio_encode_frames;
            prev_video_capture_frames = g_video_capture_frames;
            prev_video_render_frames= g_video_render_frames;
            prev_video_encode_frames = g_video_encode_frames;
            prev_time = now_time;
        }
    }
    if(g_audio_capture_frames == 0){
        audio_capture_fps = 0.0;
    }
    if(g_audio_play_frames == 0){
        audio_play_fps = 0.0;
    }
    if(g_audio_encode_frames == 0){
        audio_encode_fps = 0.0;
    }

    if(g_video_capture_frames == 0){
        video_capture_fps = 0.0;
    }
    if(g_video_render_frames == 0){
        video_render_fps = 0.0;
    }
    if(g_video_encode_frames == 0){
        video_encode_fps = 0.0;
    }
    count++;
    sprintf(p_text,
        "capture fps(v|a):%f|%f capture frams(v|a):%d|%d\n"
        "render fps(v|a):%f|%f render frams(v|a):%d|%d\n"
        "encode fps(v|a):%f|%f encode frames(v|a):%d|%d\n"
        "outputfile:%s", 
        video_capture_fps, audio_capture_fps, g_video_capture_frames, g_audio_capture_frames,
        video_render_fps, audio_play_fps, g_video_render_frames,g_audio_play_frames,
        video_encode_fps, audio_encode_fps, g_video_encode_frames, g_audio_encode_frames,
        g_out_file_name);
}

void imgui_draw_fps()
{
    char text[1024];
    bool m_b_open = true;
    ImGuiIO& io = ImGui::GetIO();
    ImVec2 pos;
    ImVec2 size;
    get_fps_text(text);
    size.x = 500;
    size.y = 70;
    ImGuiWindowFlags m_imgui_flags = ImGuiWindowFlags_NoMove |
                        ImGuiWindowFlags_NoTitleBar | 
                        ImGuiWindowFlags_NoResize  | 
                        ImGuiWindowFlags_NoScrollbar | 
                        ImGuiWindowFlags_NoCollapse |
                        ImGuiWindowFlags_AlwaysAutoResize |
                        ImGuiWindowFlags_NoSavedSettings |
                        ImGuiWindowFlags_NoFocusOnAppearing |
                        ImGuiWindowFlags_NoNav;
        pos.x = io.DisplaySize.x - size.x - 10;
        pos.y = io.DisplaySize.y - size.y - 10;
        ImGui::SetNextWindowPos(pos, ImGuiCond_Always);
        ImGui::SetNextWindowSize(size, ImGuiCond_Always);
        ImGui::SetNextWindowBgAlpha(0.5);
        ImGui::Begin("status", &m_b_open, m_imgui_flags);
        ImGui::Text("%s", text);
        ImGui::End();

}
void imgui_render()
{
    ImGui::Render();
    ImGui_ImplGlfwGL3_RenderDrawData(ImGui::GetDrawData());
    
}

void imgui_deinit()
{
    ImGui_ImplGlfwGL3_Shutdown();
    ImGui::DestroyContext();
}


static void glfw_error_callback(int error, const char* description)
{
    printf("error callback %d: %s\n", error, description);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS){
        glfwSetWindowShouldClose(window, GL_TRUE);
    }
}


void video_render()
{
    int play_index = 0;
    int dis_w = 960;
    int dis_h = 540;
    GLFWwindow* window = NULL;
    MWOpenGLRender video_render;
    GLFWvidmode* mode;
    mw_render_init_t rinit;
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()) {
        printf("glfwInit fail\n");
        return;
    }
    mode = (GLFWvidmode*)glfwGetVideoMode(glfwGetPrimaryMonitor());
    if(mode){
        dis_w = mode->width / 2;
        dis_h = mode->height/2;
    }

    window = glfwCreateWindow(dis_w, dis_h, "AVCapture", NULL, NULL);
    if(NULL == window){
        printf("create window fail\n");
        return;
    }
    glfwSetKeyCallback(window, key_callback);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    glewInit();
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_STENCIL_TEST);
    glDisable(GL_BLEND);

    imgui_init(window);

    //rinit.buffers_num = 0;
    rinit.height = CAPTURE_HEIGHT;
    rinit.width = CAPTURE_WIDTH;
    rinit.render_fourcc = RENDER_FOURCC;
    video_render.open(&rinit);
    glClearColor(0.2, 0.3, 0.3, 1.0);
    while (!glfwWindowShouldClose(window)) {
        int sleep_count = 0;
        mw_render_ctrl_t ctrl;
        glClear(GL_COLOR_BUFFER_BIT);
        imgui_choose_device();
        imgui_draw_fps();
        //imgui_record();
        glfwGetFramebufferSize(window, &dis_w, &dis_h);
        ctrl.display_h = dis_h;
        ctrl.display_w = dis_w;
        ctrl.threshold = 40;
        ctrl.val_ctrl = 40;
        ctrl.hdr_on = 0;
        if(g_video_capture){
            while(g_video_capture && (play_index == g_vfw_index)){
                sleep_count++;
                if(sleep_count > 100000){
                    play_index = g_vfw_index;
                    printf("one second have not data\n");
                    break;
                }
                usleep(10);
                continue;                
            }
            g_video_render_frames++;
            if(video_render.render(g_video_frame[play_index], &ctrl)){
                break;
            }
            play_index++;
            if(play_index >= VIDEO_FRAME_NUM){
                play_index = 0;
            }
        }else{
            glClearColor(0.2, 0.3, 0.3, 1.0);
        }
        imgui_render();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    video_render.close();
    imgui_clieck_device(0, 0);
    imgui_deinit();
    if(window){
        glfwDestroyWindow(window);
    }
    glfwTerminate();
}
void set_fourcc()
{
    if(CAPTURE_FOURCC == MWFOURCC_NV12){
        printf("set fourcc to nv12\n");
    }else if(CAPTURE_FOURCC == MWFOURCC_YV12){
        printf("set fourcc to yv12\n");
        RENDER_FOURCC = RENDER_YV12;
        ENC_FOURCC = MW_X264_ENC_FORMAT_YV12;
    }else if(CAPTURE_FOURCC == MWFOURCC_I420){
        printf("set fourcc to i420\n");
        RENDER_FOURCC = RENDER_I420;
        ENC_FOURCC = MW_X264_ENC_FORMAT_I420;
    }else if(CAPTURE_FOURCC == MWFOURCC_NV21){
        printf("set fourcc to nv21\n");
        RENDER_FOURCC = RENDER_NV21;
        ENC_FOURCC = MW_X264_ENC_FORMAT_NV21;
    }else{
        printf("not support fourcc, set to default nv12\n");
        CAPTURE_FOURCC == MWFOURCC_NV12;
    }
}
void parse_cmd(int argc, char* argv[])
{
    for(int i = 1; i < argc-1; i += 2){
        if(!memcmp(argv[i],"-width", strlen("-width"))){
            CAPTURE_WIDTH = atoi(argv[i+1]);
            printf("set width to %d\n",CAPTURE_WIDTH);
        }else if(!memcmp(argv[i],"-height", strlen("-height"))){
            CAPTURE_HEIGHT = atoi(argv[i+1]);
            printf("set height to %d\n",CAPTURE_HEIGHT);
        }else if(!memcmp(argv[i],"-fourcc", strlen("-fourcc"))){
            if(!memcmp(argv[i+1],"nv12", strlen("nv12"))){
                CAPTURE_FOURCC = MWFOURCC_NV12;
            }else if(!memcmp(argv[i+1],"yv12", strlen("yv12"))){
                CAPTURE_FOURCC = MWFOURCC_YV12;
            }else if(!memcmp(argv[i+1],"nv21", strlen("nv21"))){
                CAPTURE_FOURCC = MWFOURCC_NV21;
            }else if(!memcmp(argv[i+1],"i420", strlen("i420"))){
                CAPTURE_FOURCC = MWFOURCC_I420;
            }else if(!memcmp(argv[i+1],"yuy2", strlen("yuy2"))){
                CAPTURE_FOURCC = MWFOURCC_YUY2;
            }else if(!memcmp(argv[i+1],"rgb", strlen("rgb"))){
                CAPTURE_FOURCC = MWFOURCC_RGB24;
            }else if(!memcmp(argv[i+1],"argb", strlen("argb"))){
                CAPTURE_FOURCC = MWFOURCC_ARGB;
            }else if(!memcmp(argv[i+1],"rgba", strlen("rgba"))){
                CAPTURE_FOURCC = MWFOURCC_RGBA;
            }
            set_fourcc();
        }else if(!memcmp(argv[i],"-sample_rate", strlen("-sample_rate"))){
            CAPTURE_SAMPLE_RATE = atoi(argv[i+1]);
            printf("set sample rate to %d\n",CAPTURE_SAMPLE_RATE);
        }
    }
}

int main(int argc, char* argv[])
{
    int i;
    print_version();
    parse_cmd(argc, argv);
    if(!MWCaptureInitInstance()){
        printf("InitilizeFailed\n");
        return -1;
    }
    g_min_stride = FOURCC_CalcMinStride(CAPTURE_FOURCC, CAPTURE_WIDTH, 4);
    g_image_size = FOURCC_CalcImageSize(CAPTURE_FOURCC, CAPTURE_WIDTH, CAPTURE_HEIGHT, g_min_stride)*3/2;
    for(i = 0; i < VIDEO_FRAME_NUM; i++){
        g_video_frame[i] = (unsigned char*)malloc(g_image_size);
        if(NULL == g_video_frame[i]){
            printf("video cache malloc fail");
            goto end_and_free;
        }
    }
    g_audio_frame_len = CAPTURE_CHANNEL * MWCAP_AUDIO_SAMPLES_PER_FRAME * CAPTURE_BIT_PER_SAMPLE / 8;
    for(i = 0; i < AUDIO_FRAME_NUM; i++){
        g_audio_frame[i] = (unsigned char*)malloc(g_audio_frame_len);
        if(NULL == g_audio_frame[i]){
            printf("audio frame malloc fail");
            goto end_and_free;
        }
    }
    g_running = 1;
    list_device();
    video_render();
    g_running = 0;
end_and_free:
    MWCaptureExitInstance();
    for(i = 0; i < AUDIO_FRAME_NUM; i++){
        if(g_audio_frame[i]){
            free(g_audio_frame[i]);
        }
    }

    for(i = 0; i < VIDEO_FRAME_NUM; i++){
        if(g_video_frame[i]){
            free(g_video_frame[i]);
        }
    }
    printf("AVCapture stoped\n");
    return 0;
}
