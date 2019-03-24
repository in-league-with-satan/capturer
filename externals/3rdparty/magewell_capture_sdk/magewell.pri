INCLUDEPATH += \
    $$PWD

HEADERS += \
    $$PWD/*.h

SOURCES += \
    $$PWD/*.cpp

linux {
    DEFINES += LIB_MWCAPTURE

    INCLUDEPATH += \
        $$PWD/linux/Include \
        $$PWD/linux/Include/LibMWCapture

    HEADERS += \
        $$PWD/linux/Include/*.h \
        $$PWD/linux/Include/LibMWCapture/*.h

    INCLUDEPATH += \
        $$PWD/linux/Lib

    HEADERS += \
        $$PWD/linux/Lib/*.h

    SOURCES += \
        $$PWD/linux/Lib/*.c

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
            $$PWD/win/Include/*.h \
            $$PWD/win/Include/LibMWCapture/*.h


#        LIBS += $$PWD/win/Lib/Win32/LibMWCapture.lib
#        LIBS += $$PWD/win/Lib/x64/LibMWCapture.lib
    }
}
