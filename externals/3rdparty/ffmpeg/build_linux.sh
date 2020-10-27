#!/bin/bash

#sudo apt-get -y install autoconf automake libtool build-essential cmake git pkg-config


missing_deps=""

test_cmd() {
  if ! [ -x "$(command -v $1)" ]; then
    missing_deps="$missing_deps $2"
  fi
}

test_cmd autoconf autoconf
test_cmd automake automake
test_cmd libtoolize libtool
test_cmd make build-essential
test_cmd cmake cmake
test_cmd git git
test_cmd pkg-config pkg-config

if [[ ! -z "$missing_deps" ]]; then
  echo "following packages are required:$missing_deps"
  exit 1
fi


str_opt="-march=native -O3"
str_opt="-O2"


git_up_to_date="Already up to date."

build_counter=0

PATH_ROOT=$(dirname $(readlink -f $0))/linux
PATH_BASE=$PATH_ROOT
PATH_BUILD=$PATH_ROOT/tmp

PATH_ORIG=$PATH
PATH=$PATH_ROOT:$PATH_ROOT/lib:$PATH_ROOT/include:$PATH_ROOT/bin:$PATH_ORIG


cpu_count=`nproc`

export PKG_CONFIG_PATH=$PATH_BASE/lib/pkgconfig


if [ ! -e $PATH_BUILD ]; then
  mkdir -p $PATH_BUILD
fi


build_nasm() {
  cd $PATH_BUILD

  if [ ! -e nasm ]; then
    build_counter=$((build_counter + 1))

    git clone git://repo.or.cz/nasm.git --branch nasm-2.14.02 --single-branch --depth 1 || exit 1
    cd nasm

    autoreconf -fiv
    CFLAGS="$str_opt" ./configure --prefix="$PATH_BASE" --bindir="$PATH_BASE/bin" || exit 1

    make everything -j$cpu_count
    make install

    if [ ! -e $PATH_BASE/bin/nasm ]; then
      echo 'nasm err'
      exit 1
    fi
  fi
}

build_ogg() {
  build_required=1

  cd $PATH_BUILD

  if [ ! -e ogg ]; then
    git clone --depth 1 https://github.com/xiph/ogg.git || exit 1
    cd ogg

  else
    cd ogg
    git reset --hard
    git clean -dfx
    git pull --ff-only | grep "$git_up_to_date" && build_required=0
  fi

  if [ ! -e $PATH_BASE/lib/libogg.a ]; then
    build_required=1
  fi

  if [ "$build_required" -eq "1" ]; then
    build_counter=$((build_counter + 1))

    ./autogen.sh
    CFLAGS="$str_opt" ./configure --prefix="$PATH_BASE" --disable-shared CFLAGS="$str_opt" || exit 1
    make -j$cpu_count || exit 1
    make install || exit 1

    rm -f $PATH_BASE/lib/libvorbis.a
    rm -f $PATH_BASE/lib/libopus.a
  fi
}

build_vorbis() {
  build_required=1

  cd $PATH_BUILD

  if [ ! -e vorbis ]; then
    git clone --depth 1 https://github.com/xiph/vorbis.git || exit 1
    cd vorbis

  else
    cd vorbis
    git reset --hard
    git clean -dfx
    git pull --ff-only | grep "$git_up_to_date" && build_required=0
  fi

  if [ ! -e $PATH_BASE/lib/libvorbis.a ]; then
    build_required=1
  fi

  if [ "$build_required" -eq "1" ]; then
    build_counter=$((build_counter + 1))

    ./autogen.sh
    CFLAGS="$str_opt" LDFLAGS="-L$PATH_BASE/lib" CPPFLAGS="-I$PATH_BASE/include" ./configure --prefix="$PATH_BASE" --with-ogg="$PATH_BASE" --disable-shared CFLAGS="$str_opt" || exit 1
    make -j$cpu_count || exit 1
    make install || exit 1
  fi
}

build_opus() {
  build_required=1

  cd $PATH_BUILD

  if [ ! -e opus ]; then
    git clone --depth 1 https://github.com/xiph/opus.git || exit 1
    cd opus

  else
    cd opus
    git reset --hard
    git clean -dfx
    git pull --ff-only | grep "$git_up_to_date" && build_required=0
  fi

  if [ ! -e $PATH_BASE/lib/libopus.a ]; then
    build_required=1
  fi

  if [ "$build_required" -eq "1" ]; then
    build_counter=$((build_counter + 1))

    autoreconf -fiv
    CFLAGS="$str_opt" ./configure --prefix="$PATH_BASE" --disable-shared CFLAGS="$str_opt" || exit 1

    make -j$cpu_count || exit 1
    make install || exit 1
  fi
}

