import ctypes
from ctypes import *
MWCapture = CDLL('./c_libs/libMWCapturePy.so')

def MAKE_MWFOURCC(a,b,c,d):
    return ord(a[0]) + (ord(b[0]) << 8) + (ord(c[0]) << 16) + (ord(d[0]) << 24)

MWFOURCC_UNKs        = MAKE_MWFOURCC('0', '1', '2', '3')
MWFOURCC_UNK        = MAKE_MWFOURCC('U', 'N', 'K', 'N')
MWFOURCC_GREY       = MAKE_MWFOURCC('G', 'R', 'E', 'Y')					
MWFOURCC_Y800       = MAKE_MWFOURCC('Y', '8', '0', '0')
MWFOURCC_Y8         = MAKE_MWFOURCC('Y', '8', ' ', ' ')
MWFOURCC_Y16        = MAKE_MWFOURCC('Y', '1', '6', ' ')
MWFOURCC_RGB15      = MAKE_MWFOURCC('R', 'G', 'B', '5')
MWFOURCC_RGB16      = MAKE_MWFOURCC('R', 'G', 'B', '6')
MWFOURCC_RGB24      = MAKE_MWFOURCC('R', 'G', 'B', ' ')					
MWFOURCC_RGBA       = MAKE_MWFOURCC('R', 'G', 'B', 'A')
MWFOURCC_ARGB       = MAKE_MWFOURCC('A', 'R', 'G', 'B')
MWFOURCC_BGR15      = MAKE_MWFOURCC('B', 'G', 'R', '5')
MWFOURCC_BGR16      = MAKE_MWFOURCC('B', 'G', 'R', '6')
MWFOURCC_BGR24      = MAKE_MWFOURCC('B', 'G', 'R', ' ')
MWFOURCC_BGRA       = MAKE_MWFOURCC('B', 'G', 'R', 'A')
MWFOURCC_ABGR       = MAKE_MWFOURCC('A', 'B', 'G', 'R')
MWFOURCC_NV16       = MAKE_MWFOURCC('N', 'V', '1', '6')
MWFOURCC_NV61       = MAKE_MWFOURCC('N', 'V', '6', '1')
MWFOURCC_I422       = MAKE_MWFOURCC('I', '4', '2', '2')
MWFOURCC_YV16       = MAKE_MWFOURCC('Y', 'V', '1', '6')
MWFOURCC_YUY2       = MAKE_MWFOURCC('Y', 'U', 'Y', '2')
MWFOURCC_YUYV       = MAKE_MWFOURCC('Y', 'U', 'Y', 'V')
MWFOURCC_UYVY       = MAKE_MWFOURCC('U', 'Y', 'V', 'Y')
MWFOURCC_YVYU       = MAKE_MWFOURCC('Y', 'V', 'Y', 'U')
MWFOURCC_VYUY       = MAKE_MWFOURCC('V', 'Y', 'U', 'Y')
MWFOURCC_I420       = MAKE_MWFOURCC('I', '4', '2', '0')
MWFOURCC_IYUV       = MAKE_MWFOURCC('I', 'Y', 'U', 'V')
MWFOURCC_NV12       = MAKE_MWFOURCC('N', 'V', '1', '2')
MWFOURCC_YV12       = MAKE_MWFOURCC('Y', 'V', '1', '2')
MWFOURCC_NV21       = MAKE_MWFOURCC('N', 'V', '2', '1')
MWFOURCC_P010       = MAKE_MWFOURCC('P', '0', '1', '0')
MWFOURCC_P210       = MAKE_MWFOURCC('P', '2', '1', '0')
MWFOURCC_IYU2       = MAKE_MWFOURCC('I', 'Y', 'U', '2')
MWFOURCC_V308       = MAKE_MWFOURCC('v', '3', '0', '8')
MWFOURCC_AYUV       = MAKE_MWFOURCC('A', 'Y', 'U', 'V')
MWFOURCC_UYVA       = MAKE_MWFOURCC('U', 'Y', 'V', 'A')
MWFOURCC_V408       = MAKE_MWFOURCC('v', '4', '0', '8')
MWFOURCC_VYUA       = MAKE_MWFOURCC('V', 'Y', 'U', 'A')
MWFOURCC_V210       = MAKE_MWFOURCC('v', '2', '1', '0')
MWFOURCC_Y410       = MAKE_MWFOURCC('Y', '4', '1', '0')
MWFOURCC_V410       = MAKE_MWFOURCC('v', '4', '1', '0')
MWFOURCC_RGB10      = MAKE_MWFOURCC('R', 'G', '1', '0')
MWFOURCC_BGR10      = MAKE_MWFOURCC('B', 'G', '1', '0')
#MW_RESULT
MW_SUCCEEDED      = 0
MW_FAILED         = 1#<Operation failed.
MW_ENODATA        = 2
MW_INVALID_PARAMS = 3#Invalid parameters.
#MW_FAMILY_ID
MW_FAMILY_ID_PRO_CAPTURE = 0
MW_FAMILY_ID_ECO_CAPTURE = 1
MW_FAMILY_ID_USB_CAPTURE = 2
#MWCAP_VIDEO_SIGNAL_STATE
MWCAP_VIDEO_SIGNAL_NONE         = 0
MWCAP_VIDEO_SIGNAL_UNSUPPORTED  = 1
MWCAP_VIDEO_SIGNAL_LOCKING      = 2
MWCAP_VIDEO_SIGNAL_LOCKED	    = 3
#MWCAP_VIDEO_DEINTERLACE_MODE
MWCAP_VIDEO_DEINTERLACE_WEAVE         = 0
MWCAP_VIDEO_DEINTERLACE_BLEND         = 1
MWCAP_VIDEO_DEINTERLACE_TOP_FIELD     = 2
MWCAP_VIDEO_DEINTERLACE_BOTTOM_FIELD  = 3
#MWCAP_VIDEO_ASPECT_RATIO_CONVERT_MODE
MWCAP_VIDEO_ASPECT_RATIO_IGNORE					= 0x00
MWCAP_VIDEO_ASPECT_RATIO_CROPPING				= 0x01
MWCAP_VIDEO_ASPECT_RATIO_PADDING				= 0x02
#MWCAP_VIDEO_COLOR_FORMAT
MWCAP_VIDEO_COLOR_FORMAT_UNKNOWN				= 0x00
MWCAP_VIDEO_COLOR_FORMAT_RGB					= 0x01
MWCAP_VIDEO_COLOR_FORMAT_YUV601					= 0x02
MWCAP_VIDEO_COLOR_FORMAT_YUV709					= 0x03
MWCAP_VIDEO_COLOR_FORMAT_YUV2020				= 0x04
MWCAP_VIDEO_COLOR_FORMAT_YUV2020C				= 0x05
#MWCAP_VIDEO_SATURATION_RANGE
MWCAP_VIDEO_SATURATION_UNKNOWN					= 0x00
MWCAP_VIDEO_SATURATION_FULL						= 0x01
MWCAP_VIDEO_SATURATION_LIMITED					= 0x02
MWCAP_VIDEO_SATURATION_EXTENDED_GAMUT			= 0x03 
#MWCAP_VIDEO_QUANTIZATION_RANGE		
MWCAP_VIDEO_QUANTIZATION_UNKNOWN				= 0x00#,///<The default quantization range
MWCAP_VIDEO_QUANTIZATION_FULL					= 0x01#,///<Full range, which has 8-bit data. The black-white color range is 0-255/1023/4095/65535.
MWCAP_VIDEO_QUANTIZATION_LIMITED				= 0x02# ///<Limited range, which has 8-bit data. The black-white color range is 16/64/256/4096-235(240)/940(960)/3760(3840)/60160(61440).


