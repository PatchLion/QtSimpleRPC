QT += core network
QT -= gui

TARGET = qtsimplerpc
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    rpcsignalmapperhelper.cpp \
    rpcsignalmapper.cpp \
    rpcconnection.cpp \
    rpccommandmapper.cpp \
    qjson.cpp

HEADERS += \
    rpcsignalmapper.h \
    rpcconnection.h \
    rpccommandmapper.h \
    qjson.h
