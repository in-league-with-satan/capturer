#!/bin/bash

PATH_ROOT=`dirname "$0"`
PATH_ROOT=`(cd "$PATH_ROOT" && pwd)`


cd $PATH_ROOT

if [ -e pdcurses ]; then
    cd pdcurses
    git pull --ff-only

else
    git clone --depth 1 https://github.com/wmcbrine/PDCurses.git pdcurses || exit 1
fi


exit 0
