CC=arm-none-linux-gnueabihf-gcc
target=main
LIB+=/home/hcl/mp157_work/tool/arm-none-linux-gnueabihf/gcc-arm-9.2-2019.12-x86_64-arm-none-linux-gnueabihf/arm-none-linux-gnueabihf/lib/libdrm

all:
	$(CC) $(target).c -o $(target) -L $(LIB) -ldrm

clean:
	rm $(target)

install:
	sudo cp $(target) /opt/rootfs_mp157/home
