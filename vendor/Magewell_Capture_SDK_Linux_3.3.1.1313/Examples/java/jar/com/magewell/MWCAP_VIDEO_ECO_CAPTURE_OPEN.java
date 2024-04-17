package com.magewell;


/*
typedef struct _MWCAP_VIDEO_ECO_CAPTURE_OPEN {
	MWCAP_PTR64										hEvent;																///<Handle of capture event

	DWORD											dwFOURCC;															///<Capture format
	WORD											cx;																	///<Width
	WORD											cy;																	///<Height
	LONGLONG										llFrameDuration;													///<Interval, -1 indicates follow format of input source
} MWCAP_VIDEO_ECO_CAPTURE_OPEN;
* */
public class MWCAP_VIDEO_ECO_CAPTURE_OPEN {
	public long										hEvent;																///<Handle of capture event

	public long											dwFOURCC;															///<Capture format
	public short											cx;																	///<Width
	public short											cy;																	///<Height
	public long										llFrameDuration;													///<Interval, -1 i
	public MWCAP_VIDEO_ECO_CAPTURE_OPEN() {
	}
}
