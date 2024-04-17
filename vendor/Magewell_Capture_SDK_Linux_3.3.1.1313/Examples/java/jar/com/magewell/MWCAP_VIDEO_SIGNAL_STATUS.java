package com.magewell;

/*
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
* */
public class MWCAP_VIDEO_SIGNAL_STATUS {
	public int     state;
	public int     x;
	public int     y;
	public int     cx;
	public int     cy;
	public int     cxTotal;
	public int     cyTotal;
	public boolean bInterlaced;
	public int     dwFrameDuration;
	public int     nAspectX;
	public int     nAspectY;
	public boolean bSegmentedFrame;
	public int     frameType;
	public int     colorFormat;
	public int     quantRange;
	public int     satRange;
	public MWCAP_VIDEO_SIGNAL_STATUS() {
	}
}
