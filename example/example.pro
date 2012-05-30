QT -= gui
QT += network

TEMPLATE = app

LIBS += -L../qtsimplerpc-build-desktop -lQtSimpleRpc
INCLUDEPATH += ../include

HEADERS += exampleclass.h

SOURCES += main.cpp \
    exampleclass.cpp

