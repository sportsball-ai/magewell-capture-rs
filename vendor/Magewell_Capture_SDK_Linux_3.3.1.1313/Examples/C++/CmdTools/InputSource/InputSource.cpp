/************************************************************************************************/
// InputSource.cpp : Defines the entry point for the console application.

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

#include "LibMWCapture/MWCapture.h"

void GetVideoInputName(DWORD dwVideoInput, char* pszName, int nSize)
{
    switch (INPUT_TYPE(dwVideoInput)) {
    case MWCAP_VIDEO_INPUT_TYPE_NONE:
        strncpy(pszName, "None",nSize);
        break;
    case MWCAP_VIDEO_INPUT_TYPE_HDMI:
        strncpy(pszName, "HDMI", nSize);
        break;
    case MWCAP_VIDEO_INPUT_TYPE_VGA:
        strncpy(pszName, "VGA", nSize);
        break;
    case MWCAP_VIDEO_INPUT_TYPE_SDI:
        strncpy(pszName, "SDI",nSize);
        break;
    case MWCAP_VIDEO_INPUT_TYPE_COMPONENT:
        strncpy(pszName, "Component", nSize);
        break;
    case MWCAP_VIDEO_INPUT_TYPE_CVBS:
        strncpy(pszName, "CVBS",nSize);
        break;
    case MWCAP_VIDEO_INPUT_TYPE_YC:
        strncpy(pszName, "YC",nSize);
        break;
    }
}

void GetAudioInputName(DWORD dwAudioInput, char* pszName, int nSize)
{
    switch (INPUT_TYPE(dwAudioInput)) {
    case MWCAP_AUDIO_INPUT_TYPE_NONE:
        strncpy(pszName, "None",nSize);
        break;
    case MWCAP_AUDIO_INPUT_TYPE_HDMI:
        strncpy(pszName, "HDMI",nSize);
        break;
    case MWCAP_AUDIO_INPUT_TYPE_SDI:
        strncpy(pszName, "SDI",nSize);
        break;
    case MWCAP_AUDIO_INPUT_TYPE_LINE_IN:
        strncpy(pszName, "Line In", nSize);
        break;
    case MWCAP_AUDIO_INPUT_TYPE_MIC_IN:
        strncpy(pszName, "Mic In",nSize);
        break;
    }
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
    BYTE byMaj, byMin;
    WORD wBuild;
    MWGetVersion(&byMaj, &byMin, &wBuild);
    printf("Magewell MWCapture SDK V%d.%d.1.%d - InputSource\n",byMaj,byMin,wBuild);
    printf("Usage:\n");
    printf("InputSource <channel index>\n");
    printf("InputSource <board id>:<channel id>\n\n");

    if (argc > 2)
    {
        printf("ERROR: Invalid params!\n");
        printf("\nPress 'Enter' to exit!\n");
        getchar();
        return 0;
    }

    if(!MWCaptureInitInstance())
    {
        printf("have InitilizeFailed\n");
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
            else {
                bIndex = FALSE;

                if (strlen(argv[1]) == 3) {
                    if ((argv[1][0] >= '0' && argv[1][0] <= '9') || (argv[1][0] >= 'a' && argv[1][0] <= 'f') || (argv[1][0] >= 'A' && argv[1][0] <= 'F'))
                        byBoardId = get_id(argv[1][0]);//atoi(argv[1]);
                    else
                        byBoardId = -1;

                    if ((argv[1][2] >= '0' && argv[1][2] <= '3'))
                        byChannelId = get_id(argv[1][2]);//atoi(&argv[1][2]);
                    else
                        byChannelId = -1;
                }
                else {
                    byBoardId = -1;
                    byChannelId = -1;
                }

                if (-1 == byBoardId || -1 == byChannelId) {
                    printf("\nERROR: Invalid params!\n");
                    break;
                }
            }
        }

        // Open channel
        if (bIndex == TRUE) {
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

        // Video Input Source
        DWORD dwVideoInputCount = 0;
        xr = MWGetVideoInputSourceArray(hChannel, NULL, &dwVideoInputCount);
        if (xr == MW_SUCCEEDED && dwVideoInputCount > 0) {
            printf("Video Input Count : %d\n", dwVideoInputCount);

            DWORD* pVideoInput = new DWORD[dwVideoInputCount];
            xr = MWGetVideoInputSourceArray(hChannel, pVideoInput, &dwVideoInputCount);
            if (xr == MW_SUCCEEDED) {
                char szInputName[16] = { 0 };
                for (DWORD i = 0; i < dwVideoInputCount; i++) {
                    GetVideoInputName(pVideoInput[i], szInputName, 16);
                    printf("[%d] %s\n", i, szInputName);
                }
            }
            delete[] pVideoInput;
        }

        DWORD dwVideoInput = 0;
        xr = MWGetVideoInputSource(hChannel, &dwVideoInput);
        if (xr == MW_SUCCEEDED) {
            char szInputName[16] = { 0 };
            GetVideoInputName(dwVideoInput, szInputName, 16);
            printf("Current Video Input Source: %s\n\n", szInputName);
        }

        // Audio Input Source
        DWORD dwAudioInputCount = 0;
        xr = MWGetAudioInputSourceArray(hChannel, NULL, &dwAudioInputCount);
        if (xr == MW_SUCCEEDED && dwAudioInputCount > 0) {
            printf("Audio Input Count : %d\n", dwAudioInputCount);

            DWORD* pAudioInput = new DWORD[dwAudioInputCount];
            xr = MWGetAudioInputSourceArray(hChannel, pAudioInput, &dwAudioInputCount);
            if (xr == MW_SUCCEEDED) {
                char szInputName[16] = { 0 };
                for (DWORD i = 0; i < dwAudioInputCount; i++) {
                    GetAudioInputName(pAudioInput[i], szInputName, 16);
                    printf("[%d] %s\n", i, szInputName);
                }
            }
            delete[] pAudioInput;
        }

        DWORD dwAudioInput = 0;
        xr = MWGetAudioInputSource(hChannel, &dwAudioInput);
        if (xr == MW_SUCCEEDED) {
            char szInputName[16] = { 0 };
            GetAudioInputName(dwAudioInput, szInputName, 16);
            printf("Current Audio Input Source: %s\n", szInputName);
        }

    } while (FALSE);

    if (hChannel != NULL)
        MWCloseChannel(hChannel);

    MWCaptureExitInstance();

    printf("\nPress 'Enter' to exit!\n");
    getchar();

    return 0;
}

