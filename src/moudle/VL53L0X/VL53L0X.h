#ifndef __VL53L0X_H__
#define __VL53L0X_H__
/*
硬件连接: 
			SDA --- GPJ0_4	XciDATA1
			SCL --- GPJ0_3  XciDATA0
*/
extern volatile unsigned int *VL53L0X_GPJ0CON;
extern volatile unsigned int *VL53L0X_GPJ0DAT;

#define GPJ0CON_REGISTER_ADDR	0x11400240	//(0x11400000+0x0240)
#define GPJ0DAT_REGISTER_ADDR	0x11400244	//(0x11400000+0x0244)

#define VL53L0X_ADDR (0x29<<1)			//VL53L0X的地址
#define VL53L0X_READ_ADDR 	VL53L0X_ADDR|0x01			//VL53L0X的地址
#define VL53L0X_WRITE_ADDR	VL53L0X_ADDR|0x00			//VL53L0X的地址

#define GPJ0CON (VL53L0X_GPJ0CON)
#define GPJ0DAT (VL53L0X_GPJ0DAT)

#define VL53L0X_SCL_VAL (*GPJ0DAT&(1<< 3))
#define VL53L0X_SDA_VAL (*GPJ0DAT&(1<< 4))

//设置输入模式
#define SDA_IN	{\
					*GPJ0CON&=0xFFF0FFFF;\
					*GPJ0CON|=0x00000000;\
				}
//设置输出模式
#define SDA_OUT	{\
					*GPJ0CON&=0xFFF0FFFF;\
					*GPJ0CON|=0x00010000;\
				}
//设置输出模式
#define SCL_OUT	{\
					*GPJ0CON&=0xFFFF0FFF;\
					*GPJ0CON|=0x10001000;\
				}

#define SCL(x) 	{if(x){*GPJ0DAT|=1<<3;}else {*GPJ0DAT&=~(1<<3);}}
#define SDA(x)	{if(x){*GPJ0DAT|=1<<4;}else {*GPJ0DAT&=~(1<<4);}}

#endif

