#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdint.h>

#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include "myahrs_plus.h"

int I2C_MYAHRS_READ(int fd, unsigned char type, ahrs_data_t * ahrs_data)
{
    unsigned char temp_buf[35]={0,};
    unsigned char reg = 0;
    int ret=0;
    ahrs_data_t *p_ahrs_data = (ahrs_data_t *)ahrs_data;

#ifdef DEBUG
    int i=0;
#endif

    printf("\r\nread I2C MY AHRS+ DATA. \r\n");

    switch(type){
        case STATE_CHECK :
            reg = REG_WHO_AM_I;
            if((ret = write(fd,&reg,1))<0){
                perror("write");
                return -1;
            }
            if((ret = read(fd,temp_buf,4)) < 0){
                perror("read");
                return -1;
            }
            // 0xB1이 출력되어야 정상이다. 
            printf("WHO AM I : 0x%02X\r\n",temp_buf[0]);
            printf("FIRMWARE VERSION : %02d.%02d \r\n",temp_buf[1],temp_buf[2]);
            printf("REG_STATUS : 0x%02X\r\n",temp_buf[3]);
            break;

        case RAW_DATA :
            reg = REG_I_ACC_X_LOW;
            if((ret = write(fd,&reg,1))<0){
                perror("write");
                return -1;
            }

            if((ret = read(fd,temp_buf,18)) < 0){
                perror("read");
                return -1;
            }

#ifdef DEBUG
            while(i++ != 18)
                printf("%d 0x%2x\t",(i-1),temp_buf[i-1]);
            printf("\n");
            i=0;
#endif
            /*비트 연산 후 singed short(16bit)로 캐스팅 후 곱,나눗셈 연산을 위해 float 캐스팅*/
            p_ahrs_data->sensor.acc_x    = (float)(signed short)((temp_buf[1]<<8)|temp_buf[0]) * 16.0 / 32767.;
            p_ahrs_data->sensor.acc_y    = (float)(signed short)((temp_buf[3]<<8)|temp_buf[2]) * 16.0 / 32767.;
            p_ahrs_data->sensor.acc_z    = (float)(signed short)((temp_buf[5]<<8)|temp_buf[4]) * 16.0 / 32767.;
            
            p_ahrs_data->sensor.gyro_x   = (float)(signed short)((temp_buf[7]<<8)|temp_buf[6]) * 2000. / 32767.;
            p_ahrs_data->sensor.gyro_y   = (float)(signed short)((temp_buf[9]<<8)|temp_buf[8]) * 2000. / 32767.;
            p_ahrs_data->sensor.gyro_z   = (float)(signed short)((temp_buf[11]<<8)|temp_buf[10]) * 2000. / 32767.;
            
            p_ahrs_data->sensor.magnet_x = (float)(signed short)((temp_buf[13]<<8)|temp_buf[12]) * 0.3;
            p_ahrs_data->sensor.magnet_y = (float)(signed short)((temp_buf[15]<<8)|temp_buf[14]) * 0.3;
            p_ahrs_data->sensor.magnet_z = (float)(signed short)((temp_buf[17]<<8)|temp_buf[16]) * 0.3;
            
            reg = REG_TEMPERATURE_LOW;
            if((ret = write(fd,&reg,1))<0){
                perror("write");
                return -1;
            }
            if((ret = read(fd,temp_buf,16)) < 0){
                perror("read");
                return -1;
            }

#ifdef DEBUG
            while(i++ != 16)
                printf("%d 0x%2x\t",(i-1),temp_buf[i-1]);
            printf("\n");
            i=0;
#endif
            p_ahrs_data->temp           = (float)(signed short)((temp_buf[1]<<8)|temp_buf[0]) * 200. / 32767.;
            p_ahrs_data->euler.roll     = (float)(signed short)((temp_buf[3]<<8)|temp_buf[2]) * 180. / 32767.;
            p_ahrs_data->euler.pitch    = (float)(signed short)((temp_buf[5]<<8)|temp_buf[4]) * 180. / 32767.;
            p_ahrs_data->euler.yaw      = (float)(signed short)((temp_buf[8]<<8)|temp_buf[6]) * 180. / 32767.;
            p_ahrs_data->q.x            = (float)(signed short)((temp_buf[10]<<8)|temp_buf[8]) / 32767.;
            p_ahrs_data->q.y            = (float)(signed short)((temp_buf[11]<<8)|temp_buf[10]) / 32767.;
            p_ahrs_data->q.z            = (float)(signed short)((temp_buf[13]<<8)|temp_buf[12]) / 32767.;
            p_ahrs_data->q.w            = (float)(signed short)((temp_buf[15]<<8)|temp_buf[14]) / 32767.;
#ifdef PRINT
            printf("ACC[g]:%.2f,%.2f,%.2f\r\n", p_ahrs_data->sensor.acc_x, p_ahrs_data->sensor.acc_y, p_ahrs_data->sensor.acc_z);
            printf("GYRO[deg/s]:%.2f,%.2f,%.2f\r\n", p_ahrs_data->sensor.gyro_x, p_ahrs_data->sensor.gyro_y, p_ahrs_data->sensor.gyro_z);
            printf("MAG[uT]:%.2f,%.2f,%.2f\r\n", p_ahrs_data->sensor.magnet_x, p_ahrs_data->sensor.magnet_y, p_ahrs_data->sensor.magnet_z);
            printf("Temperature[degC]:%.2f\r\n", p_ahrs_data->temp);
            printf("R/P/Y[deg]:%.2f,%.2f,%.2f\r\n", p_ahrs_data->euler.roll,p_ahrs_data->euler.pitch, p_ahrs_data->euler.yaw);
            printf("Quaternian:%.4f,%.4f,%.4f,%.4f\r\n", p_ahrs_data->q.x, p_ahrs_data->q.y, p_ahrs_data->q.z, p_ahrs_data->q.w);
#endif
            break;
   
        case CAL_DATA : 
            /*0x22 ~ 0x43까지 모두 읽어옴*/
            reg = REG_C_ACC_X_LOW;
            if((ret = write(fd,&reg,1))<0){
                perror("write");
                return -1;
            }
            if((ret = read(fd,temp_buf,34)) < 0){
                perror("read");
                return -1;
            }
    #ifdef DEBUG
                while(i++ != 34)
                    printf("reg 0x%x: \t0x%2x\n",0x22+(i-1),temp_buf[i-1]);
                printf("\n");
                i=0;
    #endif
                /*비트 연산 후 singed short(16bit)로 캐스팅 후 곱,나눗셈 연산을 위해 float 캐스팅*/
                p_ahrs_data->sensor.acc_x   = (float)(signed short)((temp_buf[1]<<8)|temp_buf[0]) * 16.0 / 32767.;
                p_ahrs_data->sensor.acc_y   = (float)(signed short)((temp_buf[3]<<8)|temp_buf[2]) * 16.0 / 32767.;
                p_ahrs_data->sensor.acc_z   = (float)(signed short)((temp_buf[5]<<8)|temp_buf[4]) * 16.0 / 32767.;
                
                p_ahrs_data->sensor.gyro_x  = (float)(signed short)((temp_buf[7]<<8)|temp_buf[6]) * 2000. / 32767.;
                p_ahrs_data->sensor.gyro_y  = (float)(signed short)((temp_buf[9]<<8)|temp_buf[8]) * 2000. / 32767.;
                p_ahrs_data->sensor.gyro_z  = (float)(signed short)((temp_buf[11]<<8)|temp_buf[10]) * 2000. / 32767.;
                
                p_ahrs_data->sensor.magnet_x = (float)(signed short)((temp_buf[13]<<8)|temp_buf[12]) * 0.3;
                p_ahrs_data->sensor.magnet_y = (float)(signed short)((temp_buf[15]<<8)|temp_buf[14]) * 0.3;
                p_ahrs_data->sensor.magnet_z = (float)(signed short)((temp_buf[17]<<8)|temp_buf[16]) * 0.3;

                p_ahrs_data->temp           = (float)(signed short)((temp_buf[19]<<8)|temp_buf[18]) * 200. / 32767.;

                p_ahrs_data->euler.roll     = (float)(signed short)((temp_buf[21]<<8)|temp_buf[20]) * 180. / 32767.;
                p_ahrs_data->euler.pitch    = (float)(signed short)((temp_buf[23]<<8)|temp_buf[22]) * 180. / 32767.;
                p_ahrs_data->euler.yaw      = (float)(signed short)((temp_buf[25]<<8)|temp_buf[24]) * 180. / 32767.;
                
                p_ahrs_data->q.x            = (float)(signed short)((temp_buf[27]<<8)|temp_buf[26]) / 32767.;
                p_ahrs_data->q.y            = (float)(signed short)((temp_buf[29]<<8)|temp_buf[28]) / 32767.;
                p_ahrs_data->q.z            = (float)(signed short)((temp_buf[31]<<8)|temp_buf[30]) / 32767.;
                p_ahrs_data->q.w            = (float)(signed short)((temp_buf[33]<<8)|temp_buf[32]) / 32767.;
#ifdef PRINT
                printf("ACC[g]:%.2f,%.2f,%.2f\r\n", p_ahrs_data->sensor.acc_x, p_ahrs_data->sensor.acc_y, p_ahrs_data->sensor.acc_z);
                printf("GYRO[deg/s]:%.2f,%.2f,%.2f\r\n", p_ahrs_data->sensor.gyro_x, p_ahrs_data->sensor.gyro_y, p_ahrs_data->sensor.gyro_z);
                printf("MAG[uT]:%.2f,%.2f,%.2f\r\n", p_ahrs_data->sensor.magnet_x, p_ahrs_data->sensor.magnet_y, p_ahrs_data->sensor.magnet_z);
                printf("Temperature[degC]:%.2f\r\n", p_ahrs_data->temp);
                printf("R/P/Y[deg]:%.2f,%.2f,%.2f\r\n", p_ahrs_data->euler.roll,p_ahrs_data->euler.pitch, p_ahrs_data->euler.yaw);
                printf("Quaternian:%.4f,%.4f,%.4f,%.4f\r\n", p_ahrs_data->q.x, p_ahrs_data->q.y, p_ahrs_data->q.z, p_ahrs_data->q.w);
#endif
            break;

        default :
            printf("unvaild type value! select RAW_DATA Or CAL_DATA!\n");
            ret = -1;
            break;
    }
    return ret;
}


int Read(int fd, unsigned char * buf, unsigned int size, unsigned char reg)
{
    unsigned char * temp_buf = buf;
    unsigned char tmp_reg = reg;
    int ret=0;

    if(size < 1){
        printf("unvaild size!! \n");
        return -1;
    }else{
        printf("\r\nread I2C MY AHRS+ DATA. \r\n");
    
        if((ret = write(fd,&tmp_reg,1)) < 0){
            perror("write");
            return -1;
        }
    
        if((ret = read(fd,temp_buf,size)) < 0){
            perror("read");
            return -1;
        }

        return ret;
    } 
}