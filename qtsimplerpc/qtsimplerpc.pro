#############################################################################
##
## Copyright (C) 2012 Sebastian Lehmann
## Contact: contact@l3.ms
##
##
## This file is part of QtSimpleRPC.
##
## QtSimpleRPC is free software: you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation, either version 3 of the License, or
## (at your option) any later version.
##
## QtSimpleRPC is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
##
#############################################################################

QT += core network
QT -= gui

TARGET = QtSimpleRpc
TEMPLATE = lib

DEFINES += QTSIMPLERPC_LIBRARY

SOURCES += rpcsignalmapperhelper.cpp \
    rpcsignalmapper.cpp \
    rpcconnection.cpp \
    rpccommandmapper.cpp \
    qjson.cpp \
    qtsimplerpc.cpp

HEADERS += \
    rpcsignalmapper.h \
    rpcconnection.h \
    rpccommandmapper.h \
    qjson.h \
    qtsimplerpc_global.h \
    qtsimplerpc.h
