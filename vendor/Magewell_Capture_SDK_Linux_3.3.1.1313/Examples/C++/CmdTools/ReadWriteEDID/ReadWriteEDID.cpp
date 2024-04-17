/************************************************************************************************/
// ReadWriteEDID.cpp : Defines the entry point for the console application.

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
#include <string>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

using namespace std;

#include "LibMWCapture/MWCapture.h"

BOOL Getfilesuffix(const char* pFilePath)
{
    string str(pFilePath);
    str = str.substr(str.find_last_of('\\') + 1);
    str = str.substr(str.find_first_of('.') + 1);

    if (0 != strcmp("bin", str.c_str()))
        return FALSE;

    return TRUE;
}

void FileFailed(FILE* pFile)
{
    fclose(pFile);
    pFile = NULL;

    printf("ERROR: unknown EDID file!\n");
    printf("\nPress ENTER to exit...\n");
    getchar();
}
bool CheckFile()
{
    FILE* testFile = NULL;
    testFile=fopen("temp.bin","wb");
    if (NULL == testFile){
        printf("ERROR: can't create file on now dir!\n");
        printf("\nPress ENTER to exit...\n");
        getchar();
        return false;
    }
    else{
        fclose(testFile);
        testFile = NULL;
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

int main(int argc,char* argv[])
{
    // Version
    BYTE byMaj, byMin;
    WORD wBuild;
    MWGetVersion(&byMaj, &byMin, &wBuild);
    printf("Magewell MWCapture SDK V%d.%d.1.%d - ReadWriteEDID\n",byMaj,byMin,wBuild);
    printf("Usage:\n");
    printf("Read EDID: ReadWriteEDID <channel index>\n");
    printf("Write EDID: ReadWriteEDID <channel index> <EDID file full path>\n");
    printf("Read EDID: ReadWriteEDID <board id>:<channel id>\n");
    printf("Write EDID: ReadWriteEDID <board id>:<channel id> <EDID file full path>\n\n");

    //permission
    if (!CheckFile()){
        printf("\nPress ENTER to exit...\n");
        getchar();
        return 1;
    }
    //input params
    if (argc > 3) {
        printf("ERROR: Invalid params!\n");
        printf("\nPress ENTER to exit...\n");
        getchar();
        return -1;
    }

    BOOL bWriteMode = FALSE;
    if (argc == 3) {
        if (0 != (access(argv[2], 4)) || FALSE == Getfilesuffix(argv[2])) {
            printf("ERROR: unknown EDID file!\n");
            return -1;
        }

        bWriteMode = TRUE;

        FILE* pFile = NULL;
        pFile=fopen(argv[2],"rb");
        if (pFile == NULL){
            FileFailed(pFile);
            return -1;
        }

        BYTE byData = 0;
        int nCount = fread(&byData, 1, 1, pFile);
        if (nCount != 1 || byData != 0x00){
            FileFailed(pFile);
            return -1;
        }

        for (int i = 0; i < 6; i++){
            nCount = fread(&byData, 1, 1, pFile);
            if (nCount != 1 || byData != 0xff){
                FileFailed(pFile);
                return -1;
            }
        }

        nCount = fread(&byData, 1, 1, pFile);
        if (nCount != 1 || byData != 0x00){
            FileFailed(pFile);
            return -1;
        }

        fclose(pFile);
        pFile = NULL;
    }

    if(!MWCaptureInitInstance()){
        printf("InitilizeFailed\n");
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
            else
            {
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
        else
        {
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

        DWORD dwVideoSource = 0;
        DWORD dwAudioSource = 0;
        if (MW_SUCCEEDED != MWGetVideoInputSource(hChannel, &dwVideoSource)) {
            printf("ERROR: Can't get video input source!\n");
            break;
        }
        if (MW_SUCCEEDED != MWGetAudioInputSource(hChannel, &dwAudioSource)) {
            printf("ERROR: Can't get audio input source!\n");
            break;
        }
        if (INPUT_TYPE(dwVideoSource) != MWCAP_VIDEO_INPUT_TYPE_HDMI || INPUT_TYPE(dwAudioSource) != MWCAP_AUDIO_INPUT_TYPE_HDMI) {
            printf("Type of input source is not HDMI !\n");
            break;
        }

        MW_RESULT xr;

        if (bWriteMode) {
            FILE * pFile = NULL;
            pFile=fopen(argv[2], "rb");
            if (pFile == NULL) {
                printf("ERROR: Read EDID file!\n");
                break;
            }
            BYTE byData[1024];
            int nSize = (int)fread(byData, 1, 1024, pFile);

            xr = MWSetEDID(hChannel, byData, nSize);
            if (xr == MW_SUCCEEDED) {
                printf("Set EDID succeeded!\n");
            }
            else {
                printf("ERROR: Set EDID!\n");
            }

            fclose(pFile);
            pFile = NULL;
        }
        else {
            FILE * pFile = NULL;
            pFile=fopen("ReadWriteEDID.bin", "wb");
            if (pFile == NULL) {
                printf("ERROR: Open ReadWriteEDID.bin!\n");
                break;
            }
            ULONG ulSize = 256;
            BYTE byData[256];
            xr = MWGetEDID(hChannel, byData, &ulSize);
            if (xr == MW_SUCCEEDED) {
                int nWriteSize = (int)fwrite(byData, 1, 256, pFile);
                if (nWriteSize == ulSize) {
                    printf("Write EDID to ReadWriteEDID.bin OK!\n");
                }
                else {
                    printf("ERROR: Write ReadWriteEDID.bin!\n");
                }
            }
            else {
                printf("ERROR: Get EDID Info!\n");
            }

            fclose(pFile);
            pFile = NULL;
        }

    } while (FALSE);

    if (hChannel != NULL)
        MWCloseChannel(hChannel);

    MWCaptureExitInstance();

    printf("\nPress 'Enter' to exit!\n");
    getchar();

    return 0;
}

