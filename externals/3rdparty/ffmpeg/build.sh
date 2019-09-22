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

git_up_to_date="Already up to date."

build_counter=0

PATH_ROOT=`pwd`

PATH_BASE=$PATH_ROOT

PATH_BUILD=$PATH_ROOT/tmp

PATH_ORIG=$PATH
PATH=$PATH_ROOT/lib:$PATH_ROOT/include:$PATH_ROOT/bin:$PATH_ORIG


cpu_count=`nproc`

export PKG_CONFIG_PATH="$PATH_BASE/lib/pkgconfig"

HIGH_BIT_DEPTH=false


if [ ! -e $PATH_BUILD ]; then
  mkdir $PATH_BUILD
fi


build_nasm() {
  cd $PATH_BUILD

  if [ ! -e nasm ]; then
    build_counter=$((build_counter + 1))

    git clone git://repo.or.cz/nasm.git --branch nasm-2.14.02 --single-branch --depth 1
    cd nasm

    autoreconf -fiv
    CFLAGS="$str_opt" ./configure --prefix="$PATH_BASE" --bindir="$PATH_BASE/bin"
    make everything -j$cpu_count
    make install
  fi
}

build_x264() {
  build_required=1

  cd $PATH_BUILD

  if [ ! -e x264 ]; then
    git clone --depth 1 git://git.videolan.org/x264.git
    cd x264

  else
    cd x264
    git reset --hard
    git clean -dfx
    git pull | grep "$git_up_to_date" && build_required=0
  fi

  if [ "$build_required" -eq "1" ]; then
    build_counter=$((build_counter + 1))

    ./configure --prefix="$PATH_BASE" --bindir="$PATH_BASE/bin" --enable-pic --enable-static --extra-cflags="$str_opt"

    make -j$cpu_count
    make install
  fi
}

build_mfx_dispatch() {
  build_required=1

  cd $PATH_BUILD

  if [ ! -e mfx_dispatch ]; then
    git clone --depth 1 https://github.com/lu-zero/mfx_dispatch.git
    cd mfx_dispatch

  else
    cd mfx_dispatch
    git reset --hard
    git clean -dfx
    git pull | grep "$git_up_to_date" && build_required=0
  fi

  if [ "$build_required" -eq "1" ]; then
    build_counter=$((build_counter + 1))

    autoreconf -fiv
    automake --add-missing
    ./configure --prefix="$PATH_BASE" --disable-shared --enable-static --with-libva_x11 --with-libva_drm
    make -j$cpu_count
    make install

    pkg-config --exists --print-errors libmfx
  fi
}

build_nv_headers() {
  build_required=1

  cd $PATH_BUILD

  if [ ! -e nv-codec-headers ]; then
    git clone --depth 1 https://github.com/FFmpeg/nv-codec-headers.git
    cd nv-codec-headers

  else
    cd nv-codec-headers
    git pull | grep "$git_up_to_date" && build_required=0
  fi

  if [ "$build_required" -eq "1" ]; then
    build_counter=$((build_counter + 1))

    cp -rf include/ffnvcodec "$PATH_BASE/include"
    cp -f ffnvcodec.pc.in "$PATH_BASE/lib/pkgconfig/ffnvcodec.pc"
  fi
}


build_ff() {
  build_required=1

  cd $PATH_BUILD

  if [ ! -e ffmpeg ]; then
    git clone --depth 1 git://source.ffmpeg.org/ffmpeg
    cd ffmpeg

  else
    cd ffmpeg
    git reset --hard
    git clean -dfx
    git pull | grep "$git_up_to_date" && build_required=0
  fi

  if [ "$build_required" -eq "1" ] || [ "$build_counter" -gt "0" ]; then
    make distclean
    ./configure --prefix="$PATH_BASE" --extra-libs="-lpthread -lstdc++" --extra-cflags="-I$PATH_BASE/include $str_opt" --extra-ldflags="-L$PATH_BASE/lib" --bindir="$PATH_BASE/bin" --pkg-config-flags="--static" \
      --enable-pic \
      --enable-gpl \
      --disable-nonfree \
      --enable-nvenc \
      --enable-cuvid \
      --disable-libmfx \
      --enable-libx264 \
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
      --disable-txtpages

    make -j$cpu_count
    make install
  fi
}


build_nasm
build_x264
build_nv_headers
#build_mfx_dispatch
build_ff
