// header file
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <iostream>
#include <fruntime_cntl.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>

// 
#define I2C_FILE_PATH "/dev/i2c-0" // I2C device file path
#define SLAVE_ADDRESS 0x05
#define DATA_LENGTH 8
#define US2S(x) x*1000000




void* I2cThreadHandler(void* arg) {
  while (1) {
	  
    // I2C send data
    pthread_mutex_lock(&i2c_muteid1);
    if (write(i2c_file, send_data, sizeof(send_data)) != sizeof(send_data)) {
        perror("Write failed");
    }
    else printf("send\n");
    pthread_mutex_unlock(&i2c_muteid1);
    
    // I2C receive data 
    pthread_mutex_lock(&i2c_muteid1);
	read(i2c_file, received_data,DATA_LENGTH);
    pthread_mutex_unlock(&i2c_muteid1);
    
	// print encoderpos
	encoderpos=received_data[3]<<24|received_data[4]<<16|received_data[5]<<8|received_data[6];
	printf("encoderpos : %d\n",encoderpos);

    usleep(US2S(0.1));
  }
}


int main()
{
	//setup
	// variable declaration
	uint8_t send_data[DATA_LENGTH];
	uint8_t received_data[DATA_LENGTH];
	uint32_t encoderpos,runtime_cnt;
	
	// i2c 
	pthread_mutex_t i2c_muteid1,i2c_muteid2;
	int i2c_file;

	// variable definition
	encoderpos = 0;
	runtime_cnt = 0;

	// data pkg reset 
	for(uint8_t i =0;i<DATA_LENGTH;i++)
	{
		send_data[i]=0;
	}
	
	//i2c init
    i2c_file = open(I2C_FILE_PATH, O_RDWR);
    if (i2c_file < 0)
    {
        perror("Unable to open I2C device");
        return 1;
    }
    if (ioctl(i2c_file, I2C_SLAVE, SLAVE_ADDRESS) < 0) {
        perror("Failed to acquire bus access and/or talk to slave");
        return 1;
    }

	//pthread init
	pthread_mutex_init(&i2c_muteid1, NULL);
	pthread_t pthread_encoder;
	printf("Create I2cThreadHandler \n");
	pthread_create(&pthread_encoder,NULL,I2cThreadHandler,NULL);
	
	//loop
	while(1)
	{
		printf("\nTime : %3d \n",runtime_cnt);
		runtime_cnt++;
		  send_data[0]= (encoderpos&0xff000000)>>24;
		  send_data[1]= (encoderpos&0x00ff0000)>>16;
		  send_data[2]= (encoderpos&0xff00)>>8;
		  send_data[3]= (encoderpos&0x00ff); 
		  send_data[4]= (runtime_cnt&0xff000000)>>24;    // encoder MSB 8bit
		  send_data[5]= (runtime_cnt&0x00ff0000)>>16;
		  send_data[6]= (runtime_cnt&0x0000ff00)>>8;
		  send_data[7]= (runtime_cnt&0x000000ff);        // encoder LSB 8bit
		//runtime_cnt =runtime_cnt%100;
		sleep(1);
	}
	// stop i2c
	close(i2c_file);
	// stop mutex
    pthread_mutex_destroy(&i2c_muteid1);
	return 1;
}
