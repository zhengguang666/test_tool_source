#CROSS=arm-none-linux-gnueabi-

CURRENT_DIR=$(PWD)

#LINUX_DIR=$(CURRENT_DIR)/../../../linux/kernel-3.0.8
LINUX_DIR=/home/michael/work/trunk/source/linux-2.6.32-2hsoc

all: fb_test fb_display fb_display_bmp

fb_test:fb_test.c
	 ${CC_1B} -static -o fb_test fb_test.c
fb_display_bmp:fb_display_bmp.c bmp_reader.c
	 ${CC_1B} -static -o fb_display_bmp fb_display_bmp.c bmp_reader.c
fb_display:fb_display.c
	 ${CC_1B} -static -o fb_display fb_display.c
clean:
	rm -f fb_test fb_display fb_display_bmp
