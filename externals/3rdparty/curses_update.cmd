@echo off


SET PATH_ROOT=%~dp0


cd "%PATH_ROOT%"

IF exist pdcurses (
    cd pdcurses
    git pull --ff-only

) ELSE (
    git clone --depth 1 https://github.com/wmcbrine/PDCurses.git pdcurses
)
