#!/bin/bash


dest_dir=bin_win


ffmpeg_version=`curl -vs "https://ffmpeg.zeranoe.com/builds/win64/shared/?C=M&O=D" 2>&1 | grep -m 1 shared.zip | grep -Po '(?<=(ffmpeg-)).*(?=-win64-shared.zip)' | cut -c -16`


app_last_tag=$(git describe --tags `git rev-list --tags --max-count=1`)
app_version=$app_last_tag.`git rev-list $app_last_tag.. --count`-`git log -1 --pretty=format:%h`


qt_version=`qmake --version | awk '{if ($1=="Using") print $4;}'`


cd /temp


mv externals/3rdparty/ffmpeg externals/3rdparty/ffmpeg_orig
mkdir externals/3rdparty/ffmpeg
cp externals/3rdparty/ffmpeg_orig/ffmpeg.pri externals/3rdparty/ffmpeg/ffmpeg.pri

curl -s "https://ffmpeg.zeranoe.com/builds/win64/shared/ffmpeg-$ffmpeg_version-win64-shared.zip" --output ffmpeg_bin.zip
curl -s "https://ffmpeg.zeranoe.com/builds/win64/dev/ffmpeg-$ffmpeg_version-win64-dev.zip" --output ffmpeg_dev.zip


7z x ffmpeg_bin.zip
7z x ffmpeg_dev.zip


mv -f ffmpeg-$ffmpeg_version-win64-dev/* externals/3rdparty/ffmpeg


cd externals/3rdparty
bash http_server_update.sh


rm -rfd /temp/$dest_dir

cd /temp/capturer

qmake "DESTDIR=../$dest_dir" capturer.pro
make clean
make -j`nproc`


cd /temp


rm -rfd externals/3rdparty/ffmpeg
mv externals/3rdparty/ffmpeg_orig externals/3rdparty/ffmpeg
mv -f ffmpeg-$ffmpeg_version-win64-shared/bin/*.dll $dest_dir
rm ffmpeg_bin.zip
rm ffmpeg_dev.zip
rm -rfd ffmpeg-$ffmpeg_version-win64-shared
rm -rfd ffmpeg-$ffmpeg_version-win64-dev


cp license $dest_dir/license


cd $dest_dir


arcfilename="../builder/capturer-$app_version""_x86_64-win_qt-""$qt_version""_ffmpeg-$ffmpeg_version"

if [ -e $arcfilename.7z ]; then
  arcfilename=$arcfilename.`date +%Y-%m-%d_%H-%M-%S`
fi

7z a -mx9 $arcfilename.7z *

