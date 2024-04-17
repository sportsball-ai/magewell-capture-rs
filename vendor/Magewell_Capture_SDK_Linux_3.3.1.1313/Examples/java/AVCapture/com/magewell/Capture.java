package com.magewell;

import java.nio.ByteBuffer;
import com.magewell.LibMWCapture;
import java.util.regex.Matcher;

import java.util.regex.Pattern;

public class Capture{
	public static final int VIDEO_FRAME_NUM          = 4;
	public static final int AUDIO_FRAME_NUM          = 500;

	public static final int MAX_ECO_BUFFER_COUNT     = 4; //VIDEO_FRAME_NUM != ?
	public static final int CAPTURE_FRAME_RATE       = 25;

	Thread m_video_capture_thread;
	Thread m_audio_capture_thread;

	int m_device_num = 0;
	int[] m_device_index;
	int m_capturing = 0;
	int m_width = 0;
	int m_height = 0;
	long m_foucc = 0;
	long m_min_stride = 0;
	long m_video_frame_size = 0;
	int m_channels = 0;
	int m_sample_rate = 0;
	int m_bit_per_sample = 0;
	int m_audio_frame_len = 0;
	int m_is_set_audio = 0;
	int m_is_set_video = 0;

	int m_is_eco = 0;
	long m_channel_handle = 0;

	ByteBuffer[] m_video_frame;
	long[] m_video_time;
	int m_vfw_index = 0;
	int m_vfw_num = 0;
	int m_vfr_num_play = 0;
	int m_vfr_num_rec = 0;

	byte[][] m_audio_frame;
	long[] m_audio_time;
	int m_afw_index = 0;
	int m_afw_num = 0;
	int m_afr_num_play = 0;
	int m_afr_num_rec = 0;

