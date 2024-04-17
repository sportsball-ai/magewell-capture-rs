from ctypes import *
from LibMWCapture import *
import threading

VIDEO_FRAME_NUM          = 4
AUDIO_FRAME_NUM          = 500

MAX_ECO_BUFFER_COUNT     = 4 #VIDEO_FRAME_NUM != ?
CAPTURE_FRAME_RATE       = 25


def eco_video_capture(param):
    event = MWCapture.MWEcoEventCreate_Pyc()
    if 0 == event:
        print("create eco event failed")
    eco_open = MWCAP_VIDEO_ECO_CAPTURE_OPEN()
    eco_open.cx = param.m_width
    eco_open.cy = param.m_height
    eco_open.dwFOURCC = param.m_foucc
    eco_open.hEvent = event
    eco_open.llFrameDuration = 10000000 // CAPTURE_FRAME_RATE #25p/s; -1 for input frame rate 

    if MW_SUCCEEDED != MWCapture.MWStartVideoEcoCapture(param.m_channel_handle, byref(eco_open)):
        MWCapture.MWEcoEventDestory_Pyc(event)
        print("MWStartVideoEcoCapture failed! iRet\n")
        return
    eco_frame_list = []
    i=0
    while(i < MAX_ECO_BUFFER_COUNT):
        eco_frame=MWCAP_VIDEO_ECO_CAPTURE_FRAME()
        eco_frame.deinterlaceMode = MWCAP_VIDEO_DEINTERLACE_BLEND
        eco_frame.cbFrame  = param.m_video_frame_size
        #a=create_string_buffer(g_image_size)
        eco_frame.pvFrame  = addressof(param.m_video_frame[i])
        eco_frame.cbStride = param.m_min_stride
        eco_frame.bBottomUp = False
        eco_frame.pvContext = addressof(eco_frame)
        if MW_SUCCEEDED != MWCapture.MWCaptureSetVideoEcoFrame(param.m_channel_handle, eco_frame.pvContext):
            print("MWCaptureSetVideoEcoFrame init failed")
            break
        i+=1
        eco_frame_list.append(eco_frame)
    if len(eco_frame_list) < MAX_ECO_BUFFER_COUNT:
        MWCapture.MWStopVideoEcoCapture(param.m_channel_handle)
        MWCapture.MWEcoEventDestory_Pyc(event)
        return

    while(param.m_capturing):
        if MWCapture.MWEcoEventWait_Pyc(event, 1000) <= 0:
            print("wait notify error or timeout")
            break
        eco_status = MWCAP_VIDEO_ECO_CAPTURE_STATUS()
        if MW_SUCCEEDED != MWCapture.MWGetVideoEcoCaptureStatus(param.m_channel_handle, byref(eco_status)):
            print("MWGetVideoEcoCaptureStatus failed!")
            break

        #MWCapture.MWGetDeviceTime(param.m_channel_handle, byref(param.m_video_time[param.m_vfw_index]))
        param.m_video_time[param.m_vfw_index].value = eco_status.llTimestamp // 10000
        param.m_vfw_num += 1
        param.m_vfw_index += 1
        if param.m_vfw_index >= VIDEO_FRAME_NUM:
            param.m_vfw_index = 0
        if MW_SUCCEEDED != MWCapture.MWCaptureSetVideoEcoFrame(param.m_channel_handle, eco_status.pvContext):
            print("MWCaptureSetVideoEcoFrame capture failed!")
            break
    MWCapture.MWStopVideoEcoCapture(param.m_channel_handle)
    MWCapture.MWEcoEventDestory_Pyc(event)

