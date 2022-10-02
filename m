#!/bin/sh
set -e

if [ -e "/proc/cpuinfo" ]; then
    cpu_num=`grep -c "model name" /proc/cpuinfo`
else
    cpu_num=`sysctl -a | grep machdep.cpu.thread_count | sed "s/^.*\ //g"`
fi

# rm -rf ./build
# mkdir build
cd build
make -j ${cpu_num}
make test
cd ../..
