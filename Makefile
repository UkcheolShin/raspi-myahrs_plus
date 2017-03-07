obj   := imu.c myahrs_plus.c
obj-out := 2_imu.out

all :
        gcc $(obj) -o $(obj-out)
clean :
        rm 2_imu.out
