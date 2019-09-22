#!/bin/bash

docker rmi capturer-builder-windows -f
docker build --no-cache -t capturer-builder-windows .
