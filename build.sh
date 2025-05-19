#! /bin/bash

rm -r -f build
mkdir build
cd build
cmake build ..
cmake --build .