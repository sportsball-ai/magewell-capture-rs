#include <stdio.h>
#include <unistd.h>
#include <sys/eventfd.h>
#include <string.h>
#include "MWFOURCC.h"
#include "LibMWCapture/MWCapture.h"
#include "LibMWCapture/MWEcoCapture.h"
#ifdef __cplusplus
extern "C"
{
#endif


MW_RESULT MWGetVersion_Pyc(
	BYTE*							pbyMaj,
	BYTE*							pbyMin,
	WORD*							pwBuild
	)
{
    return MWGetVersion(pbyMaj,pbyMin,pwBuild);
}
	
unsigned int FOURCC_CalcMinStride_Pyc(unsigned int fourcc, int cx, unsigned int align)
{
    unsigned int stride = FOURCC_CalcMinStride(fourcc, cx, align);
    return stride;
}

unsigned int FOURCC_CalcImageSize_Pyc(unsigned int fourcc, int cx, int cy, unsigned int stride)
{
    unsigned int size = FOURCC_CalcImageSize(fourcc, cx,  cy, stride);
    return size;
}
void MWResampleAudio_Pyc(MWCAP_AUDIO_CAPTURE_FRAME *p_mw_audio_frame, unsigned char* p_pcm, int channels, int depth)
{
    DWORD *p_capture_buf = p_mw_audio_frame->adwSamples;
    unsigned char*audio_frame = p_pcm;
    int bit_per_sample = depth*8;

    if((channels != 2) || (depth != 2)){
        return;
    }
    for (int i = 0 ; i < MWCAP_AUDIO_SAMPLES_PER_FRAME; i++){
        WORD temp = p_capture_buf[0] >> (32 - bit_per_sample);
        memcpy(audio_frame, &temp, depth);
        audio_frame += depth;
            
        temp = p_capture_buf[MWCAP_AUDIO_MAX_NUM_CHANNELS / 2] >> (32 - bit_per_sample);
        memcpy(audio_frame, &temp, depth);
        audio_frame += depth;
        p_capture_buf += MWCAP_AUDIO_MAX_NUM_CHANNELS;
    }
}
int MWEcoEventCreate_Pyc()
{
	int event = eventfd(0, EFD_NONBLOCK);
    return event;
}
void MWEcoEventDestory_Pyc(int event){
    eventfd_write(event, 1);
    close(event);
}

int MWEcoEventWait_Pyc(int event, int timeout)
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

#ifdef __cplusplus
}
#endif
