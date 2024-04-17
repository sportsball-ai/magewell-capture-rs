/************************************************************************************************/
// HDMIInfoFrame.cpp : Defines the entry point for the console application.

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

void print_version_and_useage()
{
    
    BYTE byMaj, byMin;
    WORD wBuild;
    MWGetVersion(&byMaj, &byMin, &wBuild);
    printf("Magewell MWCapture SDK V%d.%d.1.%d - HDMIInfoFrame\n",byMaj,byMin,wBuild);
    printf("Only HDMI devices are supported\n");
    printf("Usage:\n");
    printf("HDMIInfoFrame <channel index>\n");
    printf("HDMIInfoFrame <board id>:<channel id>\n\n");

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

int main(int argc, char* argv[])
{
// Version
    print_version_and_useage();
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
        MWRefreshDevice();
        int nChannelCount = MWGetChannelCount();

        if (0 == nChannelCount) {
            printf("ERROR: Can't find channels!\n");
            break;
        }
        printf("Find %d channels.\n", nChannelCount);
        // Get <board id > <channel id> or <channel index>
        int byBoardId = -1;
        int byChannelId = -1;
        int nDevIndex = -1;
        BOOL bIndex = FALSE;

        MWCAP_CHANNEL_INFO videoInfo = { 0 };
        if (argc == 1) {
            bIndex = TRUE;
            nDevIndex = 0;
        }
        else {
            if (NULL == strstr(argv[1], ":")){
                bIndex = TRUE;

                if (strlen(argv[1]) > 2)
                    nDevIndex = -1;
                else if (strlen(argv[1]) == 2){
                    if ((argv[1][0] >= '0' && argv[1][0] <= '9') && (argv[1][1] >= '0' && argv[1][1] <= '9'))
                        nDevIndex = atoi(argv[1]);
                    else
                        nDevIndex = -1;
                }
                else if (strlen(argv[1]) == 1)
                    nDevIndex = (argv[1][0] >= '0' && argv[1][0] <= '9') ? atoi(argv[1]) : -1;

                if (nDevIndex < 0){
                    printf("\nERROR: Invalid params!\n");
                    break;
                }
                if(nDevIndex >= nChannelCount){
                    printf("ERROR: just have %d channel!\n",nChannelCount);
                    break;
                }
            }
            else{
                bIndex = FALSE;

                if (strlen(argv[1]) == 3){
                    if ((argv[1][0] >= '0' && argv[1][0] <= '9') || (argv[1][0] >= 'a' && argv[1][0] <= 'f') || (argv[1][0] >= 'A' && argv[1][0] <= 'F'))
                        byBoardId = get_id(argv[1][0]);//atoi(argv[1]);
                    else
                        byBoardId = -1;

                    if ((argv[1][2] >= '0' && argv[1][2] <= '3'))
                        byChannelId = get_id(argv[1][2]);//atoi(&argv[1][2]);
                    else
                        byChannelId = -1;
                }
                else{
                    byBoardId = -1;
                    byChannelId = -1;
                }

                if (-1 == byBoardId || -1 == byChannelId){
                    printf("\nERROR: Invalid params!\n");
                    break;
                }
            }
        }

        // Open channel
        if (bIndex == TRUE){
            char path[128] = {0};
            MWGetDevicePath(nDevIndex, path);
            hChannel = MWOpenChannelByPath(path);
            if (hChannel == NULL) {
                printf("ERROR: Open channel %d error!\n", nDevIndex);
                break;
            }
        }
        else {
            hChannel = MWOpenChannel(byBoardId, byChannelId);
            if (hChannel == NULL) {
                printf("ERROR: Open channel %X:%d error!\n", byBoardId, byChannelId);
                break;
            }
        }

        if (MW_SUCCEEDED != MWGetChannelInfo(hChannel, &videoInfo)) {
            printf("ERROR: Can't get channel info!\n");
            break;
        }

        printf("Open channel - BoardIndex = %X, ChannelIndex = %d.\n", videoInfo.byBoardIndex, videoInfo.byChannelIndex);
        printf("Product Name: %s\n", videoInfo.szProductName);
        printf("Board SerialNo: %s\n\n", videoInfo.szBoardSerialNo);

        MW_RESULT xr;
        MWCAP_INPUT_SPECIFIC_STATUS status;
        xr = MWGetInputSpecificStatus(hChannel, &status);
        if (xr != MW_SUCCEEDED) {
            printf("ERROR: Get Specific Status error!\n");
            break;
        }
        if (!status.bValid) {
            printf("ERROR: Input signal is invalid!\n");
            break;
        }
        else if (status.dwVideoInputType != 1) {
            printf("ERROR: Input signal is not HDMI!\n");
            break;
        }
        DWORD dwValidFlag = 0;
        xr = MWGetHDMIInfoFrameValidFlag(hChannel, &dwValidFlag);
        if (xr != MW_SUCCEEDED) {
            printf("ERROR: Get HDMI InfoFrame Flag!\n");
            break;
        }
        if (dwValidFlag == 0) {
            printf("No HDMI InfoFrame!\n");
            break;
        }
        HDMI_INFOFRAME_PACKET packet;
        if (dwValidFlag & MWCAP_HDMI_INFOFRAME_MASK_AVI) {
            xr = MWGetHDMIInfoFramePacket(hChannel, MWCAP_HDMI_INFOFRAME_ID_AVI, &packet);
            if (xr == MW_SUCCEEDED) {
                printf("Get HDMI InfoFrame AVI OK!\n");

                printf("Length = %d, Buffer is ", packet.header.byLength);
                for (int i = 0; i < packet.header.byLength; i++) {
                    printf("%02x ", packet.abyPayload[i]);
                }
                printf("\n\n");
            }
        }
        if (dwValidFlag & MWCAP_HDMI_INFOFRAME_MASK_AUDIO) {
            xr = MWGetHDMIInfoFramePacket(hChannel, MWCAP_HDMI_INFOFRAME_ID_AUDIO, &packet);
            if (xr == MW_SUCCEEDED) {
                printf("Get HDMI InfoFrame Audio OK!\n");
                printf("Length = %d, Buffer is ", packet.header.byLength);
                for (int i = 0; i < packet.header.byLength; i++) {
                    printf("%02x ", packet.abyPayload[i]);
                }
                printf("\n\n");
            }
        }
        if (dwValidFlag & MWCAP_HDMI_INFOFRAME_MASK_SPD) {
            xr = MWGetHDMIInfoFramePacket(hChannel, MWCAP_HDMI_INFOFRAME_ID_SPD, &packet);
            if (xr == MW_SUCCEEDED) {
                printf("Get HDMI InfoFrame SPD OK!\n");
                printf("Length = %d, Buffer is ", packet.header.byLength);
                for (int i = 0; i < packet.header.byLength; i++) {
                    printf("%02x ", packet.abyPayload[i]);
                }
                printf("\n\n");
            }
        }
        if (dwValidFlag & MWCAP_HDMI_INFOFRAME_MASK_MS) {
            xr = MWGetHDMIInfoFramePacket(hChannel, MWCAP_HDMI_INFOFRAME_ID_MS, &packet);
            if (xr == MW_SUCCEEDED) {
                printf("Get HDMI InfoFrame MS OK!\n");
                printf("Length = %d, Buffer is ", packet.header.byLength);
                for (int i = 0; i < packet.header.byLength; i++) {
                    printf("%02x ", packet.abyPayload[i]);
                }
                printf("\n\n");
            }
        }
        if (dwValidFlag & MWCAP_HDMI_INFOFRAME_MASK_VS) {
            xr = MWGetHDMIInfoFramePacket(hChannel, MWCAP_HDMI_INFOFRAME_ID_VS, &packet);
            if (xr == MW_SUCCEEDED) {
                printf("Get HDMI InfoFrame VS OK!\n");
                printf("Length = %d, Buffer is ", packet.header.byLength);
                for (int i = 0; i < packet.header.byLength; i++) {
                    printf("%02x ", packet.abyPayload[i]);
                }
                printf("\n\n");
            }
        }
        if (dwValidFlag & MWCAP_HDMI_INFOFRAME_MASK_ACP) {
            xr = MWGetHDMIInfoFramePacket(hChannel, MWCAP_HDMI_INFOFRAME_ID_ACP, &packet);
            if (xr == MW_SUCCEEDED) {
                printf("Get HDMI InfoFrame ACP OK!\n");
                printf("Length = %d, Buffer is ", packet.header.byLength);
                for (int i = 0; i < packet.header.byLength; i++) {
                    printf("%02x ", packet.abyPayload[i]);
                }
                printf("\n\n");
            }
        }
        if (dwValidFlag & MWCAP_HDMI_INFOFRAME_MASK_ISRC1) {
            xr = MWGetHDMIInfoFramePacket(hChannel, MWCAP_HDMI_INFOFRAME_ID_ISRC1, &packet);
            if (xr == MW_SUCCEEDED) {
                printf("Get HDMI InfoFrame ISRC1 OK!\n");
                printf("Length = %d, Buffer is ", packet.header.byLength);
                for (int i = 0; i < packet.header.byLength; i++) {
                    printf("%02x ", packet.abyPayload[i]);
                }
                printf("\n\n");
            }
        }
        if (dwValidFlag & MWCAP_HDMI_INFOFRAME_MASK_ISRC2) {
            xr = MWGetHDMIInfoFramePacket(hChannel, MWCAP_HDMI_INFOFRAME_ID_ISRC2, &packet);
            if (xr == MW_SUCCEEDED) {
                printf("Get HDMI InfoFrame ISRC2 OK!\n");
                printf("Length = %d, Buffer is ", packet.header.byLength);
                for (int i = 0; i < packet.header.byLength; i++) {
                    printf("%02x ", packet.abyPayload[i]);
                }
                printf("\n\n");
            }
        }
        if (dwValidFlag & MWCAP_HDMI_INFOFRAME_MASK_GAMUT) {
            xr = MWGetHDMIInfoFramePacket(hChannel, MWCAP_HDMI_INFOFRAME_ID_GAMUT, &packet);
            if (xr == MW_SUCCEEDED) {
                printf("Get HDMI InfoFrame GAMUT OK!\n");
                printf("Length = %d, Buffer is ", packet.header.byLength);
                for (int i = 0; i < packet.header.byLength; i++) {
                    printf("%02x ", packet.abyPayload[i]);
                }
                printf("\n\n");
            }
        }
        if (dwValidFlag & MWCAP_HDMI_INFOFRAME_MASK_HDR) {
            xr = MWGetHDMIInfoFramePacket(hChannel, MWCAP_HDMI_INFOFRAME_ID_HDR, &packet);
            if (xr == MW_SUCCEEDED) {
                printf("Get HDMI InfoFrame HDR OK!\n");
                printf("Length = %d, Buffer is ", packet.header.byLength);
                for (int i = 0; i < packet.header.byLength; i++) {
                    printf("%02x ", packet.abyPayload[i]);
                }
                printf("\n\n");
            }
        }
    } while (FALSE);

    if (hChannel != NULL)
        MWCloseChannel(hChannel);

    MWCaptureExitInstance();

    printf("\nPress 'Enter' to exit!\n");
    getchar();

    return 0;
}

