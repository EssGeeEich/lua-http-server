TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt
LIBS += -lmicrohttpd -lws2_32

include(../LuaPP/qt_luapp.pri)
INCLUDEPATH += ../LuaPP/include

VERSION_MAJOR = 1
VERSION_MINOR = 0
VERSION_PATCH = 0

#VER_MAJ = $$VERSION_MAJOR
#VER_MIN = $$VERSION_MINOR
#VER_PAT = $$VERSION_PATCH
#VERSION = $$VERSION_MAJOR"."$$VERSION_MINOR"."$$VERSION_PATCH
DEFINES += VERSION_MAJOR=$$VERSION_MAJOR VERSION_MINOR=$$VERSION_MINOR VERSION_PATCH=$$VERSION_PATCH

SOURCES += main.cpp \
    contentmanager.cpp \
    requestmanager.cpp \
    rm_keyvalues.cpp \
    rm_basicauth.cpp \
    rm_digestauth.cpp \
    rm_response.cpp \
    rm_serverinfo.cpp \
    rm_conninfo.cpp \
    rm_generic.cpp \
    virtualstate.cpp \
    sequencemanager.cpp

HEADERS += \
    spinlock_mutex.h \
    contentmanager.h \
    requestmanager.h \
    virtualstate.h \
    sequencemanager.h
