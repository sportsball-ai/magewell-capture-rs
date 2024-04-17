TARGER_CAPTUREJNI = libMWCaptureJni.so

LOCAL_INCLUDE = ../../../../Include 

CAPTUREJNI_CFLAGS += -I$(LOCAL_INCLUDE) -I $(JAVA_HOME)/include -I $(JAVA_HOME)/include/linux

CC = gcc
CXX = g++
LD = ld

CFLAGS += -O2 \
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

LOCAL_LDFLAGS = 
LOCAL_LIBS =


CLIB =  -lpthread -ldl -ludev -lasound -lv4l2
JNI_LIB = -L$(JAVA_HOME)/lib/amd64 -ljawt
CAPTURE_LIB=../../../../Lib/$(ARCH)/libMWCapture.a

#update
CAPTUREJNI_OBJS = libMWCaptureJni.o

$(TARGER_CAPTUREJNI) : $(CAPTUREJNI_OBJS)
	$(CXX) -shared -fPIC $(CFLAGS) -o $(TARGER_CAPTUREJNI) $(CAPTUREJNI_OBJS) $(CAPTUREJNI_CFLAGS) $(CAPTURE_LIB) $(JNI_LIB) $(CLIB)
	strip $(TARGER_CAPTUREJNI) 

%.o:%.cpp
	$(CXX) -o $@  -c $< $(CFLAGS) $(LOCAL_CFLAGS) $(CAPTUREJNI_CFLAGS) $(CAPTURE_LIB)

clean:	
	rm -rf  $(TARGER_CAPTUREJNI) $(CAPTUREJNI_OBJS)
