#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <signal.h>

#define MLX90614_ADDR 0x5A<<1			//MLX90614的地址
#define MLX90614_READ_ADDR 0x2E       	//读取MLX90614的地址
#define MLX90614_READ_TEMPERATURE 0x07	//读取温度


int main(int argc, char** argv)
{	
	if(argc != 2){
		printf("正确格式: <./a.out> </dev/xxx> \n");
		return -1;
	}
	
	return 0;
}

