package com.magewell;

import java.nio.ByteBuffer;
/*
typedef struct _MWCAP_VIDEO_ECO_CAPTURE_STATUS {
	MWCAP_PTR64										pvContext;															///<frame label for DWORD
	MWCAP_PTR64										pvFrame;															///<Frame data address
	LONGLONG										llTimestamp;														///<Timestamp
} MWCAP_VIDEO_ECO_CAPTURE_STATUS;
* */
public class MWCAP_VIDEO_ECO_CAPTURE_STATUS {
	public long                   pvContextIndex;															///<frame label for DWORD
	public ByteBuffer										pvFrame;															///<Frame data address
	public long										llTimestamp;														///<Timestamp
	public MWCAP_VIDEO_ECO_CAPTURE_STATUS() {
	}
}
