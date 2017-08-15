QT += \
    core


TARGET = odd_even_converter
TEMPLATE = app

CONFIG += c++14
CONFIG += console

DESTDIR = bin



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


OBJECTS_DIR = $$BUILD_OPT/$$QT_VERSION-$$LINK_OPT/odd-obj
MOC_DIR     = $$BUILD_OPT/$$QT_VERSION-$$LINK_OPT/odd-moc
RCC_DIR     = $$BUILD_OPT/$$QT_VERSION-$$LINK_OPT/odd-rcc


linux {
    contains(DEFINES, USE_X264_10B) {
        TARGET = capturer_10bit

        OBJECTS_DIR = $$BUILD_OPT/$$QT_VERSION-$$LINK_OPT/10bit-odd-obj
        MOC_DIR     = $$BUILD_OPT/$$QT_VERSION-$$LINK_OPT/10bit-odd-moc
        RCC_DIR     = $$BUILD_OPT/$$QT_VERSION-$$LINK_OPT/10bit-odd-rcc

        INCLUDEPATH += externals/3rdparty/ffmpeg/10bit/include
        LIBS += -Lexternals/3rdparty/ffmpeg/10bit/lib

    } else {
        INCLUDEPATH += externals/3rdparty/ffmpeg/8bit/include
        LIBS += -Lexternals/3rdparty/ffmpeg/8bit/lib
    }

    LIBS += -lavformat -lavcodec -lavutil -lswscale -lswresample
    LIBS += -lz -lbz2 -ldl -lvorbis -lvorbisenc -logg -lspeex -lfdk-aac -lmp3lame -lopus -lvpx -lx264 -lx265
}

windows {
    DEFINES -= USE_X264_10B

    INCLUDEPATH += externals/3rdparty/ffmpeg/include
    LIBS += -Lexternals/3rdparty/ffmpeg/lib

    LIBS += -lswresample -lavformat -lavcodec -lavutil -lswscale
}


INCLUDEPATH += \
    src_odd_even_converter \
    src/ffmpeg

SOURCES += \
    src_odd_even_converter/*.cpp \
    src/ffmpeg/odd_even_converter.cpp \
    src/ffmpeg/ff_tools.cpp

HEADERS += \
    src/ffmpeg/odd_even_converter.h \
    src/ffmpeg/ff_tools.h

