#!/bin/bash

filename=$(basename $1)

ff_bin="ffmpeg"
ch_map=""
preset="slow"
crf=18

if (( $(./ffprobe -v 0 -of compact=p=0 -select_streams 0 -show_entries stream=width $1) > 1920 )); then
    preset="medium"
    crf=20
fi

for i in "$@"
do
    if [ "${i}" == "--10bit" ]; then
        ff_bin="ffmpeg_10bit"
    fi

    if [ "${i}" == "--remap_ac" ]; then
#                             l     r     c    lfe   bl    br     sl    sr
        ch_map="-af pan=7.1|c0=c0|c1=c1|c2=c3|c3=c2|c4=c6|c5=c7|c6=c4|c7=c5"
    fi

    if [ "${i}" == "--remap_ac_6" ]; then
        ch_map="-af pan=5.1|c0=c0|c1=c1|c2=c3|c3=c2|c4=c6|c5=c7"
    fi
done


./$ff_bin -i videos/$filename -async 1 \
        -map 0:v -c:v libx264 -crf $crf -preset $preset -vlevel 5.1 \
        $ch_map \
        -map 0:a -c:a:0 flac -compression_level 6 \
        -map 0:a -c:a:1 libvorbis -ac:2 6 -aq 6 \
        -map 0:a -c:a:2 libvorbis -ac:3 2 -aq 6 \
    videos/converted/$filename

mkvpropedit videos/converted/$filename --edit track:a2 --set flag-default=0 --edit track:a3 --set flag-default=0
