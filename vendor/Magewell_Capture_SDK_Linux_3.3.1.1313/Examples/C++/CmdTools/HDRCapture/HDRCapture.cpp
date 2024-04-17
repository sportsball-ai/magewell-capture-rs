#include <stdio.h>
#include <string.h>
#include <sys/eventfd.h>
#include <sys/select.h>
#include <unistd.h>
#include <pthread.h>

#include "MWFOURCC.h"
#include "LibMWCapture/MWCapture.h"
#include "mw_venc/mw_venc.h"
//#include "Mp4Record.h"
#include "mw_mp4/mw_mp4.h"
#include "hevc_sei.h"
#include "hevc_sps.h"
#include "hevc_nalu.h"


#define CACHE_FRAME_NUM 4
#define MAX_DEVICE_NUM 32
#define CAPTURE_WIDTH 3840
#define CAPTURE_HEIGHT 2160
#define CAPTURE_FOURCC MWFOURCC_P010
#define NVENC_FOURCC EN_PIXEL_FORMAT_YUV420_10BIT

//#define CAPTURE_FRAME_NUM 1000

typedef int mw_event_t;

int g_iFpsNum;
int g_iFpsDen;
unsigned char g_ucCaptureRun = 0;
unsigned char g_ucRecordRun = 0;
unsigned char g_ucAppRun = 0;
char g_acRecordFileName[256];

unsigned char *g_apucFrmBuf[CACHE_FRAME_NUM] = {NULL, NULL, NULL, NULL};
LONGLONG allFrmCaptureTm[CACHE_FRAME_NUM];
int g_iWriteFrmIndex = 0;
int g_iRecordFrmIndex = 0;


void PrintVersion()
{
    BYTE byMaj, byMin;
    WORD wBuild;
    MWGetVersion(&byMaj, &byMin, &wBuild);
    printf("Magewell MWCapture SDK V%d.%d.1.%d - HDRCapture\n",byMaj,byMin,wBuild);
    printf("Only pro Capture HDMI 4K Devices are supported\n");
    printf("Have nvidia Graphics card(can support h265 enc,such as p2000), install driver and cuda\n");
    printf("Usage:\n");
    printf("HDRCapture <channel index>\n");
    printf("HDRCapture <board id>:<channel id>\n");
    printf("\"StartRecord\" and \"StopRecord\" to control start and stop record\n\n");
}

/*HCHANNEL ChooseAndOpenDevice()
{
    int i;
    HCHANNEL hChannel = NULL;
    int iChooseDevIdx = 0;
    int iHDRDevCount = 0;
    int g_iHDRDevChannel[MAX_DEVICE_NUM] = {-1};
    char cPath[128] = {0};
    int iChannelCount = MWGetChannelCount();
    printf("Log: Find %d channels!\n",iChannelCount);
    for (int i = 0; i < iChannelCount; i++){
        MWCAP_CHANNEL_INFO info;
        MW_RESULT mr = MWGetChannelInfoByIndex(i, &info);
        if ((MW_SUCCEEDED == mr) && (strcmp(info.szFamilyName, "Pro Capture") == 0)){
            printf("find %s\n",info.szFamilyName);
            g_iHDRDevChannel[iHDRDevCount] = i;
            iHDRDevCount++;
            if(iHDRDevCount >= MAX_DEVICE_NUM){
                break;
            }
        }
    }
    if(iHDRDevCount == 0){
        printf("not find device can capture HDR\n");
        return NULL;
    }
    if(iHDRDevCount > 1){
        char buf[8];
        printf("choose channel, you can input \"0\" to \"%d\"\n", (iHDRDevCount - 1));
        gets(buf);
        iChooseDevIdx = atoi(buf);
        if ((iChooseDevIdx > iHDRDevCount) || (iChooseDevIdx < 0)){
            iChooseDevIdx = 0;
        }
    }
                
    MWGetDevicePath(g_iHDRDevChannel[iChooseDevIdx], cPath);
    hChannel = MWOpenChannelByPath(cPath);
    if (hChannel == NULL) {
        printf("Open channel %d error!\n", iChooseDevIdx);
    }
    return hChannel;
}*/
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

