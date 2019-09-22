#!/bin/bash

docker rmi capturer-builder-linux -f
docker build --no-cache -t capturer-builder-linux .
