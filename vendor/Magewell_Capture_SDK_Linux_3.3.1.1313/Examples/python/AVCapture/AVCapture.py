import platform
import sys
from ctypes import *
from PyQt5 import QtWidgets,QtCore,QtGui
from PyQt5.QtWidgets import QMenuBar,QMenu,QAction,QLabel,QPushButton,QFileDialog
from PyQt5.QtGui import QImage,QPixmap
from PyQt5.QtCore import QByteArray
from OpenGL.arrays import *
from OpenGL.GL import *
from PyQt5.QtWidgets import QOpenGLWidget
import threading
import pyaudio
import time

from Capture import *
from VideoRenderGl import *
from AVRecord import *

CAPTURE_WIDTH            = 1920
CAPTURE_HEIGHT           = 1080
CAPTURE_FOURCC           = MWFOURCC_NV12

CAPTURE_CHANNEL          = 2
CAPTURE_SAMPLE_RATE      = 48000
CAPTURE_BIT_PER_SAMPLE   = 16


def print_fps(param):
    param.m_log_count = param.m_log_count + 1
    if param.m_log_count == 500:
        now_time = time.time()
        if now_time == param.m_prev_time:
            return
        time_temp = now_time - param.m_prev_time
        now_c_v = param.m_capture.get_video_capture_frames()
        now_c_a = param.m_capture.get_audio_capture_frames()

        print("capture fps(v|a):", 
        (now_c_v-param.m_prev_video_capture_frames)/time_temp,
        (now_c_a-param.m_prev_audio_capture_frames)/time_temp,
        " capture frams(v|a):",now_c_v,now_c_a)
        
        print("render fps(v|a):",
        (param.m_video_render_frames-param.m_prev_video_render_frames)/time_temp,
        (param.m_audio_play_frames-param.m_prev_audio_play_frames) / time_temp,
        " render frams(v|a):",param.m_video_render_frames,param.m_audio_play_frames)

        print("encode fps(v|a):",(param.m_video_encode_frames - param.m_prev_video_encode_frames)/time_temp,
        (param.m_audio_encode_frames - param.m_prev_audio_encode_frames) / time_temp,
        " encode frames(v|a):",param.m_video_encode_frames,param.m_audio_encode_frames)
        print("\n")
        param.m_prev_video_capture_frames = now_c_v
        param.m_prev_video_render_frames = param.m_video_render_frames
        param.m_prev_video_encode_frames = param.m_video_encode_frames
        param.m_prev_audio_capture_frames = now_c_a
        param.m_prev_audio_play_frames = param.m_audio_play_frames
        param.m_prev_audio_encode_frames = param.m_audio_encode_frames
        param.m_prev_time = now_time
        param.m_log_count = 0

def video_play_thread(param):
    param.m_log_count = 0
    param.m_prev_time = time.time()
    param.m_prev_video_capture_frames = param.m_capture.get_video_capture_frames()
    param.m_prev_video_render_frames = param.m_video_render_frames
    param.m_prev_video_encode_frames = param.m_video_encode_frames
    param.m_prev_audio_capture_frames = param.m_capture.get_audio_capture_frames()
    param.m_prev_audio_play_frames = param.m_audio_play_frames
    param.m_prev_audio_encode_frames = param.m_audio_encode_frames

    while(param.m_capturing):
        print_fps(param)
        video_frame = param.m_capture.get_video_play()
        if 0 == video_frame:
            time.sleep(0.005)
            continue
        param.m_video_render.put_frame(video_frame)
        param.m_video_render_frames = param.m_video_render_frames + 1
    param.m_video_render_frames = 0

def audio_play_thread(param):
    while(param.m_capturing):
        audio_frame = param.m_capture.get_audio_play()
        if 0 == audio_frame:
            time.sleep(0.005) 
            continue
        param.m_pyaudio_stream.write(audio_frame)
        param.m_audio_play_frames = param.m_audio_play_frames + 1
    param.m_audio_play_frames = 0

