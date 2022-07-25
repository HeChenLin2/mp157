CC=arm-none-linux-gnueabihf-gcc

target =main
target1=main
target2=drmFun
target3=bmp

LIB+=/home/hcl/mp157_work/tool/arm-none-linux-gnueabihf/gcc-arm-9.2-2019.12-x86_64-arm-none-linux-gnueabihf/arm-none-linux-gnueabihf/lib/libdrm

all:
	cp /mnt/hgfs/ubuntu_share/1_drm框架显示图像/* ./
	$(CC) $(target1).c $(target2).c $(target3).c -o $(target) -L $(LIB) -ldrm

clean:
	rm $(target)

install:
	sudo cp $(target) /opt/rootfs_mp157/home
