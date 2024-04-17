package com.magewell;

/*
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
* */
public class MWCAP_VIDEO_CAPTURE_STATUS {
	public long   								pvContext;																	///<The context of video capture
	public boolean											bPhysicalAddress;															///<Whether to use the physical address to store the capture data
	public long   								pvFrame;																	///<The memory address to store the capture data
	public int												iFrame;																		///<The index of capturing frame
	public boolean											bFrameCompleted;															///<Whether a frame is fully captured
	public short											cyCompleted;																///<Number of frames captured
	public short											cyCompletedPrev;
	public MWCAP_VIDEO_CAPTURE_STATUS() {
	}
}
