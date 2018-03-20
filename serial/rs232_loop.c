#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <sys/ioctl.h>


#define SleepDelayUS 100000
#define TEST_LEN 50

int main (int argc, char *argv[])
{
	int k;
	int BaudSpeed;
	struct termios options[2];
	char buf1[TEST_LEN], buf2[TEST_LEN];
	int passflag;
	int bytes;
	int fd[2];
	int loop=100;
	int ret;
	
	passflag=0;
	BaudSpeed =B115200;
	if (argc != 4)
	{
		printf("Usage:serial_loop /dev/ttyS0 /dev/ttyS1 115200\n");
		goto ret;
	}
	else
	{
		switch (atoi(argv[3])){
		case 115200:
			BaudSpeed =B115200;
			break;
		case 57600:
			BaudSpeed =B57600;
			break;
		case 38400:
			BaudSpeed =B38400;
			break;
		case 19200:
			BaudSpeed =B19200;
			break;
		case 9600:
			BaudSpeed =B9600;
			break;
		case 2400:
			BaudSpeed =B2400;
			break;
		case 1800:
			BaudSpeed =B1800;
			break;
		case 600:
			BaudSpeed =B600;
			break;
		default: 
			BaudSpeed =B115200;
			break;
		}		
	}       
	
	fd[0] = open(argv[1],O_RDWR | O_NOCTTY | O_NDELAY);
	if (fd[0] == -1)
	{
		printf("Open %s Failed\n",argv[1]);
		goto ret;
	}
	ret = tcgetattr(fd[0], &options[0]);
	ret = tcflush( fd[0], TCIOFLUSH );
	ret = cfsetispeed(&options[0], BaudSpeed);
	ret = cfsetospeed(&options[0], BaudSpeed);
	options[0].c_cflag |= ( CLOCAL | CREAD);
	options[0].c_cflag &= ~PARENB;
	options[0].c_cflag &= ~CSTOPB;
	options[0].c_cflag &= ~CSIZE;
	options[0].c_cflag |= CS8 | CRTSCTS;
	options[0].c_cflag &= ~CRTSCTS;
	options[0].c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
	options[0].c_iflag &= ~INPCK;
	options[0].c_iflag &= ~(IXON | IXOFF | IXANY);
	options[0].c_oflag &= ~OPOST;
	options[0].c_cc[VTIME] = 0;
	options[0].c_cc[VMIN] = 0;
	ret = tcsetattr(fd[0], TCSANOW, &options[0]);
	if(ret != 0)
	{
		printf("Setting %s Failed:%d\n",argv[1],ret);
		close(fd[0]);
		goto ret;
	}

	fd[1] = open(argv[2],O_RDWR | O_NOCTTY | O_NDELAY);
	if (fd[1] == -1)
	{
		printf("Open %s Failed\n",argv[2]);
		close(fd[0]);
		goto ret;
	}
	tcgetattr(fd[1], &options[1]);
	tcflush( fd[1], TCIOFLUSH );
	cfsetispeed(&options[1], BaudSpeed);
	cfsetospeed(&options[1], BaudSpeed);
	options[1].c_cflag |= ( CLOCAL | CREAD);
	options[1].c_cflag &= ~PARENB;
	options[1].c_cflag &= ~CSTOPB;
	options[1].c_cflag &= ~CSIZE;
	options[1].c_cflag |= CS8 | CRTSCTS;
	options[1].c_cflag &= ~CRTSCTS;
	options[1].c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
	options[1].c_iflag &= ~INPCK;
	options[1].c_iflag &= ~(IXON | IXOFF | IXANY);
	options[1].c_oflag &= ~OPOST;
	options[1].c_cc[VTIME] = 0;
	options[1].c_cc[VMIN] = 0;
	if(tcsetattr(fd[1], TCSANOW, &options[1])!= 0)
	{
		printf("Setting %s Failed\n",argv[2]);
		close(fd[1]);
		close(fd[0]);
		goto ret;
	}   

#if 1
	printf("Test %s --> %s\n",argv[1],argv[2]);
	memcpy(buf1,"5523456789-_=+,./?[]{}`~|mnbvcxzasdfghjklpoiuytrwq",TEST_LEN);
	buf1[TEST_LEN-1] = '\0';
	memset(buf2,0,TEST_LEN);
	int wordsWritten = write(fd[0], buf1, TEST_LEN);
	if (wordsWritten == -1)
	{
		printf("Writing Error\n");
		close(fd[1]);
		close(fd[0]);
		goto ret;
	}
#endif
#if 1
	bytes = 0;
	do{
		fd_set fs_read;
		int fs_sel = 0;
		struct timeval time;
		
		FD_ZERO(&fs_read);
		FD_SET(fd[1], &fs_read);
		time.tv_sec  = 5;	
		time.tv_usec = 0;
		fs_sel=select(fd[1]+1, &fs_read, NULL, NULL, &time);
		if(fs_sel)
		{ 
			bytes += read(fd[1], buf2+bytes, TEST_LEN);
		}
	}while(--loop && (bytes != TEST_LEN));
	buf2[TEST_LEN-1] = '\0';
#endif
#if 1
	if ((strcmp(buf1, buf2) != 0))
	{
		close(fd[0]);
		close(fd[1]);
		printf("%s ->%s loop error\n",argv[1],argv[2]);
		printf("receive:%s\n",buf2);
		goto ret;
	}
	printf("Test %s --> %s\n",argv[2],argv[1]);
	memcpy(buf1,"lpoiuytrwqmnbvcxzasdfghjk-_=+,./?[]{}0123456789`~|",TEST_LEN);
	buf1[TEST_LEN-1] = '\0';
	memset(buf2,0,TEST_LEN);
	wordsWritten = write(fd[1], buf1, TEST_LEN);
	if (wordsWritten == -1)
	{
		printf("Writing Error\n");
		close(fd[1]);
		close(fd[0]);
		goto ret;
	}

	bytes = 0;
	loop = 100;
	do{
		fd_set fs_read;
		int fs_sel = 0;
		struct timeval time;
		
		FD_ZERO(&fs_read);
		FD_SET(fd[0], &fs_read);
		time.tv_sec  = 5;	
		time.tv_usec = 0;
		fs_sel=select(fd[0]+1, &fs_read, NULL, NULL, &time);
		if(fs_sel)
		{ 
			bytes += read(fd[0], buf2+bytes, TEST_LEN);
		}
	}while(--loop && (bytes != TEST_LEN));
	buf2[TEST_LEN-1] = '\0';
#endif
	if ((strcmp(buf1, buf2) == 0))
	{
		passflag = 1;
	}
	else
	{
		printf("%s ->%s loop error\n",argv[2],argv[1]);
	       printf("receive:%s\n",buf2);
	}

	close(fd[0]);
	close(fd[1]);
ret:
	if ( passflag == 1 )
	{
		printf("UART Loop Testing PASS\n");
	}
	else
	{
		printf("UART Loop Testing FAIL\n"); 
	}
	return 0;
}
