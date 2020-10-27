#!/bin/bash

dest_dir=bin_win

#

staticff=0

if [[ "$1" == "staticff" ]] ; then
  staticff=1
fi


cd /temp


cd externals/3rdparty
bash http_server_update.sh || { echo 'http_server_update err'; exit 1; }
bash curses_update.sh || { echo 'curses_update err'; exit 1; }

cd ffmpeg
bash build_win.sh $1 || { echo 'ffmpeg build err'; exit 1; }


cd win/tmp/ffmpeg
ffmpeg_version=`git log -1 --format=%cd-%h --date=format:'%Y%m%d'`


rm -rfd /temp/$dest_dir


cd /temp/capturer

app_last_tag=$(git describe --tags `git rev-list --tags --max-count=1`)
app_version=$app_last_tag.`git rev-list $app_last_tag.. --count`-`git log -1 --pretty=format:%h`

qt_version=`qmake --version | awk '{if ($1=="Using") print $4;}'`


if [ $staticff -eq 1 ]; then
  qmake "DEFINES+=STATIC_WIN_FF" "DESTDIR=../$dest_dir" capturer.pro || exit 1

else
  qmake "DESTDIR=../$dest_dir" capturer.pro || exit 1
fi

make clean
make -j`nproc` || exit 1


cd /temp


if [ $staticff -eq 0 ]; then
  cp -f externals/3rdparty/ffmpeg/win/bin/*.dll $dest_dir
fi

echo -e 'start capturer --setup\r\n\c' > $dest_dir/capturer.setup.cmd
echo -e 'start capturer --portable-mode\r\n\c' > $dest_dir/capturer.portable.cmd
echo -e 'start capturer --portable-mode --setup\r\n\c' > $dest_dir/capturer.portable.setup.cmd
echo -e 'start capturer --portable-mode --headless\r\n\c' > $dest_dir/capturer.portable.headless.cmd
echo -e 'start capturer --portable-mode --headless-curse\r\n\c' > $dest_dir/capturer.portable.headless-curse.cmd

cp license $dest_dir/license


cd $dest_dir

arcfilename="../builder/capturer-$app_version""_x86_64-win_qt-""$qt_version""_ffmpeg-$ffmpeg_version"

if [ -e $arcfilename.7z ]; then
  arcfilename=$arcfilename.`date +%Y-%m-%d_%H-%M-%S`
fi

7z a -mx9 $arcfilename.7z *
