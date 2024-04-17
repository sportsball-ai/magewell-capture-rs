package com.magewell;

import java.nio.ByteBuffer;
import javax.swing.JApplet;
import java.io.*;
import com.magewell.MWCAP_CHANNEL_INFO;
import com.magewell.MWCAP_VIDEO_SIGNAL_STATUS;
import com.magewell.MWCAP_VIDEO_BUFFER_INFO;
import com.magewell.MWCAP_VIDEO_FRAME_INFO;
import com.magewell.MWCAP_AUDIO_CAPTURE_FRAME;
public class LibMWCapture {
	static {
		try {
			System.loadLibrary("MWCaptureJni");
		}
		catch(Exception e) {
			System.out.println("not find lib");
		}
	}
	public static long MWFOURCC(char a, char b, char c, char d){
		return ((long)a | ((long)b << 8) | ((long)c << 16) | ((long)d << 24));
	}
	public static final long MWFOURCC_UNK   = MWFOURCC('U', 'N', 'K', 'N');
	public static final long MWFOURCC_GREY  = MWFOURCC('G', 'R', 'E', 'Y');
	public static final long MWFOURCC_Y800  = MWFOURCC('Y', '8', '0', '0');
	public static final long MWFOURCC_Y8    = MWFOURCC('Y', '8', ' ', ' ');
	public static final long MWFOURCC_Y16   = MWFOURCC('Y', '1', '6', ' ');
	public static final long MWFOURCC_RGB15 = MWFOURCC('R', 'G', 'B', '5');
	public static final long MWFOURCC_RGB16 = MWFOURCC('R', 'G', 'B', '6');
	public static final long MWFOURCC_RGB24 = MWFOURCC('R', 'G', 'B', ' ');
	public static final long MWFOURCC_RGBA  = MWFOURCC('R', 'G', 'B', 'A');
	public static final long MWFOURCC_ARGB  = MWFOURCC('A', 'R', 'G', 'B');
	public static final long MWFOURCC_BGR15 = MWFOURCC('B', 'G', 'R', '5');
	public static final long MWFOURCC_BGR16 = MWFOURCC('B', 'G', 'R', '6');
	public static final long MWFOURCC_BGR24 = MWFOURCC('B', 'G', 'R', ' ');
	public static final long MWFOURCC_BGRA  = MWFOURCC('B', 'G', 'R', 'A');
	public static final long MWFOURCC_ABGR  = MWFOURCC('A', 'B', 'G', 'R');
	public static final long MWFOURCC_NV16  = MWFOURCC('N', 'V', '1', '6');
	public static final long MWFOURCC_NV61  = MWFOURCC('N', 'V', '6', '1');
	public static final long MWFOURCC_I422  = MWFOURCC('I', '4', '2', '2');
	public static final long MWFOURCC_YV16  = MWFOURCC('Y', 'V', '1', '6');
	public static final long MWFOURCC_YUY2  = MWFOURCC('Y', 'U', 'Y', '2');
	public static final long MWFOURCC_YUYV  = MWFOURCC('Y', 'U', 'Y', 'V');
	public static final long MWFOURCC_UYVY  = MWFOURCC('U', 'Y', 'V', 'Y');
	public static final long MWFOURCC_YVYU  = MWFOURCC('Y', 'V', 'Y', 'U');
	public static final long MWFOURCC_VYUY  = MWFOURCC('V', 'Y', 'U', 'Y');
	public static final long MWFOURCC_I420  = MWFOURCC('I', '4', '2', '0');
	public static final long MWFOURCC_IYUV  = MWFOURCC('I', 'Y', 'U', 'V');
	public static final long MWFOURCC_NV12  = MWFOURCC('N', 'V', '1', '2');
	public static final long MWFOURCC_YV12  = MWFOURCC('Y', 'V', '1', '2');
	public static final long MWFOURCC_NV21  = MWFOURCC('N', 'V', '2', '1');
	public static final long MWFOURCC_P010  = MWFOURCC('P', '0', '1', '0');
	public static final long MWFOURCC_P210  = MWFOURCC('P', '2', '1', '0');
	public static final long MWFOURCC_IYU2  = MWFOURCC('I', 'Y', 'U', '2');
	public static final long MWFOURCC_V308  = MWFOURCC('v', '3', '0', '8');
	public static final long MWFOURCC_AYUV  = MWFOURCC('A', 'Y', 'U', 'V');
	public static final long MWFOURCC_UYVA  = MWFOURCC('U', 'Y', 'V', 'A');
	public static final long MWFOURCC_V408  = MWFOURCC('v', '4', '0', '8');
	public static final long MWFOURCC_VYUA  = MWFOURCC('V', 'Y', 'U', 'A');
	public static final long MWFOURCC_V210  = MWFOURCC('v', '2', '1', '0');
	public static final long MWFOURCC_Y410  = MWFOURCC('Y', '4', '1', '0');
	public static final long MWFOURCC_V410  = MWFOURCC('v', '4', '1', '0');
	public static final long MWFOURCC_RGB10 = MWFOURCC('R', 'G', '1', '0');
	public static final long MWFOURCC_BGR10	= MWFOURCC('B', 'G', '1', '0');
	//MW_RESULT
	public static final int MW_SUCCEEDED      = 0;
	public static final int MW_FAILED         = 1;
	public static final int MW_ENODATA        = 2;
	public static final int MW_INVALID_PARAMS = 3;
	//MW_FAMILY_ID
	public static final short MW_FAMILY_ID_PRO_CAPTURE					= 0x00;
	public static final short MW_FAMILY_ID_VALUE_CAPTURE					= 0x01;
	public static final short MW_FAMILY_ID_USB_CAPTURE					= 0x02;
	//MWCAP_VIDEO_SIGNAL_STATE
	public static final int MWCAP_VIDEO_SIGNAL_NONE        = 0;
	public static final int MWCAP_VIDEO_SIGNAL_UNSUPPORTED = 1;
	public static final int MWCAP_VIDEO_SIGNAL_LOCKING     = 2;
	public static final int MWCAP_VIDEO_SIGNAL_LOCKED      = 3;
	//MWCAP_NOTIFY
	public static final int MWCAP_NOTIFY_INPUT_SORUCE_START_SCAN        = 0x0001;
	public static final int MWCAP_NOTIFY_INPUT_SORUCE_STOP_SCAN         = 0x0002;
	public static final int MWCAP_NOTIFY_INPUT_SORUCE_SCAN_CHANGE       = 0x0003;
	public static final int MWCAP_NOTIFY_VIDEO_INPUT_SOURCE_CHANGE		= 0x0004;
	public static final int MWCAP_NOTIFY_AUDIO_INPUT_SOURCE_CHANGE		= 0x0008;
	public static final int MWCAP_NOTIFY_INPUT_SPECIFIC_CHANGE			= 0x0010;
	public static final int MWCAP_NOTIFY_VIDEO_SIGNAL_CHANGE			= 0x0020;
	public static final int MWCAP_NOTIFY_AUDIO_SIGNAL_CHANGE			= 0x0040;
	public static final int MWCAP_NOTIFY_VIDEO_FIELD_BUFFERING			= 0x0080;
	public static final int MWCAP_NOTIFY_VIDEO_FRAME_BUFFERING			= 0x0100;
	public static final int MWCAP_NOTIFY_VIDEO_FIELD_BUFFERED			= 0x0200;
	public static final int MWCAP_NOTIFY_VIDEO_FRAME_BUFFERED			= 0x0400;
	public static final int MWCAP_NOTIFY_VIDEO_SMPTE_TIME_CODE			= 0x0800;
	public static final int MWCAP_NOTIFY_AUDIO_FRAME_BUFFERED			= 0x1000;
	public static final int MWCAP_NOTIFY_AUDIO_INPUT_RESET				= 0x2000;
	public static final int MWCAP_NOTIFY_VIDEO_SAMPLING_PHASE_CHANGE	= 0x4000;
	public static final int MWCAP_NOTIFY_LOOP_THROUGH_CHANGED			= 0x8000;
	public static final int MWCAP_NOTIFY_LOOP_THROUGH_EDID_CHANGED		= 0x10000;
	public static final int MWCAP_NOTIFY_NEW_SDI_ANC_PACKET				= 0x20000;
/*	public static final int MWCAP_NOTIFY_HDMI_INFOFRAME_AVI				(1ULL << (32 + MWCAP_HDMI_INFOFRAME_ID_AVI))
	public static final int MWCAP_NOTIFY_HDMI_INFOFRAME_AUDIO			(1ULL << (32 + MWCAP_HDMI_INFOFRAME_ID_AUDIO))
	public static final int MWCAP_NOTIFY_HDMI_INFOFRAME_SPD				(1ULL << (32 + MWCAP_HDMI_INFOFRAME_ID_SPD))
	public static final int MWCAP_NOTIFY_HDMI_INFOFRAME_MS				(1ULL << (32 + MWCAP_HDMI_INFOFRAME_ID_MS))
	public static final int MWCAP_NOTIFY_HDMI_INFOFRAME_VS				(1ULL << (32 + MWCAP_HDMI_INFOFRAME_ID_VS))
	public static final int MWCAP_NOTIFY_HDMI_INFOFRAME_ACP				(1ULL << (32 + MWCAP_HDMI_INFOFRAME_ID_ACP))
	public static final int MWCAP_NOTIFY_HDMI_INFOFRAME_ISRC1			(1ULL << (32 + MWCAP_HDMI_INFOFRAME_ID_ISRC1))
	public static final int MWCAP_NOTIFY_HDMI_INFOFRAME_ISRC2			(1ULL << (32 + MWCAP_HDMI_INFOFRAME_ID_ISRC2))
	public static final int MWCAP_NOTIFY_HDMI_INFOFRAME_GAMUT			(1ULL << (32 + MWCAP_HDMI_INFOFRAME_ID_GAMUT))*/

