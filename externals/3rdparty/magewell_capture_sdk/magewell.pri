linux {
    INCLUDEPATH += \
        $$PWD \
        $$PWD/linux/Include \
        $$PWD/linux/Include/LibMWCapture

    HEADERS += \
        $$PWD/*.h \
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
