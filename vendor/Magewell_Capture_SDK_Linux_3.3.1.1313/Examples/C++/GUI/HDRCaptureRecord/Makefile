TARGET_HDRCAPTURE := HDRCaptureRecord
IMGUI_PATH = ../../../../3rdpart/imgui
LOCAL_CFLAGS += -I $(IMGUI_PATH) -I../../../../Include -I./HDRRender -I./HevcParse  -L/usr/lib64 -ldl -fpermissive -g

CC = gcc
CXX = g++
LD = ld

CFLAGS += -O0 \
          -Wall \
          -Wextra \
          -Wno-unused-parameter \
          -Wno-sign-compare \
          -Wno-unused-variable \
          -Wno-unused-but-set-variable \
          -Wno-missing-field-initializers  \
          -fPIC \
          $(LOCAL_CFLAGS)
	  

ARCH:=$(shell uname -m)

ifeq ($(ARCH),x86_64)
override ARCH = x64
MWVENC_LIBA = -L ../../../../Lib/$(ARCH) -lmw_venc
else
ifeq ($(ARCH),i386)
MWVENC_LIBA = -L ../../../../Lib/$(ARCH) -lmw_venc
CFLAGS += -m32
else
ifeq ($(ARCH),i686)
MWVENC_LIBA = -L ../../../../Lib/$(ARCH) -lmw_venc
CFLAGS += -m32
override ARCH = i386
MWVENC_LIBA = -L ../../../../Lib/$(ARCH) -lmw_venc
else
ifeq ($(ARCH),aarch64)
override ARCH = arm64
CFLAGS += -DARCH_ARM
else
override ARCH = arm32
CFLAGS += -DARCH_ARM
endif
endif
endif
endif

CLIB =  -lGL `pkg-config --static --libs glfw3` -lGLEW -lpthread -ldl -ludev -lasound -lv4l2 -lz -lva -lva-drm -lresolv
MWCAPTURE_LIBA = -L../../../../Lib/$(ARCH) -lMWCapture
MWMP4_LIBA = -L ../../../../Lib/$(ARCH) -lmw_mp4
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
all: $(TARGET_HDRCAPTURE)
endif
endif
endif

CHECKOPENGLFAIL:
	echo -e "\033[31mplease updata opengl version to 3.0 or above \033[0m"

HDRCAPTURE_OBJS = ./HDRCaptureRecord.o \
	        HevcParse/hevc_bitstream.o HevcParse/hevc_nalu.o \
		HevcParse/hevc_sps.o HevcParse/hevc_sei.o \
		OpenGLRender/OpenGLRender.o $(IMGUI_PATH)/gl3w.o $(IMGUI_PATH)/imgui.o \
		$(IMGUI_PATH)/imgui_impl_glfw_gl3.o  $(IMGUI_PATH)/imgui_draw.o
$(TARGET_HDRCAPTURE) : $(HDRCAPTURE_OBJS)
	$(CXX) -o $(TARGET_HDRCAPTURE) $(HDRCAPTURE_OBJS) $(MWCAPTURE_LIBA) $(MWMP4_LIBA) $(MWVENC_LIBA) $(CLIB)

%.o : %.c
	$(CC) -c $(CFLAGS) $(LOCAL_CFLAGS) $(MWMP4_LIBA) $(MWVENC_LIBA) $(CLIB) $< -o $*.o
	
%.o : %.cpp
	$(CXX) -c $(CFLAGS) $(LOCAL_CFLAGS) $(MWMP4_LIBA) $(CLIB) $< -o $*.o

clean:	
	rm -rf  $(TARGET_HDRCAPTURE) $(HDRCAPTURE_OBJS)

