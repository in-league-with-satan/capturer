QT += \
    core \
    gui \
    widgets \
    multimedia \
    opengl

TARGET = capturer
TEMPLATE = app

CONFIG += c++14

DESTDIR = bin

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0




#apt-get install mesa-common-dev libgl-dev libpulse-dev

#DEFINES += USE_PULSE_AUDIO
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
}


OBJECTS_DIR = $$BUILD_OPT/$$QT_VERSION-$$LINK_OPT/obj
MOC_DIR     = $$BUILD_OPT/$$QT_VERSION-$$LINK_OPT/moc
RCC_DIR     = $$BUILD_OPT/$$QT_VERSION-$$LINK_OPT/rcc



INCLUDEPATH += \
    externals/3rdparty/blackmagic_decklink_sdk/Linux/include \
    externals/3rdparty/ffmpeg/include

SOURCES += \
    externals/3rdparty/blackmagic_decklink_sdk/Linux/include/DeckLinkAPIDispatch.cpp


LIBS += -Lexternals/3rdparty/ffmpeg/lib -lswresample  -lavformat -lavcodec -lavutil -lswscale -lswresample
LIBS += -lz -ldl -lvorbis -lvorbisenc -logg -lfdk-aac -lmp3lame -lopus -lvpx -lx264 -lx265


contains(DEFINES, USE_PULSE_AUDIO) {
    LIBS += -lpulse-simple -lpulse
}

contains(DEFINES, USE_SDL2) {
    LIBS += -lSDL2main -lSDL2
}

INCLUDEPATH += \
    src \
    src/decklink \
    src/audio_output

SOURCES += \
    src/*.cpp \
    src/decklink/*.cpp \
    src/audio_output/*.cpp

HEADERS += \
    src/*.h \
    src/decklink/*.h \
    src/audio_output/*.h
