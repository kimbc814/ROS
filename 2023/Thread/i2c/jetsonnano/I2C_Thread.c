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
#define I2C_PINFILE_PATH "/dev/i2c-0" // I2C device file path
#define SLAVE_ID 0x05
#define DATA_LENGTH 8
#define US2S(x) (x * 1000000)


void* I2cThreadHandler(void* arg)
{
    while (1)
    {
        // I2C send data package
        pthread_mutex_lock(&mutex_id_0);
        send_data[0] = (encoderpos&0xff000000)>>24;
        send_data[1] = (encoderpos&0x00ff0000)>>16;
        send_data[2] = (encoderpos&0x0000ff00)>>8;
        send_data[3] = (encoderpos&0x000000ff);         // encoderpos 4byte
        send_data[4] = (runtime_cnt&0xff000000)>>24;    
        send_data[5] = (runtime_cnt&0x00ff0000)>>16;
        send_data[6] = (runtime_cnt&0x0000ff00)>>8;
        send_data[7] = (runtime_cnt&0x000000ff);        // runtime_cnt 4byte
        pthread_mutex_unlock(&mutex_id_0);

        pthread_mutex_lock(&mutex_id_0);
        if (write(i2c_slave_address, send_data, sizeof(send_data)) != sizeof(send_data)) 
        {
            perror("Write failed.");
        }
        else printf("send\n");
        pthread_mutex_unlock(&mutex_id_0);
        
        // I2C receive data 
        pthread_mutex_lock(&mutex_id_0);
        if (read(i2c_slave_address, received_data,sizeof(received_data)) != sizeof(received_data))
        {
            printf("read failed.");
        }
        else 
        {
            // print encoderpos
            encoderpos=received_data[3]<<24|received_data[4]<<16|received_data[5]<<8|received_data[6];
        }
        pthread_mutex_unlock(&mutex_id_0);

        usleep(US2S(0.1));
    }
    pthread_exit(NULL);
}

void* stateThreadHandler(void* arg)
{
    pthread_mutex_lock(&mutex_id_0);
    pthread_cond_wait(&cond_state, &mutex_id_0);
    printf("encoderpos : %d\n",encoderpos);
    printf("\nTime : %3d \n",runtime_cnt);
    pthread_mutex_unlock(&mutex_id_0);

    pthread_exit(NULL);
}

void* mainThreadHandler(void* arg)
{
    while(1)
    {
        runtime_cnt++;
        sleep(1);
        pthread_cond_signal(&cond_state);
    }
}

void I2cExit()
{
    // stop i2c
	close(i2c_slave_address);
	// stop mutex
    pthread_mutex_destroy(&mutex_id_0);
    // stop thread
    pthread_cancel(pthread_i2c);
    pthread_cancel(pthread_state);
    pthread_cancel(pthread_main);
    // stop cond
    pthread_cond_destroy(&cond_state);
}

int main()
{
	//setup
    atexit(I2cExit);

	// variable declaration
	uint8_t send_data[DATA_LENGTH];
	uint8_t received_data[DATA_LENGTH];
	uint32_t encoderpos,runtime_cnt;
	


	// i2c 
	int i2c_slave_address;

	// variable definition
	encoderpos = 0;
	runtime_cnt = 0;

	// data pkg reset 
	for(uint8_t i =0;i<DATA_LENGTH;i++)
	{
		send_data[i]=0;
	}
	
	//i2c init
    i2c_slave_address = open(I2C_PINFILE_PATH, O_RDWR);
    if (i2c_slave_address < 0)
    {
        perror("Unable to open I2C device");
        return 1;
    }
    else
    {
        if (ioctl(i2c_slave_address, I2C_SLAVE, SLAVE_ID) < 0) 
        {
            perror("Failed to acquire bus access and/or talk to slave");
            return 1;
        }
    }

    // pthread 
    pthread_t pthread_i2c, pthread_state, pthread_main;
	pthread_mutex_t mutex_id_0,mutex_id_1;
    pthread_cond_t cond_state;

	pthread_create(&pthread_i2c,NULL,I2cThreadHandler,NULL);
    pthread_create(&pthread_state,NULL,stateThreadHandler,NULL);
    pthread_create(&pthread_main,NULL,mainThreadHandler,NULL);
    pthread_cond_init(&cond_state, NULL);
	pthread_mutex_init(&mutex_id_0, NULL);
    
	printf("Create I2cThreadHandler \n");

    pthread_join(pthread_main,NULL);

	return 1;
}