MWCAP_NOTIFY_VIDEO_FIELD_BUFFERING = 0x0080
MWCAP_NOTIFY_VIDEO_FRAME_BUFFERING = 0x0100
MWCAP_NOTIFY_VIDEO_FIELD_BUFFERED  = 0x0200
MWCAP_NOTIFY_VIDEO_FRAME_BUFFERED  = 0x0400

MWCAP_NOTIFY_AUDIO_FRAME_BUFFERED  = 0x1000
'''
typedef struct _MWCAP_CHANNEL_INFO {		
	WORD											wFamilyID;								///<Product type, refers to #MW_FAMILY_ID
	WORD											wProductID;								///<device ID, refers to  #MWCAP_PRODUCT_ID
	CHAR											chHardwareVersion;						///<Hardware version ID
	BYTE											byFirmwareID;							///<Firmware ID
	DWORD											dwFirmwareVersion;						///<Firmware version
	DWORD											dwDriverVersion;						///<Driver version
	CHAR											szFamilyName[MW_FAMILY_NAME_LEN];		///<Product name
	CHAR											szProductName[MW_PRODUCT_NAME_LEN];		///<Product type
	CHAR											szFirmwareName[MW_FIRMWARE_NAME_LEN];	///<Firmware name
	CHAR											szBoardSerialNo[MW_SERIAL_NO_LEN];		///<Hardware serial number
	BYTE											byBoardIndex;							///<Rotary ID located on the capture card, 0~F.
	BYTE											byChannelIndex;							///<Channel index of the capture card, which starts from 0.
} MWCAP_CHANNEL_INFO;
'''
class MWCAP_CHANNEL_INFO(Structure):
    _pack_ = 1
    _fields_ = [('wFamilyID',c_ushort),
               ('wProductID',c_ushort),
               ('chHardwareVersion',c_char),
               ('byFirmwareID',c_byte),
               ('dwFirmwareVersion',c_uint32),
               ('dwDriverVersion',c_uint32),
               ('szFamilyName',c_char*64),
               ('szProductName',c_char*64),
               ('szFirmwareName',c_char*64),
               ('szBoardSerialNo',c_char*16),
               ('byBoardIndex',c_byte),
               ('byChannelIndex',c_byte)]