def av_record_thread(param,file_name):
    now_time = time.localtime(time.time())
    file_name_strbuf = create_string_buffer(file_name.encode(),256)
    record = AVRecord(file_name_strbuf)
    record.set_video(param.CAPTURE_WIDTH, param.CAPTURE_HEIGHT, CAPTURE_FOURCC)
    record.set_audio(CAPTURE_CHANNEL, param.CAPTURE_SAMPLE_RATE)
    
    while(param.m_is_recording):
        write_frames = 0
        (video_frame,tm) = param.m_capture.get_video_rec()#
        if video_frame:
            record.write_video(video_frame, 0, tm)
            param.m_video_encode_frames = param.m_video_encode_frames + 1
            write_frames += 1

        (audio_frame,tm) = param.m_capture.get_audio_rec()
        if audio_frame:
            record.write_audio(audio_frame, 192*4, tm)
            param.m_audio_encode_frames = param.m_audio_encode_frames + 1
            write_frames += 1
        
        if 0 == write_frames:
            time.sleep(0.01)
    param.m_video_encode_frames = 0
    param.m_audio_encode_frames = 0
        
    

class AVCaptureWid(QtWidgets.QMainWindow):
    def __init__(self, parent=None, flags=QtCore.Qt.WindowFlags()):
        super().__init__(parent=parent, flags=flags)
        self.CAPTURE_WIDTH = CAPTURE_WIDTH
        self.CAPTURE_HEIGHT = CAPTURE_HEIGHT
        self.CAPTURE_SAMPLE_RATE = CAPTURE_SAMPLE_RATE
        self.parse_cmd(sys.argv)
        self.m_device_select_index = -1
        self.m_is_recording = 0
        self.m_capturing = 0
        self.m_capture = Capture()
        self.m_video_render_frames = 0
        self.m_audio_play_frames = 0
        self.m_video_encode_frames = 0
        self.m_audio_encode_frames = 0

        self.create_ui()

    def resizeEvent(self, event):
        self.m_video_render.move(0,0)
        self.m_video_render.resize(self.width(),self.height())
    
    def create_ui(self):
        self.setWindowTitle('AVCapture')
        self.resize(960,540)

        self.m_pyaudio = pyaudio.PyAudio()
        self.m_pyaudio_stream = self.m_pyaudio.open(
                format = pyaudio.get_format_from_width(2),
                channels = 2,
                rate = 48000,
                output = True)
        self.m_pyaudio_stream.start_stream()

        self.m_video_render = CRenderWid(self)
        self.m_video_render.move(0,0)
        self.m_video_render.resize(self.width(),self.height())
        

        self.m_menubar = self.menuBar()

        self.m_menu_file = QMenu('File',self.m_menubar)
        self.m_menu_start_act = QAction("Start Record", self)
        self.m_menu_file.addAction(self.m_menu_start_act)
        self.m_menu_start_act.setEnabled(True)
        self.m_menu_stop_act = QAction("Stop Record", self)
        self.m_menu_file.addAction(self.m_menu_stop_act)
        self.m_menu_stop_act.setEnabled(False)
        self.m_menubar.addMenu(self.m_menu_file)
        self.m_menu_file.triggered.connect(self.slot_file_selected)

        self.m_menu_device = QMenu('Device',self.m_menubar)
        self.m_menu_device_act = []
        device_name_list = self.m_capture.list_device()
        index = 0
        for device_name in device_name_list:
            if device_name.find("Pro Capture") < 0 and device_name.find("Eco Capture") < 0:
                continue

            action = QAction(device_name, self)
            self.m_menu_device_act.append(action)
            self.m_menu_device.addAction(action)
            action.setCheckable(True)

        self.m_menubar.addMenu(self.m_menu_device)
        self.m_menu_device.triggered.connect(self.slot_device_selected)
        


    def slot_device_selected(self,select_action):
        print(select_action.text())
        index = 0
        for action in self.m_menu_device_act:
            if action == select_action:
                if(index == self.m_device_select_index):
                    self.m_device_select_index = -1
                else:
                    self.m_device_select_index = index
            action.setChecked(False)
            index = index + 1
        if self.m_capturing != 0:
            print("close",select_action.text())
            self.m_capture.stop_capture()
            self.m_capturing = 0


        if self.m_device_select_index != -1:
            print("open",select_action.text())
            self.m_video_render.open_render(CAPTURE_FOURCC,self.CAPTURE_WIDTH,self.CAPTURE_HEIGHT)
            self.m_capture.set_video(CAPTURE_FOURCC,self.CAPTURE_WIDTH,self.CAPTURE_HEIGHT)
            self.m_capture.set_audio(CAPTURE_CHANNEL, self.CAPTURE_SAMPLE_RATE, CAPTURE_BIT_PER_SAMPLE)
            if self.m_capture.start_capture(self.m_device_select_index) < 0:
                return
            self.m_capturing = 1
            self.m_video_play_tid = threading.Thread(target=video_play_thread, args=(self,),name="video_play" )
            self.m_audio_play_tid = threading.Thread(target=audio_play_thread, args=(self,),name="audio_play" )
            self.m_video_play_tid.setDaemon(True)
            self.m_audio_play_tid.setDaemon(True)
            self.m_video_play_tid.start()
            self.m_audio_play_tid.start()
            select_action.setChecked(True)

    def slot_file_selected(self,select_action):
        if self.m_is_recording:
            print("stop recording")
            self.m_is_recording = 0
            self.m_av_record_tid.join()
            self.m_menu_start_act.setEnabled(True)
            self.m_menu_stop_act.setEnabled(False)
        else:
            if 0 == self.m_capturing:
                print("please start capture")
                select_action.setChecked(False)
                return
            print("start recording")
            filename,filetype = QFileDialog.getSaveFileName(self, "Save File"," ","MP4(*.mp4);;MOV(*.mov)")

            if filetype.find("MOV") >= 0:
                if filename.find(".mov") == (len(filename) - 4):
                    file_name = filename
                else:
                    file_name = filename + ".mov"
            elif filetype.find("mp4") >= 0:
                if filename.find(".mp4") == (len(filename) - 4):
                    file_name = filename
                else:
                    file_name = filename + ".mp4"
            else:
                print("input filename")
                return
            self.m_av_record_tid = threading.Thread(target=av_record_thread, args=(self,file_name),name="av_record" )
            self.m_av_record_tid.start()
            self.m_is_recording = 1
            self.m_menu_start_act.setEnabled(False)
            self.m_menu_stop_act.setEnabled(True)


    def __del__(self):
        print("exit")
        self.m_pyaudio_stream.stop_stream()
    def closeEvent(self,event):
        if self.m_is_recording:
            print("stop recording")
            self.m_is_recording = 0
            self.m_av_record_tid.join()
        if self.m_capturing != 0:
            self.m_capturing = 0
            self.m_video_play_tid.join()
            self.m_audio_play_tid.join()
            self.m_capture.stop_capture()
    def parse_cmd(self, argv):
        print("Usage:\npython AVCapture.py [-width 1920] [-height 1080] [-sample_rate 48000]\n")
        index = 0
        for cmd in argv:
            index = index + 1
        i = 0
        while(i < index):
            if argv[i] == "-width":
                self.CAPTURE_WIDTH = int(argv[i+1])
                print("set width to", self.CAPTURE_WIDTH)
            elif argv[i] == "-height":
                self.CAPTURE_HEIGHT = int(argv[i+1])
                print("set hight to", self.CAPTURE_HEIGHT)
            elif argv[i] == "-sample_rate":
                self.CAPTURE_SAMPLE_RATE = int(argv[i+1])
                print("set sample rate to",self.CAPTURE_SAMPLE_RATE)
            else:
                i = i + 1
                continue
            i = i + 2

if __name__ == '__main__':
    app = QtWidgets.QApplication(sys.argv)
    win = AVCaptureWid()
    win.show()
    sys.exit(app.exec_())