	class VideoCaptureThread implements Runnable {
		@Override
		public void run() {
			if (0 == m_is_eco) {
				pro_video_capture();
			}else {
				eco_video_capture();
			}

        }
		public void pro_video_capture() {
			long notify_event = LibMWCapture.MWCreateEvent();
			if (0 == notify_event) {
				System.out.println("create notify_event fail");
				return;
			}
			long capture_event = LibMWCapture.MWCreateEvent();
			if (0 == capture_event) {
				System.out.println("create capture_event fail");
				LibMWCapture.MWCloseEvent(notify_event);
				return;
			}
			int ret = LibMWCapture.MWStartVideoCapture(m_channel_handle, capture_event);
			if (LibMWCapture.MW_SUCCEEDED != ret) {
				System.out.println("start video capture fail!" + ret);
				LibMWCapture.MWCloseEvent(notify_event);
				LibMWCapture.MWCloseEvent(capture_event);
				return;
			}
			MWCAP_VIDEO_SIGNAL_STATUS video_signal_status = new MWCAP_VIDEO_SIGNAL_STATUS();
			if (LibMWCapture.MW_SUCCEEDED != LibMWCapture.MWGetVideoSignalStatus(m_channel_handle, video_signal_status)) {
				System.out.println("can't get video signal status");
				LibMWCapture.MWStopVideoCapture(m_channel_handle);
				LibMWCapture.MWCloseEvent(notify_event);
				LibMWCapture.MWCloseEvent(capture_event);
				return;
			}
			if (LibMWCapture.MWCAP_VIDEO_SIGNAL_NONE == video_signal_status.state) {
				System.out.println("input signal status: NONE");
			} else if (LibMWCapture.MWCAP_VIDEO_SIGNAL_UNSUPPORTED == video_signal_status.state) {
				System.out.println("input signal status: Unsupported");
			} else if (LibMWCapture.MWCAP_VIDEO_SIGNAL_LOCKING == video_signal_status.state) {
				System.out.println("input signal status: Locking");
			} else if (LibMWCapture.MWCAP_VIDEO_SIGNAL_LOCKED == video_signal_status.state) {
				System.out.println("input signal status: Locked");
			}
			if (video_signal_status.state != LibMWCapture.MWCAP_VIDEO_SIGNAL_LOCKED) {
				System.out.println("input signal status error");
				LibMWCapture.MWStopVideoCapture(m_channel_handle);
				LibMWCapture.MWCloseEvent(notify_event);
				LibMWCapture.MWCloseEvent(capture_event);
				return;
			}

			int notify_buffer_mode = LibMWCapture.MWCAP_NOTIFY_VIDEO_FRAME_BUFFERED;
			if (video_signal_status.bInterlaced) {
				notify_buffer_mode = LibMWCapture.MWCAP_NOTIFY_VIDEO_FIELD_BUFFERED;
			}
			long notify = LibMWCapture.MWRegisterNotify(m_channel_handle, notify_event, notify_buffer_mode);
			if (0 == notify) {
				System.out.println("Register Notify fail");
				LibMWCapture.MWStopVideoCapture(m_channel_handle);
				LibMWCapture.MWCloseEvent(notify_event);
				LibMWCapture.MWCloseEvent(capture_event);
				return;
			}
			for (int i = 0; i < VIDEO_FRAME_NUM; i++) {
				LibMWCapture.MWPinVideoBuffer(m_channel_handle, m_video_frame[i], m_video_frame_size);
			}

			while(0 != m_capturing){
				if(LibMWCapture.MWWaitEvent(notify_event, 1000) <= 0){
					System.out.println("wait notify error or timeout");
					break;
				}
				long[] notify_status = new long[1];
				if(LibMWCapture.MW_SUCCEEDED != LibMWCapture.MWGetNotifyStatus(m_channel_handle, notify, notify_status)){
					continue;
				}
				MWCAP_VIDEO_BUFFER_INFO video_buffer_info = new MWCAP_VIDEO_BUFFER_INFO();
				if(LibMWCapture.MW_SUCCEEDED != LibMWCapture.MWGetVideoBufferInfo(m_channel_handle, video_buffer_info)){
					continue;
				}
				MWCAP_VIDEO_FRAME_INFO video_frame_info = new MWCAP_VIDEO_FRAME_INFO();
				if(LibMWCapture.MW_SUCCEEDED != LibMWCapture.MWGetVideoFrameInfo(m_channel_handle, video_buffer_info.iNewestBufferedFullFrame, video_frame_info)){
					continue;
				}
				if(0 == (notify_status[0] & notify_buffer_mode)){
					continue;
				}
				int mode = 0;
				if(video_signal_status.bInterlaced){
					if(0 == video_buffer_info.iBufferedFieldIndex){
						mode = LibMWCapture.MWCAP_VIDEO_DEINTERLACE_TOP_FIELD;
					}else{
						mode = LibMWCapture.MWCAP_VIDEO_DEINTERLACE_BOTTOM_FIELD;
					}
				}else{
					mode = LibMWCapture.MWCAP_VIDEO_DEINTERLACE_BLEND;
				}

				ret = LibMWCapture.MWCaptureVideoFrameToVirtualAddressEx(m_channel_handle,
						video_buffer_info.iNewestBufferedFullFrame,
						m_video_frame[m_vfw_index],
						m_video_frame_size,
						m_min_stride,
						false,
						0,
						m_foucc,
						m_width,
						m_height,
						0,
						0,
						0,
						0,
						0,
						(short)100,
						(short)0,
						(short)100,
						(short)0,
						mode,
						LibMWCapture.MWCAP_VIDEO_ASPECT_RATIO_CROPPING,
						0,
						0,
						0,
						0,
						LibMWCapture.MWCAP_VIDEO_COLOR_FORMAT_UNKNOWN,
						LibMWCapture.MWCAP_VIDEO_QUANTIZATION_UNKNOWN,
						LibMWCapture.MWCAP_VIDEO_SATURATION_UNKNOWN);
				if(LibMWCapture.MW_SUCCEEDED != ret){
					continue;
				}
				MWCAP_VIDEO_CAPTURE_STATUS video_capture_status = new MWCAP_VIDEO_CAPTURE_STATUS();
				LibMWCapture.MWGetVideoCaptureStatus(m_channel_handle, video_capture_status);
				if(LibMWCapture.MWWaitEvent(capture_event, 1000) <= 0) {
					System.out.println("wait capture event error or timeout");
					break;
				}
				LibMWCapture.MWGetDeviceTime(m_channel_handle, m_video_time);
				m_video_time[m_vfw_index] = m_video_time[m_vfw_index] / 10000;
				m_vfw_num += 1;
				m_vfw_index += 1;
				if(m_vfw_index >= VIDEO_FRAME_NUM) {
					m_vfw_index = 0;
				}
			}
			for (int i = 0; i < VIDEO_FRAME_NUM; i++) {
				LibMWCapture.MWUnpinVideoBuffer(m_channel_handle, m_video_frame[i]);
			}

			LibMWCapture.MWUnregisterNotify(m_channel_handle, notify);
			LibMWCapture.MWStopVideoCapture(m_channel_handle);
			LibMWCapture.MWCloseEvent(notify_event);
			LibMWCapture.MWCloseEvent(capture_event);
		}

