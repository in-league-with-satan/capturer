#!/bin/bash

#sudo apt-get -y install autoconf automake libtool build-essential cmake git mercurial
#cpan Font::TTF::Font
#cpan Sort::Versions

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
  git clone git://github.com/yasm/yasm.git
  cd yasm
else
  cd yasm
  git reset --hard
  git pull
fi
autoreconf -fiv
CFLAGS="$str_opt" ./configure --prefix="$PATH_BASE" --bindir="$PATH_BASE/bin"
make -j$cpu_count
make install
#make distclean


cd $PATH_BUILD
if [ ! -e nasm ]; then
  git clone git://repo.or.cz/nasm.git
  cd nasm
else
  cd nasm
  git reset --hard
  git pull
fi
autoreconf -fiv
CFLAGS="$str_opt" ./configure --prefix="$PATH_BASE" --bindir="$PATH_BASE/bin"
make everything -j$cpu_count
make install


cd $PATH_BUILD
if [ ! -e numactl ]; then
  git clone https://github.com/numactl/numactl
  cd numactl
else
  cd numactl
  git reset --hard
  git pull
fi
autoreconf -fiv
CFLAGS="$str_opt" ./configure --prefix="$PATH_BASE" --bindir="$PATH_BASE/bin"
make -j$cpu_count
make install


cd $PATH_BUILD
if [ ! -e x264 ]; then
  git clone git://git.videolan.org/x264.git
  cd x264
else
  cd x264
  git reset --hard
  git pull
fi
make distclean
./configure --prefix="$PATH_BASE" --bindir="$PATH_BASE/bin" --enable-static --extra-cflags="$str_opt"
make -j$cpu_count
make install
#make distclean


cd $PATH_BUILD
if [ ! -e x265 ]; then
  hg clone https://bitbucket.org/multicoreware/x265
  cd x265/build/linux
else
  cd x265
  hg update --clean
  hg pull
  cd build/linux
fi
make clean
export CFLAGS="$str_opt"
cmake -G "Unix Makefiles" -DCMAKE_INSTALL_PREFIX="$PATH_BASE" -DENABLE_SHARED:bool=off ../../source
sed -i 's/static char\* strtok_r/char* strtok_r/g' ../../source/common/param.cpp
make -j$cpu_count
make install


cd $PATH_BUILD
if [ ! -e fdk-aac ]; then
  git clone git://git.code.sf.net/p/opencore-amr/fdk-aac
  cd fdk-aac
else
  cd fdk-aac
  git reset --hard
  git pull
fi
autoreconf -fiv
CFLAGS="$str_opt" ./configure --prefix="$PATH_BASE" --disable-shared CFLAGS="$str_opt"
make -j$cpu_count
make install
#make distclean


cd $PATH_BUILD
if [ ! -e lame-3.99.5.tar.gz ]; then
  wget http://downloads.sourceforge.net/project/lame/lame/3.99/lame-3.99.5.tar.gz
fi
if [ ! -e lame-3.99.5 ]; then
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
  cd opus
else
  cd opus
  git reset --hard
  git pull
fi
autoreconf -fiv
CFLAGS="$str_opt" ./configure --prefix="$PATH_BASE" --disable-shared CFLAGS="$str_opt"
make -j$cpu_count
make install
#make distclean


cd $PATH_BUILD
if [ ! -e libogg-1.3.2.tar.gz ]; then
  wget http://downloads.xiph.org/releases/ogg/libogg-1.3.2.tar.gz
fi
if [ ! -e libogg-1.3.2 ]; then
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
fi
if [ ! -e libvorbis-1.3.5 ]; then
  tar xzvf libvorbis-1.3.5.tar.gz
fi
cd libvorbis-1.3.5
CFLAGS="$str_opt" LDFLAGS="-L$PATH_BASE/lib" CPPFLAGS="-I$PATH_BASE/include" ./configure --prefix="$PATH_BASE" --with-ogg="$PATH_BASE" --disable-shared CFLAGS="$str_opt"
make -j$cpu_count
make install
#make distclean


