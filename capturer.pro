QT += \
    core \
    gui \
    widgets \
    multimedia \
    multimediawidgets\
    qml \
    quick \
    quickwidgets


TARGET = capturer
TEMPLATE = app

CONFIG += c++14

DESTDIR = bin

GIT_VERSION = $$system(git --git-dir $$PWD/.git --work-tree $$PWD describe --always)

linux {
    DATE_VERSION = $$system(date +%y.%-m.%-d)
    DEFINES += VERSION_STRING=\\\"$$DATE_VERSION-$$GIT_VERSION\\\"
}

windows {
    DATE_VERSION = $$system(echo '%date:~8,2%.%date:~3,2%.%date:~0,2%')
    DEFINES += VERSION_STRING=QString(\\\"$$DATE_VERSION-$$GIT_VERSION\\\").replace(\\\".0\\\",\\\".\\\")
}





# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0




#apt-get install mesa-common-dev libgl-dev libpulse-dev libsdl2-dev libbz2-dev

#DEFINES += USE_X264_10B
DEFINES += USE_PULSE_AUDIO
#DEFINES += USE_SDL2



LINK_OPT=shared
BUILD_OPT=release

static {
    LINK_OPT=static
}

CONFIG(debug, debug|release):{
    BUILD_OPT=debug

} else {
    DEFINES += QT_NO_DEBUG_OUTPUT

    linux {
        QMAKE_CFLAGS_RELEASE = "-march=native -O3 -fomit-frame-pointer -pipe"
        QMAKE_CXXFLAGS_RELEASE = "-march=native -O3 -fomit-frame-pointer -pipe"
    }
}


OBJECTS_DIR = $$BUILD_OPT/$$QT_VERSION-$$LINK_OPT/8bit-obj
MOC_DIR     = $$BUILD_OPT/$$QT_VERSION-$$LINK_OPT/8bit-moc
RCC_DIR     = $$BUILD_OPT/$$QT_VERSION-$$LINK_OPT/8bit-rcc


linux {
    INCLUDEPATH += \
        externals/3rdparty/blackmagic_decklink_sdk/Linux/include

    contains(DEFINES, USE_X264_10B) {
        TARGET = capturer_10bit

        OBJECTS_DIR = $$BUILD_OPT/$$QT_VERSION-$$LINK_OPT/10bit-obj
        MOC_DIR     = $$BUILD_OPT/$$QT_VERSION-$$LINK_OPT/10bit-moc
        RCC_DIR     = $$BUILD_OPT/$$QT_VERSION-$$LINK_OPT/10bit-rcc

        INCLUDEPATH += externals/3rdparty/ffmpeg/10bit/include
        LIBS += -Lexternals/3rdparty/ffmpeg/10bit/lib

    } else {
        INCLUDEPATH += externals/3rdparty/ffmpeg/8bit/include
        LIBS += -Lexternals/3rdparty/ffmpeg/8bit/lib
    }


    SOURCES += \
        externals/3rdparty/blackmagic_decklink_sdk/Linux/include/DeckLinkAPIDispatch.cpp

    LIBS += -lavformat -lavcodec -lavutil -lswscale -lswresample
    LIBS += -lz -lbz2 -ldl -lvorbis -lvorbisenc -logg -lspeex -lfdk-aac -lmp3lame -lopus -lvpx -lx264 -lx265
}

windows {
    DEFINES -= USE_X264_10B
    DEFINES -= USE_PULSE_AUDIO
    DEFINES -= USE_SDL2

    INCLUDEPATH += \
        externals/3rdparty/blackmagic_decklink_sdk-mingw

    HEADERS += \
        externals/3rdparty/blackmagic_decklink_sdk-mingw/*.h

    SOURCES += \
        externals/3rdparty/blackmagic_decklink_sdk-mingw/*.c

    LIBS += -lole32


    INCLUDEPATH += externals/3rdparty/ffmpeg/include
    LIBS += -Lexternals/3rdparty/ffmpeg/lib

    LIBS += -lswresample -lavformat -lavcodec -lavutil -lswscale
}

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
    src/video_output \
    src/overlay

SOURCES += \
    src/*.cpp \
    src/decklink/*.cpp \
    src/audio_output/*.cpp \
    src/video_output/*.cpp \
    src/overlay/*.cpp

HEADERS += \
    src/*.h \
    src/decklink/*.h \
    src/audio_output/*.h \
    src/video_output/*.h \
    src/overlay/*.h

RESOURCES += \
    qml.qrc
