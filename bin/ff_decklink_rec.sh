#!/bin/bash

quality=1
device=( "Intensity Pro 4K@21" )


# ----

i=1;

while [ $i -le $# ]
do
echo "e $i: ${!i}"
    if [ "${!i}" == "formats" ]; then
        ./ffmpeg -f decklink -list_formats 1 -i 'Intensity Pro 4K' 
        exit 0
    fi

    if [ "${!i}" == "-q" ]; then
        (( i++ ))
        quality="${!i}"
    fi

    if [ "${!i}" == "-m" ]; then
        (( i++ ))
        device=( "Intensity Pro 4K@${!i}" )
        mode="${!i}"
    fi

    (( i++ ))
done


filename=videos/$(date +%Y-%m-%d_%H-%M-%S)_q$quality.mkv


if [ "$quality" == "0" ]; then
    quality="0 -preset lossless" 
fi

./ffmpeg -channels 8 -f decklink -i "${device[@]}" -acodec copy -vcodec h264_nvenc -global_quality $quality $filename
