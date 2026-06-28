QT += core
QT -= gui

CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    wlNew.cpp \
    gapmanager.cpp \
    MagneticSystem.cpp \
    dos2.cpp \
    misc.cpp \
    debug_msg.cpp

LIBS += -L$$PWD/../partsEngine/ -lPartsEngine
INCLUDEPATH += $$PWD/../partsEngine
DEPENDPATH += $$PWD/../partsEngine
PRE_TARGETDEPS += $$PWD/../partsEngine/libPartsEngine.a

CONFIG+=c++11

HEADERS += \
    wlNew.h \
    gapmanager.h

# ──────────────────────────────────────────────────
# Опция отладочных сообщений
# ──────────────────────────────────────────────────
# enable_debug_messages можно передать в qmake:
#   qmake enable_debug_messages=1
#   qmake enable_debug_messages=0
#
# По умолчанию: 1 для debug-сборки, 0 для release.
CONFIG(debug, debug|release) {
    isEmpty(enable_debug_messages): enable_debug_messages = 1
} else {
    isEmpty(enable_debug_messages): enable_debug_messages = 0
}

equals(enable_debug_messages, 1) {
    DEFINES += ENABLE_DEBUG_MESSAGES
    message("Debug messages: ENABLED")
} else {
    message("Debug messages: DISABLED")
}
