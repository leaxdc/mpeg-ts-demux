#!/bin/bash
if [ ! -e build ]; then
    mkdir build
fi
if [ "$1" = "" ]; then
	build_type="Release"
else
	build_type="$1"
fi

pushd .
cd build
echo "BUILD CONFIGURATION: $build_type"
cmake ../src -DCMAKE_BUILD_TYPE=$build_type
cpu_core_count=$(grep -c '^processor' /proc/cpuinfo 2>/dev/null || sysctl -n hw.ncpu)
make -j$cpu_core_count
popd

