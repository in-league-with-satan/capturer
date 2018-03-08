#!/bin/bash

#sudo apt-get -y install autoconf automake libtool build-essential cmake git subversion mercurial pkg-config
#numactl libnuma-dev

str_opt="-march=native -O3"


PATH_ROOT=`pwd`

PATH_BASE=$PATH_ROOT/8bit

PATH_BUILD=$PATH_ROOT/tmp

PATH_ORIG=$PATH
PATH=$PATH_ROOT/8bit/lib:$PATH_ROOT/8bit/include:$PATH_ROOT/8bit/bin:$PATH_ORIG

DECKLINK_INCLUDE=""
DECKLINK_FF_OPT=""

cpu_count=`grep -c '^processor' /proc/cpuinfo`

export PKG_CONFIG_PATH="$PATH_BASE/lib/pkgconfig"

HIGH_BIT_DEPTH=false


for i in "$@"
do
  if [ "${i}" == "--decklink" ]; then
    DECKLINK_INCLUDE=-I`dirname $PATH_ROOT`/blackmagic_decklink_sdk/Linux/include
    DECKLINK_FF_OPT=--enable-decklink
  fi

  if [ "${i}" == "--high_bit_depth" ]; then
    PATH=$PATH_ROOT/10bit/lib:$PATH_ROOT/10bit/include:$PATH_ROOT/10bit/bin:$PATH_ORIG
    PATH_BASE=$PATH_ROOT/10bit

    export PKG_CONFIG_PATH="$PATH_BASE/lib/pkgconfig"

    HIGH_BIT_DEPTH=true
  fi
done


if [ ! -e $PATH_BUILD ]; then
  mkdir $PATH_BUILD
fi


build_yasm() {
  cd $PATH_BUILD

  if [ ! -e yasm ]; then
    git clone git://github.com/yasm/yasm.git
    cd yasm

  else
    cd yasm
    git reset --hard
    git clean -dfx
    git pull
  fi

  autoreconf -fiv
  CFLAGS="$str_opt" ./configure --prefix="$PATH_BASE" --bindir="$PATH_BASE/bin"
  make -j$cpu_count
  make install
}

build_nasm() {
  cd $PATH_BUILD

  if [ ! -e nasm ]; then
    git clone git://repo.or.cz/nasm.git
    cd nasm

  else
    cd nasm
    git reset --hard
    git clean -dfx
    git pull
  fi

  autoreconf -fiv
  CFLAGS="$str_opt" ./configure --prefix="$PATH_BASE" --bindir="$PATH_BASE/bin"
  make everything -j$cpu_count
  make install
}

build_numa() {
  cd $PATH_BUILD

  if [ ! -e numactl ]; then
    git clone https://github.com/numactl/numactl
    cd numactl

  else
    cd numactl
    git reset --hard
    git clean -dfx
    git pull
  fi

  autoreconf -fiv
  CFLAGS="$str_opt" ./configure --prefix="$PATH_BASE" --bindir="$PATH_BASE/bin"
  make -j$cpu_count
  make install
}

build_x264() {
  cd $PATH_BUILD

  if [ ! -e x264 ]; then
    git clone git://git.videolan.org/x264.git
    cd x264

  else
    cd x264
    git reset --hard
    git clean -dfx
    git pull
  fi

  # git checkout stable

  if $HIGH_BIT_DEPTH; then
    ./configure --prefix="$PATH_BASE" --bindir="$PATH_BASE/bin" --enable-pic --enable-static --bit-depth=10 --extra-cflags="$str_opt"

  else
    ./configure --prefix="$PATH_BASE" --bindir="$PATH_BASE/bin" --enable-pic --enable-static                --extra-cflags="$str_opt"
  fi

  make -j$cpu_count
  make install
}

build_x265() {
  cd $PATH_BUILD

  if [ ! -e x265 ]; then
    hg clone https://bitbucket.org/multicoreware/x265
    cd x265/build/linux

  else
    cd x265
    hg update --clean
    # hg checkout stable
    hg pull
    cd build/linux
    rm -rf *
  fi

  export CFLAGS="$str_opt"

  if $HIGH_BIT_DEPTH; then
    cmake -G "Unix Makefiles" -DHIGH_BIT_DEPTH=ON -DMAIN12=OFF -DCMAKE_INSTALL_PREFIX="$PATH_BASE" -DENABLE_SHARED:bool=off ../../source

  else
    cmake -G "Unix Makefiles"                                  -DCMAKE_INSTALL_PREFIX="$PATH_BASE" -DENABLE_SHARED:bool=off ../../source
  fi

  sed -i 's/static char\* strtok_r/char* strtok_r/g' ../../source/common/param.cpp
  make -j$cpu_count
  make install
}

