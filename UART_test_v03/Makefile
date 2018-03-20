#CROSS_COMPILE=/home/root_work/GF06-main/android/prebuilts/gcc/linux-x86/arm/arm-linux-androideabi-4.7/bin/arm-linux-androideabi-
#CROSS_COMPILE=/home/root_work/AM335X_v02.00.00.00.00/cross_compiler/linux-devkit/sysroots/x86_64-arago-linux/usr/bin/arm-linux-gnueabihf-
CROSS_COMPILE=arm-none-linux-gnueabi-
CC = gcc
FILE = UART_test

all: 
	$(CROSS_COMPILE)gcc -static -o $(FILE) main.c UART.c 


clean:
	rm -rf $(FILE) *~
