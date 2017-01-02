#!/bin/bash

#sudo apt-get -y install autoconf automake libtool build-essential cmake curl git mercurial


str_opt="-march=native -O3"


PATH_ROOT=`pwd`

PATH_BASE=$PATH_ROOT/8bit


PATH_BUILD=$PATH_ROOT/tmp

PATH_ORIG=$PATH
PATH=$PATH_ROOT/8bit/lib:/$PATH_ROOT/8bit/include:/$PATH_ROOT/8bit/bin:$PATH_ORIG


cpu_count=`grep -c '^processor' /proc/cpuinfo`

export PKG_CONFIG_PATH="$PATH_BASE/lib/pkgconfig"


mkdir $PATH_BUILD



cd $PATH_BUILD
git clone --depth 1 git://github.com/yasm/yasm.git
cd yasm
autoreconf -fiv
CFLAGS="$str_opt" ./configure --prefix="$PATH_BASE" --bindir="$PATH_BASE/bin"
make -j$cpu_count
make install
#make distclean


cd $PATH_BUILD
git clone --depth 1 git://git.videolan.org/x264.git
cd x264
make distclean
./configure --prefix="$PATH_BASE" --bindir="$PATH_BASE/bin" --enable-static --extra-cflags="$str_opt"
make -j$cpu_count
make install
#make distclean


cd $PATH_BUILD
hg clone https://bitbucket.org/multicoreware/x265
cd $PATH_BUILD/x265/build/linux
export CFLAGS="$str_opt"
cmake -G "Unix Makefiles" -DCMAKE_INSTALL_PREFIX="$PATH_BASE" -DENABLE_SHARED:bool=off ../../source
make -j$cpu_count
make install


cd $PATH_BUILD
git clone --depth 1 git://git.code.sf.net/p/opencore-amr/fdk-aac
cd fdk-aac
autoreconf -fiv
CFLAGS="$str_opt" ./configure --prefix="$PATH_BASE" --disable-shared CFLAGS="$str_opt"
make -j$cpu_count
make install
#make distclean


cd $PATH_BUILD
curl -L -O http://downloads.sourceforge.net/project/lame/lame/3.99/lame-3.99.5.tar.gz
tar xzvf lame-3.99.5.tar.gz
cd lame-3.99.5
CFLAGS="$str_opt" ./configure --prefix="$PATH_BASE" --bindir="$PATH_BASE/bin" --disable-shared --enable-nasm CFLAGS="$str_opt"
make -j$cpu_count
make install
#make distclean


cd $PATH_BUILD
git clone git://git.opus-codec.org/opus.git
cd opus
autoreconf -fiv
CFLAGS="$str_opt" ./configure --prefix="$PATH_BASE" --disable-shared CFLAGS="$str_opt"
make -j$cpu_count
make install
#make distclean


cd $PATH_BUILD
curl -O http://downloads.xiph.org/releases/ogg/libogg-1.3.2.tar.gz
tar xzvf libogg-1.3.2.tar.gz
cd libogg-1.3.2
CFLAGS="$str_opt" ./configure --prefix="$PATH_BASE" --disable-shared CFLAGS="$str_opt"
make -j$cpu_count
make install
#make distclean


cd $PATH_BUILD
curl -O http://downloads.xiph.org/releases/vorbis/libvorbis-1.3.5.tar.gz
tar xzvf libvorbis-1.3.5.tar.gz
cd libvorbis-1.3.5
CFLAGS="$str_opt" LDFLAGS="-L$PATH_BASE/lib" CPPFLAGS="-I$PATH_BASE/include" ./configure --prefix="$PATH_BASE" --with-ogg="$PATH_BASE" --disable-shared CFLAGS="$str_opt"
make -j$cpu_count
make install
#make distclean


cd $PATH_BUILD
git clone --depth 1 https://chromium.googlesource.com/webm/libvpx.git
cd libvpx
./configure --prefix="$PATH_BASE" --disable-examples
make -j$cpu_count
make install
#make clean


cd $PATH_BUILD
git clone --depth 1 git://source.ffmpeg.org/ffmpeg
#git clone --depth 1 https://git.ffmpeg.org/ffmpeg.git
cd ffmpeg
make distclean
./configure --prefix="$PATH_BASE" --extra-cflags="-I$PATH_BASE/include $str_opt" --extra-ldflags="-L$PATH_BASE/lib" --bindir="$PATH_BASE/bin" --pkg-config-flags="--static" \
  --enable-gpl \
  --enable-nonfree \
  --enable-libfdk-aac \
  --disable-libfreetype \
  --enable-libmp3lame \
  --enable-libopus \
  --enable-libvorbis \
  --enable-libvpx \
  --enable-libx264 \
  --enable-libx265 \
  --disable-crystalhd
make -j$cpu_count
make install


#----------------------------

PATH=$PATH_ROOT/10bit/lib:/$PATH_ROOT/10bit/include:/$PATH_ROOT/10bit/bin:$PATH_ORIG
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
export CFLAGS="$str_opt"
cmake -G "Unix Makefiles" -DCMAKE_INSTALL_PREFIX="$PATH_BASE" -DENABLE_SHARED:bool=off ../../source
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
cd libvpx
./configure --prefix="$PATH_BASE" --disable-examples
make -j$cpu_count
make install
#make clean


cd $PATH_BUILD
cd ffmpeg
make distclean
./configure --prefix="$PATH_BASE" --extra-cflags="-I$PATH_BASE/include $str_opt" --extra-ldflags="-L$PATH_BASE/lib" --bindir="$PATH_BASE/bin" --pkg-config-flags="--static" \
  --enable-gpl \
  --enable-nonfree \
  --enable-libfdk-aac \
  --disable-libfreetype \
  --enable-libmp3lame \
  --enable-libopus \
  --enable-libvorbis \
  --enable-libvpx \
  --enable-libx264 \
  --enable-libx265 \
  --disable-crystalhd
make -j$cpu_count
make install


mv $PATH_ROOT/10bit/bin/ffmpeg $PATH_ROOT/10bit/bin/ffmpeg_10bit
