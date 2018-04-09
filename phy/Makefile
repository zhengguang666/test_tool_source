EXEC = phyreg
OBJS = phyreg.o
#HEADERS = uart_api.h

all:${EXEC}
${EXEC} : ${OBJS}
	${CC} ${CFLAGS} ${LDFLAGS} -o $@ ${OBJS} 
${OBJS2} : ${HEADERS}

.PHONY : clean
clean :
	-rm -f ${OBJS} ${EXEC}
