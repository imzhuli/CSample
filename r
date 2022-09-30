#!/bin/sh
set -e

args=`getopt -o "c:" -l "sysroot:,c-complier:" -- "$@"`
eval set -- "$args"
while :
do 
    case "$1" in
        -c|--c-complier)
            ccompiler="$2";
            shift 2;;
        --sysroot)
            sysroot="$2";
            shift 2;;
        --) 
            shift; break;;
        *) echo $1; shift;;
    esac
done

if [ ! "$ccompiler" = "" ]; then
    CMAKE_C_COMPILER="-DCMAKE_C_COMPILER=$ccompiler"
fi
if [ ! "$sysroot" = "" ]; then
    if [ ! -d "$sysroot" ]; then
        echo Failed to check sysroot directory
        exit
    fi
    CMAKE_FLAGS__SYSROOT="-DCMAKE_SYSROOT=$sysroot"
fi

cpu_num=`grep -c "model name" /proc/cpuinfo`

rm -rf ./build
mkdir build

cd build
cmake "$CMAKE_C_COMPILER" "$CMAKE_FLAGS__SYSROOT" -DCMAKE_BUILD_TYPE=Release -Wno-dev ../
make -j ${cpu_num}
make test
cd ..
