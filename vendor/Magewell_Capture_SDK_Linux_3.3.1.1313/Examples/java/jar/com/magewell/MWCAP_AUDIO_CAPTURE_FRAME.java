package com.magewell;

/*
typedef struct _MWCAP_AUDIO_CAPTURE_FRAME {
DWORD											cFrameCount;																///<Number of bufferred frames
DWORD											iFrame;																		///<Current frame index
DWORD											dwSyncCode;																	///<Sync code of audio frame data
DWORD											dwReserved;																	///<Reserved
LONGLONG										llTimestamp;																///<The timestamp of audio frame
DWORD											adwSamples[MWCAP_AUDIO_SAMPLES_PER_FRAME * MWCAP_AUDIO_MAX_NUM_CHANNELS];	///<Audio sample data. Each sample is 32-bit width, and high bit effective. The priority of the path is: Left0, Left1, Left2, Left3, right0, right1, right2, right3
} MWCAP_AUDIO_CAPTURE_FRAME;
* */
public class MWCAP_AUDIO_CAPTURE_FRAME {
	public int  cFrameCount;
	public int  iFrame;
	public int  dwSyncCode;
	public int  dwReserved;
	public long llTimestamp;
	public int  adwSamples[];
	public MWCAP_AUDIO_CAPTURE_FRAME() {
	}
}