'''
typedef struct _MWCAP_AUDIO_CAPTURE_FRAME {
	DWORD											cFrameCount;																///<Number of bufferred frames
	DWORD											iFrame;																		///<Current frame index
	DWORD											dwSyncCode;																	///<Sync code of audio frame data
	DWORD											dwReserved;																	///<Reserved
	LONGLONG										llTimestamp;																///<The timestamp of audio frame
	DWORD											adwSamples[MWCAP_AUDIO_SAMPLES_PER_FRAME * MWCAP_AUDIO_MAX_NUM_CHANNELS];	///<Audio sample data. Each sample is 32-bit width, and high bit effective. The priority of the path is: Left0, Left1, Left2, Left3, right0, right1, right2, right3
} MWCAP_AUDIO_CAPTURE_FRAME;'''
MWCAP_AUDIO_SAMPLES_PER_FRAME = 192
MWCAP_AUDIO_MAX_NUM_CHANNELS = 8
MWCAP_AUDIO_FRAME_SIZE = MWCAP_AUDIO_SAMPLES_PER_FRAME*MWCAP_AUDIO_MAX_NUM_CHANNELS
class MWCAP_AUDIO_CAPTURE_FRAME(Structure):
    _pack_ = 1
    _fields_ = [('cFrameCount',c_uint32),
               ('iFrame',c_uint32),
               ('dwSyncCode',c_uint32),
               ('dwReserved',c_uint32),
               ('llTimestamp',c_int64),
               ('adwSamples',c_uint32*MWCAP_AUDIO_FRAME_SIZE)]


'''
typedef struct _MWCAP_VIDEO_SIGNAL_STATUS {
	MWCAP_VIDEO_SIGNAL_STATE						state;									///<Defines the accessibility of this video signal
	int												x;										///<Horizontal start position
	int												y;										///<Vertical start position
	int												cx;										///<Image width
	int												cy;										///<Image height
	int												cxTotal;								///<Total width
	int												cyTotal;								///<Total height
	BOOLEAN											bInterlaced;							///<Whether the signal is interlaced 
	DWORD											dwFrameDuration;						///<Frame interval of video frame
	int												nAspectX;								///<Width of video ratio
	int												nAspectY;								///<Height of video ratio
	BOOLEAN											bSegmentedFrame;						///<Whether the signal is segmented frame
	MWCAP_VIDEO_FRAME_TYPE							frameType;								///<video frame type
	MWCAP_VIDEO_COLOR_FORMAT						colorFormat;							///<video color format
	MWCAP_VIDEO_QUANTIZATION_RANGE					quantRange;								///<Quantization range
	MWCAP_VIDEO_SATURATION_RANGE					satRange;								///<saturation range
} MWCAP_VIDEO_SIGNAL_STATUS;
'''
class MWCAP_VIDEO_SIGNAL_STATUS(Structure):
    _pack_ = 1
    _fields_ = [('state',c_uint32),
               ('x',c_int32),
               ('y',c_int32),
               ('cx',c_int32),
               ('cx',c_int32),
               ('cxTotal',c_int32),
               ('cyTotal',c_int32),
               ('bInterlaced',c_bool),
               ('dwFrameDuration',c_uint32),
               ('nAspectX',c_int32),
               ('nAspectY',c_int32),
               ('bSegmentedFrame',c_bool),
               ('frameType',c_uint32),
               ('colorFormat',c_uint32),
               ('colorFormat',c_uint32),
               ('adwSamples',c_uint32)]
