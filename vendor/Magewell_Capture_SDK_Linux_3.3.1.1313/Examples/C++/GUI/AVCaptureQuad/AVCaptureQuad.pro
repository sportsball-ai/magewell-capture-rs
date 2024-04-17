#-------------------------------------------------
#
# Project created by QtCreator 2018-05-09T15:30:22
#
#-------------------------------------------------

QT       += core gui\
            opengl

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ../../../../bin/AVCaptureQuad
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

INCLUDEPATH +=  ../../../../Include\

contains(QT_ARCH, i386) {
LIBS        +=  ../../../../Lib/i386/libMWCapture.a\
                -lv4l2\
                -ludev\
                -lasound
} else {
LIBS        +=  ../../../../Lib/x64/libMWCapture.a\
                -lv4l2\
                -ludev\
                -lasound
}



SOURCES += main.cpp\
        mainwindow.cpp \
    ccapturethread.cpp \
    copenglplayer.cpp \
    cconfigfile.cpp

HEADERS  += mainwindow.h \
    ccapturethread.h \
    copenglplayer.h \
    cconfigfile.h \
    common.h

FORMS    += mainwindow.ui

DISTFILES += \
    gl_vs.vert \
    gl_fs.frag

RESOURCES += \
    res.qrc


