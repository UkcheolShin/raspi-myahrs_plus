#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include "myahrs_plus.h"

#define I2C_DEV_FILENAME    "/dev/i2c-1"
#define AHRS_SLAVE_ADDRESS 0x20

void print_imu(ahrs_data_t* ahrs_data);

void main(int argc, char * argv[])
{
    int fd=0,rtn=0,i=0,j =0;
    unsigned char buf[30] = {0,};
    unsigned char ret = -1;

    ahrs_data_t * ahrs_data = malloc(sizeof(ahrs_data_t));
    
    //i2c 디바이스파일을 오픈
    if((fd = open( I2C_DEV_FILENAME, O_RDWR )) < 0){
        perror("open()");
        exit(1);
    }else
        printf("I2C Device Driver Open Sucess!\n");

    //접근할 디바이스의 슬레이브 주소를 설정합니다.
    if((rtn = ioctl(fd, I2C_SLAVE, AHRS_SLAVE_ADDRESS)) < 0){
        perror("ioctl");
        close(fd);
        exit(1);
    }else
        printf("myAHSR+ Device Select Sucess!\n");
        
    do{
        
        ret = I2C_MYAHRS_READ(fd,CAL_DATA,ahrs_data);
        print_imu(ahrs_data);
        
        sleep(1);
        
        #if 0
        //Read test 
        ret = Read(fd,buf,2,REG_TEMPERATURE_LOW);
        printf("buf : 0x%x 0x%x \t",buf[0],buf[1]);
        printf("temp : %.2f \n",(float)(signed short)(buf[1]<<8|buf[0])*200/32767);
        #endif
    }
    while(j++ != 50);

    free(ahrs_data);
    close(fd);
}

void print_imu(ahrs_data_t* ahrs_data)
{
    printf("ACC[g]:%.2f,%.2f,%.2f\r\n", ahrs_data->sensor.acc_x, ahrs_data->sensor.acc_y, ahrs_data->sensor.acc_z);
    printf("GYRO[deg/s]:%.2f,%.2f,%.2f\r\n", ahrs_data->sensor.gyro_x, ahrs_data->sensor.gyro_y, ahrs_data->sensor.gyro_z);
    printf("MAG[uT]:%.2f,%.2f,%.2f\r\n", ahrs_data->sensor.magnet_x, ahrs_data->sensor.magnet_y, ahrs_data->sensor.magnet_z);
    printf("Temperature[degC]:%.2f\r\n", ahrs_data->temp);
    printf("R/P/Y[deg]:%.2f,%.2f,%.2f\r\n", ahrs_data->euler.roll,ahrs_data->euler.pitch, ahrs_data->euler.yaw);
    printf("Quaternian:%.4f,%.4f,%.4f,%.4f\r\n", ahrs_data->q.x, ahrs_data->q.y, ahrs_data->q.z, ahrs_data->q.w);
}