#!/bin/bash

docker rmi ffmpeg-builder-for-capturer -f
docker build -t ffmpeg-builder-for-capturer .
