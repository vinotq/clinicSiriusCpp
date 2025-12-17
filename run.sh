#!/bin/bash

cd ClinicSirius 

mkdir -p build
cd build

cmake .. 

make -j$(nproc) 

./ClinicSirius 