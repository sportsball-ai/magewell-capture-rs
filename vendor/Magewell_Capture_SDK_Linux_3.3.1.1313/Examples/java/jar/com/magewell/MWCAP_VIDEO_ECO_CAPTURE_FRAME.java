package com.magewell;

import java.nio.ByteBuffer;

/*
typedef struct _MWCAP_VIDEO_ECO_CAPTURE_FRAME {
	MWCAP_PTR64										pvFrame;															///<The storage address for video capturing
	DWORD											cbFrame;															///<The size of storage for video capturing
	DWORD											cbStride;															///<Width of capture video frame

	BOOLEAN											bBottomUp;															///<Whether to flip
	MWCAP_VIDEO_DEINTERLACE_MODE					deinterlaceMode;													///<DeinterlaceMode

	MWCAP_PTR64										pvContext;															///<Context of ECO
} MWCAP_VIDEO_ECO_CAPTURE_FRAME;
* */
public class MWCAP_VIDEO_ECO_CAPTURE_FRAME {
	public ByteBuffer										pvFrame;															///<The storage address for video capturing
	public long											cbFrame;															///<The size of storage for video capturing
	public long											cbStride;															///<Width of capture video frame
	public boolean											bBottomUp;															///<Whether to flip
	public int                                             deinterlaceMode;													///<DeinterlaceMode
	public long                   pvContextIndex;                                                                              ///<Context of ECO
	public MWCAP_VIDEO_ECO_CAPTURE_FRAME() {
	}
}
