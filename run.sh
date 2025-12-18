#!/bin/bash

sudo apt-get install qt6-base-dev qt6-declarative-dev cmake build-essential

cd ClinicSirius 

mkdir -p build
cd build

cmake .. 

make -j$(nproc) 

./ClinicSirius 