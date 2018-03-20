all:
	${CROSS_COMPILE}gcc -g -static ${CFLAGS} ${LDFLAGS} -o uart_test uart_test.c serial.c -lpthread -lrt
clean:
	rm -rf uart_test

