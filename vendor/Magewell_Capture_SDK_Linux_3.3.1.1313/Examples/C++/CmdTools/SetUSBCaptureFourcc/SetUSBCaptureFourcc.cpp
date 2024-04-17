/************************************************************************************************/
// SetUSBCaptureFourcc.cpp : Defines the entry point for the console application.

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
#include <string>

#include "MWFOURCC.h"
#include "LibMWCapture/MWCapture.h"

using namespace std;

int main(int argc, char* argv[])
{
    BYTE byMaj, byMin;
    WORD wBuild;
    MWGetVersion(&byMaj, &byMin, &wBuild);
    printf("Magewell MWCapture SDK V%d.%d.1.%d - SetUSBCaptureFourcc\n",byMaj,byMin,wBuild);
    printf("Only USB devices are supported\n");
    printf("Usage:\n");
    printf("SetUSBCaptureFourcc\n\n");
    if(!MWCaptureInitInstance()){
        printf("have InitilizeFailed\n");
    }

    do{
        MWRefreshDevice();
        int nCount = MWGetChannelCount();
        if (nCount <= 0){
            printf("ERROR: Can't find channels!\n");
            break;
		}
        MW_RESULT t_mr;
        int t_nIndex=-1;
        for(int i=0;i<nCount;i++){
            MWCAP_CHANNEL_INFO t_info;

            if(MWGetChannelInfoByIndex(i,&t_info)==MW_SUCCEEDED){
                if(strstr(t_info.szProductName,"USB")!=0)
                {
                    t_nIndex=i;
                }
            }
        }

        if(t_nIndex==-1) {
            printf("ERROR: Please insert usb device!\n");
            break;
        }

        char wPath[256] = {0};
        MW_RESULT mr = MWGetDevicePath(t_nIndex, wPath);

        HCHANNEL hChannel = MWOpenChannelByPath(wPath);
        if (hChannel == NULL){
            printf("ERROR: Open channel failed !\n");
            break;
        }

        MWCAP_CHANNEL_INFO info = {0};
        mr = MWGetChannelInfo(hChannel, &info);
		
        printf("Open channel - BoardIndex = %X, ChannelIndex = %d.\n", info.byBoardIndex, info.byChannelIndex);
        printf("Product Name: %s\n", info.szProductName);
        printf("Board SerialNo: %s\n\n", info.szBoardSerialNo);

        MWCAP_VIDEO_OUTPUT_FOURCC vFourcc = {0};
        mr = MWUSBGetVideoOutputFOURCC(hChannel, &vFourcc);

        printf("Support output fourcc count : %d\n", vFourcc.byCount);

        for (int i = 0; i < vFourcc.byCount; i++)
        {
            switch(vFourcc.adwFOURCCs[i])
            {
			case MWFOURCC_BGR24:
            {
                printf("BGR24\n");
                break;
            }
            case MWFOURCC_BGRA:
            {
                printf("BGRA\n");
                break;
            }
            case MWFOURCC_RGB24:
            {
                printf("RGB24\n");
                break;
            }
            case MWFOURCC_RGBA:
            {
                printf("RGBA\n");
                break;
            }
            case MWFOURCC_YUY2:
            {
                printf("YUY2\n");
                break;
            }
            case MWFOURCC_UYVY:
            {
                printf("UYVY\n");
                break;
            }
            case MWFOURCC_NV12:
            {
                printf("NV12\n");
                break;
            }
            default:
                printf("Unknown\n");
                break;
            }
        }

        //Set custom fourcc
        vFourcc.byCount = 3;
        vFourcc.adwFOURCCs[0] = MWFOURCC_YUY2;
        vFourcc.adwFOURCCs[1] = MWFOURCC_BGR24;
        vFourcc.adwFOURCCs[2] = MWFOURCC_BGRA;

        mr = MWUSBSetVideoOutputFOURCC(hChannel, &vFourcc);
        if (mr != MW_SUCCEEDED){
            printf("ERROR: Set USB output fourcc failed !\n");

            if (hChannel != NULL){
                MWCloseChannel(hChannel);
                hChannel = NULL;
            }

            break;
        }
        MWUSBSaveOptions(hChannel);
        MWSetPostReconfig(hChannel, 0);
        printf("\n");
        printf("Set USB output fourcc succeed !\n\n");
        
        mr = MWUSBGetVideoOutputFOURCC(hChannel, &vFourcc);
        printf("Now, support output fourcc count : %d\n", vFourcc.byCount);

        for (int i = 0; i < vFourcc.byCount; i++){
            switch(vFourcc.adwFOURCCs[i])
            {
            case MWFOURCC_BGR24:
            {
                printf("BGR24\n");
                break;
            }
            case MWFOURCC_BGRA:
            {
                printf("BGRA\n");
                break;
            }
            case MWFOURCC_YUY2:
            {
                printf("YUY2\n");
                break;
            }
            case MWFOURCC_UYVY:
            {
                printf("UYVY\n");
                break;
            }
            case MWFOURCC_NV12:
            {
                printf("NV12\n");
                break;
            }
            default:
                printf("Unknown\n");
                break;
            }
        }

        if (hChannel != NULL){
            MWCloseChannel(hChannel);
            hChannel = NULL;
        }

    } while (FALSE);

    MWCaptureExitInstance();
	printf("\nPress 'Enter' to exit!\n");
    getchar();

    return 0;
}