def pro_video_capture(param):
    notify_event = MWCapture.MWCreateEvent()

    if 0 == notify_event:
        print("create notify_event fail")
        return

    capture_event = MWCapture.MWCreateEvent()
    if 0 == capture_event:
        print("create capture_event fail")
        MWCapture.MWCloseEvent(notify_event)
        return

    if MW_SUCCEEDED != MWCapture.MWStartVideoCapture(param.m_channel_handle, capture_event):
        print("start video capture fail!")
        MWCapture.MWCloseEvent(notify_event)
        MWCapture.MWCloseEvent(capture_event)
        return

    video_signal_status = MWCAP_VIDEO_SIGNAL_STATUS()
    if MW_SUCCEEDED != MWCapture.MWGetVideoSignalStatus(param.m_channel_handle, byref(video_signal_status)):
        print("can't get video signal status")
        MWCapture.MWStopVideoCapture(param.m_channel_handle)
        MWCapture.MWCloseEvent(notify_event)
        MWCapture.MWCloseEvent(capture_event)
        return

    if MWCAP_VIDEO_SIGNAL_NONE == video_signal_status.state:
        print("input signal status: NONE")
    if MWCAP_VIDEO_SIGNAL_UNSUPPORTED == video_signal_status.state:
        print("input signal status: Unsupported")
    if MWCAP_VIDEO_SIGNAL_LOCKING == video_signal_status.state:
        print("input signal status: Locking")
    if MWCAP_VIDEO_SIGNAL_LOCKED == video_signal_status.state:
        print("input signal status: Locked")

    if video_signal_status.state != MWCAP_VIDEO_SIGNAL_LOCKED:
        print("input signal status error")
        MWCapture.MWStopVideoCapture(param.m_channel_handle)
        MWCapture.MWCloseEvent(notify_event)
        MWCapture.MWCloseEvent(capture_event)
        return

    notify_buffer_mode = MWCAP_NOTIFY_VIDEO_FRAME_BUFFERED
    if 0 != video_signal_status.bInterlaced:
        notify_buffer_mode = MWCAP_NOTIFY_VIDEO_FIELD_BUFFERED

    notify= MWCapture.MWRegisterNotify(param.m_channel_handle, notify_event, notify_buffer_mode)
    if 0 == notify:
        print("Register Notify fail")
        MWCapture.MWStopVideoCapture(param.m_channel_handle)
        MWCapture.MWCloseEvent(notify_event)
        MWCapture.MWCloseEvent(capture_event)
        return
    for data in param.m_video_frame:
        MWCapture.MWPinVideoBuffer(param.m_channel_handle, data, param.m_video_frame_size)
    while (param.m_capturing):
        if MWCapture.MWWaitEvent(notify_event, 1000) <= 0:
            print("wait notify error or timeout")
            break
        notify_status = c_uint64(0)
        if MW_SUCCEEDED != MWCapture.MWGetNotifyStatus(param.m_channel_handle, notify, byref(notify_status)):
            continue
        video_buffer_info = MWCAP_VIDEO_BUFFER_INFO()
        if MW_SUCCEEDED != MWCapture.MWGetVideoBufferInfo(param.m_channel_handle, byref(video_buffer_info)):
            continue
        video_frame_info = MWCAP_VIDEO_FRAME_INFO()
        if MW_SUCCEEDED != MWCapture.MWGetVideoFrameInfo(param.m_channel_handle, video_buffer_info.iNewestBufferedFullFrame, byref(video_frame_info)):
            continue
        if 0 == (notify_status.value & notify_buffer_mode):
            continue

        if video_signal_status.bInterlaced:
            if 0 == video_buffer_info.iBufferedFieldIndex:
                mode = MWCAP_VIDEO_DEINTERLACE_TOP_FIELD
            else:
                mode = MWCAP_VIDEO_DEINTERLACE_BOTTOM_FIELD
        else:
            mode = MWCAP_VIDEO_DEINTERLACE_BLEND
        ret = MWCapture.MWCaptureVideoFrameToVirtualAddressEx(param.m_channel_handle,
            video_buffer_info.iNewestBufferedFullFrame,
            param.m_video_frame[param.m_vfw_index], 
            param.m_video_frame_size, 
            param.m_min_stride,
            0,
            0,
            param.m_foucc,
            param.m_width,
            param.m_height,
            0,
            0,
            0,
            0,
            0,
            100,
            0,
            100,
            0,
            mode,
            MWCAP_VIDEO_ASPECT_RATIO_CROPPING,
            0,
            0,
            0,
            0,
            MWCAP_VIDEO_COLOR_FORMAT_UNKNOWN,
            MWCAP_VIDEO_QUANTIZATION_UNKNOWN,
            MWCAP_VIDEO_SATURATION_UNKNOWN)
        if MW_SUCCEEDED != ret:
            continue

        if MWCapture.MWWaitEvent(capture_event, 1000) <= 0:
            print("wait capture event error or timeout")
            break
        video_capture_status =MWCAP_VIDEO_CAPTURE_STATUS() 
        MWCapture.MWGetVideoCaptureStatus(param.m_channel_handle, byref(video_capture_status))
        
        MWCapture.MWGetDeviceTime(param.m_channel_handle, byref(param.m_video_time[param.m_vfw_index]))
        param.m_video_time[param.m_vfw_index].value = param.m_video_time[param.m_vfw_index].value // 10000
        
        param.m_vfw_num += 1
        param.m_vfw_index += 1
        if param.m_vfw_index >= VIDEO_FRAME_NUM:
            param.m_vfw_index = 0
        
    for data in param.m_video_frame:
        MWCapture.MWUnpinVideoBuffer(param.m_channel_handle, data)
    MWCapture.MWUnregisterNotify(param.m_channel_handle, notify)
    MWCapture.MWStopVideoCapture(param.m_channel_handle)
    MWCapture.MWCloseEvent(notify_event)
    MWCapture.MWCloseEvent(capture_event)

