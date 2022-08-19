#!/bin/bash

libpath1=/opt/rootfs_mp157/lib
incpath1=/opt/rootfs_mp157/usr/include
binpath1=/opt/rootfs_mp157/bin


libpath2=/home/hcl/mp157_work/tool/arm-none-linux-gnueabihf/gcc-arm-9.2-2019.12-x86_64-arm-none-linux-gnueabihf/arm-none-linux-gnueabihf/lib
incpath2=/home/hcl/mp157_work/tool/arm-none-linux-gnueabihf/gcc-arm-9.2-2019.12-x86_64-arm-none-linux-gnueabihf/arm-none-linux-gnueabihf/include


echo "开始拷贝头文件和库文件"
sudo cp -rf $PWD/myinclude/* $incpath1
sudo cp -rf $PWD/mylib/* $libpath1
sudo cp -rf $PWD/mybin/* $binpath1


cp -rf $PWD/myinclude/* $incpath2
cp -rf $PWD/mylib/* $libpath2

