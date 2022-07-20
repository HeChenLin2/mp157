#!/bin/bash

echo "开始交叉编译libdrm库"
./configure --prefix=$PWD/_install --host=arm-none-linux-gnueabihf
make
make install

mkdir $PWD/../myinclude/libdrm
mkdir $PWD/../mylib/libdrm
cp -rf $PWD/_install/include/* $PWD/../myinclude/libdrm
cp -rf $PWD/_install/lib/* $PWD/../mylib/libdrm


