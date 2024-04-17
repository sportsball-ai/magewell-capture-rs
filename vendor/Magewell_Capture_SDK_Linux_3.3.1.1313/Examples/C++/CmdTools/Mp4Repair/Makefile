TARGET_MP4REPAIR := Mp4Repair

LOCAL_CFLAGS += -I../../../../Include -ldl -fpermissive -g

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
else
override ARCH = arm32
endif
endif
endif
endif
CLIB = -ludev -lpthread
ITRD_USB = ../../../../3rdpart/libusb/lib/linux/$(ARCH)/libusb-1.0.a
MWMP4_LIBA = -L../../../../Lib/$(ARCH) -lmw_mp4
all: $(TARGET_MP4REPAIR)

MP4REPAIR_OBJS = ./Mp4Repair.o
	
$(TARGET_MP4REPAIR) : $(MP4REPAIR_OBJS) $(MWCAPTURE_LIBA)
	$(CXX) -o $(TARGET_MP4REPAIR) $(MP4REPAIR_OBJS) $(MWMP4_LIBA) $(ITRD_USB) $(CLIB)

%.o : %.cpp
	$(CXX) -c $(CFLAGS) $(LOCAL_CFLAGS) $(MWMP4_LIBA) $< -o $*.o

clean:	
	rm -rf  $(TARGET_MP4REPAIR) $(MP4REPAIR_OBJS)

