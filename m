#!/bin/sh
set -e
cpu_num=`grep -c "model name" /proc/cpuinfo`

# rm -rf ./build
# mkdir build
cd build
make -j ${cpu_num}
make test
cd ../..