    //MWCAP_VIDEO_DEINTERLACE_MODE
	public static final int MWCAP_VIDEO_DEINTERLACE_WEAVE					= 0x00;///<Weave mode
	public static final int MWCAP_VIDEO_DEINTERLACE_BLEND					= 0x01;///<Blend mode
	public static final int MWCAP_VIDEO_DEINTERLACE_TOP_FIELD				= 0x02;///<Only uses top subframe data
	public static final int MWCAP_VIDEO_DEINTERLACE_BOTTOM_FIELD			= 0x03;///<Only uses bottom subframe data
	//MWCAP_VIDEO_ASPECT_RATIO_CONVERT_MODE
	public static final int MWCAP_VIDEO_ASPECT_RATIO_IGNORE					= 0x00;///<Ignore: Ignores the original aspect ratio and stretches to full-screen.
	public static final int MWCAP_VIDEO_ASPECT_RATIO_CROPPING				= 0x01;///<Cropping: Expands to full-screen and remove parts of the image when necessary to keep the original aspect ratio.
	public static final int MWCAP_VIDEO_ASPECT_RATIO_PADDING				= 0x02;///<Padding: Fits to screen and add black borders to keep the original aspect ratio.
	//MWCAP_VIDEO_COLOR_FORMAT
	public static final int MWCAP_VIDEO_COLOR_FORMAT_UNKNOWN				= 0x00;///<unknown color format
	public static final int MWCAP_VIDEO_COLOR_FORMAT_RGB					= 0x01;///<RGB
	public static final int MWCAP_VIDEO_COLOR_FORMAT_YUV601					= 0x02;///<YUV601
	public static final int MWCAP_VIDEO_COLOR_FORMAT_YUV709					= 0x03;///<YUV709
	public static final int MWCAP_VIDEO_COLOR_FORMAT_YUV2020				= 0x04;///<YUV2020
	public static final int MWCAP_VIDEO_COLOR_FORMAT_YUV2020C				= 0x05;///<YUV2020C
	//MWCAP_VIDEO_QUANTIZATION_RANGE
	public static final int MWCAP_VIDEO_QUANTIZATION_UNKNOWN				= 0x00;///<The default quantization range
	public static final int MWCAP_VIDEO_QUANTIZATION_FULL					= 0x01;///<Full range, which has 8-bit data. The black-white color range is 0-255/1023/4095/65535.
	public static final int MWCAP_VIDEO_QUANTIZATION_LIMITED				= 0x02;///<Limited range, which has 8-bit data. The black-white color range is 16/64/256/4096-235(240)/940(960)/3760(3840)/60160(61440).
	//MWCAP_VIDEO_SATURATION_RANGE
	public static final int MWCAP_VIDEO_SATURATION_UNKNOWN					= 0x00;///<The default saturation range
	public static final int MWCAP_VIDEO_SATURATION_FULL						= 0x01;///<Full range, which has 8-bit data. The black-white color range is 0-255/1023/4095/65535
	public static final int MWCAP_VIDEO_SATURATION_LIMITED					= 0x02;///<Limited range, which has 8-bit data. The black-white color range is 16/64/256/4096-235(240)/940(960)/3760(3840)/60160(61440)
	public static final int MWCAP_VIDEO_SATURATION_EXTENDED_GAMUT			= 0x03;///<Extended range, which has 8-bit data. The black-white color range is 1/4/16/256-254/1019/4079/65279


