obj   := imu.c myahrs_plus.c
obj-out := 2_imu_example.out

all :
	gcc $(obj) -o $(obj-out)
clean :
	rm *.out
	rm *.o
