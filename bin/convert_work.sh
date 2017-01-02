#!/bin/bash

#./ffprobe -v 0 -of compact=p=0 -select_streams 0 -show_entries stream=r_frame_rate $1

filename=$(basename $1)

ff_bin="ffmpeg"

if [ "$2" == "--10bit" ]; then
    ff_bin="ffmpeg_10bit"
fi


./$ff_bin -i videos/$filename -async 1 \
        -map 0:v -c:v libx264 -crf 18 -preset slow -vlevel 5.1 \
        -map 0:a -c:a:0 flac -compression_level 6 \
        -map 0:a -c:a:1 libvorbis -ac:2 6 -aq 6 \
        -map 0:a -c:a:2 libvorbis -ac:3 2 -aq 6 \
    videos/converted/$filename

mkvpropedit videos/converted/$filename --edit track:a2 --set flag-default=0 --edit track:a3 --set flag-default=0
