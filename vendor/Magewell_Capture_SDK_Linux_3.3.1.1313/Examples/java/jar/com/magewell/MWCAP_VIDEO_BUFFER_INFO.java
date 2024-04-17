package com.magewell;

/*
typedef struct _MWCAP_VIDEO_BUFFER_INFO {
	DWORD											cMaxFrames;									///<Maximum number of frames in on-board cache
	BYTE											iNewestBuffering;							///<The number of the slices being bufferred. A frame of video data may contain multiple slices.
	BYTE											iBufferingFieldIndex;						///<The sequence number of fields being bufferred.
	BYTE											iNewestBuffered;							///<The sequence number of slices the latest bufferred piece.
	BYTE											iBufferedFieldIndex;						///<The sequence number of the latest bufferred field
	BYTE											iNewestBufferedFullFrame;					///<The sequence number of the latest bufferred frame
	DWORD											cBufferedFullFrames;						///<Number of fully bufferred full frames
} MWCAP_VIDEO_BUFFER_INFO;
* */
public class MWCAP_VIDEO_BUFFER_INFO {
	public int   cMaxFrames;									///<Maximum number of frames in on-board cache
	public byte  iNewestBuffering;							///<The number of the slices being bufferred. A frame of video data may contain multiple slices.
	public byte  iBufferingFieldIndex;						///<The sequence number of fields being bufferred.
	public byte  iNewestBuffered;							///<The sequence number of slices the latest bufferred piece.
	public byte  iBufferedFieldIndex;						///<The sequence number of the latest bufferred field
	public byte  iNewestBufferedFullFrame;					///<The sequence number of the latest bufferred frame
	public int   cBufferedFullFrames;						///<Number of fully bufferred full frames
	public MWCAP_VIDEO_BUFFER_INFO() {
	}
}
