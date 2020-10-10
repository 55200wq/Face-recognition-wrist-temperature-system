#include "VL53L0X.h"
#include <linux/kernel.h> 
#include <linux/module.h>
#include <linux/io.h>
#include <linux/gpio.h>
#include <mach/gpio.h>
#include <plat/gpio-cfg.h>
#include <linux/delay.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>



volatile unsigned int *VL53L0X_GPJ0CON;
volatile unsigned int *VL53L0X_GPJ0DAT;

void START_bit(void) //产生起始信号
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
void STOP_bit(void) //停止信号
{
	SCL(0); // Clear SCL line
	udelay( 10 ); // Wait a few microseconds
	SDA(0); // Clear SDA line
	udelay( 10 ); // Wait a few microseconds
	SCL(1); // Set SCL line
	udelay( 10 ); // Stop condition setup time(Tsu:sto=4.0us min)
	SDA(1); // Set SDA line
}

void send_bit(unsigned char bit_out)
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



unsigned char Receive_bit(void)
{
	unsigned char bit = 0;
	//设置SDA为输入模式
	SDA_IN;
	udelay(5);
	SCL(1); // Set SCL line
	udelay( 20 ); // High Level of Clock Pulse
	if(VL53L0X_SDA_VAL) // Read bit, save it in bit
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
unsigned char TX_byte(unsigned char Tx_buffer)
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
unsigned char RX_byte(unsigned char ack_nack)
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

unsigned int VL53L0X_read(unsigned char ADDR, unsigned char INDEX)
{
	char Data;
	START_bit();
	if( TX_byte(VL53L0X_ADDR) == 255 ) return -1;//Send SlaveAddress 器件默认地址是0x5A 最后一位是读写位,所以左移一位
	//TX_byte(0x07); //Send Command
	if( TX_byte(INDEX) == 255 ) return -1; //Send Command 获取SMBus address
	//------------
	STOP_bit();
	START_bit();
	if( TX_byte(VL53L0X_READ_ADDR) == 255 ) return -1; //Send SlaveAddress
	//TX_byte(0x01);
	Data = RX_byte(1);
	STOP_bit();
	return Data;
}
unsigned int VL53L0X_write(unsigned char ADDR, unsigned char INDEX, unsigned char DAT)
{
	char Data;
	START_bit();
	if( TX_byte(VL53L0X_WRITE_ADDR) == 255 ) return -1;//Send SlaveAddress 器件默认地址是0x5A 最后一位是读写位,所以左移一位
	//TX_byte(0x07); //Send Command
	if( TX_byte(INDEX) == 255 ) return -1; //Send Command 获取SMBus address
	//------------
	if( TX_byte(DAT) == 255 ) return -1; //Send SlaveAddress
	Data=RX_byte(0);
	STOP_bit();
	return Data;
}

/*
函数功能:连续读
函数参数:
unsigned char ADDR	--- 设备地址
unsigned char INDEX	---	要读取的地址
int size			---	要读取的字节数
const char* buf		---	用来接收顺序读出的数据

*/
unsigned int VL53L0X_sequential_read(unsigned char ADDR, unsigned char INDEX, int size, const unsigned char* buf)
{
	unsigned char* dat_p;
	int i;
	dat_p = buf;
	START_bit();
	if( TX_byte(VL53L0X_ADDR) == 255 ) return -1;
	if( TX_byte(INDEX) == 255 ) return -1;
	STOP_bit();
	START_bit();
	if( TX_byte(VL53L0X_READ_ADDR) == 255 ) return -1;
	for(i=0; i<size; i++){
		*dat_p = RX_byte(0);
		dat_p++;
	}
	STOP_bit();
	return 0;
}


static int VL53L0X_Init(void)
{
	int i;
	char InitInfo_buf[7];
	//char* InitInfo_buf = malloc(7);
	//进行地址映射
	VL53L0X_GPJ0CON = ioremap(GPJ0CON_REGISTER_ADDR, 4);
	VL53L0X_GPJ0DAT = ioremap(GPJ0DAT_REGISTER_ADDR, 4);
	//设置触式状态
	SCL_OUT;
	SDA_OUT;

	VL53L0X_sequential_read(0, 0xC0, 3, InitInfo_buf);
	VL53L0X_sequential_read(0, 0x51, 2, InitInfo_buf+3);
	VL53L0X_sequential_read(0, 0x61, 2, InitInfo_buf+3+2);
	for(i=0; i<7; i++){
		printk("%#X  ", InitInfo_buf[i]);
	}
	printk("\n");
	/*for(i=5; i>0; i--){
		//printk("memread = %d\n", iic_read(MLX90614_ADDR, MLX90614_READ_TEMPERATURE));
		printk("memread = %#X\n", iic_read(0, 0xC0));
		msleep(1000);
	}*/
	return 0;
}

static int	VL53L0X_open(struct inode *inode, struct file *file)
{
	
	return 0;
}

long VL53L0X_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	int val;
	val = VL53L0X_read(VL53L0X_ADDR, cmd);
	return val;
}

static int VL53L0X_release(struct inode *inode, struct file *file)
{
	
	return 0;
}


static struct file_operations VL53L0X_fops = {
	.owner			= 	THIS_MODULE,
	.open			=	VL53L0X_open,
	.release		=	VL53L0X_release,
	.unlocked_ioctl	=	VL53L0X_ioctl,
};

static struct miscdevice VL53L0X_dev = {
	.minor			=	MISC_DYNAMIC_MINOR,
	.name			=	"VL53L0X",
	.fops			=	&VL53L0X_fops,
};

static int __init VL53L0X_I2C_init(void)
{
	int ret;
	VL53L0X_Init();
	//注册杂项设备
	ret = misc_register(&VL53L0X_dev);//注册杂项设备
	if(ret < 0)
	{
		printk("杂项设备注册失败\n");
		return -1;
	}
	printk("MLX90614 驱动安装成功.\n");
    return 0;
}

static void __exit VL53L0X_I2C_exit(void)
{
	/*取消映射*/
	iounmap(GPJ0CON);
	iounmap(GPJ0DAT);
	misc_deregister(&VL53L0X_dev);
    printk("MLX90614 驱动卸载成功.\n");
}

module_init(VL53L0X_I2C_init);
module_exit(VL53L0X_I2C_exit); 

MODULE_LICENSE("GPL");
MODULE_AUTHOR("wq");


