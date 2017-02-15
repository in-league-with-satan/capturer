QT += \
    widgets


TARGET = overlay_test
TEMPLATE = app

CONFIG += c++14


DESTDIR = bin

include(overlay.pri)


LINK_OPT=shared
BUILD_OPT=release

static {
    LINK_OPT=static
}

CONFIG(debug, debug|release):{
    BUILD_OPT=debug
}

OBJECTS_DIR = $$BUILD_OPT/$$QT_VERSION-$$LINK_OPT/obj
MOC_DIR     = $$BUILD_OPT/$$QT_VERSION-$$LINK_OPT/moc
RCC_DIR     = $$BUILD_OPT/$$QT_VERSION-$$LINK_OPT/rcc


INCLUDEPATH += \
    src

SOURCES += \
    src/*.cpp

HEADERS += \
    src/*.h