        public void eco_video_capture()
		{
			long event = LibMWCapture.MWCreateEcoEvent();
			if(0 == event){
				System.out.println("create eco event failed");
				return;
			}

			MWCAP_VIDEO_ECO_CAPTURE_OPEN eco_open = new MWCAP_VIDEO_ECO_CAPTURE_OPEN();
			eco_open.cx = (short)m_width;
			eco_open.cy = (short)m_height;
			eco_open.dwFOURCC = m_foucc;
			eco_open.hEvent = event;
			eco_open.llFrameDuration = (long)(10000000 / CAPTURE_FRAME_RATE);//25p/s; -1 for input frame rate
			if(LibMWCapture.MW_SUCCEEDED != LibMWCapture.MWStartVideoEcoCapture(m_channel_handle, eco_open)) {
                LibMWCapture.MWCloseEcoEvent(event);
				System.out.println("MWStartVideoEcoCapture failed!");
				return;
			}
			MWCAP_VIDEO_ECO_CAPTURE_FRAME[] eco_frame_list = new MWCAP_VIDEO_ECO_CAPTURE_FRAME[MAX_ECO_BUFFER_COUNT];
			int i=0;
			for(;i < MAX_ECO_BUFFER_COUNT;i++) {
				eco_frame_list[i] = new MWCAP_VIDEO_ECO_CAPTURE_FRAME();
				eco_frame_list[i].deinterlaceMode = LibMWCapture.MWCAP_VIDEO_DEINTERLACE_BLEND;
				eco_frame_list[i].cbFrame = m_video_frame_size;
				eco_frame_list[i].pvFrame = m_video_frame[i];
				eco_frame_list[i].cbStride = m_min_stride;
				eco_frame_list[i].bBottomUp = false;
				eco_frame_list[i].pvContextIndex = (long) i;
				if (LibMWCapture.MW_SUCCEEDED != LibMWCapture.MWCaptureSetVideoEcoFrame(m_channel_handle, eco_frame_list[i])) {
					System.out.println("MWCaptureSetVideoEcoFrame init failed");
					break;
				}
			}
			if (i < MAX_ECO_BUFFER_COUNT){
				LibMWCapture.MWStopVideoEcoCapture(m_channel_handle);
				LibMWCapture.MWCloseEcoEvent(event);
				return;
			}
			MWCAP_VIDEO_ECO_CAPTURE_STATUS eco_status = new MWCAP_VIDEO_ECO_CAPTURE_STATUS();
			while(0 != m_capturing) {
				if(LibMWCapture.MWWaitEcoEvent(event, 1000) <= 0) {
					System.out.println("wait notify error or timeout");
					break;
				}

				if(LibMWCapture.MW_SUCCEEDED != LibMWCapture.MWGetVideoEcoCaptureStatus(m_channel_handle, eco_status)) {
					System.out.println("MWGetVideoEcoCaptureStatus failed!");
					break;
				}
				//LibMWCapture.MWGetDeviceTime(m_channel_handle, m_video_time);
				m_video_time[m_vfw_index] = eco_status.llTimestamp / 10000;
				m_vfw_num += 1;
				m_vfw_index += 1;
				if(m_vfw_index >= VIDEO_FRAME_NUM) {
					m_vfw_index = 0;
				}
				if(LibMWCapture.MW_SUCCEEDED != LibMWCapture.MWCaptureSetVideoEcoFrame(m_channel_handle, eco_frame_list[(int)eco_status.pvContextIndex])) {
					System.out.println("MWCaptureSetVideoEcoFrame capture failed!");
					break;
				}
			}
			LibMWCapture.MWStopVideoEcoCapture(m_channel_handle);
			LibMWCapture.MWCloseEcoEvent(event);
		}
    }
	class AudioCaptureThread implements Runnable {
		@Override
		public void run() {
			long notify_event = 0;
			long eco_event = 0;
			long notify = 0;
			int[] input_count = new int[1];
			input_count[0] = 0;
			LibMWCapture.MWGetAudioInputSourceArray(m_channel_handle, null, input_count);
			if (0 == input_count[0]) {
				System.out.println("no audio input");
				return;
			}
			if (LibMWCapture.MW_SUCCEEDED != LibMWCapture.MWStartAudioCapture(m_channel_handle)) {
				System.out.println("start audio capture fail");
				return;
			}
			MWCAP_AUDIO_SIGNAL_STATUS audio_signal_status = new MWCAP_AUDIO_SIGNAL_STATUS();
			if (LibMWCapture.MW_SUCCEEDED != LibMWCapture.MWGetAudioSignalStatus(m_channel_handle, audio_signal_status)) {
				System.out.println("can't get audio signal status");
				LibMWCapture.MWStopAudioCapture(m_channel_handle);
				return;
			}
			if (0 == audio_signal_status.wChannelValid) {
				System.out.println("audio signal is invalid");
				LibMWCapture.MWStopAudioCapture(m_channel_handle);
				return;
			}
			int now_channels = 0;
			int i = 0;
			while (i < (LibMWCapture.MWCAP_AUDIO_MAX_NUM_CHANNELS / 2)) {
				if (0 != (audio_signal_status.wChannelValid & (0x01 << i))) {
					now_channels += 2;
				}
				i += 1;
			}
			if (now_channels < m_channels) {
				System.out.println("audio channel error");
				LibMWCapture.MWStopAudioCapture(m_channel_handle);
				return;
			}
			if (0 == m_is_eco) {
				notify_event = LibMWCapture.MWCreateEvent();
				if (0 == notify_event) {
					System.out.println("create event fail");
					LibMWCapture.MWStopAudioCapture(m_channel_handle);
					return;
				}
				notify = LibMWCapture.MWRegisterNotify(m_channel_handle, notify_event, LibMWCapture.MWCAP_NOTIFY_AUDIO_FRAME_BUFFERED);
				if (0 == notify) {
					System.out.println("Register Notify fail");
					LibMWCapture.MWCloseEvent(notify_event);
					LibMWCapture.MWStopAudioCapture(m_channel_handle);
					return;
				}
			} else {
				eco_event = LibMWCapture.MWCreateEcoEvent();
				if (0 == eco_event) {
					System.out.println("create eco event failed");
					LibMWCapture.MWStopAudioCapture(m_channel_handle);
					return;
				}
				notify = LibMWCapture.MWRegisterNotify(m_channel_handle, eco_event, LibMWCapture.MWCAP_NOTIFY_AUDIO_FRAME_BUFFERED);
				if (0 == notify) {
					System.out.println("Register Notify fail");
					LibMWCapture.MWCloseEcoEvent(eco_event);
					LibMWCapture.MWStopAudioCapture(m_channel_handle);
					return;
				}
			}

			MWCAP_AUDIO_CAPTURE_FRAME macf = new MWCAP_AUDIO_CAPTURE_FRAME();
			while (0 != m_capturing) {
				if (m_is_eco == 0) {
					if (LibMWCapture.MWWaitEvent(notify_event, 1000) <= 0) {
						System.out.println("wait notify error or timeout");
						break;
					}
				} else {
					if (LibMWCapture.MWWaitEcoEvent(eco_event, 1000) <= 0) {
						System.out.println("wait notify error or timeout");
						break;
					}
				}
				long[] notify_status = new long[1];
				if (LibMWCapture.MW_SUCCEEDED != LibMWCapture.MWGetNotifyStatus(m_channel_handle, notify, notify_status)) {
					continue;
				}
				if (0 == (notify_status[0] & LibMWCapture.MWCAP_NOTIFY_AUDIO_FRAME_BUFFERED)) {
					continue;
				}
				while(0 != m_capturing) {
					if (LibMWCapture.MW_ENODATA == LibMWCapture.MWCaptureAudioFrame(m_channel_handle, macf)) {
						break;
					}
					//LibMWCapture.MWGetDeviceTime(m_channel_handle, m_audio_time);
					m_audio_time[m_afw_index] = macf.llTimestamp / 10000;
					LibMWCapture.MWResampleAudio(macf, m_audio_frame[m_afw_index], m_channels, m_bit_per_sample / 8);
					m_afw_index += 1;
					m_afw_num += 1;
					if (AUDIO_FRAME_NUM == m_afw_index) {
						m_afw_index = 0;
					}
				}
			}
			LibMWCapture.MWUnregisterNotify(m_channel_handle, notify);
			LibMWCapture.MWStopAudioCapture(m_channel_handle);
			if (0 == m_is_eco) {
				LibMWCapture.MWCloseEvent(notify_event);
			} else {
				LibMWCapture.MWCloseEcoEvent(eco_event);
			}
		}
	}

