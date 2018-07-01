#!/bin/bash

#sudo apt-get -y install autoconf automake libtool libtool-bin build-essential cmake git subversion mercurial pkg-config
#numactl libnuma-dev


missing_deps=""

test_cmd() {
  if ! [ -x "$(command -v $1)" ]; then
    missing_deps="$missing_deps $2"
  fi
}

test_cmd autoconf autoconf
test_cmd automake automake
test_cmd libtoolize libtool
# test_cmd libtool libtool-bin
test_cmd make build-essential
test_cmd cmake cmake
test_cmd git git
test_cmd svn subversion
test_cmd hg mercurial
test_cmd pkg-config pkg-config

if [[ ! -z "$missing_deps" ]]; then
  echo "following packages are required:$missing_deps"
  exit 1
fi


str_opt="-march=native -O3"


PATH_ROOT=`pwd`

PATH_BASE=$PATH_ROOT

PATH_BUILD=$PATH_ROOT/tmp

PATH_ORIG=$PATH
PATH=$PATH_ROOT/lib:$PATH_ROOT/include:$PATH_ROOT/bin:$PATH_ORIG

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
    git checkout nasm-2.13.03

  else
    cd nasm
    git reset --hard
    git clean -dfx
    # git pull
  fi

  autoreconf -fiv
  CFLAGS="$str_opt" ./configure --prefix="$PATH_BASE" --bindir="$PATH_BASE/bin"
  make everything -j$cpu_count
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

  ./configure --prefix="$PATH_BASE" --bindir="$PATH_BASE/bin" --enable-pic --enable-static --extra-cflags="$str_opt"

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
    hg pull -u
    cd build/linux
    rm -rf *
  fi

  export CFLAGS="$str_opt"

  mkdir -p 8bit 10bit

  cd 10bit
  cmake -G "Unix Makefiles" -DHIGH_BIT_DEPTH=ON -DEXPORT_C_API=OFF -DENABLE_CLI=OFF -DENABLE_SHARED=OFF -DNATIVE_BUILD=ON ../../../source
  make -j$cpu_count
  rm -f *.o
  ar -x libx265.a

  cd ../8bit
  ln -sf ../10bit/libx265.a libx265_main10.a
  cmake -G "Unix Makefiles" -DEXTRA_LIB="x265_main10.a" -DEXTRA_LINK_FLAGS=-L. -DLINKED_10BIT=ON -DENABLE_SHARED=OFF -DNATIVE_BUILD=ON -DCMAKE_INSTALL_PREFIX="$PATH_BASE" ../../../source
  make -j$cpu_count
  mv libx265.a libx265_main8.a
  rm -f *.o
  ar -x libx265_main8.a

  ar -r libx265.a *.o ../10bit/*.o

  # libtool --tag=CC --mode=link cc -static -o libx265.a libx265_main.a libx265_main10.a

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

  ./configure --prefix="$PATH_BASE" --enable-pic --enable-vp9-highbitdepth --disable-examples

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

build_mfx_dispatch() {
  cd $PATH_BUILD

  if [ ! -e mfx_dispatch ]; then
    git clone https://github.com/lu-zero/mfx_dispatch.git
    cd mfx_dispatch

  else
    cd mfx_dispatch
    git reset --hard
    git clean -dfx
    git pull
  fi

  autoreconf -fiv
  automake --add-missing
  ./configure --prefix="$PATH_BASE" --disable-shared --enable-static --with-libva_x11 --with-libva_drm
  make -j$cpu_count
  make install

  pkg-config --exists --print-errors libmfx
}

build_nv_headers() {
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
}


build_ff() {
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
  ./configure --prefix="$PATH_BASE" --extra-libs="-lpthread -lstdc++" --extra-cflags="-I$PATH_BASE/include $DECKLINK_INCLUDE $str_opt" --extra-ldflags="-L$PATH_BASE/lib" --bindir="$PATH_BASE/bin" --pkg-config-flags="--static" \
    --enable-pic \
    --enable-gpl \
    --enable-nonfree \
    --enable-nvenc \
    `#--enable-libmfx` \
    --enable-libx264 \
    --enable-libx265 \
    `#--enable-libvpx` \
    `#--enable-libopus` \
    `#--enable-libvorbis` \
    `#--enable-libspeex` \
    `#--enable-libmp3lame` \
    `#--enable-libfdk-aac` \
    $DECKLINK_FF_OPT \
    --disable-libfreetype \
    --disable-crystalhd \
    --disable-vaapi \
    --disable-vdpau \
    --disable-zlib \
    --disable-bzlib \
    --disable-lzma \
    --disable-libxcb

  make -j$cpu_count
  make install
}


build_yasm
build_nasm
build_x264
build_x265
#build_vpx
#build_opus
#build_ogg
#build_vorbis
#build_speex
#build_lame
#build_aac
build_nv_headers
#build_mfx_dispatch
build_ff