'''
typedef struct _MWCAP_VIDEO_CAPTURE_STATUS {
    MWCAP_PTR   								pvContext;																	///<The context of video capture

	BOOLEAN											bPhysicalAddress;															///<Whether to use the physical address to store the capture data 
	union {
        MWCAP_PTR   								pvFrame;																	///<The memory address to store the capture data 
		LARGE_INTEGER								liPhysicalAddress;															///<The physical address to store the capture data
    };

	int												iFrame;																		///<The index of capturing frame
	BOOLEAN											bFrameCompleted;															///<Whether a frame is fully captured
	WORD											cyCompleted;																///<Number of frames captured
	WORD											cyCompletedPrev;															///<Number of frames captured previously
} MWCAP_VIDEO_CAPTURE_STATUS;
'''
class MWCAP_VIDEO_CAPTURE_STATUS(Structure):
    _pack_ = 1
    _fields_ = [('pvContext',c_void_p),
               ('bPhysicalAddress',c_bool),
               ('pvFrame',c_void_p),
               ('iFrame',c_int32),
               ('bFrameCompleted',c_bool),
               ('cyCompleted',c_ushort),
               ('cyCompletedPrev',c_ushort)]

'''
typedef union _IEC60958_CHANNEL_STATUS {
    BYTE			abyData[24];
    WORD			awData[12];
    DWORD			adwData[6];

    struct {
        BYTE		byProfessional : 1;
        BYTE		byNotLPCM : 1;
        BYTE		byEncodedAudioSignalEmphasis : 3;	// 000: Emphasis not indicated, 001: No emphasis, 011: 50/15 us emphasis, 111: CCITT J.17 emphasis
        BYTE		bySourceSamplingFreqUnlocked : 1;
        BYTE		byEncodedSamplingFreq : 2;			// 00: Not indicated, 10: 48, 01: 44.1, 11: 32

        BYTE		byEncodedChannelMode : 4;
        BYTE		byEncodedUserBitsManagement : 4;

        BYTE		byZero : 1;
        BYTE		bySingleCoordinationSignal : 1;
        BYTE		byWordLength : 4;					// 0010: 16, 1100: 17, 0100: 18, 1000: 19, 1010: 20, 0011: 20, 1101: 21, 0101: 22, 1001: 23, 1011: 24
        BYTE		byAlignmentLevel : 2;				// 00: Not indicated, 10: 20 dB, 01: 18.06 dB, 11: Reserved

        BYTE		byChannelNumberOrMultiChannelMode : 7;
        BYTE		byDefinedMultiChannelMode : 1;

        BYTE		byReferenceSignal : 2;				// 00: Not a reference signal, 10: Grade 1, 01: Grade 2, 11: Reserved
        BYTE		byReserved1 : 1;
        BYTE		bySamplingFreq : 4;					// 0000: Not indicated, 0001: 24, 0010: 96, 0011: 192, 1001: 22.05, 1010: 88.2, 1011: 176.4, 1111: User defined
        BYTE		bySamplingFreqScaling : 1;

        BYTE		byReserved2;

        BYTE		achChannelOrigin[4];
        BYTE		achChannelDestination[4];
        DWORD		dwLocalSampleAddress;
        DWORD		dwTimeOfDaySampleAddress;
        BYTE		byReliabilityFlags;

        BYTE		byCRC;
    } Professional;

    struct {
        BYTE		byProfessional : 1;
        BYTE		byNotLPCM : 1;
        BYTE		byNoCopyright : 1;
        BYTE		byAdditionalFormatInfo : 3;
        BYTE		byMode : 2;

        BYTE		byCategoryCode;

        BYTE		bySourceNumber : 4;
        BYTE		byChannelNumber : 4;

        BYTE		bySamplingFreq : 4;					// 0100: 22.05, 0000: 44.1, 1000: 88.2, 1100: 176.4, 0110: 24, 0010: 48, 1010: 96, 1110: 192, 0011: 32, 0001: Not indicated, 1001: 768
        BYTE		byClockAccuracy : 2;				// 00: Level II, 10: Level I, 01: Level III, 11: Not matched
        BYTE		byReserved1 : 2;

        BYTE		byWordLength : 4;					// 0010: 16, 1100: 17, 0100: 18, 1000: 19, 1010: 20, 0011: 20, 1101: 21, 0101: 22, 1001: 23, 1011: 24
        BYTE		byOrigSamplingFreq : 4;				// 1111: 44.1, 0111: 88.2, 1011: 22.05, 0011: 176.4, 1101: 48, 0101: 96, 1001: 24, 0001: 192, 0110: 8, 1010: 11.025, 0010: 12, 1100: 32, 1000: 16, 0000: Not indicated

        BYTE		byCGMS_A;							// 00: Copying permitted, 10: Condition not be used, 01: One generation only, 11: No copying is permitted
    } Consumer;
} IEC60958_CHANNEL_STATUS;'''

