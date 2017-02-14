#!/bin/bash

#sudo apt-get -y install autoconf automake libtool build-essential cmake git mercurial


str_opt="-march=native -O3"


PATH_ROOT=`pwd`

PATH_BASE=$PATH_ROOT/8bit

PATH_BUILD=$PATH_ROOT/tmp

PATH_ORIG=$PATH
PATH=$PATH_ROOT/8bit/lib:$PATH_ROOT/8bit/include:$PATH_ROOT/8bit/bin:$PATH_ORIG

DECKLINK_INCLUDE=`dirname $PATH_ROOT`/blackmagic_decklink_sdk/Linux/include

cpu_count=`grep -c '^processor' /proc/cpuinfo`

export PKG_CONFIG_PATH="$PATH_BASE/lib/pkgconfig"



if [ ! -e $PATH_BUILD ]; then
  mkdir $PATH_BUILD
fi


cd $PATH_BUILD
if [ ! -e yasm ]; then
  git clone --depth 1 git://github.com/yasm/yasm.git
fi
cd yasm
autoreconf -fiv
CFLAGS="$str_opt" ./configure --prefix="$PATH_BASE" --bindir="$PATH_BASE/bin"
make -j$cpu_count
make install
#make distclean


cd $PATH_BUILD
if [ ! -e x264 ]; then
  git clone --depth 1 git://git.videolan.org/x264.git
fi
cd x264
make distclean
./configure --prefix="$PATH_BASE" --bindir="$PATH_BASE/bin" --enable-static --extra-cflags="$str_opt"
make -j$cpu_count
make install
#make distclean


cd $PATH_BUILD
if [ ! -e x265 ]; then
  hg clone https://bitbucket.org/multicoreware/x265
fi
cd $PATH_BUILD/x265/build/linux
make clean
export CFLAGS="$str_opt"
cmake -G "Unix Makefiles" -DCMAKE_INSTALL_PREFIX="$PATH_BASE" -DENABLE_SHARED:bool=off ../../source
make -j$cpu_count
make install


cd $PATH_BUILD
if [ ! -e fdk-aac ]; then
  git clone --depth 1 git://git.code.sf.net/p/opencore-amr/fdk-aac
fi
cd fdk-aac
autoreconf -fiv
CFLAGS="$str_opt" ./configure --prefix="$PATH_BASE" --disable-shared CFLAGS="$str_opt"
make -j$cpu_count
make install
#make distclean


cd $PATH_BUILD
if [ ! -e lame-3.99.5.tar.gz ]; then
  wget http://downloads.sourceforge.net/project/lame/lame/3.99/lame-3.99.5.tar.gz
  tar xzvf lame-3.99.5.tar.gz
fi
cd lame-3.99.5
CFLAGS="$str_opt" ./configure --prefix="$PATH_BASE" --bindir="$PATH_BASE/bin" --disable-shared --enable-nasm CFLAGS="$str_opt"
make -j$cpu_count
make install
#make distclean


cd $PATH_BUILD
if [ ! -e opus ]; then
  git clone git://git.opus-codec.org/opus.git
fi
cd opus
autoreconf -fiv
CFLAGS="$str_opt" ./configure --prefix="$PATH_BASE" --disable-shared CFLAGS="$str_opt"
make -j$cpu_count
make install
#make distclean


cd $PATH_BUILD
if [ ! -e libogg-1.3.2.tar.gz ]; then
  wget http://downloads.xiph.org/releases/ogg/libogg-1.3.2.tar.gz
  tar xzvf libogg-1.3.2.tar.gz
fi
cd libogg-1.3.2
CFLAGS="$str_opt" ./configure --prefix="$PATH_BASE" --disable-shared CFLAGS="$str_opt"
make -j$cpu_count
make install
#make distclean


cd $PATH_BUILD
if [ ! -e libvorbis-1.3.5.tar.gz ]; then
  wget http://downloads.xiph.org/releases/vorbis/libvorbis-1.3.5.tar.gz
  tar xzvf libvorbis-1.3.5.tar.gz
fi
cd libvorbis-1.3.5
CFLAGS="$str_opt" LDFLAGS="-L$PATH_BASE/lib" CPPFLAGS="-I$PATH_BASE/include" ./configure --prefix="$PATH_BASE" --with-ogg="$PATH_BASE" --disable-shared CFLAGS="$str_opt"
make -j$cpu_count
make install
#make distclean


cd $PATH_BUILD
if [ ! -e speex-1.2.0 ]; then
    wget http://downloads.us.xiph.org/releases/speex/speex-1.2.0.tar.gz
    tar xzvf speex-1.2.0.tar.gz
fi
cd speex-1.2.0
CFLAGS="$str_opt" ./configure --prefix="$PATH_BASE" --disable-shared CFLAGS="$str_opt"
make -j$cpu_count
make install
#make distclean


cd $PATH_BUILD
if [ ! -e libvpx ]; then
  git clone --depth 1 https://chromium.googlesource.com/webm/libvpx.git
