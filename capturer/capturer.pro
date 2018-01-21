QT += \
    core \
    gui \
    widgets \
    multimedia \
    multimediawidgets\
    qml \
    quick \
    quickwidgets \
    sql \
    svg


TARGET = capturer
TEMPLATE = app

CONFIG += c++14

DESTDIR = $$PWD/../bin

GIT_VERSION = $$system(git --git-dir $$PWD/../.git --work-tree $$PWD describe --always)

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
    DEFINES += NDEBUG

    linux {
        QMAKE_CFLAGS_RELEASE = "-march=native -O3 -fomit-frame-pointer -pipe"
        QMAKE_CXXFLAGS_RELEASE = "-march=native -O3 -fomit-frame-pointer -pipe"
    }
}


OBJECTS_DIR = $$BUILD_OPT/$$QT_VERSION-$$LINK_OPT/8bit-obj
MOC_DIR     = $$BUILD_OPT/$$QT_VERSION-$$LINK_OPT/8bit-moc
RCC_DIR     = $$BUILD_OPT/$$QT_VERSION-$$LINK_OPT/8bit-rcc


linux {
    GCCFLAGS += -lz

    INCLUDEPATH += \
        $$PWD/../externals/3rdparty/blackmagic_decklink_sdk/Linux/include

    contains(DEFINES, USE_X264_10B) {
        TARGET = capturer_10bit

        OBJECTS_DIR = $$BUILD_OPT/$$QT_VERSION-$$LINK_OPT/10bit-obj
        MOC_DIR     = $$BUILD_OPT/$$QT_VERSION-$$LINK_OPT/10bit-moc
        RCC_DIR     = $$BUILD_OPT/$$QT_VERSION-$$LINK_OPT/10bit-rcc

        INCLUDEPATH += $$PWD/../externals/3rdparty/ffmpeg/10bit/include
        LIBS += -L$$PWD/../externals/3rdparty/ffmpeg/10bit/lib

    } else {
        INCLUDEPATH += $$PWD/../externals/3rdparty/ffmpeg/8bit/include
        LIBS += -L$$PWD/../externals/3rdparty/ffmpeg/8bit/lib
    }


    SOURCES += \
        $$PWD/../externals/3rdparty/blackmagic_decklink_sdk/Linux/include/DeckLinkAPIDispatch.cpp

    LIBS += -lavdevice -lavfilter -lpostproc -lavformat -lavcodec -lavutil -lswscale -lswresample
    LIBS += -lz -lbz2 -ldl -lvorbis -lvorbisenc -logg -lspeex -lfdk-aac -lmp3lame -lopus -lvpx -lx264 -lx265
    LIBS +=-lxcb -lxcb-xfixes -lxcb-shape
    # -libxcb1
    # -llzma
}

windows {
    DEFINES -= USE_X264_10B
    DEFINES -= USE_PULSE_AUDIO
    DEFINES -= USE_SDL2

    INCLUDEPATH += \
        $$PWD/../externals/3rdparty/blackmagic_decklink_sdk-mingw

    HEADERS += \
        $$PWD/../externals/3rdparty/blackmagic_decklink_sdk-mingw/*.h

    SOURCES += \
        $$PWD/../externals/3rdparty/blackmagic_decklink_sdk-mingw/*.c

    LIBS += -lole32 -lstrmiids -loleaut32

    INCLUDEPATH += $$PWD/../externals/3rdparty/ffmpeg/include
    LIBS += -L$$PWD/../externals/3rdparty/ffmpeg/lib

    LIBS += -lavdevice -lswresample -lavformat -lavcodec -lavutil -lswscale
}

contains(DEFINES, USE_PULSE_AUDIO) {
    LIBS += -lpulse-simple -lpulse
}

contains(DEFINES, USE_SDL2) {
    LIBS += -lSDL2main -lSDL2
}


include($$PWD/../externals/3rdparty/http_server.pri)


INCLUDEPATH += \
    $$PWD/../shared

HEADERS += \
    $$PWD/../shared/*.h

SOURCES += \
    $$PWD/../shared/*.cpp


INCLUDEPATH += \
    $$PWD/src \
    $$PWD/src/decklink \
    $$PWD/src/ffmpeg \
    $$PWD/src/audio_output \
    $$PWD/src/overlay \
    $$PWD/src/network \
    $$PWD/src/video_sources

SOURCES += \
    $$PWD/src/*.cpp \
    $$PWD/src/decklink/*.cpp \
    $$PWD/src/ffmpeg/*.cpp \
    $$PWD/src/audio_output/*.cpp \
    $$PWD/src/overlay/*.cpp \
    $$PWD/src/network/*.cpp \
    $$PWD/src/video_sources/*.cpp

HEADERS += \
    $$PWD/src/*.h \
    $$PWD/src/decklink/*.h \
    $$PWD/src/ffmpeg/*.h \
    $$PWD/src/audio_output/*.h \
    $$PWD/src/overlay/*.h \
    $$PWD/src/network/*.h \
    $$PWD/src/video_sources/*.h

RESOURCES += \
    $$PWD/qml.qrc \
    $$PWD/images.qrc

OTHER_FILES += \
    $$PWD/qml/*.qml