    public Capture() {
		show_version();
		LibMWCapture.MWCaptureInitInstance();
		m_device_num = list_pro_device();
	}
	public int list_pro_device()
	{
		int i;
		int channel_count = LibMWCapture.MWGetChannelCount();

		m_device_index  = new int[channel_count];
		for(i = 0; i < channel_count; i++){
			MWCAP_CHANNEL_INFO channel_info = new MWCAP_CHANNEL_INFO();
			int ret = LibMWCapture.MWGetChannelInfoByIndex(i, channel_info);
			if(LibMWCapture.MW_SUCCEEDED != LibMWCapture.MWGetChannelInfoByIndex(i, channel_info)) {
				continue;
			}
			String device_name=new String(channel_info.szProductName);
			if((device_name.indexOf("Pro Capture") < 0)&&(device_name.indexOf("Eco Capture") < 0)){
				continue;
			}
			m_device_index[m_device_num] = i;
			m_device_num++;
		}
		return m_device_num;
	}
	public void finalize(){
		LibMWCapture.MWCaptureExitInstance();
	}
	public void show_version(){
		byte[] pbyMaj = new byte[1];
		byte[] pbyMin = new byte[1];
		short[] pwBuild = new short[1];
		if(LibMWCapture.MW_SUCCEEDED != LibMWCapture.MWGetVersion(pbyMaj,pbyMin,pwBuild)){
			System.out.println("MWGetVersion fail\n");
			return;
		}
		System.out.println("Magewell MWCapture SDK"+pbyMaj[0]+"."+pbyMin[0]+".1."+pwBuild[0]+" - AVCapture"+m_channel_handle);
    }