class IEC60958_CHANNEL_STATUS_BYTE(Structure):
    _pack_ = 1
    _fields_ = [('abyData',c_byte * 24)]
    
class IEC60958_CHANNEL_STATUS_WORD(Structure):
    _pack_ = 1
    _fields_ = [('awData',c_uint16 * 12)]

class IEC60958_CHANNEL_STATUS_DWORD(Structure):
    _pack_ = 1
    _fields_ = [('adwData',c_uint32 * 6)]
'''
class IEC60958_CHANNEL_STATUS_Professional(Structure):

class IEC60958_CHANNEL_STATUS_Consumer(Structure):
'''
'''
typedef struct _MWCAP_AUDIO_SIGNAL_STATUS {
	WORD											wChannelValid;							///<Valid audio channel mask.The lowest bit indicates whether the 1st and 2nd channels are valid, the second bit indicates whether the 3rd and 4th channels are valid, the third bit indicates whether the 5th and 6th channels are valid, and the fourth bit indicates whether the 7th and 8th channels are valid.
	BOOLEAN											bLPCM;									///<Whether the signal is LPCM
	BYTE											cBitsPerSample;							///<Bit depth of each audio sampling
	DWORD											dwSampleRate;							///<Sample rate
	BOOLEAN											bChannelStatusValid;					///<Whether channel status is valid
	IEC60958_CHANNEL_STATUS							channelStatus;							///<The audio channel status
} MWCAP_AUDIO_SIGNAL_STATUS;
'''


class MWCAP_AUDIO_SIGNAL_STATUS(Structure):
    _pack_ = 1
    _fields_ = [('wChannelValid',c_uint16),
               ('bLPCM',c_bool),
               ('cBitsPerSample',c_byte),
               ('dwSampleRate',c_uint32),
               ('bChannelStatusValid',c_bool),
               ('channelStatus',IEC60958_CHANNEL_STATUS_BYTE)]

'''
typedef struct _MWCAP_VIDEO_BUFFER_INFO {
	DWORD											cMaxFrames;									///<Maximum number of frames in on-board cache

	BYTE											iNewestBuffering;							///<The number of the slices being bufferred. A frame of video data may contain multiple slices.
	BYTE											iBufferingFieldIndex;						///<The sequence number of fields being bufferred.

	BYTE											iNewestBuffered;							///<The sequence number of slices the latest bufferred piece.
	BYTE											iBufferedFieldIndex;						///<The sequence number of the latest bufferred field

	BYTE											iNewestBufferedFullFrame;					///<The sequence number of the latest bufferred frame
	DWORD											cBufferedFullFrames;						///<Number of fully bufferred full frames
} MWCAP_VIDEO_BUFFER_INFO;'''

