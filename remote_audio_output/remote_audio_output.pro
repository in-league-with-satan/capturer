QT += \
    core \
    gui \
    widgets \
    network \
    multimedia


TARGET = remote_audio_output

DEFINES += STATIC_WIN_FF


TEMPLATE = app

CONFIG += c++14

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


include($$PWD/../externals/3rdparty/ffmpeg/ffmpeg.pri)


INCLUDEPATH += \
    $$PWD/../shared

HEADERS += \
    $$PWD/../shared/audio_packet.h


INCLUDEPATH += \
    $$PWD/src \
    $$PWD/../capturer/src/ffmpeg

SOURCES += \
    $$PWD/src/*.cpp \
    $$PWD/../capturer/src/ffmpeg/ff_audio_converter.cpp \
    $$PWD/../capturer/src/ffmpeg/ff_tools.cpp

HEADERS += \
    $$PWD/src/*.h \
    $$PWD/../capturer/src/ffmpeg/ff_audio_converter.h \
    $$PWD/../capturer/src/ffmpeg/ff_tools.h

