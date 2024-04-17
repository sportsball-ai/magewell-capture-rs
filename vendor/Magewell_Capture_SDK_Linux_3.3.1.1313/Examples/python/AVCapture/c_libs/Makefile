TARGET = libMWCapturePy.so libMWMp4Py.so libMWAVEncoderPy.so

VOAAC_PATH = ../../../../3rdpart/voaac

CC = gcc
CXX = g++
LD = ld
LOCAL_CFLAGS=-I../../../../Include -I $(VOAAC_PATH)/inc -I $(VOAAC_PATH)/basic_op

CFLAGS += -O2 -g\
          -Wall \
          -Wextra \
          -Wno-unused-parameter \
          -Wno-sign-compare \
          -Wno-unused-variable \
          -Wno-unused-but-set-variable \
          -Wno-missing-field-initializers  \
          -fPIC 
          	
		
ifeq ($(ARCH),)
ARCH =$(shell uname -m)
endif

ifeq ($(ARCH),x86_64)
override ARCH = x64
#CFLAGS += -DWITH_GPU_VENC
MWVENCLIB=../../../../Lib/$(ARCH)/libmw_venc.a
else
ifeq ($(ARCH),i386)
CFLAGS += -m32
CFLAGS += -DWITH_GPU_VENC
MWVENCLIB=../../../../Lib/$(ARCH)/libmw_venc.a
else
ifeq ($(ARCH),i686)
CFLAGS += -m32
override ARCH = i386
CFLAGS += -DWITH_GPU_VENC
MWVENCLIB=../../../../Lib/$(ARCH)/libmw_venc.a
else
ifeq ($(ARCH),aarch64)
override ARCH = arm64
else
override ARCH = arm32
endif
endif
endif
endif
CLIB=-lpthread -ldl -ludev -lasound -lv4l2
GPUENC_LIB = -lva -lva-drm -lx264 -lresolv
MWCAPTURELIB=../../../../Lib/$(ARCH)/libMWCapture.a
MWMP4LIB=../../../../Lib/$(ARCH)/libmw_mp4.a
USBLIB=../../../../3rdpart/libusb/lib/linux/$(ARCH)/libusb-1.0.a


VOAAC_OBJS = $(VOAAC_PATH)/basic_op/basicop2.o $(VOAAC_PATH)/basic_op/oper_32b.o \
$(VOAAC_PATH)/src/aac_rom.o $(VOAAC_PATH)/src/aacenc.o $(VOAAC_PATH)/src/aacenc_core.o \
$(VOAAC_PATH)/src/adj_thr.o $(VOAAC_PATH)/src/band_nrg.o $(VOAAC_PATH)/src/bit_cnt.o \
$(VOAAC_PATH)/src/bitbuffer.o $(VOAAC_PATH)/src/bitenc.o $(VOAAC_PATH)/src/block_switch.o \
$(VOAAC_PATH)/src/channel_map.o $(VOAAC_PATH)/src/cmnMemory.o $(VOAAC_PATH)/src/dyn_bits.o \
$(VOAAC_PATH)/src/grp_data.o $(VOAAC_PATH)/src/interface.o $(VOAAC_PATH)/src/line_pe.o \
$(VOAAC_PATH)/src/memalign.o $(VOAAC_PATH)/src/ms_stereo.o $(VOAAC_PATH)/src/pre_echo_control.o \
$(VOAAC_PATH)/src/psy_configuration.o $(VOAAC_PATH)/src/psy_main.o $(VOAAC_PATH)/src/qc_main.o \
$(VOAAC_PATH)/src/quantize.o $(VOAAC_PATH)/src/sf_estim.o $(VOAAC_PATH)/src/spreading.o \
$(VOAAC_PATH)/src/stat_bits.o $(VOAAC_PATH)/src/tns.o $(VOAAC_PATH)/src/transform.o

MWMp4Py_obj=

MWAVEncoderPy_obj=MWAVEncoder/MWAVEncoder.o \
MWAVEncoder/VoAacEnc/VoAacEnc.o MWAVEncoder/X264Enc/X264Enc.o \
$(VOAAC_OBJS)

MWCapturePy_obj=MWCapture/MWCapturePy.o
all:$(TARGET)

libMWMp4Py.so:$(MWMp4Py_obj)
	rm -rf temp
	mkdir temp
	#cp MWMp4Py.o ./temp
	cd temp && \
	ar x ../$(USBLIB) && \
	ar x ../$(MWMP4LIB) && \
	$(CXX) -shared -fPIC $(CFLAGS) -o ../libMWMp4Py.so *.o $(CLIB) &&\
	cd ../ 
#&&\
#	strip libMWMp4Py.so
# &&\
#	rm -rf temp

libMWCapturePy.so:$(MWCapturePy_obj)
	rm -rf temp
	mkdir temp
	cp $(MWCapturePy_obj) ./temp
	cd temp && \
	ar x ../$(MWCAPTURELIB) && \
	$(CXX) -shared -fPIC $(CFLAGS) -o ../libMWCapturePy.so *.o $(CLIB) &&\
	cd ../ &&\
	strip libMWCapturePy.so &&\
	rm -rf temp

ifeq ($(MWVENCLIB),)
libMWAVEncoderPy.so:$(MWAVEncoderPy_obj)
	rm -rf temp
	mkdir temp
	cp $(MWAVEncoderPy_obj) ./temp
	cd temp && \
	ar x ../$(USBLIB) && \
	$(CXX) -shared -fPIC $(CFLAGS) -o ../libMWAVEncoderPy.so *.o $(GPUENC_LIB) $(CLIB) &&\
	cd ../ &&\
	strip libMWAVEncoderPy.so &&\
	rm -rf temp
else
libMWAVEncoderPy.so:$(MWAVEncoderPy_obj)
	rm -rf temp
	mkdir temp
	cp $(MWAVEncoderPy_obj) ./temp
	cd temp && \
	ar x ../$(USBLIB) && \
	ar x ../$(MWVENCLIB) && \
	$(CXX) -shared -fPIC $(CFLAGS) -o ../libMWAVEncoderPy.so *.o $(GPUENC_LIB) $(CLIB) &&\
	cd ../ &&\
	strip libMWAVEncoderPy.so &&\
	rm -rf temp
endif
%.o : %.c 
	$(CC) -o $@  -c $< $(CFLAGS) $(LOCAL_CFLAGS)

%.o:%.cpp
	$(CXX) -o $@  -c $< $(CFLAGS) $(LOCAL_CFLAGS)


clean:	
	rm -rf  $(MWMp4Py_obj) $(MWCapturePy_obj) $(MWAVEncoderPy_obj) $(TARGET)



