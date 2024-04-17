#include <jni.h>
#include <sys/eventfd.h>
#include <unistd.h>
#include "jawt_md.h"
#include "MWFOURCC.h"
#include "LibMWCapture/MWCapture.h"
#include "LibMWCapture/MWEcoCapture.h"
#include <GLFW/glfw3.h>

#include <GL/glx.h>
#ifdef __cplusplus
extern "C" {
#endif
JNIEXPORT jboolean JNICALL Java_com_magewell_LibMWCapture_MWCaptureInitInstance(JNIEnv *, jclass)
{
    return (jboolean)MWCaptureInitInstance();
}

JNIEXPORT void JNICALL Java_com_magewell_LibMWCapture_MWCaptureExitInstance(JNIEnv *, jclass)
{
    MWCaptureExitInstance();
}
JNIEXPORT int JNICALL Java_com_magewell_LibMWCapture_MWGetChannelCount(JNIEnv *, jclass)
{
    return MWGetChannelCount();
}
JNIEXPORT int JNICALL Java_com_magewell_LibMWCapture_MWGetVersion(JNIEnv *env, jclass, jbyteArray pbyMaj, jbyteArray pbyMin, jshortArray pwBuild)
{
    MW_RESULT ret;
    jbyte* byteMaj =env->GetByteArrayElements(pbyMaj, 0);
    jbyte* byteMin =env->GetByteArrayElements(pbyMin, 0);
    jshort * shortBuild = env->GetShortArrayElements(pwBuild, 0);
    ret = MWGetVersion((BYTE*)byteMaj, (BYTE*)byteMin, (WORD*)shortBuild);
    env->ReleaseByteArrayElements(pbyMaj, byteMaj, 0);
    env->ReleaseByteArrayElements(pbyMin, byteMin, 0);
    env->ReleaseShortArrayElements(pwBuild, shortBuild, 0);
    return ret;
}

JNIEXPORT jlong JNICALL Java_com_magewell_LibMWCapture_FOURCC_1CalcImageSize
	(JNIEnv *, jclass, jlong dwFourcc, jint nWidth, jint nHeight, jlong cbStride)
{
	DWORD cbImageSize = FOURCC_CalcImageSize((DWORD)dwFourcc, nWidth, nHeight, (DWORD)cbStride);
	return (jlong)cbImageSize;
}

JNIEXPORT jlong JNICALL Java_com_magewell_LibMWCapture_FOURCC_1CalcMinStride
	(JNIEnv *, jclass, jlong dwFourcc, jint nWidth, jlong dwAlign)
{
	DWORD cbStride = FOURCC_CalcMinStride((DWORD)dwFourcc, nWidth, (DWORD)dwAlign);
	return (jlong)cbStride;
}

void set_channel_info(JNIEnv *env, MWCAP_CHANNEL_INFO *p_channel_info, jobject pChannelInfo)
{
    jclass objectClass = (env)->FindClass("com/magewell/MWCAP_CHANNEL_INFO");

    jfieldID wFamilyID = (env)->GetFieldID(objectClass, "wFamilyID", "S");
    jfieldID wProductID = (env)->GetFieldID(objectClass, "wProductID", "S");
    jfieldID chHardwareVersion = (env)->GetFieldID(objectClass, "chHardwareVersion", "B");
    jfieldID byFirmwareID = (env)->GetFieldID(objectClass, "byFirmwareID", "B");
    jfieldID dwFirmwareVersion = (env)->GetFieldID(objectClass, "dwFirmwareVersion", "I");
    jfieldID dwDriverVersion = (env)->GetFieldID(objectClass, "dwDriverVersion", "I");
    jfieldID szFamilyName = (env)->GetFieldID(objectClass, "szFamilyName", "[B");
    jfieldID szProductName = (env)->GetFieldID(objectClass, "szProductName", "[B");
    jfieldID szFirmwareName = (env)->GetFieldID(objectClass, "szFirmwareName", "[B");
    jfieldID szBoardSerialNo = (env)->GetFieldID(objectClass, "szBoardSerialNo", "[B");
    jfieldID byBoardIndex = (env)->GetFieldID(objectClass, "byBoardIndex", "B");
    jfieldID byChannelIndex = (env)->GetFieldID(objectClass, "byChannelIndex", "B");

	(env)->SetShortField(pChannelInfo, wFamilyID, p_channel_info->wFamilyID);
	(env)->SetShortField(pChannelInfo, wProductID, p_channel_info->wProductID);
	(env)->SetByteField(pChannelInfo, chHardwareVersion, p_channel_info->chHardwareVersion);
	(env)->SetByteField(pChannelInfo, byFirmwareID, p_channel_info->byFirmwareID);
	(env)->SetIntField(pChannelInfo, dwFirmwareVersion, p_channel_info->dwFirmwareVersion);
	(env)->SetIntField(pChannelInfo, dwDriverVersion, p_channel_info->dwDriverVersion);
	(env)->SetByteField(pChannelInfo, byBoardIndex, p_channel_info->byBoardIndex);
	(env)->SetByteField(pChannelInfo, byChannelIndex, p_channel_info->byChannelIndex);

    jbyteArray jByteFamilyName = env->NewByteArray(MW_FAMILY_NAME_LEN);
    env->SetByteArrayRegion(jByteFamilyName, 0, MW_FAMILY_NAME_LEN, (jbyte *)p_channel_info->szFamilyName);
    env->SetObjectField(pChannelInfo, szFamilyName, jByteFamilyName);

    jbyteArray jByteProductName = env->NewByteArray(MW_PRODUCT_NAME_LEN);
    env->SetByteArrayRegion(jByteProductName, 0, MW_PRODUCT_NAME_LEN, (jbyte *)p_channel_info->szProductName);
    env->SetObjectField(pChannelInfo, szProductName, jByteProductName);

    jbyteArray jByteFirmwareName = env->NewByteArray(MW_FIRMWARE_NAME_LEN);
    env->SetByteArrayRegion(jByteFirmwareName, 0, MW_FIRMWARE_NAME_LEN, (jbyte *)p_channel_info->szFirmwareName);
    env->SetObjectField(pChannelInfo, szFirmwareName, jByteFirmwareName);

    jbyteArray jByteBoardSerialNo = env->NewByteArray(MW_SERIAL_NO_LEN);
    env->SetByteArrayRegion(jByteBoardSerialNo, 0, MW_SERIAL_NO_LEN, (jbyte *)p_channel_info->szBoardSerialNo);
    env->SetObjectField(pChannelInfo, szBoardSerialNo, jByteBoardSerialNo);

    env->DeleteLocalRef(jByteFamilyName);
    env->DeleteLocalRef(jByteProductName);
    env->DeleteLocalRef(jByteFirmwareName);
    env->DeleteLocalRef(jByteBoardSerialNo);
    env->DeleteLocalRef(objectClass);
}

JNIEXPORT jint JNICALL Java_com_magewell_LibMWCapture_MWGetChannelInfoByIndex(JNIEnv *env, jclass, jint nIndex, jobject pChannelInfo)
{
    MWCAP_CHANNEL_INFO channel_info;
    MW_RESULT ret = MWGetChannelInfoByIndex((int)nIndex,&channel_info);
    if(MW_SUCCEEDED != ret){
        return (jint)ret;
    }
    set_channel_info(env, &channel_info, pChannelInfo);
    return (jint)ret;
}

JNIEXPORT jint JNICALL Java_com_magewell_LibMWCapture_MWGetDevicePath(JNIEnv *env, jclass, jint nIndex, jbyteArray pDevicePath)
{
    jbyte* bytePath =env->GetByteArrayElements(pDevicePath, 0);
    jint ret = MWGetDevicePath(nIndex, (char*)bytePath);
    env->ReleaseByteArrayElements(pDevicePath, bytePath, 0);
    return ret;
}

JNIEXPORT jlong JNICALL Java_com_magewell_LibMWCapture_MWOpenChannelByPath(JNIEnv *env, jclass, jbyteArray pDevicePath)
{
    jbyte* bytePath =env->GetByteArrayElements(pDevicePath, 0);
    jlong handle = (jlong)MWOpenChannelByPath((const char*)bytePath);
    env->ReleaseByteArrayElements(pDevicePath, bytePath, 0);
    return handle;
}

JNIEXPORT void JNICALL Java_com_magewell_LibMWCapture_MWCloseChannel(JNIEnv *, jclass, jlong hChannel)
{
    MWCloseChannel((HCHANNEL)hChannel);
}

JNIEXPORT jint JNICALL Java_com_magewell_LibMWCapture_MWGetChannelInfo(JNIEnv *env, jclass, jlong hChannel, jobject pChannelInfo)
{
    MWCAP_CHANNEL_INFO channel_info;
    MW_RESULT ret = MWGetChannelInfo((HCHANNEL)hChannel,&channel_info);
    if(MW_SUCCEEDED != ret){
        return (jint)ret;
    }
    set_channel_info(env, &channel_info, pChannelInfo);
    return (jint)ret;
}

JNIEXPORT jlong JNICALL Java_com_magewell_LibMWCapture_MWCreateEvent(JNIEnv *, jclass)
{
    return (jlong)MWCreateEvent();
}

JNIEXPORT jint JNICALL Java_com_magewell_LibMWCapture_MWCloseEvent(JNIEnv *, jclass, jlong hEvent)
{
    return (jint)MWCloseEvent((MWCAP_PTR)hEvent);
}

JNIEXPORT jint JNICALL Java_com_magewell_LibMWCapture_MWWaitEvent(JNIEnv *env, jclass, jlong hEvent, jint nTimeout)
{
    return MWWaitEvent((MWCAP_PTR)hEvent, nTimeout);
}

JNIEXPORT jlong JNICALL Java_com_magewell_LibMWCapture_MWCreateEcoEvent(JNIEnv *, jclass)
{
    return (jlong)eventfd(0, EFD_NONBLOCK);
}

JNIEXPORT jint JNICALL Java_com_magewell_LibMWCapture_MWCloseEcoEvent(JNIEnv *, jclass, jlong hEvent)
{
    int event = (int)hEvent;
    eventfd_write((int)event, 1);
    close((int)event);
    return MW_SUCCEEDED;
}

JNIEXPORT jint JNICALL Java_com_magewell_LibMWCapture_MWWaitEcoEvent(JNIEnv *env, jclass, jlong hEvent, jint nTimeout)
{
	fd_set rfds;
	struct timeval tv;
	struct timeval *ptv = NULL;
	eventfd_t value = 0;
	int retval;
    int event = (int)hEvent;
	FD_ZERO(&rfds);
	FD_SET(event, &rfds);

	if (nTimeout < 0) {
		ptv = NULL;
	} else if (nTimeout == 0) {
		tv.tv_sec = nTimeout / 1000;
		tv.tv_usec = (nTimeout % 1000) * 1000;
		ptv = &tv;
	} else {
		tv.tv_sec = nTimeout / 1000;
		tv.tv_usec = (nTimeout % 1000) * 1000;
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

JNIEXPORT jint JNICALL Java_com_magewell_LibMWCapture_MWStartVideoCapture(JNIEnv *, jclass, jlong hChannel, jlong hEvent)
{
    return (jint)MWStartVideoCapture((HCHANNEL)hChannel,(MWCAP_PTR)hEvent);
}
JNIEXPORT jint JNICALL Java_com_magewell_LibMWCapture_MWStopVideoCapture(JNIEnv *, jclass, jlong hChannel)
{
    return (jint)MWStopVideoCapture((HCHANNEL)hChannel);
}

JNIEXPORT jint JNICALL Java_com_magewell_LibMWCapture_MWStartAudioCapture(JNIEnv *, jclass, jlong hChannel)
{
    return (jint)MWStartAudioCapture((HCHANNEL)hChannel);
}
JNIEXPORT jint JNICALL Java_com_magewell_LibMWCapture_MWStopAudioCapture(JNIEnv *, jclass, jlong hChannel)
{
    return (jint)MWStopAudioCapture((HCHANNEL)hChannel);
}
JNIEXPORT jint JNICALL Java_com_magewell_LibMWCapture_MWGetVideoSignalStatus(JNIEnv *env, jclass, jlong hChannel, jobject pSignalStatus)
{
    MWCAP_VIDEO_SIGNAL_STATUS video_signal_status;
    MW_RESULT ret = MWGetVideoSignalStatus((HCHANNEL)hChannel,&video_signal_status);
    if(MW_SUCCEEDED != ret){
        return (jint)ret;
    }
    jclass objectClass = (env)->FindClass("com/magewell/MWCAP_VIDEO_SIGNAL_STATUS");

    jfieldID state = (env)->GetFieldID(objectClass, "state", "I");
    jfieldID x = (env)->GetFieldID(objectClass, "x", "I");
    jfieldID y = (env)->GetFieldID(objectClass, "y", "I");
    jfieldID cx = (env)->GetFieldID(objectClass, "cx", "I");
    jfieldID cy = (env)->GetFieldID(objectClass, "cy", "I");
    jfieldID cxTotal = (env)->GetFieldID(objectClass, "cxTotal", "I");
    jfieldID cyTotal = (env)->GetFieldID(objectClass, "cyTotal", "I");
    jfieldID bInterlaced = (env)->GetFieldID(objectClass, "bInterlaced", "Z");
    jfieldID dwFrameDuration = (env)->GetFieldID(objectClass, "dwFrameDuration", "I");
    jfieldID nAspectX = (env)->GetFieldID(objectClass, "nAspectX", "I");
    jfieldID nAspectY = (env)->GetFieldID(objectClass, "nAspectY", "I");
    jfieldID bSegmentedFrame = (env)->GetFieldID(objectClass, "bSegmentedFrame", "Z");
    jfieldID frameType = (env)->GetFieldID(objectClass, "frameType", "I");
    jfieldID colorFormat = (env)->GetFieldID(objectClass, "colorFormat", "I");
    jfieldID quantRange = (env)->GetFieldID(objectClass, "quantRange", "I");
    jfieldID satRange = (env)->GetFieldID(objectClass, "satRange", "I");

	(env)->SetIntField(pSignalStatus, state, video_signal_status.state);
	(env)->SetIntField(pSignalStatus, x, video_signal_status.x);
	(env)->SetIntField(pSignalStatus, y, video_signal_status.y);
	(env)->SetIntField(pSignalStatus, cx, video_signal_status.cx);
	(env)->SetIntField(pSignalStatus, cy, video_signal_status.cy);
	(env)->SetIntField(pSignalStatus, cxTotal, video_signal_status.cxTotal);
	(env)->SetIntField(pSignalStatus, cyTotal, video_signal_status.cyTotal);
	(env)->SetBooleanField(pSignalStatus, bInterlaced, video_signal_status.bInterlaced);
    (env)->SetIntField(pSignalStatus, dwFrameDuration, video_signal_status.dwFrameDuration);
    (env)->SetIntField(pSignalStatus, nAspectX, video_signal_status.nAspectX);
    (env)->SetIntField(pSignalStatus, nAspectY, video_signal_status.nAspectY);
    (env)->SetBooleanField(pSignalStatus, bSegmentedFrame, video_signal_status.bSegmentedFrame);
    (env)->SetIntField(pSignalStatus, frameType, video_signal_status.frameType);
    (env)->SetIntField(pSignalStatus, colorFormat, video_signal_status.colorFormat);
    (env)->SetIntField(pSignalStatus, quantRange, video_signal_status.quantRange);
    (env)->SetIntField(pSignalStatus, satRange, video_signal_status.satRange);
    env->DeleteLocalRef(objectClass);
    return (jint)ret;
}
JNIEXPORT jlong JNICALL Java_com_magewell_LibMWCapture_MWRegisterNotify(JNIEnv *env, jclass, jlong hChannel, jlong hEvent, jint dwEnableBits)
{
    return (jlong)MWRegisterNotify((HCHANNEL)hChannel, (MWHANDLE)hEvent, (DWORD)dwEnableBits);
}

JNIEXPORT jlong JNICALL Java_com_magewell_LibMWCapture_MWUnregisterNotify(JNIEnv *env, jclass, jlong hChannel, jlong hEvent)
{
    return (jint)MWUnregisterNotify((HCHANNEL)hChannel, (MWHANDLE)hEvent);
}
JNIEXPORT jint JNICALL Java_com_magewell_LibMWCapture_MWPinVideoBuffer(JNIEnv *env, jclass, jlong hChannel, jobject pbFrame, jlong cbFrame)
{
    BYTE *frame = (BYTE *)env->GetDirectBufferAddress(pbFrame);
    return (jint)MWPinVideoBuffer((HCHANNEL)hChannel, (MWCAP_PTR)frame, (DWORD)cbFrame);
}
JNIEXPORT jint JNICALL Java_com_magewell_LibMWCapture_MWUnpinVideoBuffer(JNIEnv *env, jclass, jlong hChannel, jobject pbFrame)
{
    BYTE *frame = (BYTE *)env->GetDirectBufferAddress(pbFrame);
    return (jint)MWUnpinVideoBuffer((HCHANNEL)hChannel, (LPBYTE)frame);
}
JNIEXPORT jint JNICALL Java_com_magewell_LibMWCapture_MWGetNotifyStatus(JNIEnv *env, jclass, jlong hChannel, jlong hNotify, jlongArray pullStatus)
{
    jlong* longStatus =env->GetLongArrayElements(pullStatus, 0);
    jint ret = (jint)MWGetNotifyStatus((HCHANNEL)hChannel, (HNOTIFY)hNotify, (ULONGLONG*)longStatus);
    env->ReleaseLongArrayElements(pullStatus, longStatus, 0);
    return ret;
}
JNIEXPORT jint JNICALL Java_com_magewell_LibMWCapture_MWGetVideoBufferInfo(JNIEnv *env, jclass, jlong hChannel, jobject pVideoBufferInfo)
{
    MWCAP_VIDEO_BUFFER_INFO video_buffer_info;
    MW_RESULT ret = MWGetVideoBufferInfo((HCHANNEL)hChannel, &video_buffer_info);
    if(MW_SUCCEEDED != ret){
        return (jint)ret;
    }
    jclass objectClass = (env)->FindClass("com/magewell/MWCAP_VIDEO_BUFFER_INFO");

    jfieldID cMaxFrames = (env)->GetFieldID(objectClass, "cMaxFrames", "I");
    jfieldID iNewestBuffering = (env)->GetFieldID(objectClass, "iNewestBuffering", "B");
    jfieldID iBufferingFieldIndex = (env)->GetFieldID(objectClass, "iBufferingFieldIndex", "B");
    jfieldID iNewestBuffered = (env)->GetFieldID(objectClass, "iNewestBuffered", "B");
    jfieldID iBufferedFieldIndex = (env)->GetFieldID(objectClass, "iBufferedFieldIndex", "B");
    jfieldID iNewestBufferedFullFrame = (env)->GetFieldID(objectClass, "iNewestBufferedFullFrame", "B");
    jfieldID cBufferedFullFrames = (env)->GetFieldID(objectClass, "cBufferedFullFrames", "I");

    (env)->SetIntField(pVideoBufferInfo, cMaxFrames, video_buffer_info.cMaxFrames);
    (env)->SetByteField(pVideoBufferInfo, iNewestBuffering, video_buffer_info.iNewestBuffering);
    (env)->SetByteField(pVideoBufferInfo, iBufferingFieldIndex, video_buffer_info.iBufferingFieldIndex);
    (env)->SetByteField(pVideoBufferInfo, iNewestBuffered, video_buffer_info.iNewestBuffered);
    (env)->SetByteField(pVideoBufferInfo, iBufferedFieldIndex, video_buffer_info.iBufferedFieldIndex);
    (env)->SetByteField(pVideoBufferInfo, iNewestBufferedFullFrame, video_buffer_info.iNewestBufferedFullFrame);
    (env)->SetIntField(pVideoBufferInfo, cBufferedFullFrames, video_buffer_info.cBufferedFullFrames);

    env->DeleteLocalRef(objectClass);
    return (jint)ret;
    return 0;
}
JNIEXPORT jint JNICALL Java_com_magewell_LibMWCapture_MWGetVideoFrameInfo(JNIEnv *env, jclass, jlong hChannel, jbyte i, jobject pVideoFrameInfo)
{
    MWCAP_VIDEO_FRAME_INFO video_frame_info;
    MW_RESULT ret = MWGetVideoFrameInfo((HCHANNEL)hChannel, i, &video_frame_info);
    if(MW_SUCCEEDED != ret){
        return (jint)ret;
    }
    jclass objectClass = (env)->FindClass("com/magewell/MWCAP_VIDEO_FRAME_INFO");

    jfieldID state = (env)->GetFieldID(objectClass, "state", "I");
    jfieldID bInterlaced = (env)->GetFieldID(objectClass, "bInterlaced", "Z");
    jfieldID bSegmentedFrame = (env)->GetFieldID(objectClass, "bSegmentedFrame", "Z");
    jfieldID bTopFieldFirst = (env)->GetFieldID(objectClass, "bTopFieldFirst", "Z");
    jfieldID bTopFieldInverted = (env)->GetFieldID(objectClass, "bTopFieldInverted", "Z");
    jfieldID cx = (env)->GetFieldID(objectClass, "cx", "I");
    jfieldID cy = (env)->GetFieldID(objectClass, "cy", "I");
    jfieldID nAspectX = (env)->GetFieldID(objectClass, "nAspectX", "I");
    jfieldID nAspectY = (env)->GetFieldID(objectClass, "nAspectY", "I");
    jfieldID allFieldStartTimes = (env)->GetFieldID(objectClass, "allFieldStartTimes", "[J");
    jfieldID allFieldBufferedTimes = (env)->GetFieldID(objectClass, "allFieldBufferedTimes", "[J");
    jfieldID aSMPTETimeCodes = (env)->GetFieldID(objectClass, "aSMPTETimeCodes", "[I");

    (env)->SetIntField(pVideoFrameInfo, state, video_frame_info.state);
    (env)->SetBooleanField(pVideoFrameInfo, bInterlaced, video_frame_info.bInterlaced);
    (env)->SetBooleanField(pVideoFrameInfo, bSegmentedFrame, video_frame_info.bSegmentedFrame);
    (env)->SetBooleanField(pVideoFrameInfo, bTopFieldFirst, video_frame_info.bTopFieldFirst);
    (env)->SetBooleanField(pVideoFrameInfo, bTopFieldInverted, video_frame_info.bTopFieldInverted);
    (env)->SetIntField(pVideoFrameInfo, cx, video_frame_info.cx);
    (env)->SetIntField(pVideoFrameInfo, cy, video_frame_info.cy);
    (env)->SetIntField(pVideoFrameInfo, nAspectX, video_frame_info.nAspectX);
    (env)->SetIntField(pVideoFrameInfo, nAspectY, video_frame_info.nAspectY);

    env->DeleteLocalRef(objectClass);
    return (jint)ret;
}

JNIEXPORT jint JNICALL Java_com_magewell_LibMWCapture_MWCaptureVideoFrameToVirtualAddressEx(JNIEnv *env, jclass,jlong hChannel, jint iFrame, jobject pbFrame,
								 jlong cbFrame, jlong cbStride, jboolean bBottomUp, jlong pvContext, jlong dwFOURCC,
								 jint cx, jint cy, jint dwProcessSwitchs, jint cyParitalNotify, jlong hOSDImage,
								 jlong pOSDRects, jint cOSDRects, jshort sContrast, jshort sBrightness,
								 jshort sSaturation, jshort sHue, jint deinterlaceMode, jint aspectRatioConvertMode,
								 jlong pRectSrc, jlong pRectDest, jint nAspectX, jint nAspectY,
								 jint colorFormat, jint quantRange, jint satRange)
{
    BYTE *byteFrame = (BYTE *)env->GetDirectBufferAddress(pbFrame);
    return MWCaptureVideoFrameToVirtualAddressEx((HCHANNEL)hChannel,
                                                  (int)iFrame,
                                                  (LPBYTE)byteFrame,
                                                  (DWORD)cbFrame,
                                                  (DWORD)cbStride,
                                                  (BOOLEAN)bBottomUp,
                                                  (MWCAP_PTR64)pvContext,
                                                  (DWORD)dwFOURCC,
                                                  (int)cx,
                                                  (int)cy,
                                                  (DWORD)dwProcessSwitchs,
                                                  (int)cyParitalNotify,
                                                  (HOSD)hOSDImage,
                                                  (const RECT *)pOSDRects,
                                                  (int)cOSDRects,
                                                  (SHORT)sContrast,
                                                  (SHORT)sBrightness,
                                                  (SHORT)sSaturation,
                                                  (SHORT)sHue,
                                                  (MWCAP_VIDEO_DEINTERLACE_MODE)deinterlaceMode,
                                                  (MWCAP_VIDEO_ASPECT_RATIO_CONVERT_MODE)aspectRatioConvertMode,
                                                  (const RECT *)pRectSrc,
                                                  (const RECT *)pRectDest,
                                                  (int)nAspectX,
                                                  (int)nAspectY,
                                                  (MWCAP_VIDEO_COLOR_FORMAT)colorFormat,
                                                  (MWCAP_VIDEO_QUANTIZATION_RANGE)quantRange,
                                                  (MWCAP_VIDEO_SATURATION_RANGE)satRange);
}

JNIEXPORT jint JNICALL Java_com_magewell_LibMWCapture_MWGetVideoCaptureStatus(JNIEnv *env, jclass, jlong hChannel, jobject pStatus)
{
    MWCAP_VIDEO_CAPTURE_STATUS video_capture_status;
    MW_RESULT ret = MWGetVideoCaptureStatus((HCHANNEL)hChannel, &video_capture_status);
    if(MW_SUCCEEDED != ret){
        return (jint)ret;
    }
    jclass objectClass = (env)->FindClass("com/magewell/MWCAP_VIDEO_CAPTURE_STATUS");

    jfieldID pvContext = (env)->GetFieldID(objectClass, "pvContext", "J");
    jfieldID bPhysicalAddress = (env)->GetFieldID(objectClass, "bPhysicalAddress", "Z");
    jfieldID pvFrame = (env)->GetFieldID(objectClass, "pvFrame", "J");
    jfieldID iFrame = (env)->GetFieldID(objectClass, "iFrame", "I");
    jfieldID bFrameCompleted = (env)->GetFieldID(objectClass, "bFrameCompleted", "Z");
    jfieldID cyCompleted = (env)->GetFieldID(objectClass, "cyCompleted", "S");
    jfieldID cyCompletedPrev = (env)->GetFieldID(objectClass, "cyCompletedPrev", "S");

    (env)->SetLongField(pStatus, pvContext, video_capture_status.pvContext);
    (env)->SetBooleanField(pStatus, bPhysicalAddress, video_capture_status.bPhysicalAddress);
    (env)->SetLongField(pStatus, pvFrame, video_capture_status.pvFrame);
    (env)->SetIntField(pStatus, iFrame, video_capture_status.iFrame);
    (env)->SetBooleanField(pStatus, bFrameCompleted, video_capture_status.bFrameCompleted);
    (env)->SetShortField(pStatus, cyCompleted, video_capture_status.cyCompleted);
    (env)->SetShortField(pStatus, cyCompletedPrev, video_capture_status.cyCompletedPrev);


    env->DeleteLocalRef(objectClass);
    return (jint)ret;
}

JNIEXPORT jint JNICALL Java_com_magewell_LibMWCapture_MWStartVideoEcoCapture(JNIEnv *env, jclass, jlong hChannel, jobject pEcoCaptureOpen)
{
    MWCAP_VIDEO_ECO_CAPTURE_OPEN eco_capture_open;
    jclass objectClass = (env)->FindClass("com/magewell/MWCAP_VIDEO_ECO_CAPTURE_OPEN");

    jfieldID hEvent = (env)->GetFieldID(objectClass, "hEvent", "J");
    jfieldID dwFOURCC = (env)->GetFieldID(objectClass, "dwFOURCC", "J");
    jfieldID cx = (env)->GetFieldID(objectClass, "cx", "S");
    jfieldID cy = (env)->GetFieldID(objectClass, "cy", "S");
    jfieldID llFrameDuration = (env)->GetFieldID(objectClass, "llFrameDuration", "J");
    eco_capture_open.hEvent = (MWCAP_PTR64)(env)->GetLongField(pEcoCaptureOpen, hEvent);
    eco_capture_open.dwFOURCC = (DWORD)(env)->GetLongField(pEcoCaptureOpen, dwFOURCC);
    eco_capture_open.cx = (WORD)(env)->GetShortField(pEcoCaptureOpen, cx);
    eco_capture_open.cy = (WORD)(env)->GetShortField(pEcoCaptureOpen, cy);
    eco_capture_open.llFrameDuration = (LONGLONG)(env)->GetLongField(pEcoCaptureOpen, llFrameDuration);

    env->DeleteLocalRef(objectClass);
    return (jint)MWStartVideoEcoCapture((HCHANNEL)hChannel, &eco_capture_open);
}

JNIEXPORT jint JNICALL Java_com_magewell_LibMWCapture_MWStopVideoEcoCapture(JNIEnv *env, jclass, jlong hChannel)
{
    return (jint)MWStopVideoEcoCapture((HCHANNEL)hChannel);
}

JNIEXPORT jint JNICALL Java_com_magewell_LibMWCapture_MWCaptureSetVideoEcoFrame(JNIEnv *env, jclass, jlong hChannel, jobject pFrame)
{
    MWCAP_VIDEO_ECO_CAPTURE_FRAME eco_capture_frame;

    jclass objectClass = (env)->FindClass("com/magewell/MWCAP_VIDEO_ECO_CAPTURE_FRAME");

    jfieldID pvFrame = (env)->GetFieldID(objectClass, "pvFrame", "Ljava/nio/ByteBuffer;");
    jfieldID cbFrame = (env)->GetFieldID(objectClass, "cbFrame", "J");
    jfieldID cbStride = (env)->GetFieldID(objectClass, "cbStride", "J");
    jfieldID bBottomUp = (env)->GetFieldID(objectClass, "bBottomUp", "Z");
    jfieldID deinterlaceMode = (env)->GetFieldID(objectClass, "deinterlaceMode", "I");
    jfieldID pvContext = (env)->GetFieldID(objectClass, "pvContextIndex", "J");
    jobject frame = (env)->GetObjectField(pFrame, pvFrame);
    eco_capture_frame.pvFrame = (MWCAP_PTR64)env->GetDirectBufferAddress(frame);
    eco_capture_frame.cbFrame = (DWORD)(env)->GetLongField(pFrame, cbFrame);
    eco_capture_frame.cbStride = (DWORD)(env)->GetLongField(pFrame, cbStride);
    eco_capture_frame.bBottomUp = (BOOLEAN)(env)->GetBooleanField(pFrame, bBottomUp);
    eco_capture_frame.deinterlaceMode = (MWCAP_VIDEO_DEINTERLACE_MODE)(env)->GetIntField(pFrame, deinterlaceMode);
    eco_capture_frame.pvContext = (MWCAP_PTR64)(env)->GetLongField(pFrame, pvContext);

    env->DeleteLocalRef(objectClass);
    return (jint)MWCaptureSetVideoEcoFrame((HCHANNEL)hChannel, &eco_capture_frame);

}

JNIEXPORT jint JNICALL Java_com_magewell_LibMWCapture_MWGetVideoEcoCaptureStatus(JNIEnv *env, jclass, jlong hChannel, jobject  pStatus)
{
    MWCAP_VIDEO_ECO_CAPTURE_STATUS eco_capture_status;

    MW_RESULT ret = MWGetVideoEcoCaptureStatus((HCHANNEL)hChannel, &eco_capture_status);
    if(MW_SUCCEEDED != ret){
        return (jint)ret;
    }
    jclass objectClass = (env)->FindClass("com/magewell/MWCAP_VIDEO_ECO_CAPTURE_STATUS");
    jfieldID pvContext = (env)->GetFieldID(objectClass, "pvContextIndex", "J");
    jfieldID pvFrame = (env)->GetFieldID(objectClass, "pvFrame", "Ljava/nio/ByteBuffer;");
    jfieldID llTimestamp = (env)->GetFieldID(objectClass, "llTimestamp", "J");


     (env)->SetLongField(pStatus, pvContext, (jlong)eco_capture_status.pvContext);
     //(env)->SetLongField(pStatus, pvFrame, (jlong)eco_capture_status.pvFrame);
     (env)->SetLongField(pStatus, llTimestamp, (jlong)eco_capture_status.llTimestamp);

    env->DeleteLocalRef(objectClass);
    return (jint)ret;
}

JNIEXPORT jint JNICALL Java_com_magewell_LibMWCapture_MWGetDeviceTime(JNIEnv *env, jclass, jlong hChannel, jlongArray pllTime)
{
    jlong* longTime =env->GetLongArrayElements(pllTime, 0);
    jint ret = (jint)MWGetDeviceTime((HCHANNEL)hChannel, (LONGLONG*)longTime);
    env->ReleaseLongArrayElements(pllTime, longTime, 0);
    return ret;
}
JNIEXPORT jint JNICALL Java_com_magewell_LibMWCapture_MWGetAudioSignalStatus(JNIEnv *env, jclass, jlong hChannel, jobject pSignalStatus)
{
    MWCAP_AUDIO_SIGNAL_STATUS audio_signal_status;
    MW_RESULT ret = MWGetAudioSignalStatus((HCHANNEL)hChannel, &audio_signal_status);
    if(MW_SUCCEEDED != ret){
        return (jint)ret;
    }
    jclass objectClass = (env)->FindClass("com/magewell/MWCAP_AUDIO_SIGNAL_STATUS");

    jfieldID wChannelValid = (env)->GetFieldID(objectClass, "wChannelValid", "S");
    jfieldID bLPCM = (env)->GetFieldID(objectClass, "bLPCM", "Z");
    jfieldID cBitsPerSample = (env)->GetFieldID(objectClass, "cBitsPerSample", "B");
    jfieldID dwSampleRate = (env)->GetFieldID(objectClass, "dwSampleRate", "I");
    jfieldID bChannelStatusValid = (env)->GetFieldID(objectClass, "bChannelStatusValid", "Z");

    (env)->SetShortField(pSignalStatus, wChannelValid, audio_signal_status.wChannelValid);
    (env)->SetBooleanField(pSignalStatus, bLPCM, audio_signal_status.bLPCM);
    (env)->SetByteField(pSignalStatus, cBitsPerSample, audio_signal_status.cBitsPerSample);
    (env)->SetIntField(pSignalStatus, dwSampleRate, audio_signal_status.dwSampleRate);
    (env)->SetBooleanField(pSignalStatus, bChannelStatusValid, audio_signal_status.bChannelStatusValid);

    env->DeleteLocalRef(objectClass);
    return (jint)ret;
}

JNIEXPORT jint JNICALL Java_com_magewell_LibMWCapture_MWCaptureAudioFrame
	(JNIEnv *env, jclass, jlong hChannel, jobject pAudioCaptureFrame)
{
    MWCAP_AUDIO_CAPTURE_FRAME audioFrame;
	MW_RESULT ret = MWCaptureAudioFrame((HCHANNEL)hChannel, &audioFrame);
    if(MW_SUCCEEDED != ret){
        return (jint)ret;
    }
    jclass objectClass = (env)->FindClass("com/magewell/MWCAP_AUDIO_CAPTURE_FRAME");

    jfieldID cFrameCount = (env)->GetFieldID(objectClass, "cFrameCount", "I");
    jfieldID iFrame = (env)->GetFieldID(objectClass, "iFrame", "I");
    jfieldID dwSyncCode = (env)->GetFieldID(objectClass, "dwSyncCode", "I");
    jfieldID dwReserved = (env)->GetFieldID(objectClass, "dwReserved", "I");
    jfieldID llTimestamp = (env)->GetFieldID(objectClass, "llTimestamp", "J");
    jfieldID adwSamples = (env)->GetFieldID(objectClass, "adwSamples", "[I");

    (env)->SetIntField(pAudioCaptureFrame, cFrameCount, audioFrame.cFrameCount);
    (env)->SetIntField(pAudioCaptureFrame, iFrame, audioFrame.iFrame);
    (env)->SetIntField(pAudioCaptureFrame, dwSyncCode, audioFrame.dwSyncCode);
    (env)->SetIntField(pAudioCaptureFrame, dwReserved, audioFrame.dwReserved);
    (env)->SetLongField(pAudioCaptureFrame, llTimestamp, audioFrame.llTimestamp);

    jintArray jsamples = env->NewIntArray(sizeof(audioFrame.adwSamples)/sizeof(audioFrame.adwSamples[0]));

    env->SetIntArrayRegion(jsamples, 0, sizeof(audioFrame.adwSamples)/sizeof(audioFrame.adwSamples[0]), (jint*)audioFrame.adwSamples);
    env->SetObjectField(pAudioCaptureFrame, adwSamples, jsamples);
    env->DeleteLocalRef(jsamples);
    env->DeleteLocalRef(objectClass);
    return (jint)ret;
}

JNIEXPORT void JNICALL Java_com_magewell_LibMWCapture_MWResampleAudio(JNIEnv *env, jclass, jobject pAudioCaptureFrame, jbyteArray pcm, jint channels, jint depth)
{
    jclass objectClass = (env)->FindClass("com/magewell/MWCAP_AUDIO_CAPTURE_FRAME");
    jfieldID adwSamples = (env)->GetFieldID(objectClass, "adwSamples", "[I");
    jintArray pcmArray = (jintArray)env->GetObjectField(pAudioCaptureFrame, adwSamples);
    jint *intPcm = env->GetIntArrayElements(pcmArray, 0);
    DWORD *p_capture_buf = (DWORD *)intPcm;

    jbyte* bytePcm = env->GetByteArrayElements(pcm, 0);
    unsigned char*audio_frame = (unsigned char*)bytePcm;
    int bit_per_sample = depth*8;

    if((channels != 2) || (depth != 2)){
        return;
    }
    for (int i = 0 ; i < MWCAP_AUDIO_SAMPLES_PER_FRAME; i++){
        WORD temp = p_capture_buf[0] >> (32 - bit_per_sample);
        memcpy(audio_frame, &temp, depth);
        audio_frame += depth;

        temp = p_capture_buf[MWCAP_AUDIO_MAX_NUM_CHANNELS / 2] >> (32 - bit_per_sample);
        memcpy(audio_frame, &temp, depth);
        audio_frame += depth;
        p_capture_buf += MWCAP_AUDIO_MAX_NUM_CHANNELS;
    }
    env->ReleaseByteArrayElements(pcm, bytePcm, 0);
    env->ReleaseIntArrayElements(pcmArray, intPcm, 0);
}

JNIEXPORT jint JNICALL Java_com_magewell_LibMWCapture_MWGetAudioInputSourceArray(JNIEnv *env, jclass, jlong hChannel, jintArray pdwInputSource, jintArray pdwInputCount){
    jint* intSource = pdwInputSource ? env->GetIntArrayElements(pdwInputSource, 0): NULL;
    jint* intCount = env->GetIntArrayElements(pdwInputCount, 0);
    jint ret = (jint)MWGetAudioInputSourceArray((HCHANNEL)hChannel, (DWORD*)intSource, (DWORD*)intCount);
    if(intSource){
        env->ReleaseIntArrayElements(pdwInputSource, intSource, 0);
    }
    env->ReleaseIntArrayElements(pdwInputCount, intCount, 0);
    return ret;
}


JNIEXPORT jint JNICALL Java_com_magewell_LibMWCapture_GetFrame(JNIEnv *env, jclass, jobject pbFrame)
{
    static int count = 0;
    printf("1\n");
    BYTE *byteFrame = (BYTE *)env->GetDirectBufferAddress(pbFrame);printf("2\n");
    memset(byteFrame, count++, 1920*1080*2);printf("3\n");
    return 1;
}
#ifdef __cplusplus
}
#endif