class MWCAP_VIDEO_BUFFER_INFO(Structure):
    _pack_ = 1
    _fields_ = [('cMaxFrames',c_uint32),
               ('iNewestBuffering',c_ubyte),
               ('iBufferingFieldIndex',c_ubyte),
               ('iNewestBuffered',c_ubyte),
               ('iBufferedFieldIndex',c_ubyte),
               ('iNewestBufferedFullFrame',c_ubyte),
               ('cBufferedFullFrames',c_uint32)]
'''
typedef struct _MWCAP_SMPTE_TIMECODE {
	BYTE 											byFrames;									///<Frames number
	BYTE											bySeconds;									///<Seconds
	BYTE											byMinutes;									///<Minutes
	BYTE											byHours;									///<Hours
} MWCAP_SMPTE_TIMECODE;'''

class MWCAP_SMPTE_TIMECODE(Structure):
    _pack_ = 1
    _fields_ = [('byFrames',c_ubyte),
               ('bySeconds',c_ubyte),
               ('byMinutes',c_ubyte),
               ('byHours',c_ubyte)]
'''
typedef struct _MWCAP_VIDEO_FRAME_INFO {
	MWCAP_VIDEO_FRAME_STATE							state;										///<The state of the video framess

	BOOLEAN											bInterlaced;								///<Whether an interlaced signal
	BOOLEAN											bSegmentedFrame;							///<Whether a segmented frame
	BOOLEAN											bTopFieldFirst;								///<Whether the top subframe is in front
	BOOLEAN											bTopFieldInverted;							///<Whether to reverse the top subframe

	int												cx;											///<Width of video frames
	int												cy;											///<Height of video frames
	int												nAspectX;									///<Width of the ratio 
	int												nAspectY;									///<Height of the ratio

	LONGLONG										allFieldStartTimes[2];						///<Start time of capturing top and bottom subframe respectively
	LONGLONG										allFieldBufferedTimes[2];					///<Fully bufferred time of top and bottom frame respectively
	MWCAP_SMPTE_TIMECODE							aSMPTETimeCodes[2];							///<Time code of top and bottom frame respectively
} MWCAP_VIDEO_FRAME_INFO;'''

class MWCAP_VIDEO_FRAME_INFO(Structure):
    _pack_ = 1
    _fields_ = [('state',c_uint32),
               ('bInterlaced',c_bool),
               ('bSegmentedFrame',c_bool),
               ('bTopFieldFirst',c_bool),
               ('bTopFieldInverted',c_bool),
               ('cx',c_int32),
               ('cy',c_int32),
               ('nAspectX',c_int32),
               ('nAspectY',c_int32),
               ('allFieldStartTimes',c_int64*2),
               ('allFieldBufferedTimes',c_int64*2),
               ('aSMPTETimeCodes',MWCAP_SMPTE_TIMECODE*2)]
'''
typedef struct _MWCAP_VIDEO_ECO_CAPTURE_OPEN {
	MWCAP_PTR64										hEvent;																///<Handle of capture event

	DWORD											dwFOURCC;															///<Capture format
	WORD											cx;																	///<Width
	WORD											cy;																	///<Height
	LONGLONG										llFrameDuration;													///<Interval, -1 indicates follow format of input source
} MWCAP_VIDEO_ECO_CAPTURE_OPEN;'''

class MWCAP_VIDEO_ECO_CAPTURE_OPEN(Structure):
    _pack_ = 1
    _fields_ = [('hEvent',c_void_p),
               ('dwFOURCC',c_uint32),
               ('cx',c_uint16),
               ('cy',c_uint16),
               ('llFrameDuration',c_int64)]

'''
typedef struct _MWCAP_VIDEO_ECO_CAPTURE_FRAME {
	MWCAP_PTR64										pvFrame;															///<The storage address for video capturing
	DWORD											cbFrame;															///<The size of storage for video capturing
	DWORD											cbStride;															///<Width of capture video frame

	BOOLEAN											bBottomUp;															///<Whether to flip
	MWCAP_VIDEO_DEINTERLACE_MODE					deinterlaceMode;													///<DeinterlaceMode

	MWCAP_PTR64										pvContext;															///<Context of ECO 
} MWCAP_VIDEO_ECO_CAPTURE_FRAME;'''

