#!/bin/bash

ROOT_DIR=$(dirname $(readlink -f $0))

cd "$ROOT_DIR/linux"

bash build.sh


cd "$ROOT_DIR/win"

bash build.sh staticff
