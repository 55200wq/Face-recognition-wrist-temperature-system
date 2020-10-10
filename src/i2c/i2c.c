#include <stdio.h>
#include <linux/delay.h>
#include <linux/io.h>
#include <linux/gpio.h>
#include <mach/gpio.h>
#include <plat/gpio-cfg.h>
#include <linux/fs.h>

#include "i2c.h"

void START_bit(struct i2c_reg_info i2c) //产生起始信号
{
	SDA(1); // Set SDA line
	udelay( 10 ); // 等待几微秒
	SCL(1); // Set SCL line
	udelay( 10 ); // Generate bus free time between Stop
	// and Start condition (Tbuf=4.7us min)
	SDA(0); // Clear SDA line
	udelay( 10 ); //  开始后的保持时间
	// Condition. After this period, the first clock is generated.
	//(Thd:sta=4.0us min)
	SCL(0); // Clear SCL line
	udelay( 10 ); // Wait a few microseconds
}
void STOP_bit(struct i2c_reg_info i2c) //停止信号
{
	SCL(0); // Clear SCL line
	udelay( 10 ); // Wait a few microseconds
	SDA(0); // Clear SDA line
	udelay( 10 ); // Wait a few microseconds
	SCL(1); // Set SCL line
	udelay( 10 ); // Stop condition setup time(Tsu:sto=4.0us min)
	SDA(1); // Set SDA line
}

void send_bit(struct i2c_reg_info i2c, unsigned char bit_out)
{
	if(bit_out==0) // Check bit
	{
		SDA(0); // Set SDA if bit_out=1
	}
	else{
		SDA(1); // Clear SDA if bit_out=0
	}
	udelay(1);//Tsu:dat = 250ns minimum
	SCL(1); // Set SCL line
	udelay( 20 ); // High Level of Clock Pulse
	SCL(0); // Clear SCL line
	udelay( 20 ); // Low Level of Clock Pulse
	// mSDA_HIGH(); // Master release SDA line ,
	return;
}//End of send_bit()



unsigned char Receive_bit(struct i2c_reg_info i2c)
{
	unsigned char bit = 0;
	//设置SDA为输入模式
	SDA_IN;
	udelay(5);
	SCL(1); // Set SCL line
	udelay( 20 ); // High Level of Clock Pulse
	if(SDA_VAL) // Read bit, save it in bit
		bit=1;
	else
		bit=0;
	udelay(1);
	SCL(0); // Clear SCL line
	udelay( 20 ); // Low Level of Clock Pulse
	SDA_OUT;
	return bit;
}//End of Receive_bit()


//发送一个字节
unsigned char TX_byte(struct i2c_reg_info i2c, unsigned char Tx_buffer)
{
	unsigned char Bit_counter;
	unsigned char Ack_bit = 0;
	unsigned char bit_out;
	for(Bit_counter=8; Bit_counter; Bit_counter--)
	{
		if(Tx_buffer&0x80)
			bit_out=1; // If the current bit of Tx_buffer is 1 set bit_out
		else
			bit_out=0; // else clear bit_out
		send_bit(bit_out); // Send the current bit on SDA
		Tx_buffer<<=1; // Get next bit for checking
	}
	Ack_bit=Receive_bit(); // Get acknowledgment bit
	if(Ack_bit == 1){
		printk("非应答\n");
		return 255;
	}
	return Ack_bit;
}// End of TX_byte()

//接收一个字节
unsigned char RX_byte(struct i2c_reg_info i2c, unsigned char ack_nack)
{
	unsigned char RX_buffer = 0;
	unsigned char Bit_Counter;
	for(Bit_Counter=8; Bit_Counter; Bit_Counter--)
	{
		if(Receive_bit()) // Read a bit from the SDA line
		{
			RX_buffer <<= 1; // If the bit is HIGH save 1 in RX_buffer
			RX_buffer |=0x01;
		}
		else
		{
			RX_buffer <<= 1; // If the bit is LOW save 0 in RX_buffer
		}
	}
	send_bit(ack_nack); // Send acknowledgment bit
	return RX_buffer;
}// End of RX_byte()

/*
函数功能:根据硬件进行i2c初始化
*/
int i2c_Init()
{

	return 0;
}
