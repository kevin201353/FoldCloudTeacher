QT += core gui network xml
QT -= gui

CONFIG += c++11

TARGET = Eclass_Client
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    mytcpclient.cpp \
    tcpserver.cpp \
    log.cpp \
    qthread.cpp

HEADERS += \
    mytcpclient.h \
    tcpserver.h \
    global.h \
    log.h \
    qthread.h
