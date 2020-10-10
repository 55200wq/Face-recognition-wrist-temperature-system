#ifndef __MLX90614_H__
#define __MLX90614_H__
/*
硬件连接: 
			SDA --- GPB_6  
			SCL --- GPB_7
*/
extern volatile unsigned int *MLX90614_GPBCON;
extern volatile unsigned int *MLX90614_GPBDAT;

#define MLX90614_ADDR (0x5A<<1)			//MLX90614的地址
#define MLX90614_READ_ADDR 0x2E       	//读取MLX90614的地址
#define MLX90614_READ_TEMPERATURE 0x07	//读取温度

//设置输入模式
#define SDA_IN	{\
					*MLX90614_GPBCON&=0xF0FFFFFF;\
					*MLX90614_GPBCON|=0x00000000;\
				}
//设置输出模式
#define SDA_OUT	{\
					*MLX90614_GPBCON&=0xF0FFFFFF;\
					*MLX90614_GPBCON|=0x01000000;\
				}
//设置输出模式
#define SCL_OUT	{\
					*MLX90614_GPBCON&=0x0FFFFFFF;\
					*MLX90614_GPBCON|=0x10000000;\
				}

#define SCL(x) 	{if(x){*MLX90614_GPBDAT|=1<<7;}else {*MLX90614_GPBDAT&=~(1<<7);}}
#define SDA(x)	{if(x){*MLX90614_GPBDAT|=1<<6;}else {*MLX90614_GPBDAT&=~(1<<6);}}

#endif