cd $PATH_BUILD
if [ ! -e speex-1.2.0.tar.gz ]; then
    wget http://downloads.xiph.org/releases/speex/speex-1.2.0.tar.gz
fi
if [ ! -e speex-1.2.0 ]; then
    tar xzvf speex-1.2.0.tar.gz
fi
cd speex-1.2.0
CFLAGS="$str_opt" ./configure --prefix="$PATH_BASE" --disable-shared CFLAGS="$str_opt"
make -j$cpu_count
make install
#make distclean


cd $PATH_BUILD
if [ ! -e libvpx ]; then
  git clone https://chromium.googlesource.com/webm/libvpx.git
  cd libvpx
else
  cd libvpx
  git reset --hard
  git pull
fi
./configure --prefix="$PATH_BASE" --disable-examples
make -j$cpu_count
make install
#make clean


cd $PATH_BUILD
if [ ! -e ffmpeg ]; then
  git clone git://source.ffmpeg.org/ffmpeg
  cd ffmpeg
else
  cd ffmpeg
  git reset --hard
  git pull
fi

git checkout release/3.4

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
  --disable-crystalhd
make -j$cpu_count
make install

exit 0
#----------------------------

PATH=$PATH_ROOT/10bit/lib:$PATH_ROOT/10bit/include:$PATH_ROOT/10bit/bin:$PATH_ORIG
PATH_BASE=$PATH_ROOT/10bit

export PKG_CONFIG_PATH="$PATH_BASE/lib/pkgconfig"


cd $PATH_BUILD
cd yasm
CFLAGS="$str_opt" ./configure --prefix="$PATH_BASE" --bindir="$PATH_BASE/bin"
make install


cd $PATH_BUILD
cd nasm
CFLAGS="$str_opt" ./configure --prefix="$PATH_BASE" --bindir="$PATH_BASE/bin"
make install


cd $PATH_BUILD
cd numactl
autoreconf -fiv
CFLAGS="$str_opt" ./configure --prefix="$PATH_BASE" --bindir="$PATH_BASE/bin"
make install


cd $PATH_BUILD
cd x264
make distclean
./configure --prefix="$PATH_BASE" --bindir="$PATH_BASE/bin" --enable-static --bit-depth=10 --extra-cflags="$str_opt"
make -j$cpu_count
make install


cd $PATH_BUILD/x265/build/linux
#make clean
#export CFLAGS="$str_opt"
cmake -G "Unix Makefiles" -DHIGH_BIT_DEPTH=ON -DMAIN12=OFF -DCMAKE_INSTALL_PREFIX="$PATH_BASE" -DENABLE_SHARED:bool=off ../../source
#make -j$cpu_count
make install


cd $PATH_BUILD
cd fdk-aac
autoreconf -fiv
CFLAGS="$str_opt" ./configure --prefix="$PATH_BASE" --disable-shared CFLAGS="$str_opt"
make install


cd $PATH_BUILD
cd lame-3.99.5
CFLAGS="$str_opt" ./configure --prefix="$PATH_BASE" --bindir="$PATH_BASE/bin" --disable-shared --enable-nasm CFLAGS="$str_opt"
make install


cd $PATH_BUILD
cd opus
autoreconf -fiv
CFLAGS="$str_opt" ./configure --prefix="$PATH_BASE" --disable-shared CFLAGS="$str_opt"
make install


cd $PATH_BUILD
cd libogg-1.3.2
CFLAGS="$str_opt" ./configure --prefix="$PATH_BASE" --disable-shared CFLAGS="$str_opt"
make install


cd $PATH_BUILD
cd libvorbis-1.3.5
CFLAGS="$str_opt" LDFLAGS="-L$PATH_BASE/lib" CPPFLAGS="-I$PATH_BASE/include" ./configure --prefix="$PATH_BASE" --with-ogg="$PATH_BASE" --disable-shared CFLAGS="$str_opt"
make install


cd $PATH_BUILD
cd speex-1.2.0
CFLAGS="$str_opt" ./configure --prefix="$PATH_BASE" --disable-shared CFLAGS="$str_opt"
make install


cd $PATH_BUILD
cd libvpx
./configure --prefix="$PATH_BASE" --disable-examples
make install


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
