linux {
    GCCFLAGS += -lz

    contains(DEFINES, USE_X264_10B) {
        INCLUDEPATH += $$PWD/10bit/include
        LIBS += -L$$PWD/10bit/lib

    } else {
        INCLUDEPATH += $$PWD/8bit/include
        LIBS += -L$$PWD/8bit/lib
    }

    LIBS += -lavdevice -lavfilter -lpostproc -lavformat -lavcodec -lavutil -lswscale -lswresample
    LIBS += -lz -lbz2 -ldl -lvorbis -lvorbisenc -logg -lspeex -lfdk-aac -lmp3lame -lopus -lvpx -lx264 -lx265
    LIBS +=-lxcb -lxcb-xfixes -lxcb-shape
    # -libxcb1
    LIBS += -llzma
}

windows {
    INCLUDEPATH += $$PWD/include
    LIBS += -L$$PWD/lib

    contains(DEFINES, STATIC_WIN_FF) {
        LIBS += -lavdevice -lavfilter -lavformat -lavcodec -lavresample -lavutil -lpostproc -lswresample -lswscale
        LIBS += -lvfw32 -lcaca -lSDL2 -lbs2b -lrubberband -lfftw3 -lsamplerate -lmysofa -lflite_cmu_us_awb -lflite_cmu_us_kal -lflite_cmu_us_kal16 -lflite_cmu_us_rms -lflite_cmu_us_slt
        LIBS += -lflite_usenglish -lflite_cmulex -lflite -lfribidi -lass -liconv -lfontconfig -lfreetype -lxml2 -lbz2 -lvidstab -lzimg -lmfx -lgme -lmodplug -lbluray -lgnutls
        LIBS += -lcrypt32 -lbcrypt
        LIBS += -lhogweed -lgmp -lnettle -lvpx -lopencore-amrwb -lzvbi -lsnappy -lgsm -lilbc -lmp3lame -lopencore-amrnb -lopenjp2 -lopus -lspeex -ltheoraenc -ltheoradec -logg -ltwolame
        LIBS += -lvo-amrwbenc -lvorbis -lvorbisenc -lx264 -lx265 -lkernel32 -lxavs -lxvidcore -lopenh264 -laom -lsoxr -ldl -lz -llzma -lpsapi
        LIBS += -ltesseract -llept

    } else {
        LIBS += -lavdevice -lswresample -lavformat -lavcodec -lavutil -lswscale
    }
}

