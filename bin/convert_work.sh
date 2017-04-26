#!/bin/bash

filename=$(basename $1)

ff_bin="ffmpeg"
v_enc="libx264"
pix_fmt=""
ch_map=""
preset="slow"
crf=18

#if (( $(./ffprobe -v 0 -of compact=p=0 -select_streams 0 -show_entries stream=width $1) > 1920 )); then
#    preset="medium"
#    crf=20
#fi

for i in "$@"
do
    if [ "${i}" == "--10bit" ]; then
        ff_bin="ffmpeg_10bit"
        pix_fmt="-pix_fmt yuv444p10le"
    fi

    if [ "${i}" == "--x265" ]; then
        v_enc="libx265"
        preset="medium -x265-params strong-intra-smoothing=0"
        crf=17
    fi

    if [ "${i}" == "--remap_ac" ]; then
#                             l     r     c    lfe   bl    br    sl    sr
        ch_map="-af pan=7.1|c0=c0|c1=c1|c2=c3|c3=c2|c4=c6|c5=c7|c6=c4|c7=c5"
    fi

    if [ "${i}" == "--remap_ac_6" ]; then
        ch_map="-af pan=5.1|c0=c0|c1=c1|c2=c3|c3=c2|c4=c6|c5=c7"
    fi
done

./$ff_bin -i videos/$filename \
        -map 0:v -c:v $v_enc -crf $crf -preset $preset $pix_fmt -vlevel 5.1 \
        $ch_map \
        -map 0:a -c:a:0 flac -compression_level 6 \
    videos/converted/$filename

#mkvpropedit videos/converted/$filename --edit track:a2 --set flag-default=0 --edit track:a3 --set flag-default=0
