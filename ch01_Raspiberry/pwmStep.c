/*
* PCA9685 - 16-Channel, 12-Bit PWM FM + I2C BUS LED Controller
* -. Each 12-bit resoultion (4096 steps) fixed freq
* 
**/

#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <stdio.h>
#include <stdint.h>

#define I2C_DEV 			"/dev/i2c-1"
#define CLOCK_FREQ			25000000.0
#define PCA_ADDR			0x40
#define LED_STEP			200
#define TIME_VAL			4096

// Register Addr
#define MODE1				0x00
#define MODE2				0x01
#define SERVO_ON_L		0x42
#define SERVO_ON_H		0x43
#define SERVO_OFF_L	0x44
#define SERVO_OFF_H	0x45
#define PRE_SCALE		0xFE

#define STEP1_ON_L 	0x2E
#define STEP1_ON_H  	0x2F
#define STEP1_OFF_L 	0x30
#define STEP1_OFF_H 	0X31

int fd;
unsigned char buffer[3] = {0};

int reg_read8(unsigned char addr)
{
	int length = 1;
	buffer[0] = addr;

	if(write(fd, buffer,length)!=length)
	{
		printf("Failed to write from the i2c bus\n");
	}
	
	if(read(fd,buffer, length) != length)
	{
		printf("Failed to read from the i2c bus\n");
	}
	printf("addr[%d] = %d\n", addr, buffer[0]);
	
	return 0;
}

int reg_write8(unsigned char addr, unsigned char data)
{

	int length=2;
	
	buffer[0] = addr;
	buffer[1] = data;

	if(write(fd,buffer,length)!=length)
	{
		printf("reg_write8: Failed to write from the i2c bus\n");
		return -1;
	}
	
	return 0;
}

int reg_read16(unsigned char addr)
{
	unsigned short temp;
	reg_read8(addr);
	temp = 0xff & buffer[0];
	reg_read8(addr+1);
	temp |= (buffer[0] <<8);
	printf("reg_read16: addr=0x%x, data=%d\n", addr, temp);
	 
	return 0;	
}

int reg_write16(unsigned char addr, unsigned short data)
{
	int length =2;
	reg_write8(addr, (data & 0xff));
	reg_write8(addr+1, (data>>8) & 0xff);
	return 0;
}

int pca9685_restart(void)
{
	int length;
	
	reg_write8(MODE1, 0x00);
	reg_write8(MODE2, 0x04);
	return 0;
}

int pca9685_freq(int updateRate)
{
	int length = 2;
	uint8_t pre_val = (CLOCK_FREQ / (TIME_VAL * updateRate)) -1; 
	printf("pca9685_freq: prescale_val = %d\n", pre_val);
	 
	reg_write8(MODE1, 0x10);				// OP : OSC OFF
	reg_write8(PRE_SCALE, pre_val);			// OP : WRITE PRE_SCALE VALUE
	reg_write8(MODE1, 0x80);				// OP : RESTART
	reg_write8(MODE2, 0x04);				// OP : TOTEM POLE 
	return 0;
}

int servo_on()
{
	int rotateDegree;

	// unsigned short time_val=4095;
	while(1){
		reg_write16(SERVO_ON_L, 0);
		reg_write16(SERVO_OFF_L, 530);
		fprintf(stdout, "servo_on: (+) 90 Degree\n");
		sleep(3);

		reg_write16(SERVO_ON_L, 0);
		reg_write16(SERVO_OFF_L, 320);
		fprintf(stdout, "servo_on: (0) 0 Degree\n");
		sleep(3);

		reg_write16(SERVO_ON_L, 0);
		reg_write16(SERVO_OFF_L, 110);
		fprintf(stdout, "servo_on: (-) 90 Degree\n");
		sleep(3);

		reg_write16(SERVO_ON_L, 0);
		reg_write16(SERVO_OFF_L, 320);
		fprintf(stdout, "servo_on: (0) 0 Degree\n");
		sleep(3);

	}
	return 0;
}

int pca9685_sleep()
{
    reg_write8(MODE1, 0X10);
    reg_write8(MODE2, 0X10);
    return 0;

}

int stepMotorSpeedControl(void)
{ 
	int i;
	for(i = 0; i < TIME_VAL; i++){
		reg_write16(STEP1_ON_L, 0);
		reg_write16(STEP1_OFF_L, i);
	}

	for(i = TIME_VAL; i > 0; i--){
		reg_write16(STEP1_ON_L, 0);
		reg_write16(STEP1_OFF_L, i);
	}
}


int main(void)
{
	unsigned short value=2047;
	if((fd=open(I2C_DEV, O_RDWR))<0)
	{
		printf("main: Failed open i2c-1 bus\n");
		return -1;
	}

	if(ioctl(fd, I2C_SLAVE, PCA_ADDR)<0)
	{
		printf("main: Failed to acquire bus access and/or talk to slave\n");
		return -1;
	}	
	pca9685_restart();
	pca9685_freq(50);
	// servo_on();
		
	stepMotorSpeedControl();
	
	pca9685_sleep();

	return 0;
}
