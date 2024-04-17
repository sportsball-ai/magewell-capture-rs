TARGET_APP := OSDPreview
IMGUI_PATH = ../../../../3rdpart/imgui
LOCAL_CFLAGS += -I../../../../Include -I $(IMGUI_PATH)


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

	
AVCAPTURE_OBJS = ./OSDPreview.o ./Text.o\
./OpenGLRender/OpenGLRender.o \
$(IMGUI_PATH)/imgui.o $(IMGUI_PATH)/imgui_impl_glfw_gl3.o $(IMGUI_PATH)/imgui_draw.o \

$(TARGET_APP) : $(AVCAPTURE_OBJS) $(MWCAPTURE_LIBA) 
	$(CXX) -o $(TARGET_APP) $(AVCAPTURE_OBJS) $(MWCAPTURE_LIBA) $(CLIB)

%.o : %.cpp
	$(CXX) -c $(CFLAGS) $(LOCAL_CFLAGS) $< -o $*.o

clean:	
	rm -rf  $(TARGET_APP) $(AVCAPTURE_OBJS)