def video_capture(param):
    if param.m_is_eco == 0:
        pro_video_capture(param)
    else:
        eco_video_capture(param)

def audio_capture(param):
    input_count = c_uint32(0)
    MWCapture.MWGetAudioInputSourceArray(param.m_channel_handle, 0, byref(input_count))
    if 0 == input_count:
        print("no audio input")
        return

    if MW_SUCCEEDED != MWCapture.MWStartAudioCapture(param.m_channel_handle):
        print("start audio capture fail")
        return

    audio_signal_status = MWCAP_AUDIO_SIGNAL_STATUS()
    if MW_SUCCEEDED != MWCapture.MWGetAudioSignalStatus(param.m_channel_handle, byref(audio_signal_status)):
        print("can't get audio signal status")
        MWCapture.MWStopAudioCapture(param.m_channel_handle)
        return
    if 0 == audio_signal_status.wChannelValid:
        print("audio signal is invalid")
        MWCapture.MWStopAudioCapture(param.m_channel_handle)
        return
    now_channels = 0
    i = 0
    while (i < (MWCAP_AUDIO_MAX_NUM_CHANNELS / 2)):
        if audio_signal_status.wChannelValid & (0x01 << i):
            now_channels += 2
        i += 1
    if now_channels < param.m_channels:
        print("audio channel error")
        MWCapture.MWStopAudioCapture(param.m_channel_handle)
        return

    if 0 == param.m_is_eco:
        notify_event = MWCapture.MWCreateEvent()
        if 0 == notify_event:
            print("create event fail")
            MWCapture.MWStopAudioCapture(param.m_channel_handle)
            return
        notify= MWCapture.MWRegisterNotify(param.m_channel_handle, notify_event, MWCAP_NOTIFY_AUDIO_FRAME_BUFFERED)
        if 0 == notify:
            print("Register Notify fail")
            MWCapture.MWCloseEvent(notify_event)
            MWCapture.MWStopAudioCapture(param.m_channel_handle)
            return
    else:
        eco_event = MWCapture.MWEcoEventCreate_Pyc()
        if 0 == eco_event:
            print("create eco event failed")
            MWCapture.MWStopAudioCapture(param.m_channel_handle)
            return
        notify= MWCapture.MWRegisterNotify(param.m_channel_handle, eco_event, MWCAP_NOTIFY_AUDIO_FRAME_BUFFERED)
        if 0 == notify:
            print("Register Notify fail")
            MWCapture.MWEcoEventDestory_Pyc(eco_event)
            MWCapture.MWStopAudioCapture(param.m_channel_handle)
            return

    macf=MWCAP_AUDIO_CAPTURE_FRAME()
    while(param.m_capturing):
        if param.m_is_eco == 0:
            if MWCapture.MWWaitEvent(notify_event, 1000) <= 0:
                print("wait notify error or timeout")
                break
        else:
            if MWCapture.MWEcoEventWait_Pyc(eco_event, 1000) <= 0:
                print("wait notify error or timeout")
                break
        notify_status = c_uint64(0)
        if MW_SUCCEEDED != MWCapture.MWGetNotifyStatus(param.m_channel_handle, notify, byref(notify_status)):
            continue
        if 0 == (notify_status.value & MWCAP_NOTIFY_AUDIO_FRAME_BUFFERED):
            continue
        
        while(param.m_capturing):
            if MW_ENODATA == MWCapture.MWCaptureAudioFrame(param.m_channel_handle,byref(macf)):
                break
            #MWCapture.MWGetDeviceTime(param.m_channel_handle, byref(param.m_audio_time[param.m_afw_index]))
            param.m_audio_time[param.m_afw_index].value = macf.llTimestamp // 10000
            MWCapture.MWResampleAudio_Pyc(byref(macf), param.m_audio_frame[param.m_afw_index], param.m_channels, param.m_bit_per_sample // 8)

            param.m_afw_index += 1
            param.m_afw_num += 1
            if AUDIO_FRAME_NUM == param.m_afw_index:
                param.m_afw_index = 0
        
    MWCapture.MWUnregisterNotify(param.m_channel_handle, notify)
    MWCapture.MWStopAudioCapture(param.m_channel_handle)
    if 0 == param.m_is_eco:
        MWCapture.MWCloseEvent(notify_event)
    else:
        MWCapture.MWEcoEventDestory_Pyc(eco_event)


class Capture(object):
    def __init__(self):
        self.show_version()
        MWCapture.MWCaptureInitInstance()
        self.m_width = 0
        self.m_height = 0
        self.m_foucc = 0
        self.m_min_stride = 0
        self.m_video_frame_size = 0
        self.m_channels = 0
        self.m_sample_rate = 0
        self.m_bit_per_sample = 0
        self.m_audio_frame_len = 0
        self.m_is_set_audio = 0
        self.m_is_set_video = 0
        self.m_capturing = 0

    def show_version(self):
        maj = c_ubyte(1)
        min = c_ubyte(1)
        build = c_short(1)
        MWCapture.MWGetVersion_Pyc(byref(maj), byref(min), byref(build))
        print("Magewell MWCapture SDK %d.%d.1.%d - AVCapture" % (
        maj.value, min.value, build.value))

    def list_device(self):
        channel_count = MWCapture.MWGetChannelCount()
        index = 0
        device_name_list =[]
        while index<channel_count:
            channel_info = MWCAP_CHANNEL_INFO()
            ret = MWCapture.MWGetChannelInfoByIndex(index, byref(channel_info))
            index = index + 1
            if ret != MW_SUCCEEDED:
                continue
            device_name = '%d-%s' %(index,str(channel_info.szProductName,encoding='UTF-8'))
            
            if device_name.find("Pro Capture") < 0 and device_name.find("Eco Capture") < 0:
                continue
            device_name_list.append(device_name)
        return device_name_list

    def __del__(self):
        if self.m_capturing:
            self.stop_capture()
        MWCapture.MWCaptureExitInstance()

    def set_video(self, foucc, width, height):
        self.m_width = width
        self.m_height = height
        self.m_foucc = foucc
        self.m_min_stride = MWCapture.FOURCC_CalcMinStride_Pyc(foucc, width, 4)
        self.m_video_frame_size = MWCapture.FOURCC_CalcImageSize_Pyc(foucc, width, height, self.m_min_stride)
        if MWFOURCC_NV12 == foucc:
            self.m_video_frame_size = self.m_video_frame_size // 3 * 4  #nv12 opengl width*height*2
        self.m_video_frame  = []
        self.m_video_time   = []
        self.m_vfw_index = 0
        self.m_vfw_num = 0
        self.m_vfr_num_play = 0
        self.m_vfr_num_rec = 0

        i=0
        while i<VIDEO_FRAME_NUM:
            tm = c_uint64(0)
            self.m_video_time.append(tm)
            data = create_string_buffer(self.m_video_frame_size)
            self.m_video_frame.append(data)
            i = i + 1
        self.m_is_set_video = 1

    def set_audio(self, channels, sample_rate, bit_per_sample):
        self.m_channels = channels
        self.m_sample_rate = sample_rate
        self.m_bit_per_sample = bit_per_sample
        self.m_audio_frame_len = channels * MWCAP_AUDIO_SAMPLES_PER_FRAME * bit_per_sample // 8
        self.m_audio_frame  = []
        self.m_audio_time = []
        self.m_afw_index = 0
        self.m_afw_num = 0
        self.m_afr_num_play = 0
        self.m_afr_num_rec = 0
        i = 0
        while i<AUDIO_FRAME_NUM:
            tm = c_uint64(0)
            self.m_audio_time.append(tm)
            data = create_string_buffer(self.m_audio_frame_len)
            self.m_audio_frame.append(data)
            i = i + 1
        self.m_is_set_audio = 1

    def start_capture(self, index):
        path = create_string_buffer(128)
        MWCapture.MWGetDevicePath(index, path)
        self.m_channel_handle=MWCapture.MWOpenChannelByPath(path)
        if 0 == self.m_channel_handle:
            print("open channel fail", path.raw)
            MWCapture.MWCloseChannel(self.m_channel_handle)
            self.m_channel_handle = 0
            return -1
        channel_info=MWCAP_CHANNEL_INFO()
        if MW_SUCCEEDED != MWCapture.MWGetChannelInfo(self.m_channel_handle, byref(channel_info)):
            print("Can't get channel info!")
            MWCapture.MWCloseChannel(self.m_channel_handle)
            self.m_channel_handle = 0
            return -1
        if channel_info.wFamilyID == MW_FAMILY_ID_PRO_CAPTURE:
            self.m_is_eco = 0
        else:
            self.m_is_eco = 1
        self.m_capturing = 1

        self.m_video_capture_tid = threading.Thread(target=video_capture, args=(self,),name="video_capture" )
        self.m_audio_capture_tid = threading.Thread(target=audio_capture, args=(self,),name="audio_capture" )

        self.m_video_capture_tid.setDaemon(True)
        self.m_audio_capture_tid.setDaemon(True)

        self.m_video_capture_tid.start()
        self.m_audio_capture_tid.start()
        return 0

    def stop_capture(self):
        self.m_capturing = 0
        self.m_video_capture_tid.join()
        self.m_audio_capture_tid.join()
        MWCapture.MWCloseChannel(self.m_channel_handle)
        self.m_channel_handle = 0
        pass

    def get_audio_play(self):
        if self.m_afr_num_play == self.m_afw_num:
            return 0
        if (self.m_afw_num - self.m_afr_num_play) > (AUDIO_FRAME_NUM // 2):
            self.m_afr_num_play = self.m_afw_num - 1
        afr_index_play = self.m_afr_num_play % AUDIO_FRAME_NUM
        self.m_afr_num_play += 1
        return self.m_audio_frame[afr_index_play]

    def get_video_play(self):
        if self.m_vfr_num_play == self.m_vfw_num:
            return 0
        if (self.m_vfw_num - self.m_vfr_num_play) > (VIDEO_FRAME_NUM // 2):
            self.m_vfr_num_play = self.m_vfw_num - 1
        vfr_index_play = self.m_vfr_num_play % VIDEO_FRAME_NUM
        self.m_vfr_num_play += 1
        return self.m_video_frame[vfr_index_play]


    def get_audio_rec(self):
        if self.m_afr_num_rec == self.m_afw_num:
            return (0,0)
        if (self.m_afw_num - self.m_afr_num_rec) > (AUDIO_FRAME_NUM // 2):
            self.m_afr_num_rec = self.m_afw_num - 1
        afr_index_play = self.m_afr_num_rec % AUDIO_FRAME_NUM
        self.m_afr_num_rec += 1
        return (self.m_audio_frame[afr_index_play], self.m_audio_time[afr_index_play].value)

    def get_video_rec(self):
        if self.m_vfr_num_rec == self.m_vfw_num:
            return (0,0)
        if (self.m_vfw_num - self.m_vfr_num_rec) > (VIDEO_FRAME_NUM // 2):
            self.m_vfr_num_rec = self.m_vfw_num - 1
        vfr_index_play = self.m_vfr_num_rec % VIDEO_FRAME_NUM
        self.m_vfr_num_rec += 1
        return (self.m_video_frame[vfr_index_play], self.m_video_time[vfr_index_play].value)

    def get_video_capture_frames(self):
        return self.m_vfw_num

    def get_audio_capture_frames(self):
        return self.m_afw_num
