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

int speed_arr[] =
{
	B115200,
	B38400,
	B19200,
	B9600,
	B4800,
	B2400,
	B1200,
	B300,
	B38400,
	B19200,
	B9600,
	B4800,
	B2400,
	B1200,
	B600,
};
int name_arr[] =
{
	115200,
	38400,
	19200,
	9600,
	4800,
	2400,
	1200,
	300,
	38400,
	19200,
	9600,
	4800,
	2400,
	1200,
	600,
};

void set_speed( int fd, int speed )
{
	int i;
	int status;
	struct termios Opt;

	//printf("speed:%d\n",speed);
	tcgetattr( fd, &Opt );
	for ( i = 0; i < sizeof( speed_arr ) / sizeof( int ); i++ )
	{
		if ( speed == name_arr[i] )
		{
			tcflush( fd, TCIOFLUSH );    
			cfsetispeed( &Opt, speed_arr[i] ); 
			cfsetospeed( &Opt, speed_arr[i] );  
			status = tcsetattr( fd, TCSANOW, &Opt ); 
			if ( status != 0 )
			{
				perror( "tcsetattr fd" ); 
				return;
			}   
			tcflush( fd, TCIOFLUSH );
			break;
		}
	}
	if (i >= sizeof(speed_arr)/sizeof(int))
		printf("speed not support,please check\n");
}

int set_Parity( int fd, int databits, int stopbits, int parity )
{
	struct termios options;
	if ( tcgetattr( fd, &options ) != 0 )
	{
		perror( "SetupSerial 1" );    
		return -1;
	}
	options.c_cflag &= ~CSIZE;
	options.c_lflag &= ~( ICANON | ECHO | ECHOE | ISIG );  /*Input*/
	options.c_oflag &= ~OPOST;   /*Output*/

	switch ( databits )
	{
	case 7:
		options.c_cflag |= CS7;
		break;
	case 8:
		options.c_cflag |= CS8;
		break;  
	default:
		fprintf( stderr, "Unsupported data size/n" );
		return -1;
	}
	switch ( parity )
	{
	case 'n':
	case 'N':
		options.c_cflag &= ~PARENB;   /* Clear parity enable */
		options.c_iflag &= ~INPCK;     /* Enable parity checking */
		break; 
	case 'o':
	case 'O':
		options.c_cflag |= ( PARODD | PARENB ); 
		options.c_iflag |= INPCK;             /* Disnable parity checking */
		break; 
	case 'e':
	case 'E':
		options.c_cflag |= PARENB;     /* Enable parity */   
		options.c_cflag &= ~PARODD;  
		options.c_iflag |= INPCK;       /* Disnable parity checking */
		break;
	case 'S':
	case 's':
		/*as no parity*/  
		options.c_cflag &= ~PARENB;
		options.c_cflag &= ~CSTOPB;
		break; 
	default:
		fprintf( stderr, "Unsupported parity/n" );   
	return -1;
	} 

	switch ( stopbits )
	{
	case 1:
		options.c_cflag &= ~CSTOPB; 
		break; 
	case 2:
		options.c_cflag |= CSTOPB; 
		break;
	default:
		fprintf( stderr, "Unsupported stop bits/n" ); 
	return -1;
	}
	/* Set input parity option */
	if ( parity != 'n' )
	{
		options.c_iflag |= INPCK;
	}

	tcflush( fd, TCIFLUSH );

	options.c_cc[VTIME] = 0; 

	options.c_cc[VMIN] = 0; /* define the minimum bytes data to be readed*/

	if ( tcsetattr( fd, TCSANOW, &options ) != 0 )
	{
		perror( "SetupSerial 3" );  
		return -1;
	}
	return 0;
}

int main(int argc, char **argv)
{
	int ret;
	int fd;
	unsigned char buff[50];
	int nread;
	unsigned int speed;
	int i;
	int tmp;
	char parity;

	printf("uart serial port test.\n");

	if (argc < 4)
	{
		printf("usage:%s devname \n",argv[0]);
		printf("eg: %s /dev/ttyS0 115200 e\n",argv[0]);
		return 1;
	}

	fd = open( argv[1], O_RDWR );
	if ( -1 == fd )
	{
		printf( "Can't Open Serial Port %s ",argv[1]);
		return -1;
	}
	speed = atoi(argv[2]);
	parity = argv[3][0];
	printf("speed:%d,parity:%c\n",speed,parity);
	set_speed( fd, speed );
	if ( set_Parity( fd, 7, 2, parity ) == -1 )
	{
		printf( "Set Parity Error/n" );
		return -1;
	}
	printf("setting finish,start receive and echo\n");
	tmp = argc - 4;
	for(i=0;i<tmp;i++)
	{
		buff[i] = atoi(argv[i+4]);
	}
	//buff[4] = '\n';
	//printf("write:%s\n",buff);
	write(fd,buff,tmp);
	//nread = read(fd, buff, 5);
	//sleep(1);
	memset(buff,0,50);
#if 1
	while(1){
		fd_set fs_read;
		int fs_sel = 0;
		struct timeval time;
		
		FD_ZERO(&fs_read);
		FD_SET(fd, &fs_read);

		time.tv_sec  = 10;		//vic
		time.tv_usec = 0;
#endif
		fs_sel=select(fd+1, &fs_read, NULL, NULL, &time);
		printf("fs sel %d\n", fs_sel);
		if(fs_sel)
		{ 
			nread = read(fd, buff, 50);
		//while((nread = read(fd, buff, 5))==0);
			printf("Len %d : ",nread);
			for(i=0;i<nread;i++)
				printf( "0x%x ", buff[i]);  
			printf("\n");
		}
	}
	printf("\n");
	close( fd );
	return 0;
}
