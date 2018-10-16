#install mesa-common-dev libgl-dev libpulse-dev libbz2-dev liblzma-dev libnuma-dev

QT += \
    core \
    gui \
    widgets \
    multimedia \
    multimediawidgets\
    qml \
    quick \
    quickwidgets \
    quickcontrols2 \
    sql \
    svg


TARGET = capturer

DEFINES += QT_DEPRECATED_WARNINGS
DEFINES += QT_MESSAGELOGCONTEXT

DEFINES += USE_PULSE_AUDIO
#DEFINES += STATIC_WIN_FF
#DEFINES += __linux__


TEMPLATE = app

CONFIG += c++14
#windows:CONFIG += console

DESTDIR = $$PWD/../bin


GIT_VERSION = $$system(git --git-dir $$PWD/../.git --work-tree $$PWD describe --always --tags)
DEFINES += VERSION_STRING=\\\"$$GIT_VERSION\\\"


LINK_OPT=shared
BUILD_OPT=release

static {
    LINK_OPT=static
}

CONFIG(debug, debug|release):{
    BUILD_OPT=debug

} else {
    DEFINES += QT_NO_DEBUG_OUTPUT
    DEFINES += NDEBUG

    linux {
        QMAKE_CFLAGS_RELEASE = "-march=native -O3 -fomit-frame-pointer -pipe"
        QMAKE_CXXFLAGS_RELEASE = "-march=native -O3 -fomit-frame-pointer -pipe"
    }
}


OBJECTS_DIR = $$BUILD_OPT/$$QT_VERSION-$$LINK_OPT/obj
MOC_DIR     = $$BUILD_OPT/$$QT_VERSION-$$LINK_OPT/moc
RCC_DIR     = $$BUILD_OPT/$$QT_VERSION-$$LINK_OPT/rcc


linux {
    contains(DEFINES, USE_PULSE_AUDIO) {
        LIBS += -lpulse-simple -lpulse
    }
}

windows {
    DEFINES -= USE_PULSE_AUDIO
}


include($$PWD/../externals/3rdparty/ffmpeg/ffmpeg.pri)
include($$PWD/../externals/3rdparty/http_server.pri)
include($$PWD/../externals/3rdparty/magewell_capture_sdk/magewell.pri)
include($$PWD/../externals/3rdparty/decklink.pri)


INCLUDEPATH += \
    $$PWD/../shared

HEADERS += \
    $$PWD/../shared/*.h

SOURCES += \
    $$PWD/../shared/*.cpp


INCLUDEPATH += \
    $$PWD/src \
    $$PWD/src/ffmpeg \
    $$PWD/src/audio_output \
    $$PWD/src/overlay \
    $$PWD/src/network \
    $$PWD/src/nv \
    $$PWD/src/video_sources \
    $$PWD/src/video_sources/dummy \
    $$PWD/src/video_sources/ffmpeg \
    $$PWD/src/video_sources/magewell \
    $$PWD/src/video_sources/decklink

SOURCES += \
    $$PWD/src/*.cpp \
    $$PWD/src/ffmpeg/*.cpp \
    $$PWD/src/audio_output/*.cpp \
    $$PWD/src/overlay/*.cpp \
    $$PWD/src/network/*.cpp \
    $$PWD/src/nv/*.cpp \
    $$PWD/src/video_sources/dummy/*.cpp \
    $$PWD/src/video_sources/ffmpeg/*.cpp \
    $$PWD/src/video_sources/magewell/*.cpp \
    $$PWD/src/video_sources/decklink/*.cpp

HEADERS += \
    $$PWD/src/*.h \
    $$PWD/src/ffmpeg/*.h \
    $$PWD/src/audio_output/*.h \
    $$PWD/src/overlay/*.h \
    $$PWD/src/network/*.h \
    $$PWD/src/nv/*.h \
    $$PWD/src/video_sources/*.h \
    $$PWD/src/video_sources/dummy/*.h \
    $$PWD/src/video_sources/ffmpeg/*.h \
    $$PWD/src/video_sources/magewell/*.h \
    $$PWD/src/video_sources/decklink/*.h

RESOURCES += \
    $$PWD/qml.qrc \
    $$PWD/images.qrc

OTHER_FILES += \
    $$PWD/qml/*.qml
