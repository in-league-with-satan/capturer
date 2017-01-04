#!/bin/bash

mkdir ./videos/converted

find ./videos -maxdepth 1 -name '*.mkv' -type f \
    -exec ./convert_work.sh "{}" $@ ';'
