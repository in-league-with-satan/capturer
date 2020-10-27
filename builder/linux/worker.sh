#!/bin/bash

dest_dir=bin_linux

cd /temp


cd externals/3rdparty
bash http_server_update.sh || { echo 'http_server_update err'; exit 1; }


cd ffmpeg
bash build_linux.sh || { echo 'ffmpeg build err'; exit 1; }

cd linux/tmp/ffmpeg
ffmpeg_version=`git log -1 --format=%cd-%h --date=format:'%Y%m%d'`


rm -rfd /temp/$dest_dir


cd /temp/capturer

app_last_tag=$(git describe --tags `git rev-list --tags --max-count=1`)
app_version=$app_last_tag.`git rev-list $app_last_tag.. --count`-`git log -1 --pretty=format:%h`

qt_version=`qmake --version | awk '{if ($1=="Using") print $4;}'`


qmake "DESTDIR=../$dest_dir" capturer.pro || exit 1
make clean
make -j`nproc` || exit 1


cd /temp


echo -e '#!/bin/bash\n\n./capturer --setup\n\c' > $dest_dir/capturer.setup.sh
echo -e '#!/bin/bash\n\n./capturer --portable-mode\n\c' > $dest_dir/capturer.portable.sh
echo -e '#!/bin/bash\n\n./capturer --portable-mode --setup\n\c' > $dest_dir/capturer.portable.setup.sh
echo -e '#!/bin/bash\n\n./capturer --portable-mode --headless\n\c' > $dest_dir/capturer.portable.headless.sh
echo -e '#!/bin/bash\n\n./capturer --portable-mode --headless-curse\n\c' > $dest_dir/capturer.portable.headless-curse.sh

cp license $dest_dir/license


cd $dest_dir

arcfilename="../builder/capturer-$app_version""_x86_64-linux_qt-""$qt_version""_ffmpeg-$ffmpeg_version"

if [ -e $arcfilename.7z ]; then
  arcfilename=$arcfilename.`date +%Y-%m-%d_%H-%M-%S`
fi

7z a -mx9 $arcfilename.7z *
