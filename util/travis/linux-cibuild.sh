#!/bin/bash

set -e
set -x

cd build
cmake ..
make -j 2
