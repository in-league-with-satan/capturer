linux {
    # GCCFLAGS += -lz

    INCLUDEPATH += $$PWD/linux/include
    LIBS += -L$$PWD/linux/lib

    LIBS += -lavdevice -lavfilter -lpostproc -lavformat -lavcodec -lavutil -lswscale -lswresample
    LIBS += -ldl -lopus -lvorbis -lvorbisenc -logg -lx264
    LIBS += -lva -lva-drm
    # LIBS += -lmfx
}

windows {
    INCLUDEPATH += $$PWD/win/include
    LIBS += -L$$PWD/win/lib

    contains(DEFINES, STATIC_WIN_FF) {
        LIBS += -lavdevice -lavfilter -lpostproc -lavformat -lavcodec -lavutil -lswscale -lswresample
        LIBS += -ldl -lopus -lvorbis -lvorbisenc -logg -lx264
        LIBS += -lmfx -lvfw32 -lbcrypt

    } else {
        LIBS += -lavdevice -lswresample -lavformat -lavcodec -lavutil -lswscale
    }
}
