#!/bin/bash

dts_dir=./kernel/arch/arm/boot/dts
target_dts_dir=~/mp157_work/kernel/linux-stm32mp-5.4.31-r0/linux-5.4.31/arch/arm/boot/dts

echo "开始配置内核"
files=$(ls $dts_dir)
for filename in $files
do
	echo "cp $dts_dir/$filename $target_dts_dir"
    cp $dts_dir/$filename $target_dts_dir
done

