#!/bin/bash

cd "$(dirname $0)"

PATH_HOST=$(pwd)

USER_ID=$(id -u)
GROUP_ID=$(id -g)

#

docker run --rm -v $PATH_HOST:/temp:z --user=$USER_ID:$GROUP_ID -i -t ffmpeg-builder-for-capturer /bin/bash -c "cd /temp; bash build.sh $1"
