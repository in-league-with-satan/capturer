QT += \
    core \
    gui \
    widgets \
    multimedia \
    multimediawidgets \
    opengl

TARGET = capturer
TEMPLATE = app

CONFIG += c++14

DESTDIR = ../bin

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0




#apt-get install mesa-common-dev libgl-dev libpulse-dev libsdl2-dev

#DEFINES += USE_X264_10B
DEFINES += USE_PULSE_AUDIO
DEFINES += USE_SDL2



LINK_OPT=shared
BUILD_OPT=release

static {
    LINK_OPT=static
}

CONFIG(debug, debug|release):{
    BUILD_OPT=debug

} else {
    DEFINES += QT_NO_DEBUG_OUTPUT

    QMAKE_CFLAGS_RELEASE = "-march=native -O3 -fomit-frame-pointer -pipe"
    QMAKE_CXXFLAGS_RELEASE = "-march=native -O3 -fomit-frame-pointer -pipe"
}


OBJECTS_DIR = $$BUILD_OPT/$$QT_VERSION-$$LINK_OPT/8bit-obj
MOC_DIR     = $$BUILD_OPT/$$QT_VERSION-$$LINK_OPT/8bit-moc
RCC_DIR     = $$BUILD_OPT/$$QT_VERSION-$$LINK_OPT/8bit-rcc


include(../externals/overlay/overlay.pri)


INCLUDEPATH += \
    ../externals/3rdparty/blackmagic_decklink_sdk/Linux/include

contains(DEFINES, USE_X264_10B) {
    TARGET = capturer_10bit

    OBJECTS_DIR = $$BUILD_OPT/$$QT_VERSION-$$LINK_OPT/10bit-obj
    MOC_DIR     = $$BUILD_OPT/$$QT_VERSION-$$LINK_OPT/10bit-moc
    RCC_DIR     = $$BUILD_OPT/$$QT_VERSION-$$LINK_OPT/10bit-rcc

    INCLUDEPATH += ../externals/3rdparty/ffmpeg/10bit/include
    LIBS += -L../externals/3rdparty/ffmpeg/10bit/lib

} else {
    INCLUDEPATH += ../externals/3rdparty/ffmpeg/8bit/include
    LIBS += -L../externals/3rdparty/ffmpeg/8bit/lib
}



SOURCES += \
    ../externals/3rdparty/blackmagic_decklink_sdk/Linux/include/DeckLinkAPIDispatch.cpp

LIBS += -lswresample  -lavformat -lavcodec -lavutil -lswscale -lswresample
LIBS += -lz -ldl -lvorbis -lvorbisenc -logg -lspeex -lfdk-aac -lmp3lame -lopus -lvpx -lx264 -lx265


contains(DEFINES, USE_PULSE_AUDIO) {
    LIBS += -lpulse-simple -lpulse
}

contains(DEFINES, USE_SDL2) {
    LIBS += -lSDL2main -lSDL2
}

INCLUDEPATH += \
    src \
    src/decklink \
    src/audio_output \
    src/video_output

SOURCES += \
    src/*.cpp \
    src/decklink/*.cpp \
    src/audio_output/*.cpp \
    src/video_output/*.cpp

HEADERS += \
    src/*.h \
    src/decklink/*.h \
    src/audio_output/*.h \
    src/video_output/*.h
