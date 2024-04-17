TARGET_APP := AVCapture

IMGUI_PATH = ../../../../3rdpart/imgui
VOAAC_PATH = ../../../../3rdpart/voaac
LOCAL_CFLAGS += -I../../../../Include -I $(IMGUI_PATH) -I $(VOAAC_PATH)/inc -I $(VOAAC_PATH)/basic_op -I ../../../../3rdpart/ffmpeg/include


CC = gcc
CXX = g++
LD = ld

CFLAGS += -O0 \
          -Wall \
          -Wextra\
          -Wno-unused-parameter \
          -Wno-sign-compare \
          -Wno-unused-variable \
          -Wno-unused-but-set-variable \
          -Wno-missing-field-initializers  \
          -fPIC \
          -DLIBMWVIDEORENDER_LINUX_DEF \
          $(LOCAL_CFLAGS) \
	  

ifeq ($(ARCH),)
ARCH =$(shell uname -m)
endif

ifeq ($(ARCH),x86_64)
override ARCH = x64
else
ifeq ($(ARCH),i386)
CFLAGS += -m32
else
ifeq ($(ARCH),i686)
CFLAGS += -m32
override ARCH = i386
else
ifeq ($(ARCH),aarch64)
override ARCH = arm64
else
override ARCH = arm32
endif
endif
endif
endif


CLIB = -lGL -lGLEW `pkg-config --static --libs glfw3` -lpthread -ldl -ludev -lasound -lv4l2 
MWCAPTURE_LIBA = ../../../../Lib/$(ARCH)/libMWCapture.a
X264_LIB = -lx264
MWMP4_LIBA = -L ../../../../Lib/$(ARCH)/ -lmw_mp4
#-L ../../../../3rdpart/ffmpeg/lib/$(ARCH) -lavformat -lavcodec -lswresample -lswscale -lavutil -lm -lz
OPENGLSTR1=$(shell glxinfo | grep "OpenGL version string:")

OPENGLSTR=$(subst ., ,$(OPENGLSTR1))
OPENGLVERSION = $(word 4,$(OPENGLSTR))
ifeq ($(OPENGLVERSION),2)
all: CHECKOPENGLFAIL
else
ifeq ($(OPENGLVERSION),1)
all: CHECKOPENGLFAIL
else
ifeq ($(OPENGLVERSION),0)
all: CHECKOPENGLFAIL
else
all: $(TARGET_APP)
	
endif
endif
endif


CHECKOPENGLFAIL:
	echo -e "\033[31mplease updata opengl version to 3.0 or above \033[0m"

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
	
AVCAPTURE_OBJS = ./AVCapture.o \
./OpenGLRender/OpenGLRender.o ./AlsaPlayer/AlsaPlayerDevice.o \
$(IMGUI_PATH)/imgui.o $(IMGUI_PATH)/imgui_impl_glfw_gl3.o $(IMGUI_PATH)/imgui_draw.o \
X264Enc/X264Enc.o \
VoAacEnc/VoAacEnc.o \
$(VOAAC_OBJS)

#FFmpegMp4Muxer/Mp4Muxer.o 

$(TARGET_APP) : $(AVCAPTURE_OBJS) $(MWCAPTURE_LIBA) 
	$(CXX) -o $(TARGET_APP) $(AVCAPTURE_OBJS) $(CFLAGS) $(MWCAPTURE_LIBA) $(X264_LIB) $(MWMP4_LIBA) $(CLIB)

%.o : %.cpp
	$(CXX) -c $(CFLAGS) $(LOCAL_CFLAGS) $< -o $*.o

clean:	
	rm -rf  $(TARGET_APP) $(AVCAPTURE_OBJS)