build_x264() {
  build_required=1

  cd $PATH_BUILD

  if [ ! -e x264 ]; then
    git clone --depth 1 https://code.videolan.org/videolan/x264.git || exit 1
    cd x264

  else
    cd x264
    git reset --hard
    git clean -dfx
    git pull --ff-only | grep "$git_up_to_date" && build_required=0
  fi

  if [ ! -e $PATH_BASE/lib/libx264.a ]; then
    build_required=1
  fi

  if [ "$build_required" -eq "1" ]; then
    build_counter=$((build_counter + 1))

    ./configure --prefix="$PATH_BASE" --bindir="$PATH_BASE/bin" --enable-pic --enable-static --extra-cflags="$str_opt" || exit 1

    make -j$cpu_count || exit 1
    make install || exit 1
  fi
}

build_nv_headers() {
  build_required=1

  cd $PATH_BUILD

  if [ ! -e nv-codec-headers ]; then
    git clone --depth 1 https://github.com/FFmpeg/nv-codec-headers.git || exit 1
    cd nv-codec-headers

  else
    cd nv-codec-headers
    git pull --ff-only | grep "$git_up_to_date" && build_required=0
  fi

  if [ ! -e $PATH_BASE/include/ffnvcodec/nvEncodeAPI.h ]; then
    build_required=1
  fi

  if [ "$build_required" -eq "1" ]; then
    build_counter=$((build_counter + 1))

    cp -rf include/ffnvcodec "$PATH_BASE/include"
    cp -f ffnvcodec.pc.in "$PATH_BASE/lib/pkgconfig/ffnvcodec.pc"
    sed -i -e "s/#define NVENCAPI_MAJOR_VERSION \([0-9]\{1,\}\)/#define NVENCAPI_MAJOR_VERSION 9/g" "$PATH_BASE/include/ffnvcodec/nvEncodeAPI.h"
    sed -i -e "s/#define NVENCAPI_MINOR_VERSION \([0-9]\{1,\}\)/#define NVENCAPI_MINOR_VERSION 1/g" "$PATH_BASE/include/ffnvcodec/nvEncodeAPI.h"
  fi
}

build_ff() {
  build_required=1

  cd $PATH_BUILD

  if [ ! -e ffmpeg ]; then
    git clone --depth 1 git://source.ffmpeg.org/ffmpeg || exit 1
    cd ffmpeg

  else
    cd ffmpeg
    git reset --hard
    git clean -dfx
    git pull --ff-only | grep "$git_up_to_date" && build_required=0
  fi

  if [ ! -e $PATH_BASE/lib/libavcodec.a ]; then
    build_required=1
  fi

  if [ "$build_required" -eq "1" ] || [ "$build_counter" -gt "0" ]; then
    ./configure --prefix="$PATH_BASE" --extra-libs="-lpthread -lstdc++" --extra-cflags="-I$PATH_BASE/include $str_opt" --extra-ldflags="-L$PATH_BASE/lib" --bindir="$PATH_BASE/bin" --pkg-config-flags="--static" \
      --enable-pic \
      --enable-gpl \
      --disable-nonfree \
      --enable-nvenc \
      --enable-cuvid \
      --enable-libopus \
      --enable-libvorbis \
      --enable-libx264 \
      --disable-libmfx \
      --disable-libfreetype \
      --disable-crystalhd \
      --enable-vaapi \
      --disable-vdpau \
      --disable-zlib \
      --disable-bzlib \
      --disable-lzma \
      --disable-libxcb \
      --disable-alsa \
      --disable-indev=alsa \
      --disable-outdev=alsa \
      --disable-dxva2 \
      --disable-ffplay \
      --disable-ffprobe \
      --disable-doc \
      --disable-htmlpages \
      --disable-manpages \
      --disable-podpages \
      --disable-txtpages \
      --disable-sdl2

    if [[ $? -ne 0 ]]; then
      exit 1
    fi

    make -j$cpu_count || exit 1
    make install || exit 1
  fi
}


build_nasm
build_ogg
build_vorbis
build_opus
build_x264
build_nv_headers
build_ff

exit 0
