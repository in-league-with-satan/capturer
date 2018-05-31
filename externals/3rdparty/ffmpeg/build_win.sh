#!/bin/bash

cd ffmpeg-windows-build-helpers

./cross_compile_ffmpeg.sh --build-ffmpeg-static=y --build-ffmpeg-shared=n --disable-nonfree=y --build-intel-qsv=y --git-get-latest=y --high-bitdepth=y --compiler-flavors=win64 # --cflags='-march=skylake -O3'

