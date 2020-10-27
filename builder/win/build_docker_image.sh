#!/bin/bash

docker rmi capturer-builder-windows -f
docker build -t capturer-builder-windows .
