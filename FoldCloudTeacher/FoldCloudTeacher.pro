#-------------------------------------------------
#
# Project created by QtCreator 2017-03-20T17:29:16
#
#-------------------------------------------------

QT       += core gui network xml

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = FoldCloudTeacher
TEMPLATE = app

DESTDIR = ../Release


SOURCES += main.cpp\
        flodcloudwidget.cpp \
    mytableview.cpp \
    stulist.cpp \
    log.cpp \
    streamparsexml.cpp \
    thread.cpp \
    message.cpp \
    myhttp.cpp \
    myqlist.cpp \
    classesform.cpp \
    tcpserver.cpp \
    mytcpclient.cpp \
    myjson.cpp

HEADERS  += flodcloudwidget.h \
    cmytableview.h \
    stulist.h \
    global.h \
    log.h \
    streamparsexml.h \
    thread.h \
    myhttp.h \
    myqlist.h \
    classesform.h \
    tcpserver.h \
    mytcpclient.h \
    myjson.h

FORMS    += flodcloudwidget.ui \
    stulist.ui \
    classesform.ui

RESOURCES += \
    sources.qrc
