QT += \
    core \
    gui \
    widgets \
    network \
    multimedia


TARGET = remote_audio_output
TEMPLATE = app

CONFIG += c++14

DESTDIR = ../bin

GIT_VERSION = $$system(git --git-dir $$PWD/../.git --work-tree $$PWD describe --always)

linux {
    DATE_VERSION = $$system(date +%y.%-m.%-d)
    DEFINES += VERSION_STRING=\\\"$$DATE_VERSION-$$GIT_VERSION\\\"
}

windows {
    DATE_VERSION = $$system(echo '%date:~8,2%.%date:~3,2%.%date:~0,2%')
    DEFINES += VERSION_STRING=QString(\\\"$$DATE_VERSION-$$GIT_VERSION\\\").replace(\\\".0\\\",\\\".\\\")
}





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
    INCLUDEPATH += ../externals/3rdparty/ffmpeg/8bit/include
    LIBS += -L../externals/3rdparty/ffmpeg/8bit/lib

    LIBS += -lavformat -lavcodec -lavutil -lswscale -lswresample
    LIBS += -lz -lbz2 -ldl -lvorbis -lvorbisenc -logg -lspeex -lfdk-aac -lmp3lame -lopus -lvpx -lx264 -lx265
}

windows {
    INCLUDEPATH += ../externals/3rdparty/ffmpeg/include
    LIBS += -L../externals/3rdparty/ffmpeg/lib

    LIBS += -lswresample -lavformat -lavutil
}

INCLUDEPATH += \
    ../shared

HEADERS += \
    ../shared/audio_packet.h



INCLUDEPATH += \
    src \
    ../src/ffmpeg


SOURCES += \
    src/*.cpp \
    ../src/ffmpeg/ff_audio_converter.cpp \
    ../src/ffmpeg/ff_tools.cpp

HEADERS += \
    src/*.h \
    ../src/ffmpeg/ff_audio_converter.h \
    ../src/ffmpeg/ff_tools.h

