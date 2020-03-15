INCLUDEPATH += \
    $$PWD

HEADERS += \
    $$files($$PWD/*.h)

SOURCES += \
    $$files($$PWD/*.cpp)

linux {
    DEFINES += LIB_MWCAPTURE

    INCLUDEPATH += \
        $$PWD/linux/Include \
        $$PWD/linux/Include/LibMWCapture

    HEADERS += \
        $$files($$PWD/linux/Include/*.h) \
        $$files($$PWD/linux/Include/LibMWCapture/*.h)

    INCLUDEPATH += \
        $$PWD/linux/Lib

    HEADERS += \
        $$files($$PWD/linux/Lib/*.h)

    SOURCES += \
        $$files($$PWD/linux/Lib/*.c)

#    LIBS += $$PWD/linux/Lib/x64/libMWCapture.a
#    LIBS += -ludev -lasound -lv4l2
}

windows {
    exists($$PWD/win/Include/LibMWCapture/MWCapture.h) {
        DEFINES += LIB_MWCAPTURE

        INCLUDEPATH += \
            $$PWD/win/Include \
            $$PWD/win/Include/LibMWCapture

        HEADERS += \
            $$files($$PWD/win/Include/*.h) \
            $$files($$PWD/win/Include/LibMWCapture/*.h)


#        LIBS += $$PWD/win/Lib/Win32/LibMWCapture.lib
#        LIBS += $$PWD/win/Lib/x64/LibMWCapture.lib
    }
}
