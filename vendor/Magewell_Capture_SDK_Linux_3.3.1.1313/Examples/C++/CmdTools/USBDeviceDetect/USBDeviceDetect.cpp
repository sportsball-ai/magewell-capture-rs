/************************************************************************************************/
// USBDeviceDetect.cpp : Defines the entry point for the console application.

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
#include <unistd.h>

#include "LibMWCapture/MWCapture.h"
#include "LibMWCapture/MWUSBCapture.h"



static void HotplugCheckCallback(MWUSBHOT_PLUG_EVETN event, const char *pszDevicePath, void* pParam)
{
    switch(event)
    {
    case USBHOT_PLUG_EVENT_DEVICE_ARRIVED:
        printf("\nDevice reconnect\n");
        break;
    case USBHOT_PLUG_EVENT_DEVICE_LEFT:
        printf("\nDevice disconnect\n");
        break;
    default:
        break;
    }
}
void print_version_and_useage()
{
    
    BYTE byMaj, byMin;
    WORD wBuild;
    MWGetVersion(&byMaj, &byMin, &wBuild);
    printf("Magewell MWCapture SDK V%d.%d.1.%d - USBDeviceDetect\n",byMaj,byMin,wBuild);
    printf("Only USB devices are supported\n");
    printf("Usage:\n");
    printf("USBDeviceDetect\n");
}

int main(int argc, char* argv[])
{
    print_version_and_useage();

    MWCaptureInitInstance();
    do{
        if (MWUSBRegisterHotPlug(HotplugCheckCallback, NULL) != MW_SUCCEEDED) {
            printf("ERROR: Set usb device detect event failed\n");
            break;
        }
        printf("Please disconnect and reconnect the specific usb device\n");
	    printf("It will listen for USB devices connection change events for 10 seconds.\n");
    	for(int i=0; i<10; i++){
    		printf("%ds\n",i);
    		usleep(1000000);
    	}
        MWUSBUnRegisterHotPlug();
    }while(0);
    
    MWCaptureExitInstance();

    printf("\nPress 'Enter' to exit!\n");
    getchar();
    return 0;
}

