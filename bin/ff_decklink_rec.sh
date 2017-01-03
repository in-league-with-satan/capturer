#!/bin/bash


filename=videos/$(date +%Y-%m-%d_%H-%M-%S).mkv

./ffmpeg -channels 8 -f decklink -i 'Intensity Pro 4K@21' -acodec copy -vcodec nvenc_h264 -global_quality 1 $filename