	public static final int MWCAP_AUDIO_SAMPLES_PER_FRAME = 192;
	public static final int MWCAP_AUDIO_MAX_NUM_CHANNELS = 8;

	public static native int MWGetVersion(byte[] pbyMaj, byte[] pbyMin, short[] pwBuild);
	public static native boolean MWCaptureInitInstance();
	public static native void MWCaptureExitInstance();
	public static native int MWGetChannelCount();
	public static native long FOURCC_CalcMinStride(long dwFOURCC, int cx, long dwAlign);
	public static native long FOURCC_CalcImageSize(long dwFOURCC, int cx, int cy, long cbStride);

	public static native int MWGetChannelInfoByIndex(int index, MWCAP_CHANNEL_INFO pChannelInfo);
	public static native int MWGetDevicePath(int nIndex, byte[] pDevicePath);
	public static native long MWOpenChannelByPath(byte[] pDevicePath);
	public static native int MWGetChannelInfo(long hChannel, MWCAP_CHANNEL_INFO pChannelInfo);
	public static native void MWCloseChannel(long hChannel);

	public static native long MWCreateEvent();
	public static native int MWCloseEvent(long hEvent);
	public static native long MWCreateEcoEvent();
	public static native int MWCloseEcoEvent(long hEvent);

	public static native int MWStartVideoCapture(long hChannel, long hEvent);
	public static native int MWStopVideoCapture(long hChannel);
	public static native int MWStartAudioCapture(long hChannel);
	public static native int MWStopAudioCapture(long hChannel);
	public static native int MWGetVideoSignalStatus(long hChannel, MWCAP_VIDEO_SIGNAL_STATUS pSignalStatus);
	public static native long MWRegisterNotify(long hChannel, long hEvent, int dwEnableBits);
	public static native int MWUnregisterNotify(long hChannel, long hEvent);
	public static native int MWPinVideoBuffer(long hChannel, ByteBuffer pbFrame, long cbFrame);
	public static native int MWUnpinVideoBuffer(long hChannel, ByteBuffer pbFrame);
	public static native int MWWaitEvent(long hEvent, int nTimeout);
	public static native int MWWaitEcoEvent(long hEvent, int nTimeout);
	public static native int MWGetNotifyStatus(long	hChannel, long hNotify, long[] pullStatus);
	public static native int MWGetVideoBufferInfo(long hChannel, MWCAP_VIDEO_BUFFER_INFO pVideoBufferInfo);
	public static native int MWGetVideoFrameInfo(long hChannel, byte i, MWCAP_VIDEO_FRAME_INFO pVideoFrameInfo);
	public static native int MWCaptureVideoFrameToVirtualAddressEx(long hChannel, int iFrame, ByteBuffer pbFrame,
								 long cbFrame, long cbStride, boolean bBottomUp, long pvContext, long dwFOURCC,
								 int cx, int cy, int dwProcessSwitchs, int cyParitalNotify, long hOSDImage,
								 long pOSDRects, int cOSDRects, short sContrast, short sBrightness,
								 short sSaturation, short sHue, int deinterlaceMode, int aspectRatioConvertMode,
								 long pRectSrc, long pRectDest, int nAspectX, int nAspectY,
								 int colorFormat, int quantRange, int satRange);
	public static native int MWGetVideoCaptureStatus(long hChannel, MWCAP_VIDEO_CAPTURE_STATUS pStatus);
	public static native int MWStartVideoEcoCapture(long hChannel, MWCAP_VIDEO_ECO_CAPTURE_OPEN	pEcoCaptureOpen);
	public static native int MWStopVideoEcoCapture(long hChannel);
	public static native int MWCaptureSetVideoEcoFrame(long hChannel, MWCAP_VIDEO_ECO_CAPTURE_FRAME pFrame);
	public static native int MWGetVideoEcoCaptureStatus(long hChannel, MWCAP_VIDEO_ECO_CAPTURE_STATUS  pStatus);

	public static native int MWGetAudioSignalStatus(long hChannel, MWCAP_AUDIO_SIGNAL_STATUS pSignalStatus);
	public static native int MWCaptureAudioFrame(long hChannel, MWCAP_AUDIO_CAPTURE_FRAME pAudioCaptureFrame);
	public static native void MWResampleAudio(MWCAP_AUDIO_CAPTURE_FRAME pAudioCaptureFrame, byte[] pcm,int channels, int depth);
	public static native int MWGetDeviceTime(long hChannel, long[] pllTime);

	public static native int MWGetAudioInputSourceArray(long hChannel, int[] pdwInputSource, int[] pdwInputCount);
	public static native int GetFrame(ByteBuffer pbFrame);

}
