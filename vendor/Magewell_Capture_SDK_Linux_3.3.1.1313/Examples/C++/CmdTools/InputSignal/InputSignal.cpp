/************************************************************************************************/
// InputSignal.cpp : Defines the entry point for the console application.

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

void GetVideoColorName(MWCAP_VIDEO_COLOR_FORMAT color, char* pszName, int nSize)
{
    switch (color) {
    case MWCAP_VIDEO_COLOR_FORMAT_UNKNOWN:
        strncpy(pszName, "Unknown", nSize);
        break;
    case MWCAP_VIDEO_COLOR_FORMAT_RGB:
        strncpy(pszName,"RGB",nSize);
        break;
    case MWCAP_VIDEO_COLOR_FORMAT_YUV601:
        strncpy(pszName, "YUV BT.601",nSize );
        break;
    case MWCAP_VIDEO_COLOR_FORMAT_YUV709:
        strncpy(pszName, "YUV BT.709", nSize);
        break;
    case MWCAP_VIDEO_COLOR_FORMAT_YUV2020:
        strncpy(pszName, "YUV BT.2020",nSize);
        break;
    case MWCAP_VIDEO_COLOR_FORMAT_YUV2020C:
        strncpy(pszName, "YUV BT.2020C", nSize);
        break;
    default:
        strncpy(pszName, "Unknown",nSize);
        break;
    }
}

//sdi
void GetVideoSDIType(SDI_TYPE type, char* pTypeName)
{
    switch (type) {
    case SDI_TYPE_SD:
        strcpy(pTypeName, "SD");
        break;
    case SDI_TYPE_HD:
        strcpy(pTypeName, "HD");
        break;
    case SDI_TYPE_3GA:
        strcpy(pTypeName, "3GA");
        break;
    case SDI_TYPE_3GB_DL:
        strcpy(pTypeName, "3GB_DL");
        break;
    case SDI_TYPE_3GB_DS:
        strcpy(pTypeName, "3GB_DS");
        break;
    case SDI_TYPE_DL_CH1:
        strcpy(pTypeName, "DL_CH1");
        break;
    case SDI_TYPE_DL_CH2:
        strcpy(pTypeName, "DL_CH2");
        break;
    case SDI_TYPE_6G_MODE1:
        strcpy(pTypeName, "6G_MODE1");
        break;
    case SDI_TYPE_6G_MODE2:
        strcpy(pTypeName, "6G_MODE2");
        break;
    default:
        strcpy(pTypeName, "Unknown");
        break;
    }
}

void GetVideoScanFmt(SDI_SCANNING_FORMAT type, char* pFmtName)
{
    switch (type) {
    case SDI_SCANING_INTERLACED:
        strcpy(pFmtName, "INTERLACED");
        break;
    case SDI_SCANING_SEGMENTED_FRAME:
        strcpy(pFmtName, "SEGMENTED_FRAME");
        break;
    case SDI_SCANING_PROGRESSIVE:
        strcpy(pFmtName, "PROGRESSIVE");
        break;
    default:
        strcpy(pFmtName, "Unknown");
        break;
    }
}

void GetVideoSamplingStruct(SDI_SAMPLING_STRUCT type, char* pStructName)
{
    switch (type) {
    case SDI_SAMPLING_422_YCbCr:
        strcpy(pStructName, "422_YCbCr");
        break;
    case SDI_SAMPLING_444_YCbCr:
        strcpy(pStructName, "444_YCbCr");
        break;
    case SDI_SAMPLING_444_RGB:
        strcpy(pStructName, "444_RGB");
        break;
    case SDI_SAMPLING_420_YCbCr:
        strcpy(pStructName, "420_YCbCr");
        break;
    case SDI_SAMPLING_4224_YCbCrA:
        strcpy(pStructName, "4224_YCbCrA");
        break;
    case SDI_SAMPLING_4444_YCbCrA:
        strcpy(pStructName, "4444_YCbCrA");
        break;
    case SDI_SAMPLING_4444_RGBA:
        strcpy(pStructName, "4444_RGBA");
        break;
    case SDI_SAMPLING_4224_YCbCrD:
        strcpy(pStructName, "4224_YCbCrD");
        break;
    case SDI_SAMPLING_4444_YCbCrD:
        strcpy(pStructName, "4444_YCbCrD");
        break;
    case SDI_SAMPLING_4444_RGBD:
        strcpy(pStructName, "4444_RGBD");
        break;
    case SDI_SAMPLING_444_XYZ:
        strcpy(pStructName, "444_XYZ");
        break;
    default:
        strcpy(pStructName, "Unknown");
        break;
    }
}

void GetVideoBitDepth(SDI_BIT_DEPTH type, char* pTypeName)
{
    switch (type) {
    case SDI_BIT_DEPTH_8BIT:
        strcpy(pTypeName, "8bit");
        break;
    case SDI_BIT_DEPTH_10BIT:
        strcpy(pTypeName, "10bit");
        break;
    case SDI_BIT_DEPTH_12BIT:
        strcpy(pTypeName, "12bit");
        break;
    default:
        strcpy(pTypeName, "Unknown");
        break;
    }
}

