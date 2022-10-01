#!/bin/bash

set -e

if [ -e "/proc/cpuinfo" ]; then
    cpu_num=`grep -c "model name" /proc/cpuinfo`
else
    cpu_num=`sysctl -a | grep machdep.cpu.thread_count | sed "s/^.*\ //g"`
fi

while :
do
    if [ -z ${1+x} ]; then
        break;
    fi

    if getopts c:s: opt; then
        case "$opt" in
            c)
                echo ccompiler="$OPTARG"
                ccompiler="$OPTARG"
                shift $(($OPTIND - 1))
                ;;
            s)
                echo sysroot="$OPTARG"
                sysroot="$OPTARG"
                shift $(($OPTIND - 1))
                ;;
            :)  # 没有为需要参数的选项指定参数
                echo "This option -$OPTARG requires an argument."
                exit 1 ;;
            ?) # 发现了无效的选项
                echo "-$OPTARG is not an option"
                exit 2 ;;
        esac
    else
        args+=("$1")
        shift
    fi

    OPTIND=0
done

echo "Extra parameters:"
for arg in "${args[@]}"
do
    echo --- $arg
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

rm -rf ./build
mkdir build

cd build
cmake $CMAKE_C_COMPILER $CMAKE_FLAGS__SYSROOT -DCMAKE_BUILD_TYPE=Debug -Wno-dev ../
make -j ${cpu_num}
make test
cd ..
