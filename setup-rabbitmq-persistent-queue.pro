# (C) Copyright 2018
# Urs Fässler, bbv Software Services, http://bbv.ch
#
# SPDX-License-Identifier: GPL-3.0-or-later

TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt
CONFIG += warn_on

QMAKE_CXXFLAGS += "-Wextra -pedantic"

HEADERS += \
    utility.h \

SOURCES += \
    setup-rabbitmq-persistent-queue.cpp \
    utility.cpp \

LIBS += -lrabbitmq

target.path = /usr/bin/
INSTALLS += target
