#!/bin/bash

set -e

if [ -e "/proc/cpuinfo" ]; then
    cpu_num=`grep -c "model name" /proc/cpuinfo`
else
    cpu_num=`sysctl -a | grep machdep.cpu.thread_count | sed "s/^.*\ //g"`
fi

while :
do
    if [ -z $s1 ]; then
        break;
    fi

    if getopts c:x:s: opt; then
        case "$opt" in
            c)
                echo ccompiler="$OPTARG"
                ccompiler="$OPTARG"
                shift $(($OPTIND - 1))
                ;;
            x)
                echo cxxcompiler="$OPTARG"
                cxxcompiler="$OPTARG"
                shift $(($OPTIND - 1))
                echo "cxxcompiler detected: " "$cxxcompiler"
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
    CMAKE_FLAGS+=("-DCMAKE_C_COMPILER=$ccompiler")
fi
if [ ! "$cxxcompiler" = "" ]; then
    CMAKE_FLAGS+=("-DCMAKE_CXX_COMPILER=$cxxcompiler")
fi
if [ ! "$sysroot" = "" ]; then
    if [ ! -d "$sysroot" ]; then
        echo Failed to check sysroot directory
        exit
    fi
    CMAKE_FLAGS+=("-DCMAKE_SYSROOT=$sysroot")
fi
CMAKE_FLAGS+=("-DCMAKE_BUILD_TYPE=Release")

rm -rf ./build
mkdir build

cd build
cmake "${CMAKE_FLAGS[@]}" -Wno-dev ../
make -j ${cpu_num}
make test
cd ..
