#!/bin/bash

mkdir -p build
cd build
cmake ../
make -j
cp sf-interpreter ../../bin/
cd ..
