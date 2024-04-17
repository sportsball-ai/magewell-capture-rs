TARGET_HDRCAPTURE := HDRCapture

LOCAL_CFLAGS += -I../../../../Include -I./HevcParse -L/usr/lib64 -ldl -fpermissive -g

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
not_support:
	echo "\033[31marm not support this example\033[0m"
	exit 
else
override ARCH = arm32
not_support:
	echo "\033[31marm not support this example\033[0m"
	exit 
endif
endif
endif
endif


CLIB =  -lpthread -ldl -ludev -lasound -lv4l2 -lz -lva -lva-drm -lresolv
MWCAPTURE_LIBA = -L../../../../Lib/$(ARCH) -lMWCapture
MWMP4_LIBA = -L../../../../3rdpart/ffmpeg/lib/$(ARCH) -lmw_mp4
MWVENC_LIBA = -L ../../../../Lib/$(ARCH) -lmw_venc
all: $(TARGET_HDRCAPTURE)

HDRCAPTURE_OBJS = ./HDRCapture.o HevcParse/hevc_bitstream.o HevcParse/hevc_nalu.o HevcParse/hevc_sps.o HevcParse/hevc_sei.o 
	
$(TARGET_HDRCAPTURE) : $(HDRCAPTURE_OBJS)
	$(CXX) -o $(TARGET_HDRCAPTURE) $(HDRCAPTURE_OBJS) $(MWCAPTURE_LIBA) $(MWMP4_LIBA)  $(MWVENC_LIBA) $(CLIB)

%.o : %.cpp
	$(CXX) -c $(CFLAGS) $(LOCAL_CFLAGS) $(MWMP4_LIBA)  $(MWVENC_LIBA) $(CLIB) $< -o $*.o

clean:	
	rm -rf  $(TARGET_HDRCAPTURE) $(HDRCAPTURE_OBJS)