HCHANNEL OpenChannel(int argc, char* argv[]){
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
        if (0 == strcmp(info.szFamilyName, "Pro Capture") && strstr(info.szProductName, "4K")){
            nProDevChannel[nProDevCount] = i;
            nProDevCount++;
        }
    }
    if (nProDevCount <= 0){
        printf("\nERROR: Can't find pro 4K channels!\n");
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

int GetVideoStatus(HCHANNEL hChannel, int *piFpsNum, int *piFpsDen)
{
    MWCAP_VIDEO_SIGNAL_STATUS stVideoSignalStatus;
    MWGetVideoSignalStatus(hChannel, &stVideoSignalStatus);
    
    switch (stVideoSignalStatus.state) {
    case MWCAP_VIDEO_SIGNAL_NONE:
        printf("Input signal status: NONE\n");
        break;
    case MWCAP_VIDEO_SIGNAL_UNSUPPORTED:
        printf("Input signal status: Unsupported\n");
        break;
    case MWCAP_VIDEO_SIGNAL_LOCKING:
        printf("Input signal status: Locking\n");
        break;
    case MWCAP_VIDEO_SIGNAL_LOCKED:
        printf("Input signal status: Locked\n");
        break;
    }
    
    if (stVideoSignalStatus.state == MWCAP_VIDEO_SIGNAL_LOCKED) {
        *piFpsDen = stVideoSignalStatus.dwFrameDuration;
        if(stVideoSignalStatus.bInterlaced){
            *piFpsNum = 20000000;
        }else{
            *piFpsNum = 10000000;
        }
        printf("fps is%d/%d\n",*piFpsNum, *piFpsDen);
        return 1;
    }
    printf("can not get fps\n");
    return 0;
}

unsigned char *GetInfoFrameAndAddHdrInfo(HCHANNEL hChannel, unsigned char **ppucStreamBuf, unsigned int *puiStreamLen)
{
    unsigned char *pucOutStream = NULL;
    unsigned char *pucSeiStream = NULL;
    unsigned char *pucSpsStream = NULL;
    unsigned int uiSeiLen = 0;
    unsigned int uiSpsLen = 0;
    int iSpsStart,iSpsEnd;
    bool bHaveSps = false;
    MW_RESULT Ret;
    unsigned int uiValidFlag = 0;
    MWCAP_INPUT_SPECIFIC_STATUS stInputStatus;
    HDMI_INFOFRAME_PACKET packet;
    HDMI_HDR_INFOFRAME stHdrInfo = {0};
    ST_HEVC_SPS stSpsInfo = {0};
    bHaveSps = HevcFindSps(*ppucStreamBuf, *puiStreamLen, &iSpsStart, &iSpsEnd);
    if(!bHaveSps){
        return NULL;
    }
    if(MW_SUCCEEDED != MWGetInputSpecificStatus(hChannel, &stInputStatus)){
        return NULL;
    }
    if(!stInputStatus.bValid){
        printf("Input signal is invalid!\n");
        return NULL;
    }
    else if(stInputStatus.dwVideoInputType != 1){
        printf("Input signal is not HDMI!\n");
        return NULL;
    }
    if(MW_SUCCEEDED != MWGetHDMIInfoFrameValidFlag(hChannel, &uiValidFlag)){
        return NULL;
    }
    if(0 == uiValidFlag){
        printf("No HDMI InfoFrame!\n");
        return NULL;
    }

    if (0 == (uiValidFlag & MWCAP_HDMI_INFOFRAME_MASK_HDR)) {
        return NULL;
    }
    if(MW_SUCCEEDED != MWGetHDMIInfoFramePacket(hChannel, MWCAP_HDMI_INFOFRAME_ID_HDR, &packet)){
        return NULL;
    }
    stHdrInfo.usDisplayPromariesX[0] |= packet.hdrInfoFramePayload.display_primaries_lsb_x1;
    stHdrInfo.usDisplayPromariesX[0] |= packet.hdrInfoFramePayload.display_primaries_msb_x1 << 8;
    stHdrInfo.usDisplayPromariesX[1] |= packet.hdrInfoFramePayload.display_primaries_lsb_x2;
    stHdrInfo.usDisplayPromariesX[1] |= packet.hdrInfoFramePayload.display_primaries_msb_x2 << 8;
    stHdrInfo.usDisplayPromariesX[2] |= packet.hdrInfoFramePayload.display_primaries_lsb_x0;
    stHdrInfo.usDisplayPromariesX[2] |= packet.hdrInfoFramePayload.display_primaries_msb_x0 << 8;

    stHdrInfo.usDisplayPromariesY[0] |= packet.hdrInfoFramePayload.display_primaries_lsb_y1;
    stHdrInfo.usDisplayPromariesY[0] |= packet.hdrInfoFramePayload.display_primaries_msb_y1 << 8;
    stHdrInfo.usDisplayPromariesY[1] |= packet.hdrInfoFramePayload.display_primaries_lsb_y2;
    stHdrInfo.usDisplayPromariesY[1] |= packet.hdrInfoFramePayload.display_primaries_msb_y2 << 8;
    stHdrInfo.usDisplayPromariesY[2] |= packet.hdrInfoFramePayload.display_primaries_lsb_y0;
    stHdrInfo.usDisplayPromariesY[2] |= packet.hdrInfoFramePayload.display_primaries_msb_y0 << 8;

    stHdrInfo.usWhitePointX |= packet.hdrInfoFramePayload.white_point_lsb_x;
    stHdrInfo.usWhitePointX |= packet.hdrInfoFramePayload.white_point_msb_x << 8;
    stHdrInfo.usWhitePointY |= packet.hdrInfoFramePayload.white_point_lsb_y;
    stHdrInfo.usWhitePointY |= packet.hdrInfoFramePayload.white_point_msb_y << 8;

    stHdrInfo.uiMaxDisplayMasteringLuminance |= packet.hdrInfoFramePayload.max_display_mastering_lsb_luminance;
    stHdrInfo.uiMaxDisplayMasteringLuminance |= packet.hdrInfoFramePayload.max_display_mastering_msb_luminance << 8;

    stHdrInfo.uiMinDisplayMasteringLuminance |= packet.hdrInfoFramePayload.min_display_mastering_lsb_luminance;
    stHdrInfo.uiMinDisplayMasteringLuminance |= packet.hdrInfoFramePayload.min_display_mastering_msb_luminance << 8;

    stHdrInfo.usMaxContentLightLevel |= packet.hdrInfoFramePayload.maximum_content_light_level_lsb;
    stHdrInfo.usMaxContentLightLevel |= packet.hdrInfoFramePayload.maximum_content_light_level_msb << 8;

    stHdrInfo.usMaxFrameAverageLightLevel |= packet.hdrInfoFramePayload.maximum_frame_average_light_level_lsb;
    stHdrInfo.usMaxFrameAverageLightLevel |= packet.hdrInfoFramePayload.maximum_frame_average_light_level_msb << 8;

    
    pucSeiStream = CreateHdrSeiPrefix(&stHdrInfo, &uiSeiLen);
    memset(&stSpsInfo, 0, sizeof(stSpsInfo));
    HevcParseSPS(&stSpsInfo, *ppucStreamBuf + iSpsStart + 5, iSpsEnd - iSpsStart - 5);
    pucSpsStream = HevcAddHdrInfoToSps(&stSpsInfo, &uiSpsLen);
    pucOutStream = (unsigned char *)malloc(*puiStreamLen - (iSpsEnd - iSpsStart - 5) + uiSpsLen + uiSeiLen);
    if((pucSeiStream == NULL) || (pucSpsStream == NULL) || (pucOutStream == NULL)){
        if(pucSeiStream){
            free(pucSeiStream);
        }
        if(pucSpsStream){
            free(pucSpsStream);
        }
        if(pucOutStream){
            free(pucOutStream);
        }
        printf("malloc fail\n");
        return NULL;
    }
    memcpy(pucOutStream, pucSeiStream, uiSeiLen);
    memcpy(pucOutStream + uiSeiLen, *ppucStreamBuf, iSpsStart + 5);
    memcpy(pucOutStream + uiSeiLen + iSpsStart + 5, pucSpsStream, uiSpsLen);
    memcpy(pucOutStream + uiSeiLen + iSpsStart + 5 + uiSpsLen, *ppucStreamBuf + iSpsEnd, *puiStreamLen - iSpsEnd);
    *puiStreamLen = *puiStreamLen - iSpsEnd + uiSpsLen + iSpsStart + 5 + uiSeiLen;
    *ppucStreamBuf = pucOutStream;
    free(pucSeiStream);
    free(pucSpsStream);
    return pucOutStream;
}

void *CaptureThread(void *parm)
{  
    int i;
    HCHANNEL hChannel = *(HCHANNEL*)parm;
    HNOTIFY hNotify=0;
    MWCAP_PTR hNotifyEvent=0;
    MWCAP_PTR hCaptureEvent=0;
    unsigned char ucCaptureStart = 0;
    unsigned int uiMinStride;
    unsigned int uiImageSize;
    MWCAP_VIDEO_BUFFER_INFO stVideoBufferInfo;
    MWCAP_VIDEO_FRAME_INFO stVideoFrameInfo;
    printf("capture start\n");
    hCaptureEvent=MWCreateEvent();
    if(hCaptureEvent==0){
        printf("Create timer event error\n");
        goto end_and_free;
    }
    hNotifyEvent=MWCreateEvent();
    if(hNotifyEvent==0){
        printf("Create notify event error\n");
        goto end_and_free;
    }

    if(MW_SUCCEEDED != MWStartVideoCapture(hChannel, hCaptureEvent)){
        printf("Open Video Capture error!\n");
        goto end_and_free;
    }
    ucCaptureStart = 1;
    uiMinStride=FOURCC_CalcMinStride(CAPTURE_FOURCC, CAPTURE_WIDTH, 4);
    uiImageSize=FOURCC_CalcImageSize(CAPTURE_FOURCC, CAPTURE_WIDTH, CAPTURE_HEIGHT, uiMinStride);
    for(i = 0; i < CACHE_FRAME_NUM; i++){
        if(NULL == g_apucFrmBuf[i]){
            g_apucFrmBuf[i] = (unsigned char*)malloc(uiImageSize);
        }
        if(NULL == g_apucFrmBuf[i]){
            printf("alloc cache fail\n");
            goto end_and_free;
        }
    }
    

    if(GetVideoStatus(hChannel, &g_iFpsNum, &g_iFpsDen) <= 0){
        goto end_and_free;
    }

    hNotify = MWRegisterNotify(hChannel, hNotifyEvent, MWCAP_NOTIFY_VIDEO_FRAME_BUFFERED);
    if (hNotify == 0) {
        printf("Register Notify error.\n");
        goto end_and_free;
    }

    printf("Begin to Capture\n");
    for(i = 0; i < CACHE_FRAME_NUM; i++){
        MWPinVideoBuffer(hChannel, (MWCAP_PTR)g_apucFrmBuf[i], uiImageSize);
    }
    while(g_ucCaptureRun) {
        ULONGLONG ullStatusBits = 0;
        MWCAP_VIDEO_CAPTURE_STATUS stCaptureStatus;
        if(MWWaitEvent(hNotifyEvent,1000) <= 0){
            printf("wait notify error or timeout\n");
            break;
        }
        if (MWGetNotifyStatus(hChannel, hNotify, &ullStatusBits) != MW_SUCCEEDED){
            continue;
        }
        if (MWGetVideoBufferInfo(hChannel, &stVideoBufferInfo) != MW_SUCCEEDED){
            continue;
        }
        if (MWGetVideoFrameInfo(hChannel, stVideoBufferInfo.iNewestBufferedFullFrame, &stVideoFrameInfo) != MW_SUCCEEDED)
            continue;

        if (ullStatusBits & MWCAP_NOTIFY_VIDEO_FRAME_BUFFERED) {
            if(MW_SUCCEEDED != MWCaptureVideoFrameToVirtualAddressEx(hChannel,
                                   stVideoBufferInfo.iNewestBufferedFullFrame,(unsigned char *)g_apucFrmBuf[g_iWriteFrmIndex % CACHE_FRAME_NUM], uiImageSize, uiMinStride,
                                   0,0,CAPTURE_FOURCC,CAPTURE_WIDTH,CAPTURE_HEIGHT,0,0,0,0,0,100,0,100,0,MWCAP_VIDEO_DEINTERLACE_BLEND,
                                   MWCAP_VIDEO_ASPECT_RATIO_CROPPING,0,0,0,0,MWCAP_VIDEO_COLOR_FORMAT_UNKNOWN,
                                   MWCAP_VIDEO_QUANTIZATION_UNKNOWN,MWCAP_VIDEO_SATURATION_UNKNOWN)){
                continue;
            }

            if(MWWaitEvent(hCaptureEvent,1000) <= 0)
            {
                printf("wait capture event error or timeout\n");
                break;
            }
            MWGetDeviceTime(hChannel, &allFrmCaptureTm[g_iWriteFrmIndex % CACHE_FRAME_NUM]);
            g_iWriteFrmIndex++;
            MWGetVideoCaptureStatus(hChannel, &stCaptureStatus);
        }
    }
    for(i = 0; i < CACHE_FRAME_NUM; i++){
        MWUnpinVideoBuffer(hChannel, (LPBYTE)g_apucFrmBuf[i]);
    }
end_and_free:
    if(hNotify){
        MWUnregisterNotify(hChannel, hNotify);
        hNotify=0;
    }
    if(ucCaptureStart){
        MWStopVideoCapture(hChannel);
    }

    if(hNotifyEvent!=0)
    {
        MWCloseEvent(hNotifyEvent);
        hNotifyEvent=0;
    }

    if(hCaptureEvent!=0)
    {
        MWCloseEvent(hCaptureEvent);
        hCaptureEvent=0;
    }
	g_ucCaptureRun = 0;
    printf("capture stop\n");
    return NULL;
}  

mw_mp4_handle_t g_hMp4;
unsigned char ucNeedSetVideo = 1;
void VencCallback(void * user_ptr, const uint8_t * p_frame, uint32_t frame_len, mw_venc_frame_info_t *p_frame_info)
{
    unsigned char *pucOutBuf = NULL;
    HCHANNEL hChannel = (HCHANNEL)user_ptr;
    if((NULL == g_hMp4) && g_ucRecordRun){
        return;
    }
    pucOutBuf = GetInfoFrameAndAddHdrInfo(hChannel, &p_frame, &frame_len);
    if(ucNeedSetVideo){
        mw_mp4_video_info_t video_info = { 0 };
        video_info.codec_type = MW_MP4_VIDEO_TYPE_H265;
        video_info.timescale = 1000;
        video_info.width = CAPTURE_WIDTH;
        video_info.height = CAPTURE_HEIGHT;
        video_info.hevc.vps = NULL;
        video_info.hevc.vps_size = 0;
        video_info.hevc.sps = NULL;
        video_info.hevc.sps_size = 0;
        video_info.hevc.pps = NULL;
        video_info.hevc.pps_size = 0;
        mw_mp4_set_video(g_hMp4, &video_info);
        //hMp4Record->WriteMp4Header((const unsigned char*)pucOutFrame, uiOutFrameLen);
        ucNeedSetVideo = 0;
    
    }
    //hMp4Record->WriteVideoFrame((const unsigned char*)pucOutFrame, uiOutFrameLen, llFrmTm/10000);
    mw_mp4_write_video(g_hMp4, (const unsigned char*)p_frame, frame_len, p_frame_info->pts);
    if(pucOutBuf){
        free(pucOutBuf);
    }

}

mw_venc_handle_t SelectAndCreateEnc(HCHANNEL hChannel)
{
    mw_venc_handle_t hVEnc;
    mw_venc_param_t stParam;
    
    int32_t iSelectGpu = -1;
    int32_t iGpuNum =  mw_venc_get_gpu_num();
    if(iGpuNum <= 0){
        printf("can not find gpu\n");
        return NULL;
    }
    mw_venc_get_default_param(&stParam);
    stParam.width = CAPTURE_WIDTH;
    stParam.height = CAPTURE_HEIGHT;
    stParam.fps.den = g_iFpsDen;
    stParam.fps.num = g_iFpsNum;
    stParam.fourcc = MW_VENC_FOURCC_P010;
    stParam.code_type = MW_VENC_CODE_TYPE_H265;
    stParam.rate_control.mode = MW_VENC_RATECONTROL_CBR;
    stParam.rate_control.target_bitrate = 4096;
    stParam.gop_pic_size = 60;
    for(iSelectGpu = 0; iSelectGpu < iGpuNum; iSelectGpu++){
        mw_venc_gpu_info_t info;
        mw_venc_get_gpu_info_by_index(iSelectGpu, &info);
        if(info.platform == MW_VENC_PLATFORM_AMD){
            continue;
        }
        hVEnc = mw_venc_create_by_index(iSelectGpu, &stParam, VencCallback, hChannel);
        if(hVEnc){
            return hVEnc;
        }
    }
    printf("not find gpu support p010 h265\n");
    return NULL;
}

void *EncAndRecordThread(void *parm)
{  
    mw_venc_handle_t hVEnc = NULL;
    HCHANNEL hChannel = *(HCHANNEL*)parm;
    //Mp4Record *hMp4Record = NULL;
    LONGLONG llFrmTm;//allFrmCaptureTm[CACHE_FRAME_NUM];
    printf("record start\n");
    g_hMp4 = mw_mp4_open(g_acRecordFileName);
    hVEnc = SelectAndCreateEnc(hChannel);
    ucNeedSetVideo = 1;
    while(g_ucRecordRun && g_ucCaptureRun){
        if(g_iRecordFrmIndex >= g_iWriteFrmIndex){
            usleep(1);
            continue;
        }
        if(g_iWriteFrmIndex - g_iRecordFrmIndex > 2){
            printf("drop fram %d\n",g_iWriteFrmIndex - 1 - g_iRecordFrmIndex);
            g_iRecordFrmIndex = g_iWriteFrmIndex - 1;
        }
        llFrmTm = allFrmCaptureTm[g_iRecordFrmIndex % CACHE_FRAME_NUM];
        if (mw_venc_put_frame_ex(hVEnc, g_apucFrmBuf[g_iRecordFrmIndex % CACHE_FRAME_NUM], llFrmTm/10000) == MW_VENC_STATUS_SUCCESS){
            g_iRecordFrmIndex++;
        }
    }
    if(hVEnc){
        mw_venc_destory(hVEnc);
        hVEnc = NULL;
    }
    if(g_hMp4){
        mw_mp4_close(g_hMp4);
        g_hMp4= NULL;
    }

    printf("record stop\n");
    g_ucRecordRun = 0;
    return NULL;
}

void *MsgThread(void *parm)
{
    char acBuf[1024];
    pthread_t pidCaptureThread = 0;
    pthread_t pidEncAndRecordThread = 0;
    HCHANNEL hChannel = *(HCHANNEL*)parm;
    while(g_ucAppRun){
        printf("please input cmd:\nStartCapture?\nStopCapture?\nStartRecord?\nStopRecord?\nStopApp?\n");
        scanf("%s", &acBuf);
        if (!strncmp(acBuf, "StartCapture", strlen("StartCapture"))){
            g_ucCaptureRun = 1;
            pthread_create(&pidCaptureThread, NULL, CaptureThread, &hChannel);

        }
        else if(!strncmp(acBuf, "StopCapture", strlen("StopCapture"))){
            g_ucCaptureRun = 0;
        }
        else if(!strncmp(acBuf, "StartRecord", strlen("StartRecord"))){
            if(g_ucCaptureRun == 0){
                g_ucCaptureRun = 1;
                pthread_create(&pidCaptureThread, NULL, CaptureThread, &hChannel);
            }
            usleep(100000);
            printf("please input mp4 name\n");
            scanf("%s", &acBuf);
            g_ucRecordRun = 1;
            memcpy(g_acRecordFileName, acBuf, 256);
            pthread_create(&pidEncAndRecordThread, NULL, EncAndRecordThread, &hChannel);

        }
        else if(!strncmp(acBuf, "StopRecord", strlen("StopRecord"))){
            g_ucRecordRun = 0;
        }
        else if(!strncmp(acBuf, "StopApp", strlen("StopApp"))){
            break;
        }
    }
    g_ucCaptureRun = 0;
    g_ucRecordRun = 0;
    if(pidCaptureThread){
        int *thread_ret = NULL; 
        pthread_join(pidCaptureThread, (void**)&thread_ret );  
    }
    if(pidEncAndRecordThread){
        int *thread_ret = NULL; 
        pthread_join(pidEncAndRecordThread, (void**)&thread_ret );  
    }
	g_ucAppRun= 0;
    return NULL;
}

int main(int argc, char* argv[])
{
    int i;
    HCHANNEL hChannel;
    pthread_t pidMsgThread; 
    MWCAP_CHANNEL_INFO videoInfo = { 0 };

    PrintVersion();
    mw_venc_init();
    if(!MWCaptureInitInstance()){
        printf("InitilizeFailed\n");
        return -1;
    }
    MWRefreshDevice();
    hChannel = OpenChannel(argc, argv);//ChooseAndOpenDevice();
    if(hChannel == NULL){
        return -1;
    }
    g_ucAppRun = 1;
    pthread_create(&pidMsgThread, NULL, MsgThread, &hChannel);
    while(g_ucAppRun){
        usleep(100000);
    }


    if (hChannel != NULL)
    {
        MWCloseChannel(hChannel);
        hChannel=NULL;
    }
    MWCaptureExitInstance();
    mw_venc_deinit();
    for(i = 0; i < CACHE_FRAME_NUM; i++){
        if(g_apucFrmBuf[i]){
            free(g_apucFrmBuf[i]);
        }
    }

    printf("\nPress 'Enter' to exit!\n");
    getchar();

    return 0;
}

