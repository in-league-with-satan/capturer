# linux build dependencies: mesa-common-dev libgl-dev libpulse-dev libbz2-dev liblzma-dev

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

#DEFINES += QT_DEPRECATED_WARNINGS
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000
DEFINES += QT_MESSAGELOGCONTEXT

DEFINES += USE_PULSE_AUDIO
#DEFINES += STATIC_WIN_FF
#DEFINES += __linux__


TEMPLATE = app

CONFIG += c++17
#windows:CONFIG += console
#CONFIG += console
CONFIG -= qtquickcompiler


!defined(DESTDIR, var): DESTDIR = $$PWD/../bin


GIT_HASH = $$system(git --git-dir $$PWD/../.git log -1 --pretty=format:%h)
GIT_TAG_REV = $$system(git rev-list --tags --max-count=1)
GIT_LAST_TAG = $$system(git --git-dir $$PWD/../.git describe --tags $$GIT_TAG_REV)
GIT_CMT_COUNT = $$system(git --git-dir $$PWD/../.git rev-list '$$GIT_LAST_TAG'.. --count)

VERSION_APP = $$GIT_LAST_TAG"."$$GIT_CMT_COUNT-$$GIT_HASH
VERSION = $$GIT_LAST_TAG"."$$GIT_CMT_COUNT

DEFINES += VERSION_STRING=\\\"$$VERSION_APP\\\"

#message(v$$VERSION_APP)

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

    RC_ICONS += $$PWD/../icon/capturer.ico
    QMAKE_TARGET = "capturer v$$VERSION"
    QMAKE_TARGET_DESCRIPTION = "capturer"
}


include($$PWD/../externals/3rdparty/ffmpeg/ffmpeg.pri)
include($$PWD/../externals/3rdparty/http_server.pri)
include($$PWD/../externals/3rdparty/magewell_capture_sdk/magewell.pri)
include($$PWD/../externals/3rdparty/decklink.pri)
include($$PWD/../externals/3rdparty/curses.pri)

INCLUDEPATH += \
    $$PWD/../shared

HEADERS += \
    $$files($$PWD/../shared/*.h)

SOURCES += \
    $$files($$PWD/../shared/*.cpp)


INCLUDEPATH += \
    $$PWD/src \
    $$PWD/src/ffmpeg \
    $$PWD/src/audio_output \
    $$PWD/src/overlay \
    $$PWD/src/network \
    $$PWD/src/nv \
    $$PWD/src/term \
    $$PWD/src/term/cursed \
    $$PWD/src/video_sources \
    $$PWD/src/video_sources/dummy \
    $$PWD/src/video_sources/ffmpeg \
    $$PWD/src/video_sources/magewell \
    $$PWD/src/video_sources/decklink

SOURCES += \
    $$files($$PWD/src/*.cpp) \
    $$files($$PWD/src/ffmpeg/*.cpp) \
    $$files($$PWD/src/audio_output/*.cpp) \
    $$files($$PWD/src/overlay/*.cpp) \
    $$files($$PWD/src/network/*.cpp) \
    $$files($$PWD/src/nv/*.cpp) \
    $$files($$PWD/src/term/*.cpp) \
    $$files($$PWD/src/term/cursed/*.cpp) \
    $$files($$PWD/src/video_sources/dummy/*.cpp) \
    $$files($$PWD/src/video_sources/ffmpeg/*.cpp) \
    $$files($$PWD/src/video_sources/magewell/*.cpp) \
    $$files($$PWD/src/video_sources/decklink/*.cpp)

HEADERS += \
    $$files($$PWD/src/*.h) \
    $$files($$PWD/src/ffmpeg/*.h) \
    $$files($$PWD/src/audio_output/*.h) \
    $$files($$PWD/src/overlay/*.h) \
    $$files($$PWD/src/network/*.h) \
    $$files($$PWD/src/nv/*.h) \
    $$files($$PWD/src/term/*.h) \
    $$files($$PWD/src/term/cursed/*.h) \
    $$files($$PWD/src/video_sources/*.h) \
    $$files($$PWD/src/video_sources/dummy/*.h) \
    $$files($$PWD/src/video_sources/ffmpeg/*.h) \
    $$files($$PWD/src/video_sources/magewell/*.h) \
    $$files($$PWD/src/video_sources/decklink/*.h)

RESOURCES += \
    $$PWD/qml.qrc \
    $$PWD/images.qrc \
    $$PWD/html.qrc

OTHER_FILES += \
    $$files($$PWD/qml/*.qml) \
    $$files($$PWD/html/*)
