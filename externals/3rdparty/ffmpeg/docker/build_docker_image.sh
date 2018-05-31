#!/bin/bash

docker rmi ffmpeg-builder -f
docker build -t ffmpeg-builder .
