package com.magewell;

/*
typedef struct _MWCAP_AUDIO_SIGNAL_STATUS {
	WORD											wChannelValid;							///<Valid audio channel mask.The lowest bit indicates whether the 1st and 2nd channels are valid, the second bit indicates whether the 3rd and 4th channels are valid, the third bit indicates whether the 5th and 6th channels are valid, and the fourth bit indicates whether the 7th and 8th channels are valid.
	BOOLEAN											bLPCM;									///<Whether the signal is LPCM
	BYTE											cBitsPerSample;							///<Bit depth of each audio sampling
	DWORD											dwSampleRate;							///<Sample rate
	BOOLEAN											bChannelStatusValid;					///<Whether channel status is valid
	IEC60958_CHANNEL_STATUS							channelStatus;							///<The audio channel status
} MWCAP_AUDIO_SIGNAL_STATUS;
* */
public class MWCAP_AUDIO_SIGNAL_STATUS {
	short											wChannelValid;							///<Valid audio channel mask.The lowest bit indicates whether the 1st and 2nd channels are valid, the second bit indicates whether the 3rd and 4th channels are valid, the third bit indicates whether the 5th and 6th channels are valid, and the fourth bit indicates whether the 7th and 8th channels are valid.
	boolean											bLPCM;									///<Whether the signal is LPCM
	byte											cBitsPerSample;							///<Bit depth of each audio sampling
	int											    dwSampleRate;							///<Sample rate
	boolean											bChannelStatusValid;					///<Whether channel status is valid
	byte[]							                channelStatus;							///<The audio channel status
	public MWCAP_AUDIO_SIGNAL_STATUS() {
	}
}