class MWCAP_VIDEO_ECO_CAPTURE_FRAME(Structure):
    _pack_ = 1
    _fields_ = [('pvFrame',c_void_p),
               ('cbFrame',c_uint32),
               ('cbStride',c_uint32),
               ('bBottomUp',c_bool),
               ('deinterlaceMode',c_uint32),
               ('pvContext',c_void_p)]

'''
typedef struct _MWCAP_VIDEO_ECO_CAPTURE_STATUS {
	MWCAP_PTR64										pvContext;															///<frame label for DWORD
	MWCAP_PTR64										pvFrame;															///<Frame data address
	LONGLONG										llTimestamp;														///<Timestamp
} MWCAP_VIDEO_ECO_CAPTURE_STATUS;'''

class MWCAP_VIDEO_ECO_CAPTURE_STATUS(Structure):
    _pack_ = 1
    _fields_ = [('pvContext',c_void_p),
               ('pvFrame',c_void_p),
               ('llTimestamp',c_int64)]

MWCapture.MWOpenChannelByPath.restype = c_void_p
MWCapture.MWCloseChannel.argtypes=[c_void_p]
MWCapture.MWCreateEvent.restype = c_void_p
MWCapture.MWCloseEvent.argtypes=[c_void_p]
MWCapture.MWWaitEvent.argtypes=[c_void_p, c_int32]
MWCapture.MWRegisterNotify.argtypes=[c_void_p, c_void_p, c_int32]
MWCapture.MWUnregisterNotify.argtypes=[c_void_p, c_void_p]

MWCapture.MWRegisterNotify.restype = c_void_p
MWCapture.MWRegisterNotify.argtypes = [c_void_p,c_void_p, c_int32]

MWCapture.MWStartAudioCapture.argtypes=[c_void_p]
MWCapture.MWStopAudioCapture.argtypes=[c_void_p]
MWCapture.MWStartVideoCapture.argtypes=[c_void_p, c_void_p]
MWCapture.MWStopVideoCapture.argtypes=[c_void_p]

MWCapture.MWCaptureAudioFrame.argtypes=[c_void_p, c_void_p]

MWCapture.MWGetVideoSignalStatus.argtypes=[c_void_p, c_void_p]
MWCapture.MWGetAudioSignalStatus.argtypes=[c_void_p, c_void_p]

MWCapture.MWPinVideoBuffer.argtypes=[c_void_p, c_void_p, c_int32]
MWCapture.MWUnpinVideoBuffer.argtypes=[c_void_p, c_void_p]

MWCapture.MWGetChannelInfo.argtypes=[c_void_p,c_void_p]

MWCapture.MWGetNotifyStatus.argtypes=[c_void_p, c_void_p, c_void_p]
MWCapture.MWGetVideoBufferInfo.argtypes=[c_void_p, c_void_p]
MWCapture.MWGetVideoFrameInfo.argtypes=[c_void_p, c_ubyte, c_void_p]

MWCapture.MWGetDeviceTime.argtypes=[c_void_p, c_void_p]

MWCapture.MWGetAudioInputSourceArray.argtypes=[c_void_p, c_void_p, c_void_p]

MWCapture.MWCaptureVideoFrameToVirtualAddressEx.argtypes=[c_void_p, c_int32, c_void_p, c_uint32, c_uint32, c_bool, c_void_p, c_uint32,c_int32,c_int32,c_uint32,c_int32,c_void_p,c_void_p,c_int32,c_short,c_short,c_short,c_short, c_uint32, c_uint32,c_void_p,c_void_p,c_int32,c_int32,c_uint32,c_uint32,c_uint32]
MWCapture.MWGetVideoCaptureStatus.argtypes=[c_void_p,c_void_p]

MWCapture.MWStartVideoEcoCapture.argtypes=[c_void_p, c_void_p]
MWCapture.MWCaptureSetVideoEcoFrame.argtypes=[c_void_p, c_void_p]
MWCapture.MWStopVideoEcoCapture.argtypes=[c_void_p]
MWCapture.MWGetVideoEcoCaptureStatus.argtypes=[c_void_p, c_void_p]

MWCapture.MWResampleAudio_Pyc.argtypes=[c_void_p, c_void_p, c_int32, c_int32]