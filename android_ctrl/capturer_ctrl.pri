QT += \
    network \
    qml \
    quick \

android: QT += androidextras

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Additional import path used to resolve QML modules just for Qt Quick Designer
QML_DESIGNER_IMPORT_PATH =

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
#DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

## Default rules for deployment.
#qnx: target.path = /tmp/$${TARGET}/bin
#else: unix:!android: target.path = /opt/$${TARGET}/bin
#!isEmpty(target.path): INSTALLS += target


CONFIG += c++17

!android {
    DESTDIR = $$PWD/../bin
}


TARGET = capturer_ctrl


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

INCLUDEPATH += \
    $$PWD/../shared

HEADERS += \
    $$files($$PWD/../shared/*.h)

SOURCES += \
    $$files($$PWD/../shared/*.cpp)


INCLUDEPATH += \
    $$PWD/src \
    $$PWD/src/network

SOURCES += \
    $$files($$PWD/src/*.cpp) \
    $$files($$PWD/src/network/*.cpp)

HEADERS += \
    $$files($$PWD/src/*.h) \
    $$files($$PWD/src/network/*.h)


RESOURCES += \
    $$PWD/qml.qrc

OTHER_FILES += \
    $$files($$PWD/qml/*.qml)
