linux {
    exists($$PWD/blackmagic_decklink_sdk/Linux/include/DeckLinkAPIDispatch.cpp) {
        DEFINES += LIB_DECKLINK

        INCLUDEPATH += \
            $$PWD/blackmagic_decklink_sdk/Linux/include

        SOURCES += \
            $$PWD/blackmagic_decklink_sdk/Linux/include/DeckLinkAPIDispatch.cpp
    }
}

windows {
    DEFINES += LIB_DECKLINK

    INCLUDEPATH += \
        $$PWD/blackmagic_decklink_sdk-mingw

    HEADERS += \
        $$files($$PWD/blackmagic_decklink_sdk-mingw/*.h)

    SOURCES += \
        $$files($$PWD/blackmagic_decklink_sdk-mingw/*.c)

    LIBS += -lole32 -lstrmiids -loleaut32
}
