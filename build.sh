#!/bin/bash

mkdir build
cd build
cmake ..
make
mv RepCRec ..
cd ..
rm -r build