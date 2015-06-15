EMPLATE = app

QT      += core gui widgets serialport
TARGET   = pelican-gui
DEFINES += QT_NO_STATEMACHINE

INCLUDEPATH = . \
    $(QPCPP)/include \
    $(QPCPP)/ports/qt \

SOURCES += \
    main.cpp \
    bsp.cpp \
    pelican.cpp \
    gui.cpp

HEADERS += \
    bsp.h \
    pelican.h \
    gui.h

FORMS += \
    gui.ui

CONFIG(debug, debug|release) {
    DEFINES += Q_SPY
    INCLUDEPATH += $(QTOOLS)/qspy/include
    SOURCES += $(QTOOLS)/qspy/source/qspy.c
    HEADERS += qs_port.h
    LIBS += -L$(QPCPP)/ports/qt/mingw/debug
} else {
    LIBS += -L$(QPCPP)/ports/qt/mingw/release
}

LIBS += -lqp
    
RESOURCES =

#win32:RC_FILE = gui.rc
