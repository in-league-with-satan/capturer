#!/bin/bash

PATH_ROOT=$(dirname $0)

cd "$PATH_ROOT/.."

PATH_HOST=$(pwd)

USER_ID=$(id -u)
GROUP_ID=$(id -g)

#

docker run --rm -v $PATH_HOST:/temp --user=$USER_ID:$GROUP_ID -i -t ffmpeg-builder /bin/bash -c "cd /temp/ffmpeg; bash build.sh $1"