	public void set_video(long foucc, int width, int height){
		int i;
		m_width = width;
		m_height = height;
		m_foucc = foucc;
		m_min_stride = LibMWCapture.FOURCC_CalcMinStride(foucc, width, 4);
		m_video_frame_size = LibMWCapture.FOURCC_CalcImageSize(foucc, width, height, m_min_stride);
		if(LibMWCapture.MWFOURCC_NV12 == foucc){
			m_video_frame_size = m_video_frame_size / 3 * 4;  //nv12 opengl width*height*2
		}
		m_video_frame  = new ByteBuffer[VIDEO_FRAME_NUM];
		m_video_time   = new long[VIDEO_FRAME_NUM];
		m_vfw_index = 0;
		m_vfw_num = 0;
		m_vfr_num_play = 0;
		m_vfr_num_rec = 0;
		for(i = 0; i < VIDEO_FRAME_NUM; i++){
			m_video_frame[i] = ByteBuffer.allocateDirect((int)m_video_frame_size);
			m_video_frame[i].clear();
		}
		m_is_set_video = 1;
	}

	public void set_audio(int channels, int sample_rate, int bit_per_sample){
		int i;
		m_channels = channels;
		m_sample_rate = sample_rate;
		m_bit_per_sample = bit_per_sample;
		m_audio_frame_len = channels * LibMWCapture.MWCAP_AUDIO_SAMPLES_PER_FRAME * bit_per_sample / 8;
		m_audio_frame  = new byte[AUDIO_FRAME_NUM][];
		m_audio_time = new long[AUDIO_FRAME_NUM];
		m_afw_index = 0;
		m_afw_num = 0;
		m_afr_num_play = 0;
		m_afr_num_rec = 0;
		for(i = 0; i<AUDIO_FRAME_NUM;i++){
			m_audio_frame[i] = new byte[m_audio_frame_len];
		}
		m_is_set_audio = 1;
	}

