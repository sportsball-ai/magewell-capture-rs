/************************************************************************************************/
// GetANCPacket.cpp : Defines the entry point for the console application.

// MAGEWELL PROPRIETARY INFORMATION

// The following license only applies to head files and library within Magewell's SDK 
// and not to Magewell's SDK as a whole. 

// Copyrights ?Nanjing Magewell Electronics Co., Ltd. ("Magewell") All rights reserved.

// Magewell grands to any person who obtains the copy of Magewell's head files and library 
// the rights,including without limitation, to use, modify, publish, sublicense, distribute
// the Software on the conditions that all the following terms are met:
// - The above copyright notice shall be retained in any circumstances.
// -The following disclaimer shall be included in the software and documentation and/or 
// other materials provided for the purpose of publish, distribution or sublicense.

// THE SOFTWARE IS PROVIDED BY MAGEWELL "AS IS" AND ANY EXPRESS, INCLUDING BUT NOT LIMITED TO,
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
// IN NO EVENT SHALL MAGEWELL BE LIABLE 

// FOR ANY CLAIM, DIRECT OR INDIRECT DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT,
// TORT OR OTHERWISE, ARISING IN ANY WAY OF USING THE SOFTWARE.

// CONTACT INFORMATION:
// SDK@magewell.net
// http://www.magewell.com/
//
/************************************************************************************************/
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/eventfd.h>
#include <sys/select.h>
#include <unistd.h>
#include <string.h>
#include "MWFOURCC.h"
#include "LibMWCapture/MWCapture.h"
#include "LibMWCapture/MWEcoCapture.h"
typedef int mw_event_t;

void print_version_and_useage()
{
    
    BYTE byMaj, byMin;
    WORD wBuild;
    MWGetVersion(&byMaj, &byMin, &wBuild);
    printf("Magewell MWCapture SDK V%d.%d.1.%d - GetANCPacket\n",byMaj,byMin,wBuild);
    printf("Only SDI devices are supported, need Closed Caption signal\n");
    printf("Usage:\n");
    printf("GetANCPacket <channel index>\n");
    printf("GetANCPacket <board id>:<channel id>\n\n");
}
bool CheckFile()
{
    FILE* wavFile = NULL;
    wavFile=fopen("temp.bin","wb");
    if (NULL == wavFile){
        printf("ERROR: can't create file on now dir!\n");
        printf("\nPress ENTER to exit...\n");
        getchar();
        return false;
    }
    else{
        fclose(wavFile);
        wavFile = NULL;
        remove("temp.bin");
    }
    return true;
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
        if (strstr(info.szProductName, "SDI")){
            printf("find %s\n",info.szProductName);
            nProDevChannel[nProDevCount] = i;
            nProDevCount++;
        }
    }
    if (nProDevCount <= 0){
        printf("\nERROR: Can't find SDI channels!\n");
        return NULL;
    }

    printf("Find %d SDI channels.\n", nProDevCount);

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
    return hChannel;
}

int EcoEventWait(mw_event_t event, int timeout/*ms*/)
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

int main(int argc, char *argv[])
{

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

    int nIndex = 0;
    HCHANNEL hChannel = NULL;

    do {
        bool bProDevice = true;
        MWRefreshDevice();
        hChannel = open_channel(argc, argv);
        if(NULL == hChannel){
            break;
        }
        MWCAP_CHANNEL_INFO videoInfo = { 0 };
        if (MW_SUCCEEDED != MWGetChannelInfo(hChannel, &videoInfo)) {
            printf("ERROR: Can't get channel info!\n");
            break;
        }
        printf("Open channel - BoardIndex = %X, ChannelIndex = %d.\n", videoInfo.byBoardIndex, videoInfo.byChannelIndex);
        printf("Product Name: %s\n", videoInfo.szProductName);
        printf("Board SerialNo: %s\n\n", videoInfo.szBoardSerialNo);
        if(strcmp(videoInfo.szFamilyName, "Pro Capture")){
            bProDevice = false;
        }
        int iCCNum = 2;
        MW_RESULT ret = MWCaptureSetSDIANCType(
                    hChannel,
                    0 /*byIndex*/,
                    FALSE /*bHANC*/,
                    TRUE /*bVANC*/,
                    0x61 /*byDID*/,
                    0x01 /*bySDID*/);
        if(MW_SUCCEEDED != ret){
            iCCNum--;
        }
        ret = MWCaptureSetSDIANCType(
                    hChannel,
                    1 /*byIndex*/,
                    FALSE /*bHANC*/,
                    TRUE /*bVANC*/,
                    0x61 /*byDID*/,
                    0x02 /*bySDID*/);
        if(MW_SUCCEEDED != ret){
            iCCNum--;
        }
        if(iCCNum == 0){
            printf("ERROR: may not support anc\n");
            break;
        }
        // Clear old packets
        MWCAP_SDI_ANC_PACKET ancPacket = { 0 };
        do {
            ret = MWCaptureGetSDIANCPacket(hChannel, &ancPacket);
        } while (ret == MW_SUCCEEDED && ancPacket.byDID != 0);

        MWHANDLE hNotifyEvent;
        if(bProDevice){
            hNotifyEvent = (MWHANDLE)MWCreateEvent();
        }
        else{
            hNotifyEvent = (MWHANDLE)eventfd(0, EFD_NONBLOCK);
        }
        HNOTIFY hNotify = MWRegisterNotify(hChannel, hNotifyEvent, MWCAP_NOTIFY_NEW_SDI_ANC_PACKET);
        FILE * fp = fopen("anc_data.bin", "wb");
        int iDataLen = 0;
        for (int i = 0; i < 600; i++) {
            if(bProDevice){
                MWWaitEvent(hNotifyEvent, 100);
            }
            else{
                EcoEventWait(hNotifyEvent, 100);
            }


            ULONGLONG ullStatusBits;
            ret = MWGetNotifyStatus(hChannel, hNotify, &ullStatusBits);

            if (!(ullStatusBits & MWCAP_NOTIFY_NEW_SDI_ANC_PACKET)) {
                continue;
            }
            while(1) {
                ret = MWCaptureGetSDIANCPacket(hChannel, &ancPacket);
                if (ret != MW_SUCCEEDED || 0 == ancPacket.byDID) {
                    break;
                }
                if(fp)
                    fwrite(&ancPacket, ancPacket.byDC + 3, 1, fp);
                iDataLen += ancPacket.byDC + 3;
                printf("GetSDIANCPacket[%d], DID[0x%02X], SDID[0x%02X], Length[%d]\n", i, ancPacket.byDID, ancPacket.bySDID, ancPacket.byDC);
            }
        }
        if(fp)
            fclose(fp);
        if(iDataLen == 0){
            printf("not have Closed Caption signal\n");
        }
        MWUnregisterNotify(hChannel, hNotify);
        if(bProDevice){
            MWCloseEvent(hNotifyEvent);
        }
        else{
            eventfd_write(hNotifyEvent, 1);
            close(hNotifyEvent);
        }

    }while(FALSE);
    if(hChannel)
        MWCloseChannel(hChannel);
    MWCaptureExitInstance();

    printf("\nPress 'Enter' to exit!\n");
    getchar();

    return 0;
}
