TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt
CONFIG += warn_on

QMAKE_CXXFLAGS += "-Wextra -pedantic"

HEADERS += \
    utility.h \

SOURCES += \
    main.cpp \
    utility.cpp \

LIBS += -lrabbitmq

target.path = /usr/bin/
INSTALLS += target
