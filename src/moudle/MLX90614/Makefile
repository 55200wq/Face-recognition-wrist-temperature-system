Linux-ADD=/home/wq/work/bord-proj/tiny4412/linux_3.5/linux-3.5
CC = arm-linux-gcc
all:
	make -C $(Linux-ADD) M=`pwd` modules #编译指定目标modules
	cp *.ko ../drv_ko -fv
	make -C $(Linux-ADD) M=`pwd` clean   #清除目标
	$(CC) MLX90614_test.c -o app_test
obj-m += MLX90614.o