fi
cd libvpx
./configure --prefix="$PATH_BASE" --disable-examples
make -j$cpu_count
make install
#make clean


cd $PATH_BUILD
if [ ! -e ffmpeg ]; then
  git clone --depth 1 git://source.ffmpeg.org/ffmpeg
#  git clone --depth 1 https://git.ffmpeg.org/ffmpeg.git
fi
cd ffmpeg
make distclean
./configure --prefix="$PATH_BASE" --extra-cflags="-I$PATH_BASE/include -I$DECKLINK_INCLUDE $str_opt" --extra-ldflags="-L$PATH_BASE/lib" --bindir="$PATH_BASE/bin" --pkg-config-flags="--static" \
  --enable-gpl \
  --enable-nonfree \
  --enable-libfdk-aac \
  --disable-libfreetype \
  --enable-libmp3lame \
  --enable-libopus \
  --enable-libvorbis \
  --enable-libspeex \
  --enable-libvpx \
  --enable-libx264 \
  --enable-libx265 \
  --enable-decklink \
  --disable-crystalhd
make -j$cpu_count
make install


#----------------------------

PATH=$PATH_ROOT/10bit/lib:$PATH_ROOT/10bit/include:$PATH_ROOT/10bit/bin:$PATH_ORIG
PATH_BASE=$PATH_ROOT/10bit

export PKG_CONFIG_PATH="$PATH_BASE/lib/pkgconfig"


cd $PATH_BUILD
cd yasm
autoreconf -fiv
CFLAGS="$str_opt" ./configure --prefix="$PATH_BASE" --bindir="$PATH_BASE/bin"
make -j$cpu_count
make install
#make distclean


cd $PATH_BUILD
cd x264
make distclean
./configure --prefix="$PATH_BASE" --bindir="$PATH_BASE/bin" --enable-static --bit-depth=10 --extra-cflags="$str_opt"
make -j$cpu_count
make install


cd $PATH_BUILD
cd $PATH_BUILD/x265/build/linux
make clean
export CFLAGS="$str_opt"
cmake -G "Unix Makefiles" -DHIGH_BIT_DEPTH=ON -DMAIN12=OFF -DCMAKE_INSTALL_PREFIX="$PATH_BASE" -DENABLE_SHARED:bool=off ../../source
make -j$cpu_count
make install


cd $PATH_BUILD
cd fdk-aac
autoreconf -fiv
CFLAGS="$str_opt" ./configure --prefix="$PATH_BASE" --disable-shared CFLAGS="$str_opt"
make -j$cpu_count
make install
#make distclean


cd $PATH_BUILD
cd lame-3.99.5
CFLAGS="$str_opt" ./configure --prefix="$PATH_BASE" --bindir="$PATH_BASE/bin" --disable-shared --enable-nasm CFLAGS="$str_opt"
make -j$cpu_count
make install
#make distclean


cd $PATH_BUILD
cd opus
autoreconf -fiv
CFLAGS="$str_opt" ./configure --prefix="$PATH_BASE" --disable-shared CFLAGS="$str_opt"
make -j$cpu_count
make install
#make distclean


cd $PATH_BUILD
cd libogg-1.3.2
CFLAGS="$str_opt" ./configure --prefix="$PATH_BASE" --disable-shared CFLAGS="$str_opt"
make -j$cpu_count
make install
#make distclean


cd $PATH_BUILD
cd libvorbis-1.3.5
CFLAGS="$str_opt" LDFLAGS="-L$PATH_BASE/lib" CPPFLAGS="-I$PATH_BASE/include" ./configure --prefix="$PATH_BASE" --with-ogg="$PATH_BASE" --disable-shared CFLAGS="$str_opt"
make -j$cpu_count
make install
#make distclean


cd $PATH_BUILD
cd speex-1.2.0
CFLAGS="$str_opt" ./configure --prefix="$PATH_BASE" --disable-shared CFLAGS="$str_opt"
make -j$cpu_count
make install
#make distclean


cd $PATH_BUILD
cd libvpx
./configure --prefix="$PATH_BASE" --disable-examples
make -j$cpu_count
make install
#make clean


cd $PATH_BUILD
cd ffmpeg
make distclean
./configure --prefix="$PATH_BASE" --extra-cflags="-I$PATH_BASE/include -I$DECKLINK_INCLUDE $str_opt" --extra-ldflags="-L$PATH_BASE/lib" --bindir="$PATH_BASE/bin" --pkg-config-flags="--static" \
  --enable-gpl \
  --enable-nonfree \
  --enable-libfdk-aac \
  --disable-libfreetype \
  --enable-libmp3lame \
  --enable-libopus \
  --enable-libvorbis \
  --enable-libspeex \
  --enable-libvpx \
  --enable-libx264 \
  --enable-libx265 \
  --enable-decklink \
  --disable-crystalhd
make -j$cpu_count
make install


mv $PATH_ROOT/10bit/bin/ffmpeg $PATH_ROOT/10bit/bin/ffmpeg_10bit
