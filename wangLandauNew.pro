QT += core
QT -= gui

CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    wlNew.cpp \
    gapmanager.cpp

LIBS += -L$$PWD/../partsEngine/ -lPartsEngine
INCLUDEPATH += $$PWD/../partsEngine
DEPENDPATH += $$PWD/../partsEngine
PRE_TARGETDEPS += $$PWD/../partsEngine/libPartsEngine.a

CONFIG(release,debug|release){
    DEFINES += QT_NO_DEBUG_OUTPUT
}

CONFIG+=c++11

HEADERS += \
    wlNew.h \
    gapmanager.h
