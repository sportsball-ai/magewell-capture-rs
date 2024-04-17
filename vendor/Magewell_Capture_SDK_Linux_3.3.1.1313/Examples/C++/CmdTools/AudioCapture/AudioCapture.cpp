/************************************************************************************************/
// AudioCapture.cpp : Defines the entry point for the console application.

// MAGEWELL PROPRIETARY INFORMATION

// The following license only applies to head files and library within Magewell’s SDK 
// and not to Magewell’s SDK as a whole. 

// Copyrights © Nanjing Magewell Electronics Co., Ltd. (“Magewell”) All rights reserved.

// Magewell grands to any person who obtains the copy of Magewell’s head files and library 
// the rights,including without limitation, to use, modify, publish, sublicense, distribute
// the Software on the conditions that all the following terms are met:
// - The above copyright notice shall be retained in any circumstances.
// -The following disclaimer shall be included in the software and documentation and/or 
// other materials provided for the purpose of publish, distribution or sublicense.

// THE SOFTWARE IS PROVIDED BY MAGEWELL “AS IS” AND ANY EXPRESS, INCLUDING BUT NOT LIMITED TO,
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
// IN NO EVENT SHALL MAGEWELL BE LIABLE 

// FOR ANY CLAIM, DIRECT OR INDIRECT DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT,
// TORT OR OTHERWISE, ARISING IN ANY WAY OF USING THE SOFTWARE.

// CONTACT INFORMATION:
// SDK@magewell.net
// http://www.magewell.com/
//
/************************************************************************************************/
#include <string.h>
#include <sys/eventfd.h>
#include <sys/select.h>
#include <pthread.h>
#include "WaveFile.h"
#include "wchar.h"
#include "stdio.h"
#include "unistd.h"

#include "LibMWCapture/MWCapture.h"

