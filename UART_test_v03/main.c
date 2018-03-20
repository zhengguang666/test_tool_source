#include"adv_uart.h"      

#define HARD 1
#define SOFTE 2

#define READ 11
#define WRITE 12
#define LOOP 13

#define    COLOR_NONE                    "\033[0m"
#define     FONT_COLOR_RED             "\033[0;31m"
#define p_err(s) printf(FONT_COLOR_RED s COLOR_NONE)

int N=1;

void my_write(fd,f_mode)
{
	int len=0;
	char send_buf[20]="1234567890abcdefghij";  
	while(1){
		if(f_mode == 0) //无流控，sleep N 
			sleep(N);  
		
		len = UART_Send(fd,send_buf,20);  
		if(len > 0){  
			if(DEBUG)printf(" %d sssssssssssssssend%dsssssssssssssssend data successful\n",len,f_mode);  
		}else{  
			if(DEBUG)p_err("ssssssffffffffffffffffffffffsend data failed!\n");  
		}

	}

}


void my_read(fd,f_mode)
{
	int len=0;
	char rcv_buf[30];         
	while(1){
		if(f_mode != 0)
			sleep(1);//有流控，sleep 1
		else
			sleep(N);
		memset(rcv_buf,'\0',30);
		len = UART_Recv(fd, rcv_buf,30);  
		if(len > 0)  
		{  
			rcv_buf[len] = '\0';  
			if(DEBUG)printf("rrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrreceive data is %s\n",rcv_buf);
		}	  
		else  
		{  
			if(DEBUG)p_err("cannot receive data\n");  
		}  
	}
}

int main(int argc, char **argv)  
{  
	int fd;                            //文件描述符  
	FILE *fp;
	int err;                           //返回调用函数的状态  
	int len;                          
	int pid; 
	if(argc == 1)  
	{  
		printf("Usage: %s  /dev/ttySn -b speed [options] .more info try: %s -h\n",argv[0],argv[0]);  
		return FALSE;  
	}  

	int opt;  
	char *optstring = "rslf:hp:b:t:";  
	int arg;
	int u_mode=0;
	int f_mode=0;
	char* portname;
	char* b;

	while ((opt = getopt(argc, argv, optstring)) != -1)  
	{  
		switch (opt){
		case 't':
			N = atoi(optarg);//u_mode = READ;
			break;
		case 'b':
			b = optarg;//u_mode = READ;
			break;
		case 'r':
			u_mode = READ;
			break;
		case 's':
			u_mode = WRITE;
			break;
		case 'l':
			u_mode = LOOP;
			break;
		case 'f':
			arg = optarg[0];
			if (arg == 'h')
				f_mode = HARD;
			else if (arg == 's')
				f_mode = SOFTE;
			break;
		case 'h':
			printf("Usage: %s  /dev/ttySn -b speed [options]\n",argv[0]);
			p_err(  "-h    show this help page.\n"
					"-b    baudrate.\n"
					"-r    receive.\n"
					"-s    send.\n"
					"-l    loop.\n"
					"-t    sleep time.before send and receive sleep N secend.(default N=1)\n"
					"-f    chose flow control mode.\n"
							"\t\t'hard'---hardware flow control\n"
							"\t\t'soft'---software flow control\n");
			exit(0);
		}
	}  

	//打开串口，返回文件描述符
	portname = argv[optind];
	fd = UART_Open(portname);   
	if (FALSE == fd ) 
		printf("open err :%s!!\n",portname);
	else 
		printf("%s opened.\n",portname);

	//设置串口
	err = UART_Set(fd,atoi(b),f_mode,8,1,'N');
	if (FALSE == err)
		p_err("uart set err !!\n");
	else
		printf("Set Port OK!\n");
	



	switch(u_mode) {
		case READ:
			my_read(fd,f_mode);
			break;
		case WRITE:
			my_write(fd,f_mode);
			break;
		case LOOP:
			pid=fork();
			if(pid == 0){
				my_write(fd,f_mode);
			}
			else{
				my_read(fd,f_mode);
			}
			break;
		default:
			p_err("error mode!\n");
	}
	UART_Close(fd);   
	return 0;
} 