	public int start_capture(int index){
		if(index >= m_device_num){
			System.out.println("index "+index+" is out range[>="+m_device_num+"]");
			return -1;
		}
		byte[] path=new byte[128];
		LibMWCapture.MWGetDevicePath(index, path);
		String str=new String(path);
		m_channel_handle=LibMWCapture.MWOpenChannelByPath(path);
		if(0 == m_channel_handle){
			return -1;
		}
		MWCAP_CHANNEL_INFO channel_info=new MWCAP_CHANNEL_INFO();
		if(LibMWCapture.MW_SUCCEEDED != LibMWCapture.MWGetChannelInfo(m_channel_handle, channel_info)){
			System.out.println("Can't get channel info!");
			LibMWCapture.MWCloseChannel(m_channel_handle);
			m_channel_handle = 0;
			return -1;
		}

		if(channel_info.wFamilyID == LibMWCapture.MW_FAMILY_ID_PRO_CAPTURE) {
			m_is_eco = 0;
		}else {
			m_is_eco = 1;
		}
		m_capturing = 1;
		m_video_capture_thread = new Thread(new VideoCaptureThread());
		System.out.println("Video Capture Thread: " + m_video_capture_thread);
		m_video_capture_thread.setPriority(Thread.MAX_PRIORITY);
		m_video_capture_thread.start();
		m_audio_capture_thread = new Thread(new AudioCaptureThread());
		System.out.println("Audio Capture Thread: " + m_audio_capture_thread);
		m_audio_capture_thread.setPriority(Thread.MAX_PRIORITY);
		m_audio_capture_thread.start();
		return 0;
	}
	public void stop_capture(){
		m_capturing = 0;
		try {
			m_audio_capture_thread.join();
		} catch (InterruptedException e) {
			System.out.println("Audio Capture Thread Interrupt fail !");
			e.printStackTrace();
		}
		try {
			m_video_capture_thread.join();
		} catch (InterruptedException e) {
			System.out.println("Video Capture Thread Interrupt fail !");
			e.printStackTrace();
		}

		LibMWCapture.MWCloseChannel(m_channel_handle);
		m_channel_handle = 0;
	}

	public byte[] get_audio_play(){
		if(m_afr_num_play == m_afw_num){
			return null;
		}
		int afr_index_play;
		if((m_afw_num - m_afr_num_play) > (AUDIO_FRAME_NUM / 2)) {
			m_afr_num_play = m_afw_num - 1;
		}
		afr_index_play = m_afr_num_play % AUDIO_FRAME_NUM;
		m_afr_num_play += 1;

		return m_audio_frame[afr_index_play];
	}

	public ByteBuffer get_video_play(){
		if(m_vfr_num_play == m_vfw_num){
			return null;
		}
		int vfr_index_play;
		if((m_vfw_num - m_vfr_num_play) > (VIDEO_FRAME_NUM / 2)) {
			m_vfr_num_play = m_vfw_num - 1;
		}
		vfr_index_play = m_vfr_num_play % VIDEO_FRAME_NUM;
		m_vfr_num_play += 1;
		return m_video_frame[vfr_index_play];
	}

	public int get_audio_capture_frames(){
		return m_afw_num;
	}

	public int get_video_capture_frames(){
		return m_vfw_num;
	}
}
