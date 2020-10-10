#include "MLX90614.h"
#include <linux/kernel.h> 
#include <linux/module.h>
#include <linux/io.h>
#include <linux/gpio.h>
#include <mach/gpio.h>
#include <plat/gpio-cfg.h>
#include <linux/delay.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>



volatile unsigned int *MLX90614_GPBCON;
volatile unsigned int *MLX90614_GPBDAT;

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
	if(*(MLX90614_GPBDAT) & (1<<6)) // Read bit, save it in bit
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

unsigned int MLX90614_read(unsigned char ADDR, unsigned char CMD)
{
	char DataL, DataH, Pecreg;
	START_bit();
	if( TX_byte(ADDR) == 255 ) return -1;//Send SlaveAddress 器件默认地址是0x5A 最后一位是读写位,所以左移一位
	//TX_byte(0x07); //Send Command
	if( TX_byte(CMD) == 255 ) return -1; //Send Command 获取SMBus address
	//------------
	START_bit();
	if( TX_byte(ADDR) == 255 ) return -1; //Send SlaveAddress
	//TX_byte(0x01);
	DataL=RX_byte(0);
	DataH=RX_byte(0);
	Pecreg=RX_byte(1);
	STOP_bit();
	return ((DataH<<8)|DataL);
}

unsigned int memread(void)
{
	char DataL, DataH, Pecreg;
	START_bit();
	TX_byte(0x5A<<1); //Send SlaveAddress 器件默认地址是0x5A 最后一位是读写位,所以左移一位
	TX_byte(0x07); //Send Command
	//TX_byte(0x2E); //Send Command 获取SMBus address
	//------------
	START_bit();
	TX_byte(0x00); //Send SlaveAddress
	//TX_byte(0x01);
	DataL=RX_byte(0);
	DataH=RX_byte(0);
	Pecreg=RX_byte(1);
	STOP_bit();
	return ((DataH<<8)|DataL);
}

static int MLX90614_Init(void)
{
	int i;
	//进行地址映射
	MLX90614_GPBCON = ioremap(0x11400040, 4);
	MLX90614_GPBDAT = ioremap(0x11400044, 4);
	//设置触式状态
	SCL_OUT;
	SDA_OUT;
	for(i=5; i>0; i--){
		//printk("memread = %d\n", MLX90614_read(MLX90614_ADDR, MLX90614_READ_TEMPERATURE));
		printk("memread = %d\n", memread());
		msleep(1000);
	}
	return 0;
}

static int	MLX90614_open(struct inode *inode, struct file *file)
{
	
	return 0;
}

long MLX90614__ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	int val;
	val = MLX90614_read(MLX90614_ADDR, cmd);
	return val;
}

static int MLX90614_release(struct inode *inode, struct file *file)
{
	
	return 0;
}


static struct file_operations MLX90614_fops = {
	.owner			= 	THIS_MODULE,
	.open			=	MLX90614_open,
	.release		=	MLX90614_release,
	.unlocked_ioctl	=	MLX90614__ioctl,
};

static struct miscdevice MLX90614_dev = {
	.minor			=	MISC_DYNAMIC_MINOR,
	.name			=	"MLX90614",
	.fops			=	&MLX90614_fops,
};

static int __init MLX90614_I2C_init(void)
{
	int ret;
	MLX90614_Init();
	//注册杂项设备
	ret = misc_register(&MLX90614_dev);//注册杂项设备
	if(ret < 0)
	{
		printk("杂项设备注册失败\n");
		return -1;
	}
	printk("MLX90614 驱动安装成功.\n");
    return 0;
}

static void __exit MLX90614_I2C_exit(void)
{
	/*取消映射*/
	iounmap(MLX90614_GPBCON);
	iounmap(MLX90614_GPBDAT);
	misc_deregister(&MLX90614_dev);
    printk("MLX90614 驱动卸载成功.\n");
}

module_init(MLX90614_I2C_init);
module_exit(MLX90614_I2C_exit); 

MODULE_LICENSE("GPL");
MODULE_AUTHOR("wq");