//vga
void GetVideoSyncType(BYTE type, char* pTypeName)
{
    switch (type) {
    case VIDEO_SYNC_ALL:
        strcpy(pTypeName, "ALL");
        break;
    case VIDEO_SYNC_HS_VS:
        strcpy(pTypeName, "HS_VS");
        break;
    case VIDEO_SYNC_CS:
        strcpy(pTypeName, "CS");
        break;
    case VIDEO_SYNC_EMBEDDED:
        strcpy(pTypeName, "EMBEDDED");
        break;
    default:
        strcpy(pTypeName, "Unknown");
        break;
    }
}

//cvbs
void GetVideoSDStandard(MWCAP_SD_VIDEO_STANDARD type, char* pTypeName)
{
    switch (type) {
    case MWCAP_SD_VIDEO_NONE:
        strcpy(pTypeName, "NONE");
        break;
    case MWCAP_SD_VIDEO_NTSC_M:
        strcpy(pTypeName, "NTSC_M");
        break;
    case MWCAP_SD_VIDEO_NTSC_433:
        strcpy(pTypeName, "NTSC_433");
        break;
    case MWCAP_SD_VIDEO_PAL_M:
        strcpy(pTypeName, "PAL_M");
        break;
    case MWCAP_SD_VIDEO_PAL_60:
        strcpy(pTypeName, "PAL_60");
        break;
    case MWCAP_SD_VIDEO_PAL_COMBN:
        strcpy(pTypeName, "PAL_COMBN");
        break;
    case MWCAP_SD_VIDEO_PAL_BGHID:
        strcpy(pTypeName, "PAL_BGHID");
        break;
    case MWCAP_SD_VIDEO_SECAM:
        strcpy(pTypeName, "SECAM");
        break;
    case MWCAP_SD_VIDEO_SECAM_60:
        strcpy(pTypeName, "SECAM_60");
        break;
    default:
        strcpy(pTypeName, "Unknown");
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
    printf("Magewell MWCapture SDK V%d.%d.1.%d - InputSignal\n",byMaj,byMin,wBuild);
    printf("Usage:\n");
    printf("InputSignal <channel index>\n");
    printf("InputSignal <board id>:<channel id>\n\n");

    if (argc > 2){
        printf("ERROR: Invalid params!\n");
        printf("\nPress 'Enter' to exit!\n");
        getchar();
        return 0;
    }

    if(!MWCaptureInitInstance()){
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
            if (NULL == strstr(argv[1],":")){
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
        else{
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

        // Specific Status
        MWCAP_INPUT_SPECIFIC_STATUS status;
        xr = MWGetInputSpecificStatus(hChannel, &status);
        if (xr == MW_SUCCEEDED) {
            printf("Input Signal Valid: %d\n", status.bValid);

            if (status.bValid) {
                switch(status.dwVideoInputType)
                {
                case MWCAP_VIDEO_INPUT_TYPE_NONE:
                {
                    printf("---Type: NONE\n");
                    break;
                }
                case MWCAP_VIDEO_INPUT_TYPE_HDMI:
                {
                    printf("---Type: HDMI\n");
                    break;
                }
                case MWCAP_VIDEO_INPUT_TYPE_VGA:
                {
                    printf("---Type: VGA\n");
                    break;
                }
                case MWCAP_VIDEO_INPUT_TYPE_SDI:
                {
                    printf("---Type: SDI\n");
                    break;
                }
                case MWCAP_VIDEO_INPUT_TYPE_COMPONENT:
                {
                    printf("---Type: COMPONENT\n");
                    break;
                }
                case MWCAP_VIDEO_INPUT_TYPE_CVBS:
                {
                    printf("---Type: CVBS\n");
                    break;
                }
                case MWCAP_VIDEO_INPUT_TYPE_YC:
                {
                    printf("---Type: YC\n");
                    break;
                }
                default:
                    break;
                }

                if (status.dwVideoInputType == MWCAP_VIDEO_INPUT_TYPE_HDMI) {
                    printf("---HDMI HDCP: %d\n", status.hdmiStatus.bHDCP);
                    printf("---HDMI HDMI Mode: %d\n", status.hdmiStatus.bHDMIMode);
                    printf("---HDMI Bit Depth: %d\n", status.hdmiStatus.byBitDepth);
                }
                else if (status.dwVideoInputType == MWCAP_VIDEO_INPUT_TYPE_SDI) {
                    char chType[32] = {0};
                    GetVideoSDIType(status.sdiStatus.sdiType, chType);

                    char chScanfmt[32] = {0};
                    GetVideoScanFmt(status.sdiStatus.sdiScanningFormat, chScanfmt);

                    char chSampleStruct[32] = {0};
                    GetVideoSamplingStruct(status.sdiStatus.sdiSamplingStruct, chSampleStruct);

                    char chBitDepth[32] = {0};
                    GetVideoBitDepth(status.sdiStatus.sdiBitDepth, chBitDepth);

                    printf("---SDI Type: %s\n", chType);
                    printf("---SDI Scanning Format: %s\n", chScanfmt);
                    printf("---SDI Bit Depth: %s\n", chBitDepth);
                    printf("---SDI Sampling Struct: %s\n", chSampleStruct);
                }
                else if (status.dwVideoInputType == MWCAP_VIDEO_INPUT_TYPE_VGA) {
                    char chSyncType[32] = {0};
                    GetVideoSyncType(status.vgaComponentStatus.syncInfo.bySyncType, chSyncType);

                    double dFrameDuration = (status.vgaComponentStatus.syncInfo.bInterlaced == TRUE) ? (double)20000000 / status.vgaComponentStatus.syncInfo.dwFrameDuration : (double)10000000 / status.vgaComponentStatus.syncInfo.dwFrameDuration;
                    printf("---VGA SyncType: %s\n", chSyncType);
                    printf("---VGA bHSPolarity: %d\n", status.vgaComponentStatus.syncInfo.bHSPolarity);
                    printf("---VGA bVSPolarity: %d\n", status.vgaComponentStatus.syncInfo.bVSPolarity);
                    printf("---VGA bInterlaced: %d\n", status.vgaComponentStatus.syncInfo.bInterlaced);
                    printf("---VGA FrameDuration: %0.2f\n", dFrameDuration);
                }
                else if (status.dwVideoInputType == MWCAP_VIDEO_INPUT_TYPE_CVBS) {
                    char chSDStandard[32] = {0};
                    GetVideoSDStandard(status.cvbsYcStatus.standard, chSDStandard);

                    printf("---CVBS standard: %s\n", chSDStandard);
                    printf("---CVBS b50Hz: %d\n", status.cvbsYcStatus.b50Hz);
                }
            }
        }
        printf("\n");

        // Video Signal Status
        MWCAP_VIDEO_SIGNAL_STATUS vStatus;
        xr = MWGetVideoSignalStatus(hChannel, &vStatus);
        if (xr == MW_SUCCEEDED) {
            switch(vStatus.state)
            {
            case MWCAP_VIDEO_SIGNAL_LOCKED:
            {
                printf("Video Signal status: LOCKED\n");
                break;
            }
            case MWCAP_VIDEO_SIGNAL_LOCKING:
            {
                printf("Video Signal status: LOCKING\n");
                break;
            }
            case MWCAP_VIDEO_SIGNAL_UNSUPPORTED:
            {
                printf("Video Signal status: UNSUPPORTED\n");
                break;
            }
            case MWCAP_VIDEO_SIGNAL_NONE:
            {
                printf("Video Signal status: NONE\n");
                break;
            }
            default:
                break;
            }

            if (vStatus.state == MWCAP_VIDEO_SIGNAL_LOCKED)
            {
                char szColorName[16];
                GetVideoColorName(vStatus.colorFormat, szColorName, 16);

                double dFrameDuration = (vStatus.bInterlaced == TRUE) ? (double)20000000 / vStatus.dwFrameDuration : (double)10000000 / vStatus.dwFrameDuration;

                printf("Video Signal: \n");
                printf("---x, y: (%d, %d)\n", vStatus.x, vStatus.y);
                printf("---cx x cy: (%d x %d)\n", vStatus.cx, vStatus.cy);
                printf("---cxTotal x cyTotal: (%d x %d)\n", vStatus.cxTotal, vStatus.cyTotal);
                printf("---bInterlaced: %d\n", vStatus.bInterlaced);
                printf("---dwFrameDuration: %0.2f\n", dFrameDuration);
                printf("---nAspectX: %d\n", vStatus.nAspectX);
                printf("---nAspectY: %d\n", vStatus.nAspectY);
                printf("---bSegmentedFrame: %d\n", vStatus.bSegmentedFrame);
                printf("---colorFormat: %s\n", szColorName);
            }

        }

        printf("\n");

        // Audio Signal Status
        MWCAP_AUDIO_SIGNAL_STATUS aStatus;
        xr = MWGetAudioSignalStatus(hChannel, &aStatus);
        if (xr == MW_SUCCEEDED) {
            printf("Audio Signal Valid: %d\n", aStatus.bChannelStatusValid);

            if (aStatus.bChannelStatusValid == 1){
                char chSupChannels[128] = {0};
                for (int i = 0; i < 4; i++){
                    if (aStatus.wChannelValid & (0x01 << i))
                        sprintf(chSupChannels, "%s %d&%d;", chSupChannels, (i * 2 + 1), (i * 2 + 2));
                }

                printf("Audio Signal: \n");
                printf("---wChannelValid: %s\n", chSupChannels);
                printf("---bLPCM: %d\n", aStatus.bLPCM);
                printf("---cBitsPerSample: %d\n", aStatus.cBitsPerSample);
                printf("---dwSampleRate: %d\n", aStatus.dwSampleRate);
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

