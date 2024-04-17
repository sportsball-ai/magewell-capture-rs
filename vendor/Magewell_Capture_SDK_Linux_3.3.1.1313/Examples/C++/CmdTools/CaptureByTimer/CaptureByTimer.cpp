/************************************************************************************************/
// CaptureByTimer.cpp : Defines the entry point for the console application.

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

#include <stdio.h>
#include <string.h>

#include "LibMWCapture/MWCapture.h"
#include "MWFOURCC.h"

#include "bitmap.h"

#define NUM_CAPTURE		100
#define TIMER_DURATION	400000LL	// 25 fps

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
        if (0 == strcmp(info.szFamilyName, "Pro Capture")){
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

    MWCAP_CHANNEL_INFO videoInfo = { 0 };
    if (argc == 1) {
        if (MW_SUCCEEDED != MWGetChannelInfoByIndex(nProDevChannel[0], &videoInfo)) {
            printf("ERROR: Can't get channel info!\n");
            return NULL;
        }

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

    if (MW_SUCCEEDED != MWGetChannelInfo(hChannel, &videoInfo)) {
        printf("ERROR: Can't get channel info!\n");
        return NULL;
    }

    printf("Open channel - BoardIndex = %X, ChannelIndex = %d.\n", videoInfo.byBoardIndex, videoInfo.byChannelIndex);
    printf("Product Name: %s\n", videoInfo.szProductName);
    printf("Board SerialNo: %s\n\n", videoInfo.szBoardSerialNo);
    return hChannel;
}
void print_version_and_useage()
{
    
    BYTE byMaj, byMin;
    WORD wBuild;
    MWGetVersion(&byMaj, &byMin, &wBuild);
    printf("Magewell MWCapture SDK V%d.%d.1.%d - CaptureByTimer\n",byMaj,byMin,wBuild);
    printf("Only Pro Capture Series Devices are supported\n");
    printf("Usage:\n");
    printf("CaptureByTimer <channel index>\n");
    printf("CaptureByTimer <board id>:<channel id>\n\n");

}
bool CheckFile()
{
    FILE* wavFile = NULL;
    wavFile=fopen("temp.bmp","wb");
    if (NULL == wavFile){
        printf("ERROR: can't create file on now dir!\n");
        printf("\nPress ENTER to exit...\n");
        getchar();
        return false;
    }
    else{
        fclose(wavFile);
        wavFile = NULL;
        remove("temp.bmp");
    }
    return true;
}


int main(int argc, char* argv[])
{
// Version
    print_version_and_useage();
//permission
    if(!CheckFile()){
        printf("\nPress 'Enter' to exit!\n");
        getchar();
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

    HCHANNEL hChannel = NULL;
    MWCAP_PTR hNotifyEvent=0;
    MWCAP_PTR hCaptureEvent=0;

    HANDLE64 pbImage=0;//Image Data pointer

    int nRet;

    do {
        MWRefreshDevice();
        hChannel = open_channel(argc, argv);
        if(NULL == hChannel){
            break;
        }
        // Capture frames on input signal frequency
        hCaptureEvent=MWCreateEvent();
        if(hCaptureEvent==0){
            printf("ERROR: Create timer event error\n");
            break;
        }
        hNotifyEvent=MWCreateEvent();
        if(hNotifyEvent==0){
            printf("ERROR: Create notify event error\n");
            break;
        }

        int cx=1920;
        int cy=1080;

        DWORD dwFourcc=MWFOURCC_BGR24;
        DWORD dwMinStride=FOURCC_CalcMinStride(dwFourcc,cx,4);
        DWORD dwImageSize=FOURCC_CalcImageSize(dwFourcc,cx,cy,dwMinStride);
        pbImage=(HANDLE64)(unsigned long)malloc(dwImageSize);

        MW_RESULT xr;
        xr = MWStartVideoCapture(hChannel, hCaptureEvent);
        if (xr != MW_SUCCEEDED) {
            printf("ERROR: Start Capture error!\n");
            break;
        } 
        HTIMER hTimerEvent = MWRegisterTimer(hChannel, hNotifyEvent);

        printf("Begin capture %d frames in 1920 x 1080, RGB32, %d fps...\n", NUM_CAPTURE, 10000000 / TIMER_DURATION);
        MWPinVideoBuffer(hChannel, (MWCAP_PTR)pbImage, dwImageSize);

        LONGLONG llBegin = 0LL;
        xr = MWGetDeviceTime(hChannel, &llBegin);
        int i;
        for (i = 0; i < NUM_CAPTURE; i++) {
            xr = MWScheduleTimer(hChannel, hTimerEvent, llBegin + i * TIMER_DURATION);
            if (xr != MW_SUCCEEDED)
                continue;

            nRet=MWWaitEvent(hNotifyEvent,1000);
            if(nRet<=0){
                printf("Error:wait notify error or timeout\n");
                break;
            }

            MWCAP_VIDEO_BUFFER_INFO videoBufferInfo;
            xr = MWGetVideoBufferInfo(hChannel, &videoBufferInfo);
            if (xr != MW_SUCCEEDED)
                continue;

            MWCAP_VIDEO_FRAME_INFO videoFrameInfo;
            xr = MWGetVideoFrameInfo(hChannel, videoBufferInfo.iNewestBufferedFullFrame, &videoFrameInfo);
            if (xr != MW_SUCCEEDED)
                continue;

            xr = MWCaptureVideoFrameToVirtualAddressEx(hChannel,
                                                       -1,(unsigned char *)pbImage,dwImageSize,dwMinStride,
                                                       1,0,dwFourcc,cx,cy,0,0,0,0,0,100,0,100,0,MWCAP_VIDEO_DEINTERLACE_BLEND,
                                                       MWCAP_VIDEO_ASPECT_RATIO_CROPPING,0,0,0,0,MWCAP_VIDEO_COLOR_FORMAT_UNKNOWN,
                                                       MWCAP_VIDEO_QUANTIZATION_UNKNOWN,MWCAP_VIDEO_SATURATION_UNKNOWN);
            if(xr!=MW_SUCCEEDED)
                continue;

            nRet=MWWaitEvent(hCaptureEvent,1000);
            if(nRet<=0){
                printf("Error:wait capture event error or timeout\n");
                break;
            }

            MWCAP_VIDEO_CAPTURE_STATUS captureStatus;
            xr = MWGetVideoCaptureStatus(hChannel, &captureStatus);

            char szBitmapName[64];
            sprintf(szBitmapName,"./video_%d.BMP", NUM_CAPTURE);
            if(i==NUM_CAPTURE-1)
                create_bitmap(szBitmapName,pbImage,dwImageSize,cx,cy);
        }
        MWUnpinVideoBuffer(hChannel, (LPBYTE)pbImage);
        LONGLONG llEnd = 0LL;
        xr = MWGetDeviceTime(hChannel, &llEnd);
        if(i >= NUM_CAPTURE){
            printf("\nStop capture\n");
            printf("Each frame average duration is %d (100 ns).\n", (LONG)((llEnd - llBegin) / NUM_CAPTURE));
            printf("Write 100th frame to file Video_%d.BMP\n", NUM_CAPTURE);

        } 
        xr = MWUnregisterTimer(hChannel, hTimerEvent);
        hTimerEvent=0;
        xr = MWStopVideoCapture(hChannel); 
    } while (FALSE);

    if(hNotifyEvent!=0){
        MWCloseEvent(hNotifyEvent);
        hNotifyEvent=0;
    }

    if(hCaptureEvent!=0){
        MWCloseEvent(hCaptureEvent);
        hCaptureEvent=0;
    }

    if (hChannel != NULL){
        MWCloseChannel(hChannel);
    }
    if(pbImage!=0){
        free((void*)(unsigned long)pbImage);
        pbImage=0;
    }

    MWCaptureExitInstance();

    printf("\nPress 'Enter' to exit!\n");
    getchar();

    return 0;
}