#define AUDIO_CAPTURE_SECONDS 10
HCHANNEL g_channel_handle = NULL;
static bool g_runing = true;


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
    char is_eco = (char)((long long)parm);
    int frmae_count;
    int byte_per_sample;
    MWCAP_PTR notify_event = 0;
    int eco_event = 0;
    HNOTIFY notify = 0;
    bool capture_start = 0;
    DWORD input_count = 0;
    int now_channels;
    int channel_offset;
    unsigned char *capture_buf = NULL;
    //unsigned char *audio_cache_end = g_audio_cache + g_audio_cache_size;
    MWCAP_AUDIO_SIGNAL_STATUS audio_signal_status;
    CWaveFile file;
    if(is_eco){
        eco_event = eventfd(0, EFD_NONBLOCK);
        if (0 == eco_event) {
            printf("create eco event failed\n");
            return NULL;
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
        printf("can't find audio input\n");
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
    byte_per_sample = audio_signal_status.cBitsPerSample / 8;
    for (int i = 0; i < (MWCAP_AUDIO_MAX_NUM_CHANNELS / 2); i++){
        now_channels += (audio_signal_status.wChannelValid & (0x01 << i)) ? 2 : 0;
    }
    if(0 == now_channels){
        printf("audio channel %d error\n", now_channels);
        goto audio_capture_stoped;
    }
    channel_offset = now_channels / 2;
    capture_buf = (unsigned char*)malloc(now_channels*byte_per_sample*MWCAP_AUDIO_SAMPLES_PER_FRAME);
    if(NULL == capture_buf){
        printf("capture_buf malloc fial\n");
        goto audio_capture_stoped;
    }
    file.Init("AudioCapture.wav", audio_signal_status.dwSampleRate, now_channels, audio_signal_status.cBitsPerSample);
    while(g_runing){
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
            for (int j = 0; j < (now_channels/2); j++){
                for (int i = 0 ; i < MWCAP_AUDIO_SAMPLES_PER_FRAME; i++){
                    int write_pos = (i * now_channels + j * 2) * byte_per_sample;
                    int read_pos = (i * MWCAP_AUDIO_MAX_NUM_CHANNELS + j);
                    int read_pos2 = (i * MWCAP_AUDIO_MAX_NUM_CHANNELS + j + MWCAP_AUDIO_MAX_NUM_CHANNELS/2);
                    DWORD left = macf.adwSamples[read_pos] >> (32 - audio_signal_status.cBitsPerSample);
                    DWORD right = macf.adwSamples[read_pos2] >> (32 - audio_signal_status.cBitsPerSample);
                    memcpy(&capture_buf[write_pos], &left, byte_per_sample);
                    memcpy(&capture_buf[write_pos + byte_per_sample], &right, byte_per_sample);
                }
            }
            if (file.IsOpen()){
                file.Write((const unsigned char *)capture_buf, MWCAP_AUDIO_SAMPLES_PER_FRAME * now_channels * byte_per_sample);
            }
        }
    }
audio_capture_stoped:
    if(capture_buf){
        free(capture_buf);
    }
    if(eco_event){
        eventfd_write(eco_event, 1);
        close(eco_event);
    }
    if(file.IsOpen()){
        file.Exit();
    }

    if(notify){
        MWUnregisterNotify(g_channel_handle, notify);
        notify = 0;
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


int get_id(char c)
{
    if(c >= '0' && c <= '9')
        return (int)(c - '0');
    if(c >= 'a' && c <= 'f')
        return (int)(c - 'a' + 10);
    if(c >= 'A' && c <= 'F')
        return (int)(c - 'F' + 10);
    return 0;
}

HCHANNEL open_channel(int argc, char* argv[]){
    HCHANNEL hChannel = NULL;
    int nChannelCount = MWGetChannelCount();

    if (0 == nChannelCount) {
        printf("ERROR: Can't find channels!\n");
        return NULL;
    }
    printf("Find %d channels!\n",nChannelCount);
    int nProDevCount = 0;
    int nProDevChannel[32] = {-1};
    for (int i = 0; i < nChannelCount; i++){
        MWCAP_CHANNEL_INFO info;
        MW_RESULT mr = MWGetChannelInfoByIndex(i, &info);
        if (0 == (strcmp(info.szFamilyName, "Pro Capture")) || (0 == strcmp(info.szFamilyName, "Eco Capture"))){
            nProDevChannel[nProDevCount] = i;
            nProDevCount++;
        }
    }
    if (nProDevCount <= 0){
        printf("\nERROR: Can't find pro channels!\n");
        return NULL;
    }

    printf("Find %d pro channels.\n", nProDevCount);

    // Get <board id > <channel id> or <channel index>
    int byBoardId = -1;
    int byChannelId = -1;
    int nDevIndex = -1;
    BOOL bIndex = FALSE;

    if (argc == 1) {
        bIndex = TRUE;
        nDevIndex = 0;
    }
    else if (NULL == strstr(argv[1], ":")){
        bIndex = TRUE;
        if ((strlen(argv[1]) > 2) || (argv[1][0] > '9') || argv[1][0] < '0') {
            printf("\nERROR: Invalid params!\n");
            return NULL;
        }
        nDevIndex = atoi(argv[1]);
        if(nDevIndex >= nProDevCount){
            printf("ERROR: just have %d channel!\n",nProDevCount);
            return NULL;
        }
    }
    else{
        bIndex = FALSE;
        if (strlen(argv[1]) == 3){
            if ((argv[1][0] >= '0' && argv[1][0] <= '9') || (argv[1][0] >= 'a' && argv[1][0] <= 'f') || (argv[1][0] >= 'A' && argv[1][0] <= 'F')){
                byBoardId = get_id(argv[1][0]);//atoi(argv[1]);
            }

            if ((argv[1][2] >= '0' && argv[1][2] <= '3')){
                byChannelId = get_id(argv[1][2]);//atoi(&argv[1][2]);
            }
        }

        if (-1 == byBoardId || -1 == byChannelId) {
            printf("\nERROR: Invalid params!\n");
            return NULL;
        }
    }

    // Open channel
    if (bIndex == TRUE){
        char path[128] = {0};
        MWGetDevicePath(nProDevChannel[nDevIndex], path);
        hChannel = MWOpenChannelByPath(path);
        if (hChannel == NULL) {
            printf("ERROR: Open channel %d error!\n", nDevIndex);
            return NULL;
        }
    }
    else{
        hChannel = MWOpenChannel(byBoardId, byChannelId);
        if (hChannel == NULL) {
            printf("ERROR: Open channel %X:%d error!\n", byBoardId, byChannelId);
            return NULL;
        }
    }
    return hChannel;
}
void print_version_and_useage()
{
    
    BYTE byMaj, byMin;
    WORD wBuild;
    MWGetVersion(&byMaj, &byMin, &wBuild);
    printf("Magewell MWCapture SDK V%d.%d.1.%d - AudioCapture\n",byMaj,byMin,wBuild);
    printf("USB Devices are not supported\n");
    printf("Usage:\n");
    printf("AudioCapture <channel index>\n");
    printf("AudioCapture <board id>:<channel id>\n");

}
bool CheckFile()
{
    FILE* wav_file = NULL;
    wav_file=fopen("temp.wav","wb");
    if (NULL == wav_file){
        printf("ERROR: can't create file on now dir!\n");
        printf("\nPress ENTER to exit...\n");
        getchar();
        return false;
    }
    else{
        fclose(wav_file);
        wav_file = NULL;
        remove("temp.wav");
    }
    return true;
}
int main(int argc, char* argv[])
{
    char is_eco = 0;
    void *status;
    pthread_t audio_capture_tid;
// Version
    print_version_and_useage();
//permission
    if(!CheckFile()){
        return 0;
    }

    if (argc > 2){
        printf("ERROR: Invalid params!\n");
        printf("\nPress 'Enter' to exit!\n");
        getchar();
        return 0;
    }

    if(!MWCaptureInitInstance()){
        printf("have InitilizeFailed");
    }

    do {
        MWCAP_CHANNEL_INFO channel_info;
        MWRefreshDevice();
        g_channel_handle = open_channel(argc, argv);
        if(NULL == g_channel_handle){
            break;
        }
        if (MW_SUCCEEDED != MWGetChannelInfo(g_channel_handle, &channel_info)) {
            printf("ERROR: Can't get channel info!\n");
            break;
        }

        printf("Open channel - BoardIndex = %X, ChannelIndex = %d.\n", channel_info.byBoardIndex, channel_info.byChannelIndex);
        printf("Product Name: %s\n", channel_info.szProductName);
        printf("Board SerialNo: %s\n\n", channel_info.szBoardSerialNo);
        if(0 == strcmp(channel_info.szFamilyName, "Eco Capture")){
            is_eco = 1;
        }
        g_runing = 1;
        pthread_create(&audio_capture_tid, NULL, audio_capture, (void*)is_eco);
        for(int i = 0; i < AUDIO_CAPTURE_SECONDS; i++){
            printf("\rStart capture audio data -- %d s\n", i+1);
            sleep(1);
        }
        g_runing = 0;
        pthread_join(audio_capture_tid, &status);
    } while (FALSE);

    if (g_channel_handle)
        MWCloseChannel(g_channel_handle);

    MWCaptureExitInstance();

    printf("\nPress 'Enter' to exit!\n");
    getchar();

    return 0;
}

