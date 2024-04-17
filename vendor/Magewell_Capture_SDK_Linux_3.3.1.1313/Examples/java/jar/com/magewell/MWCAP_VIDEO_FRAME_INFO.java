package com.magewell;

/*

 typedef struct _MWCAP_SMPTE_TIMECODE {
	BYTE 											byFrames;									///<Frames number
	BYTE											bySeconds;									///<Seconds
	BYTE											byMinutes;									///<Minutes
	BYTE											byHours;									///<Hours
} MWCAP_SMPTE_TIMECODE;

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
} MWCAP_VIDEO_FRAME_INFO;
* */
public class MWCAP_VIDEO_FRAME_INFO {
	public int                   state;										///<The state of the video framess
	public boolean               bInterlaced;								///<Whether an interlaced signal
	public boolean               bSegmentedFrame;							///<Whether a segmented frame
	public boolean				 bTopFieldFirst;							///<Whether the top subframe is in front
	public boolean				 bTopFieldInverted;							///<Whether to reverse the top subframe
	public int                   cx;										///<Width of video frames
	public int					 cy;										///<Height of video frames
	public int					 nAspectX;									///<Width of the ratio
	public int                   nAspectY;									///<Height of the ratio
	public long[]				 allFieldStartTimes;						///<Start time of capturing top and bottom subframe respectively
	public long[]				 allFieldBufferedTimes;					    ///<Fully bufferred time of top and bottom frame respectively
	public int[]				 aSMPTETimeCodes;							///<Time code of top and bottom frame respectively
	public MWCAP_VIDEO_FRAME_INFO() {
	}
}
