LINUX_DIR=../../linux-socfpga

all:serial_test rs232_loop rs232_test

serial_test: serial_test.c
	${CROSS_COMPILE}gcc -static -o serial_test serial_test.c -I${LINUX_DIR}/include -I${LINUX_DIR}/arch/arm/include

rs232_loop: rs232_loop.c
	${CROSS_COMPILE}gcc -static -o rs232_loop rs232_loop.c -I${LINUX_DIR}/include -I${LINUX_DIR}/arch/arm/include

rs232_test: rs232_test.c
	${CROSS_COMPILE}gcc -static -o rs232_test rs232_test.c -I${LINUX_DIR}/include -I${LINUX_DIR}/arch/arm/include

clean:
	rm -f serial_test rs232_loop rs232_test *.o
