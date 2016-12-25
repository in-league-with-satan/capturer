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
    externals/3rdparty/blackmagic_decklink_sdk/Linux/include

SOURCES += \
    externals/3rdparty/blackmagic_decklink_sdk/Linux/include/DeckLinkAPIDispatch.cpp

LIBS += -lswresample  -lavformat -lavcodec -lavutil -lswscale -lswresample
LIBS += -ldl -lz -llzma -lbz2 -lvorbis -lvorbisenc -lmp3lame -lopus -lx264 -lva -lvdpau -lX11 -lva-drm -lva-x11

INCLUDEPATH += \
    src \
    src/decklink

SOURCES += \
    src/*.cpp \
    src/decklink/*.cpp

HEADERS += \
    src/*.h \
    src/decklink/*.h
