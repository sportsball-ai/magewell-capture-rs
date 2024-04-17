/************************************************************************************************/
// USBCapture.cpp : Defines the entry point for the console application.

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
#include <stdio.h>
#include <string.h>
#include <string>

#include "MWFOURCC.h"
#include "LibMWCapture/MWCapture.h"
#include "WaveFile.h"
#include "bitmap.h"

using namespace std;

#define  CAPTURE_WIDTH	1920
#define  CAPTURE_HEIGHT	1080
#define  FRAME_DURATION 400000

int g_nFrameCount = 0;
int g_nAudioCount = 0;
CWaveFile g_audioFile;

void OnVideoCaptureCallback(BYTE *pBuffer, long BufferLen, void* pParam)
{
    g_nFrameCount++;
    if (g_nFrameCount == 100){
        create_bitmap("USBVideoCapture.bmp",(unsigned long)pBuffer,BufferLen, CAPTURE_WIDTH, CAPTURE_HEIGHT);
        printf("Write last frame to USBVideoCapture.bmp.\n");
    }
}

void OnAudioCaptureCallback(const BYTE * pbFrame, int cbFrame, uint64_t u64TimeStamp, void* pParam)
{
    g_nAudioCount++;
    bool bRet = false;
    if (g_nAudioCount <= 500)
        bRet = g_audioFile.Write(pbFrame, /*480 * sizeof(short) * 2*/cbFrame);
    else if (g_nAudioCount == 501){
        g_audioFile.Exit();
        printf("Save audio file to USBAudioCapture.wav\n");
    }
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

void print_version_and_useage()
{
    
    BYTE byMaj, byMin;
    WORD wBuild;
    MWGetVersion(&byMaj, &byMin, &wBuild);
    printf("Magewell MWCapture SDK V%d.%d.1.%d - USBCapture\n",byMaj,byMin,wBuild);
    printf("Only USB devices are supported\n");
    printf("Usage:\n");
    printf("USBCapture\n");
}

HCHANNEL open_channel(int argc, char* argv[])
{
    HCHANNEL hChannel = NULL;
    int nCount = MWGetChannelCount();
    if (nCount <= 0){
        printf("ERROR: Can't find channels!\n");
        return NULL;
    }
    printf("Log: Find %d channels!\n",nCount);
    MW_RESULT t_mr;
    int t_nIndex=-1;
    for(int i=0; i<nCount; i++){
        MWCAP_CHANNEL_INFO t_info;
    
        if(MWGetChannelInfoByIndex(i,&t_info)==MW_SUCCEEDED){
            printf("%d:%s\n",i,t_info.szProductName);
            if(strstr(t_info.szProductName,"USB")!=0){
                t_nIndex=i;
            }
        }
    }
    
    if(t_nIndex==-1){
        printf("ERROR: Please insert usb device!\n");
        return NULL;
    }
    
    char wPath[256] = {0};
    MW_RESULT mr = MWGetDevicePath(t_nIndex, wPath);
    
    hChannel = MWOpenChannelByPath(wPath);
    if (hChannel == NULL){
        printf("ERROR: Open channel failed !\n");
        return NULL;
    }
    
    MWCAP_CHANNEL_INFO info = {0};
    mr = MWGetChannelInfo(hChannel, &info);
    
    char wProductName[256] = {0};
    memcpy(wProductName,info.szProductName,strlen(info.szProductName));
    
    if (strstr(wProductName, "USB") == 0){
        printf("ERROR: Please insert usb device!\n");
        if (hChannel != NULL){
            MWCloseChannel(hChannel);
            hChannel = NULL;
        }
        return NULL;
    }
    printf("Open channel - BoardIndex = %X, ChannelIndex = %d.\n", info.byBoardIndex, info.byChannelIndex);
    printf("Product Name: %s\n", info.szProductName);
    printf("Board SerialNo: %s\n\n", info.szBoardSerialNo);
    return hChannel;
}


int main(int argc, char* argv[])
{
    HCHANNEL hChannel = NULL;
    print_version_and_useage();

    MWCaptureInitInstance();
    MWRefreshDevice();
    do
    {    
        if(!CheckFile()){
            printf("\nPress 'Enter' to exit!\n");
            break;
        }

        hChannel = open_channel(argc, argv);
        if(NULL == hChannel){
            break;
        }
        HANDLE hAudio=MWCreateAudioCapture(hChannel,MWCAP_AUDIO_CAPTURE_NODE_DEFAULT,48000,16,2,OnAudioCaptureCallback,NULL);
        if (hAudio == NULL)
            break;

        g_audioFile.Init("USBAudioCapture.wav", 48000, 2, 16);
        bool bRet = g_audioFile.IsOpen();
        printf("Create audio capture !\n");

        HANDLE hVideo = MWCreateVideoCapture(hChannel, CAPTURE_WIDTH, CAPTURE_HEIGHT, MWFOURCC_BGR24, FRAME_DURATION, OnVideoCaptureCallback, NULL);
        if (hVideo == NULL)
            break;

        printf("Create video capture !\n");
        while(g_nFrameCount <= 100 || g_nAudioCount <= 500){
            ;
        }

        if (g_nFrameCount > 100){
            if (hVideo != NULL){
                MWDestoryVideoCapture(hVideo);
                hVideo = NULL;
            }
        }

        if (g_nAudioCount > 500){
            if (hAudio != NULL){
                MWDestoryAudioCapture(hAudio);
                hAudio = NULL;
            }
        }

    } while (FALSE);

    if (hChannel != NULL)
    {
        MWCloseChannel(hChannel);
        hChannel = NULL;
    }

    MWCaptureExitInstance();

    printf("\nPress any key to exit!\n");
    getchar();
    return 0;
}

