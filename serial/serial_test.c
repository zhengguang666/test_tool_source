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

int main (int argc, char *argv[])
{
	int i;
	int BaudSpeed;
	struct termios options[1];
	char *buf1, *buf2;
	int passflag;
	int bytes;
	int fd[1];
	int loop=10;
	int ret;
	int basechar = 0x13;
	int stepcount = 15;
	int test_len = 150;
	char flow;

	passflag=0;
	BaudSpeed =B115200;
	if (argc != 6)
	{
		printf("Usage:%s /dev/ttyS0 115200 h\/s\/n len rec_delay_time\n",argv[0]);
		return 0;
	}
	else
	{
		switch (atoi(argv[2])){
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
		flow = argv[3][0];
		test_len = atoi(argv[4]);
		loop = atoi(argv[5]);
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
	options[0].c_cflag |= CS8;
	options[0].c_cflag &= ~CRTSCTS;
	options[0].c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
	options[0].c_iflag &= ~INPCK;
	options[0].c_iflag &= ~(IXON | IXOFF | IXANY);
	options[0].c_oflag &= ~OPOST;
	options[0].c_cc[VTIME] = 0;
	options[0].c_cc[VMIN] = 0;
	if('h' == flow)
	{
		options[0].c_cflag |= CRTSCTS;
	}
	else if('s' == flow)
	{
		options[0].c_iflag |= (IXON | IXOFF);
	}
	ret = tcsetattr(fd[0], TCSANOW, &options[0]);
	if(ret != 0)
	{
		printf("Setting %s Failed:%d\n",argv[1],ret);
		close(fd[0]);
		goto ret;
	}

	//printf("Test send...\n");
	buf1 = malloc(test_len);
	buf2 = malloc(test_len);
	if(!buf1 || !buf2)
		goto ret;
	basechar = 65;
	stepcount = 58;
	for(i = 0; i < test_len; i++)
		buf1[i] = basechar + (i % stepcount);
	buf1[test_len-1] = '\0';
	printf("send:%s\n",buf1);
	memset(buf2,0,test_len);
	int wordsWritten = write(fd[0], buf1, test_len);
	if (wordsWritten == -1)
	{
		printf("Writing Error\n");
		close(fd[0]);
		goto ret;
	}

	//printf("Test receive...\n");
	bytes = 0;
	do{
		fd_set fs_read;
		int fs_sel = 0;
		struct timeval time;
		
		FD_ZERO(&fs_read);
		FD_SET(fd[0], &fs_read);
		time.tv_sec  = 1;	
		time.tv_usec = 0;
		fs_sel=select(fd[0]+1, &fs_read, NULL, NULL, &time);
		if(fs_sel > 0)
		{ 
			bytes += read(fd[0], buf2+bytes, test_len-bytes);
		}
	}while(--loop && (bytes < test_len));
	buf2[test_len-1] = '\0';
	printf("rece:%s\n",buf2);
		
	if ((strcmp(buf1, buf2) == 0))
	{
		passflag = 1;
	}

	free(buf1);
	free(buf2);
	close(fd[0]);
ret:
	if ( passflag == 1 )
	{
		printf("UART Testing PASS\n");
	}
	else
	{
		printf("UART Testing FAIL\n"); 
	}
	return 0;
}
