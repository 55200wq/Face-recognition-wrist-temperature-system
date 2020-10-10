#ifndef __IIC_H__
#define __IIC_H__

/*
设置寄存器的值
参数说明:
		reg:寄存器地址
		bit_width:控制位宽度(4位则bit_width为F,5位位0x1F)
		bit：表示需要设置的其实位
		val:要设置的值
*/
#if 1
	#define MMU
#endif

#define SET_REG(reg, bit_width, offset, val)	{\
					reg&=~(bit_width<<offset);\
					reg|=val<<offset;\
				}
#define SDA_IN(pin_ctl_addr, bit_width, offset) 	SET_REG(pin_ctl_addr, bit_width, offset, 0)
#define SDA_OUT(pin_ctl_addr, bit_width, offset) 	SET_REG(pin_ctl_addr, bit_width, offset, 1)
#define SCL_IN(pin_ctl_addr, bit_width, offset)		SET_REG(pin_ctl_addr, bit_width, offset, 0)

#define SCL(reg, x) {if(x){*MLX90614_GPBDAT|=1<<7;}else {*MLX90614_GPBDAT&=~(1<<7);}}
#define SDA(reg, offset, x)	{if(x){*MLX90614_GPBDAT|=1<<6;}else {*MLX90614_GPBDAT&=~(1<<6);}}

struct i2c_reg_info{
	char* name;	//名字
	unsigned long PIN_CON_mac;//物理地址
	unsigned long PIN_DAT_mac;//
#ifdef MMU
	unsigned int *PIN_CON;//引脚控制寄存器地址
	unsigned int *PIN_DAT;
#endif
	int PIN_CON_bit_width;
	int PIN_CON_offset;
	int PIN_DAT_bit_width;
	int PIN_DAT_offset;
};

/***************函数声明******************/
void START_bit(struct i2c_reg_info i2c); //产生起始信号
void STOP_bit(struct i2c_reg_info i2c);
void send_bit(struct i2c_reg_info i2c, unsigned char bit_out);
unsigned char Receive_bit(struct i2c_reg_info i2c);
unsigned char TX_byte(struct i2c_reg_info i2c, unsigned char Tx_buffer);
unsigned char RX_byte(struct i2c_reg_info i2c, unsigned char ack_nack);
int i2c_Init();
#endif

