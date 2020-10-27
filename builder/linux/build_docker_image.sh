#!/bin/bash

docker rmi capturer-builder-linux -f
docker build -t capturer-builder-linux .
