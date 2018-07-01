#install mesa-common-dev libgl-dev libpulse-dev libsdl2-dev libbz2-dev liblzma-dev libnuma-dev

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

DEFINES += USE_PULSE_AUDIO
#DEFINES += USE_SDL2
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


linux {
    INCLUDEPATH += \
        $$PWD/../externals/3rdparty/blackmagic_decklink_sdk/Linux/include

    SOURCES += \
        $$PWD/../externals/3rdparty/blackmagic_decklink_sdk/Linux/include/DeckLinkAPIDispatch.cpp

    contains(DEFINES, USE_PULSE_AUDIO) {
        LIBS += -lpulse-simple -lpulse
    }

    contains(DEFINES, USE_SDL2) {
        LIBS += -lSDL2main -lSDL2
    }
}

windows {
    DEFINES -= USE_PULSE_AUDIO
    DEFINES -= USE_SDL2

    INCLUDEPATH += \
        $$PWD/../externals/3rdparty/blackmagic_decklink_sdk-mingw

    HEADERS += \
        $$PWD/../externals/3rdparty/blackmagic_decklink_sdk-mingw/*.h

    SOURCES += \
        $$PWD/../externals/3rdparty/blackmagic_decklink_sdk-mingw/*.c

    LIBS += -lole32 -lstrmiids -loleaut32
}


include($$PWD/../externals/3rdparty/ffmpeg/ffmpeg.pri)
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
