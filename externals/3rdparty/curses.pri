# ncurses-devel

linux: exists(/usr/include/ncursesw/curses.h) | exists(/usr/include/curses.h) {
    DEFINES += LIB_CURSES
    LIBS += -lncursesw
    LIBS += -lpanel

    INCLUDEPATH += \
        /usr/include/ncursesw
}

windows: exists($$PWD/pdcurses/curses.h) {
    DEFINES += LIB_CURSES
    DEFINES += PDC_WIDE
    DEFINES += PDC_FORCE_UTF8

    INCLUDEPATH += \
        $$PWD/pdcurses \
        $$PWD/pdcurses/pdcurses \
        $$PWD/pdcurses/wincon

    SOURCES += \
        $$PWD/pdcurses/pdcurses/addch.c \
        $$PWD/pdcurses/pdcurses/addchstr.c \
        $$PWD/pdcurses/pdcurses/addstr.c \
        $$PWD/pdcurses/pdcurses/attr.c \
        $$PWD/pdcurses/pdcurses/beep.c \
        $$PWD/pdcurses/pdcurses/bkgd.c \
        $$PWD/pdcurses/pdcurses/border.c \
        $$PWD/pdcurses/pdcurses/clear.c \
        $$PWD/pdcurses/pdcurses/color.c \
        $$PWD/pdcurses/pdcurses/debug.c \
        $$PWD/pdcurses/pdcurses/delch.c \
        $$PWD/pdcurses/pdcurses/deleteln.c \
        $$PWD/pdcurses/pdcurses/getch.c \
        $$PWD/pdcurses/pdcurses/getstr.c \
        $$PWD/pdcurses/pdcurses/getyx.c \
        $$PWD/pdcurses/pdcurses/inch.c \
        $$PWD/pdcurses/pdcurses/inchstr.c \
        $$PWD/pdcurses/pdcurses/initscr.c \
        $$PWD/pdcurses/pdcurses/inopts.c \
        $$PWD/pdcurses/pdcurses/insch.c \
        $$PWD/pdcurses/pdcurses/insstr.c \
        $$PWD/pdcurses/pdcurses/instr.c \
        $$PWD/pdcurses/pdcurses/kernel.c \
        $$PWD/pdcurses/pdcurses/keyname.c \
        $$PWD/pdcurses/pdcurses/mouse.c \
        $$PWD/pdcurses/pdcurses/move.c \
        $$PWD/pdcurses/pdcurses/outopts.c \
        $$PWD/pdcurses/pdcurses/overlay.c \
        $$PWD/pdcurses/pdcurses/pad.c \
        $$PWD/pdcurses/pdcurses/panel.c \
        $$PWD/pdcurses/pdcurses/printw.c \
        $$PWD/pdcurses/pdcurses/refresh.c \
        $$PWD/pdcurses/pdcurses/scanw.c \
        $$PWD/pdcurses/pdcurses/scr_dump.c \
        $$PWD/pdcurses/pdcurses/scroll.c \
        $$PWD/pdcurses/pdcurses/slk.c \
        $$PWD/pdcurses/pdcurses/termattr.c \
        $$PWD/pdcurses/pdcurses/touch.c \
        $$PWD/pdcurses/pdcurses/util.c \
        $$PWD/pdcurses/pdcurses/window.c \
        $$PWD/pdcurses/wincon/pdcclip.c \
        $$PWD/pdcurses/wincon/pdcdisp.c \
        $$PWD/pdcurses/wincon/pdcgetsc.c \
        $$PWD/pdcurses/wincon/pdckbd.c \
        $$PWD/pdcurses/wincon/pdcscrn.c \
        $$PWD/pdcurses/wincon/pdcsetsc.c \
        $$PWD/pdcurses/wincon/pdcutil.c

    HEADERS += \
        $$PWD/pdcurses/curses.h \
        $$PWD/pdcurses/curspriv.h \
        $$PWD/pdcurses/panel.h \
        $$PWD/pdcurses/wincon/pdcwin.h
}