build_vpx() {
  cd $PATH_BUILD

  if [ ! -e libvpx ]; then
    git clone https://chromium.googlesource.com/webm/libvpx.git
    cd libvpx

  else
    cd libvpx
    git reset --hard
    git clean -dfx
    git pull
  fi

  if $HIGH_BIT_DEPTH; then
    ./configure --prefix="$PATH_BASE" --enable-pic --enable-vp9-highbitdepth --disable-examples

  else
    ./configure --prefix="$PATH_BASE" --enable-pic                           --disable-examples
  fi


  make -j$cpu_count
  make install
}

build_opus() {
  cd $PATH_BUILD

  if [ ! -e opus ]; then
    git clone https://github.com/xiph/opus.git
    cd opus

  else
    cd opus
    git reset --hard
    git clean -dfx
    git pull
  fi

  autoreconf -fiv
  CFLAGS="$str_opt" ./configure --prefix="$PATH_BASE" --disable-shared CFLAGS="$str_opt"
  make -j$cpu_count
  make install
}

build_ogg() {
  cd $PATH_BUILD

  if [ ! -e ogg ]; then
    git clone https://github.com/xiph/ogg.git
    cd ogg

  else
    cd ogg
    git reset --hard
    git clean -dfx
    git pull
  fi

  ./autogen.sh
  CFLAGS="$str_opt" ./configure --prefix="$PATH_BASE" --disable-shared CFLAGS="$str_opt"
  make -j$cpu_count
  make install
}

build_vorbis() {
  cd $PATH_BUILD

  if [ ! -e vorbis ]; then
    git clone https://github.com/xiph/vorbis.git
    cd vorbis

  else
    cd vorbis
    git reset --hard
    git clean -dfx
    git pull
  fi

  ./autogen.sh
  CFLAGS="$str_opt" LDFLAGS="-L$PATH_BASE/lib" CPPFLAGS="-I$PATH_BASE/include" ./configure --prefix="$PATH_BASE" --with-ogg="$PATH_BASE" --disable-shared CFLAGS="$str_opt"
  make -j$cpu_count
  make install
}

build_speex() {
  cd $PATH_BUILD

  if [ ! -e speex ]; then
    git clone https://github.com/xiph/speex.git
    cd speex

  else
    cd speex
    git reset --hard
    git clean -dfx
    git pull
  fi

  ./autogen.sh
  CFLAGS="$str_opt" ./configure --prefix="$PATH_BASE" --disable-shared CFLAGS="$str_opt"
  make -j$cpu_count
  make install
}

build_lame() {
  cd $PATH_BUILD

  if [ ! -e lame ]; then
    svn checkout https://svn.code.sf.net/p/lame/svn/trunk/lame lame
    cd lame

  else
    cd lame
    svn update
  fi

  CFLAGS="$str_opt" ./configure --prefix="$PATH_BASE" --bindir="$PATH_BASE/bin" --disable-shared --enable-nasm CFLAGS="$str_opt"
  make -j$cpu_count
  make install
}

build_aac() {
  cd $PATH_BUILD

  if [ ! -e fdk-aac ]; then
    git clone git://git.code.sf.net/p/opencore-amr/fdk-aac
    cd fdk-aac

  else
    cd fdk-aac
    git reset --hard
    git clean -dfx
    git pull
  fi

  autoreconf -fiv
  CFLAGS="$str_opt" ./configure --prefix="$PATH_BASE" --disable-shared CFLAGS="$str_opt"
  make -j$cpu_count
  make install
}

build_ff() {
  cd $PATH_BUILD

  if [ ! -e nv-codec-headers ]; then
    git clone https://github.com/FFmpeg/nv-codec-headers.git
    cd nv-codec-headers

  else
    cd nv-codec-headers
    git pull
  fi

  cp -rf include/ffnvcodec "$PATH_BASE/include"
  cp -f ffnvcodec.pc.in "$PATH_BASE/lib/pkgconfig/ffnvcodec.pc"

  #

  cd $PATH_BUILD

  if [ ! -e ffmpeg ]; then
    git clone git://source.ffmpeg.org/ffmpeg
    cd ffmpeg

  else
    cd ffmpeg
    git reset --hard
    git clean -dfx
    git pull
  fi

  make distclean
  ./configure --prefix="$PATH_BASE" --extra-libs=-lpthread --extra-cflags="-I$PATH_BASE/include $DECKLINK_INCLUDE $str_opt" --extra-ldflags="-L$PATH_BASE/lib" --bindir="$PATH_BASE/bin" --pkg-config-flags="--static" \
    --enable-pic \
    --enable-gpl \
    --enable-nonfree \
    --enable-nvenc \
    --enable-libx264 \
    --enable-libx265 \
    --enable-libvpx \
    --enable-libopus \
    --enable-libvorbis \
    --enable-libspeex \
    --enable-libmp3lame \
    --enable-libfdk-aac \
    $DECKLINK_FF_OPT \
    --disable-libfreetype \
    --disable-crystalhd \
    --disable-vaapi \
    --disable-vdpau
  make -j$cpu_count
  make install
}


build_yasm
build_nasm
#build_numa
build_x264
build_x265
build_vpx
build_opus
build_ogg
build_vorbis
build_speex
build_lame
build_aac
build_ff
