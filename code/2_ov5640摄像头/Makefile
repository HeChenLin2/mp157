CC=arm-none-linux-gnueabihf-gcc

target =main

target1=main
target2=drmFun
target3=bmp
target4=cameraFun
target5=msgbufFun
target6=conversionPicDate


LIB+=/home/hcl/mp157_work/tool/arm-none-linux-gnueabihf/gcc-arm-9.2-2019.12-x86_64-arm-none-linux-gnueabihf/arm-none-linux-gnueabihf/lib/libdrm

all:
	cp /mnt/hgfs/ubuntu_share/2_ov5640摄像头/* ./ -rf
	$(CC) $(target1).c $(target2).c $(target3).c $(target4).c $(target5).c $(target6).c -o $(target) -L $(LIB) -ldrm -lpthread

clean:
	rm $(target)

install:
	sudo cp $(target) /opt/rootfs_mp157/home
