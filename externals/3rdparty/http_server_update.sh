#!/bin/bash

PATH_ROOT=`dirname "$0"`
PATH_ROOT=`(cd "$PATH_ROOT" && pwd)`


cd $PATH_ROOT

if [ -e http-parser ]; then
    cd http-parser
    git pull --ff-only

else
    git clone https://github.com/nodejs/http-parser.git || exit 1
fi


cd $PATH_ROOT

if [ -e qhttp ]; then
    cd qhttp
    git pull --ff-only

else
    git clone https://github.com/azadkuh/qhttp.git  || exit 1
fi


exit 0
