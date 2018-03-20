#include "adv.h"

#define RECV_TIMEOUT	3  // ms
enum interaction {
	IA_ACTIVE = 0, // active will send anyway
	IA_PASSIVE,	// passive will send after first incoming character
	IA_LOOP
};

int f_send = 0, f_recv = 0, baudrate = 230400;
int f_verify = 0, f_output = 0;
int f_dataerror = 0;
enum interaction f_interaction = IA_ACTIVE;
enum parity f_parity = PARITY_NONE;
int databit = 8, stopbit = 1;
enum flowcontrol f_flowcontrol = FC_NONE;
enum mode f_mode = MODE_RS232;
int pause_period = 20;
int packet_size = MAX_DATA_LEN;
int bRunning = 0;
int f_firstchar = 0;
int f_stopOnError = 0;
int stop_time = 10;
int first_send_sleep_time=5;
int end_recv_sleep_time=5;
FILE *send_file=NULL;
FILE *recv_file=NULL;

unsigned char datamask = 0xFF;
unsigned long long SendThroughput = 0;
unsigned long long RecvThroughput = 0;
unsigned long long SendTotal = 0;
unsigned long long RecvTotal = 0;
unsigned long long SendElapsed = 0;
unsigned long long RecvElapsed = 0;
unsigned char WriteData[MAX_DATA_LEN];
int basechar = 0x13;
int stepcount = 15;
int timeoutcount = 0;
struct timespec starttime;
struct timespec nowtime;
struct timespec recvstarttime;
struct timespec recvnowtime;
struct timespec sendstarttime;
struct timespec sendnowtime;
struct serial_icounter_struct icount;

char *ProgName = 0;

void StopTime(void)
{
	sleep(stop_time-end_recv_sleep_time);
	f_send=0;
	sleep(end_recv_sleep_time);
	bRunning = 0;	
}

void *SendThread(void *param)
{
	fd_set fds;
	struct timeval waitTime;
	int WrittenSize = 0, Size;
	unsigned char flagstart = 0;
	struct timespec sendstarttime;
	struct timespec sendnowtime;
	int SendPortHandle = (int)param;
	int sleep_period = pause_period * 1000;
	int ret;
	
	sleep(first_send_sleep_time);
	while(bRunning && f_send) {
		if(f_interaction && !f_firstchar) {
			usleep(10000);
			continue;
		}

		FD_ZERO(&fds);
		FD_SET(SendPortHandle, &fds);
		waitTime.tv_sec = 0;
//		waitTime.tv_usec = RECV_TIMEOUT * 100000;
		waitTime.tv_usec = (((packet_size * 10 * 1000) / baudrate) + 10) * 1000;
		if(select(SendPortHandle + 1, 0, &fds, 0, &waitTime) > 0) {
			if(!flagstart) {
				clock_gettime(CLOCK_REALTIME, &sendstarttime);
				flagstart = 1;
			}

   			Size = write(SendPortHandle,
   				&WriteData[WrittenSize],
   				packet_size - WrittenSize);
	   		if(Size < 0) {
	   			printf("Thread 1 write failed!\n");
	   			goto ENDSENDCLOSE;
	   		}
			if(Size != packet_size)
				debug_printf("SEND:%d\n", Size);
//			tcdrain(SendPortHandle);
			
		   	SendTotal += Size;
			clock_gettime(CLOCK_REALTIME, &sendnowtime);
			SendElapsed = sendnowtime.tv_sec - sendstarttime.tv_sec;
			if(SendElapsed > 0)
				SendThroughput = SendTotal/SendElapsed;

			if(send_file) {
				ret = 0;
				ret = fwrite(&WriteData[WrittenSize],1,Size,send_file);
				if(ret < Size)
					printf("save send data to file error\n");
			}
			WrittenSize += Size;
			if(WrittenSize == packet_size)
				WrittenSize = 0;

			if(sleep_period)
				usleep(sleep_period);
	   	} else
	   		timeoutcount++;
	}

ENDSENDCLOSE:
	f_send = 0;
	tcflush(SendPortHandle, TCOFLUSH);
	pthread_exit(0);
	return 0;
}

void *RecvThread(void *param)
{
	int ReadSize = 0;
	fd_set fds;
	struct timeval waitTime;
	char ReadData[MAX_DATA_LEN];
	int RecvPortHandle = (int)param;
	int ReadIndex, i;
	int ret;
	struct termios PortTermios;
	int status;
	
	tcgetattr(RecvPortHandle, &PortTermios);
	if (ioctl(RecvPortHandle, TIOCMGET, &status) == -1) {
		printf("RecvThread TIOCMGET fail\n");
		goto ENDRECVCLOSE;
	}
	if (PortTermios.c_cflag & CRTSCTS)
		status |= TIOCM_RTS;
	if (ioctl(RecvPortHandle, TIOCMSET, &status) == -1) {
		printf("RecvThread TIOCMSET fail\n");
		goto ENDRECVCLOSE;
	}
	while(bRunning) {
		memset(ReadData, 0, MAX_DATA_LEN);
		FD_ZERO(&fds);
		FD_SET(RecvPortHandle, &fds);
		waitTime.tv_sec = 0;
		waitTime.tv_usec = RECV_TIMEOUT * 100000;
		if(select(RecvPortHandle + 1, &fds, 0, 0, &waitTime) > 0) {
			if(!f_firstchar) {
				clock_gettime(CLOCK_REALTIME, &recvstarttime);
				f_firstchar = 1;
			}

	   		ReadSize = read(RecvPortHandle, &ReadData[0], packet_size);
	   		if(ReadSize == 0)
	   			continue;
		   	if(ReadSize < 0) {
		   		printf("Thread 2 Recv Failed!\n");
		   		goto ENDRECVCLOSE;
		   	}
			//printf("ReadSize:%d\n", ReadSize);
			if(f_verify) {
			   	ReadIndex = 0;
			   	do {
			   		if(ReadData[ReadIndex] !=
			   			((basechar + (RecvTotal % packet_size % stepcount)) & datamask)) {
			   			printf("%u, %X, %X\n",
			   				RecvTotal, ReadData[ReadIndex],
			   				(basechar + (RecvTotal % packet_size % stepcount)) & datamask);
			   			printf("ReadSize:%d\n", ReadSize);
		   				for(i = 0;i < ReadSize; i++) {
		   					printf("%02X ",
		   					ReadData[i]);
		   					if((i % 20) == 19)
		   						printf("\n");
		   				}
		   				printf("\n");
		   				fflush(stdout);
			   			f_dataerror = 1;
			   			f_verify = 0;
			   			if(f_stopOnError && f_dataerror) {
			   				bRunning = 0;
			   			}
			   		}
			   		ReadIndex++;
			   		RecvTotal++;
			   	} while ((ReadSize != ReadIndex) && f_verify);
			 } else {
				RecvTotal += ReadSize;
			}

			clock_gettime(CLOCK_REALTIME, &recvnowtime);
			RecvElapsed = recvnowtime.tv_sec - recvstarttime.tv_sec;
			if(RecvElapsed > 0)
				RecvThroughput = RecvTotal/RecvElapsed;
			if(recv_file) {
				ret = 0;
				ret = fwrite(&ReadData[0],1,ReadSize,recv_file);
				if(ret < ReadSize)
					printf("save recv data to file error\n");
			}
		}
	}

ENDRECVCLOSE:
	f_recv = 0;
	pthread_exit(0);
	return 0;
}

void *ShowThread(void *param)
{
	int PortHandle = (int)param;

	while(bRunning | f_send | f_recv) {
		if (ioctl(PortHandle, TIOCGICOUNT, &icount) < 0) {
			printf("Cannot get interrupt counters");
			close(PortHandle);
			goto END;
		}
		clock_gettime(CLOCK_REALTIME, &nowtime);
		printf("\rR %llu, %llu BPS<= (%02u:%02u:%02u) =>S %llu, %llu BPS, %d%5s",
			RecvTotal, RecvThroughput,
			(nowtime.tv_sec - starttime.tv_sec) / (60 * 60),
			((nowtime.tv_sec - starttime.tv_sec) % (60 * 60)) / 60,
			(nowtime.tv_sec - starttime.tv_sec) % 60,
			SendTotal, SendThroughput,
			timeoutcount,
			f_dataerror?", data error":"");
		fflush(stdout);
		usleep(500000);
	}

END:
	printf("\n");
	f_output = 0;
	pthread_exit(0);
	return 0;
}

void *LoopThread(void *param)
{
	int ReadSize, SendTotal, WrittenSize;
	fd_set fds;
	struct timeval waitTime;
	char ReadData[MAX_DATA_LEN];
	int PortHandle = (int)param;

	while(bRunning) {
		memset(ReadData, 0, MAX_DATA_LEN);
		FD_ZERO(&fds);
		FD_SET(PortHandle, &fds);
		waitTime.tv_sec = 0;
		waitTime.tv_usec = RECV_TIMEOUT * 100000;
		if(select(PortHandle + 1, &fds, 0, 0, &waitTime) > 0) {
	   		ReadSize = read(PortHandle, &ReadData[0], MAX_DATA_LEN);
	   		if(ReadSize == 0)
	   			continue;
		   	if(ReadSize < 0) {
		   		printf("Thread 2 Recv Failed!%d(%X)\n", ReadSize, ReadSize);
		   		goto ENDCLOSE;
		   	}

			SendTotal = 0;
			while(SendTotal != ReadSize) {
				FD_ZERO(&fds);
				FD_SET(PortHandle, &fds);
				waitTime.tv_sec = 0;
				waitTime.tv_usec = RECV_TIMEOUT * 100000;
				if(select(PortHandle + 1, 0, &fds, 0, &waitTime) > 0) {
		   			WrittenSize = write(PortHandle,
		   				&ReadData[SendTotal],
		   				ReadSize - SendTotal);
			   		if(WrittenSize < 0) {
			   			printf("Thread 1 write failed!\n");
			   			goto ENDCLOSE;
			   		}
				   	SendTotal += WrittenSize;
			   	}
			}
		}
	}

ENDCLOSE:
	pthread_exit(0);
	return 0;
}

void *RS485Thread(void *param)
{
	int ReadSize, ReadTotal, SendTotal, WrittenSize;
	fd_set fds;
	struct timeval waitTime;
	char ReadData[MAX_DATA_LEN];
	int PortHandle = (int)param;
	int sleep_period = pause_period * 1000;
	int transnit_timeout 
		= ((packet_size * 10 * 1000) / baudrate) + 10; //ms, 10 for delay
	// n81 to be 10;
	// 1s = 1000 ms;

	debug_printf("RS485Thread running!\n");
	if(f_interaction == IA_ACTIVE) {
		SendTotal = 0;
		FD_ZERO(&fds);
		FD_SET(PortHandle, &fds);
		waitTime.tv_sec = 0;
		waitTime.tv_usec = RECV_TIMEOUT * 100000;
		if(select(PortHandle + 1, 0, &fds, 0, &waitTime) > 0) {
   			WrittenSize = write(PortHandle,
   				&WriteData[SendTotal % packet_size],
   				packet_size * 2 - SendTotal);
	   		if(WrittenSize <= 0) {
	   			printf("RS485Thread write failed!\n");
	   			goto ENDCLOSE;
	   		}
		   	SendTotal += WrittenSize;
//		   	tcdrain(PortHandle);
	   	}
	}

	while(bRunning) {
		ReadTotal = 0;
		while(ReadTotal < packet_size) {
			memset(ReadData, 0, MAX_DATA_LEN);
			FD_ZERO(&fds);
			FD_SET(PortHandle, &fds);
			if(f_interaction == IA_PASSIVE) {
				waitTime.tv_sec = 100;
				waitTime.tv_usec = 0;
			} else {
				waitTime.tv_sec = 0;
				waitTime.tv_usec = transnit_timeout * 1000 * 2;
				// double transmittion time
			}
			if(select(PortHandle + 1, &fds, 0, 0, &waitTime) > 0) {
	   			if(!f_firstchar) {
					clock_gettime(CLOCK_REALTIME, &recvstarttime);
					f_firstchar = 1;
				}

				ReadSize = read(PortHandle,
					&ReadData[ReadTotal],
					packet_size - ReadTotal);
		   		if(ReadSize == 0)
		   			continue;
			   	if(ReadSize < 0) {
			   		printf("\nThread 2 Recv Failed!%d(%X)\n",
			   			ReadSize, ReadSize);
			   		goto ENDCLOSE;
			   	}
			   	ReadTotal += ReadSize;

			   	RecvTotal += ReadSize;
				if(f_output) {
					clock_gettime(CLOCK_REALTIME, &recvnowtime);
					RecvElapsed = recvnowtime.tv_sec - recvstarttime.tv_sec;
					if(RecvElapsed)
						RecvThroughput = RecvTotal/RecvElapsed;
				}
			} else {
				if(f_interaction == IA_ACTIVE) {
					timeoutcount++;
					break;
				}
			}
		}
//		usleep(2 * 1000);
		if(sleep_period)
			usleep(sleep_period);
		SendTotal = 0;
		while(SendTotal < packet_size) {
			FD_ZERO(&fds);
			FD_SET(PortHandle, &fds);
			waitTime.tv_sec = 0;
			waitTime.tv_usec = RECV_TIMEOUT * 100000;
			if(select(PortHandle + 1, 0, &fds, 0, &waitTime) > 0) {
	   			WrittenSize = write(PortHandle,
	   				&WriteData[SendTotal],
	   				packet_size - SendTotal);
		   		if(WrittenSize < 0) {
		   			printf("Thread 1 write failed!\n");
		   			goto ENDCLOSE;
		   		}
			   	SendTotal += WrittenSize;
		   	}
		}
	}

ENDCLOSE:
	pthread_exit(0);
	return 0;
}

void usage(void)
{
	printf("usage:\t %s serial-device -sSrRbBcCfFvVoOdDaApPlLmMzZkK "
		"[arg] ... \n\n", ProgName);
	printf("%s is a simple serial test program for Advantech Embedded ",ProgName);
	printf("Linux System.\n");
	printf("-s, --send\n");
	printf("\tsend specific pattern data to serial port\n");
	printf("-r, --receive\n");
	printf("\treceive data from serial port\n");
	printf("-b, --baudrate\n");
	printf("\tSet using baudrate\n");
	printf("-c, --communication\n");
	printf("\tSet communication parameter [parity | databit | stopbit]\n");
	printf("\t\tParity can be following value\n");
	printf("\t\t\t'n' means no parity\n");
	printf("\t\t\t'e' means even parity\n");
	printf("\t\t\t'o' means odd parity\n");
	printf("\t\t\t'm' means mark parity\n");
	printf("\t\t\t's' means space parity\n");
	printf("\t\tDatabit can be 5, 6, 7 or 8\n");
	printf("\t\tStopbit can be 1 or 2\n");
	printf("\t\tStopbit will be 1.5 actually, when databit is 5\n");
	printf("\t\tn81 means no parity, 8 databit, 1 stopbit\n");
	printf("Press ENTER to continue...\n");
	getchar();
//	StopTime(); //add by tingting.wu
	bRunning = 0;	
	printf("-f, --flowcontrol\n");
	printf("\tFlow control can be following value\n");
	printf("\tnone means no flow control\n");
	printf("\txonxoff means XOn/XOff software flow control\n");
	printf("\trtscts means RTS/CTS hardware flow control\n");
	printf("-v, --verify\n");
	printf("\tVerify that receive data is the same with specific pattern or not\n");
	printf("-o, --output\n");
	printf("\tOutput statistic data\n");
	printf("-d, --debug\n");
	printf("\tOutput debug messages\n");
	printf("-a, --active\n");
	printf("-p, --passive\n");
	printf("\tPassive side will wait for the first character input from active side\n");
  	printf("Press ENTER to continue...\n");
        getchar();
//	StopTime();
	printf("-l, --loopback\n");
	printf("\tSend received data back to original serial port\n");
	printf("-m, --mode\n");
	printf("\tTransmit mode will be 232, 422 or 485\n");
	printf("-k, --packet\n");
	printf("\tTransmit packet size\n");
	printf("-z, --pause\n");
	printf("\tTransmit will pause for specific ms per packet\n");
	printf("-t, --stop\n");
	printf("\tStop on error.\n");
	printf("-i, --daemon\n");
	printf("\tRun test as a daemon in the back-ground.\n");
	printf("-h");
	printf("\tTimeout\n");

	exit(0);
}

static struct option long_options[] = {
	{"send", no_argument, 0, 's'},
	{"receive", no_argument, 0, 'r'},
	{"baudrate", required_argument, 0, 'b'},
	{"communication", required_argument, 0, 'c'},
	{"flowcontrol", required_argument, 0, 'f'},
	{"verify", no_argument, 0, 'v'},
	{"output", no_argument, 0, 'o'},
	{"debug", no_argument, 0, 'd'},
	{"active", no_argument, 0, 'a'},
	{"passive", no_argument, 0, 'p'},
	{"loopback", no_argument, 0, 'l'},
	{"mode", no_argument, 0, 'm'},
	{"pause", no_argument, 0, 'z'},
	{"packet", no_argument, 0, 'k'},
	{"stop", no_argument, 0, 't'},
	{"daemon", no_argument, 0, 'i'},
	{"time",required_argument,0,'h'},
	{0, 0, 0, 0}
};

static struct FC_DATA {
	char *fc_string;
	enum flowcontrol fc;
} flowcontrolopt_arr[] = {
	{"NONE", FC_NONE},
	{"none", FC_NONE},
	{"xonxoff", FC_XONXOFF},
	{"XONXOFF", FC_XONXOFF},
	{"rtscts", FC_RTSCTS},
	{"RTSCTS", FC_RTSCTS},
	{0, FC_NONE}
};

int main(int argc, char *argv[], char *envp[])
{
	pthread_t SendThreadHandle, RecvThreadHandle,
		ShowThreadHandle, LoopThreadHandle,
		RS485ThreadHandle;
	int CreateThreadResult = -1;
	int PortHandle = -1;
	struct termios PortTermios;
	struct serial_struct serinfo;
	pthread_attr_t ThreadAttr;
	int i;
	int exitcount = 0;
	struct sched_param ThreadParam;
	int option_index = 0;
	int c;
	int f_daemon = 0;
	unsigned char tcr, cpr;
	unsigned short dlldlm;
	char send_file_name[128];
	char recv_file_name[128];
	char *temp;

	ProgName = argv[0];
	if(argc == 1)
		usage();

	// setup send data buffer
	basechar = 33;
	stepcount = 93;
	for(i = 0; i < MAX_DATA_LEN; i++)
		WriteData[i] = basechar + (i % stepcount);

	while(1) {
		c = getopt_long (argc, argv,
			"tTiIsSrRaApPb:B:c:C:f:F:vVoOdDlLm:M:z:Z:k:K:h:",
            long_options, &option_index);
		/* Detect the end of the options. */
        if (c == -1)
        	break;

		switch (c) {
			case 0:
				/* If this option set a flag, do nothing else now. */
				if (long_options[option_index].flag != 0)
					break;
				printf ("option %s", long_options[option_index].name);
				if (optarg)
					printf (" with arg %s", optarg);
				printf ("\n");
				break;
			case 't':
			case 'T':
				if (optarg)
					printf("option t sould not have extra parameter\n");
				f_stopOnError = 1;
				break;
			case 'i':
			case 'I':
				if (optarg)
					printf("option i sould not have extra parameter\n");
				f_daemon = 1;
				break;
			case 's':
			case 'S':
				if (optarg)
					printf("option s sould not have extra parameter\n");
				f_send = 1;
				break;
			case 'r':
			case 'R':
				if (optarg)
					printf("option r sould not have extra parameter\n");
				f_recv = 1;
				break;
			case 'm':
			case 'M':
				if(strcmp(optarg, "232") == 0)
					f_mode = MODE_RS232;
				else if(strcmp(optarg, "422") == 0)
					f_mode = MODE_RS422;
				else if(strcmp(optarg, "485") == 0)
					f_mode = MODE_RS485;
				else {
					printf("operation mode invalid\n");
					return -1;
				}
				break;
			case 'a':
			case 'A':
				if (optarg)
					printf("option a sould not have extra parameter\n");
				f_interaction = IA_ACTIVE;
				break;
			case 'p':
			case 'P':
				if (optarg)
					printf("option p sould not have extra parameter\n");
				f_interaction = IA_PASSIVE;
				break;
			case 'l':
			case 'L':
				if (optarg)
					printf("option l sould not have extra parameter\n");
				f_interaction = IA_LOOP;
				break;
			case 'b':
			case 'B':
				if(!optarg)
					printf("baudrate parameter invalid\n");
				baudrate = atoi(optarg);
				break;
			case 'h':
			case 'H':
				if(!optarg)
					printf("Time parameter invalid\n");
				stop_time = atoi(optarg)+first_send_sleep_time+end_recv_sleep_time;
				break;
				
			case 'c':
			case 'C':
				if(!optarg)
					printf("communication parameter invalid\n");
				switch(*optarg) {
					case 'n':
					case 'N':
						/* no parity*/
						f_parity = PARITY_NONE;
						break;
					case 'e':
					case 'E':
						/* even parity*/
						f_parity = PARITY_EVEN;
						break;
					case 'o':
					case 'O':
						/* odd parity*/
						f_parity = PARITY_ODD;
						break;
					case 'm':
					case 'M':
						/* mark parity*/
						f_parity = PARITY_MARK;
						break;
					case 's':
					case 'S':
						/* space parity*/
						f_parity = PARITY_SPACE;
						break;
					default:
						printf("communication parity parameter invalid\n");
						return -1;
						break;
				}
				databit = (*(optarg + 1)) - '0';
				if(databit > 8 || databit < 5)
					printf("communication databit parameter invalid\n");
				stopbit = (*(optarg + 2)) - '0';
				if(stopbit > 2 || stopbit < 1)
					printf("communication stopbit parameter invalid\n");
				break;
			case 'f':
			case 'F':
				if(!optarg)
					printf("flow control parameter invalid\n");
				i = 0;
				while(flowcontrolopt_arr[i].fc_string != 0) {
					if(strcmp(flowcontrolopt_arr[i].fc_string, optarg) == 0) {
						f_flowcontrol = flowcontrolopt_arr[i].fc;
						break;
					}
					i++;
				}
				if(!flowcontrolopt_arr[i].fc_string) {
					printf("flow control parameter invalid\n");
					return -1;
				}
				break;
			case 'k':
			case 'K':
				if(!optarg)
					printf("packet size parameter invalid\n");
				if(atoi(optarg) > 0 && atoi(optarg) <= MAX_DATA_LEN) {
					packet_size = atoi(optarg);
				} else
					printf("packet size parameter invalid (1~%d)\n",
						MAX_DATA_LEN);
				break;
			case 'z':
			case 'Z':
				if(!optarg)
					printf("packet pause period parameter invalid\n");
				if(atoi(optarg) >= 0) {
					pause_period = atoi(optarg);
				} else
					printf("packet pause period parameter invalid\n");
				break;
			case 'v':
			case 'V':
				if (optarg)
					printf("option v sould not have extra parameter\n");
				f_verify = 1;
				break;
			case 'o':
			case 'O':
				if (optarg)
					printf("option o sould not have extra parameter\n");
				f_output = 1;
				break;
			case 'd':
			case 'D':
				if (optarg)
					printf("option d sould not have extra parameter\n");
				f_debug = 1;
				break;
			case '?':
				/* getopt_long already printed an error message. */
				break;
			default:
				printf("invalid parameters\n");
				usage();
				return -2;
				break;
		}
	}

	printf("Program will\n");
	printf("using %s as testing serial port\n", argv[optind]);
	if(f_send) {
		printf("send specific pattern date to testing serial port\n");
		printf("sending length will be %d bytes per packet\n", packet_size);
		printf("sending will pause %d ms per packet\n", pause_period);
	}
	if(f_recv) {
		printf("receive date from testing serial port\n");
		if(f_verify)
			printf("verify the received data\n");
		else
			printf("not verify the received data\n");
	}
	switch(f_interaction) {
		case IA_ACTIVE:
			printf("start transmit immediately\n");
			break;
		case IA_PASSIVE:
			printf("wait for first character from active side\n");
			break;
		case IA_LOOP:
			printf("act as a loop-back connector\n");
			break;
		default:
			break;
	}

	if(f_output)
		printf("update statistic data during testing\n");
	else
		printf("show statistic data after terminated\n");

	if(f_debug)
		printf("print some debug message\n");

	printf("using baudrate %d bits/sec\n", baudrate);
	PortHandle = open((char *)argv[optind], O_RDWR | O_NOCTTY | O_NDELAY);
	if(PortHandle == -1) {
		printf("open_port: Unable to open %s\n", (char *)argv[optind]);
		return -1;
	}
	printf("Run time is:%d\n",stop_time);
	debug_printf("PortHandle:%d\n", PortHandle);
	sleep(1);

	tcflush(PortHandle, TCIOFLUSH);
	tcgetattr(PortHandle, &PortTermios);
	sleep(1);

	PortTermios.c_cflag |= HUPCL | CLOCAL | CREAD;
	PortTermios.c_cflag &= ~CBAUD;
	PortTermios.c_cflag |= B115200;

	// clear up relative attributes we will set later
	PortTermios.c_cflag &= ~PARENB;
	PortTermios.c_cflag &= ~CMSPAR;
	PortTermios.c_cflag &= ~PARODD;
	//PortTermios.c_cflag &= ~CSIZE;
	PortTermios.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
	PortTermios.c_iflag &= ~(INPCK);   /* disable input parity checking */
	//PortTermios.c_oflag &= ~OPOST;
	//PortTermios.c_cc[VMIN] = 0;

	switch(f_parity) {
	case PARITY_NONE:
		printf("using no parity\n");
		break;
	case PARITY_EVEN:
		printf("using even parity\n");
		PortTermios.c_cflag |= PARENB;
		PortTermios.c_iflag |= INPCK;      /* enable input parity checking */
		break;
	case PARITY_ODD:
		printf("using odd parity\n");
		PortTermios.c_cflag |= PARENB | PARODD;
		PortTermios.c_iflag |= INPCK;      /* enable input parity checking */
 		break;
	case PARITY_MARK:
		printf("using mark parity\n");
		PortTermios.c_cflag |= PARENB | CMSPAR | PARODD;
		PortTermios.c_iflag |= INPCK;      /* enable input parity checking */
		break;
	case PARITY_SPACE:
		printf("using space parity\n");
		PortTermios.c_cflag |= PARENB | CMSPAR;
		PortTermios.c_iflag |= INPCK;      /* enable input parity checking */
		break;
	default:
		goto ERROR_END;
		break;
	}

	PortTermios.c_cflag &= ~CSIZE;
	switch(databit) {
	case 5:
		printf("using 5 databit\n");
		PortTermios.c_cflag |= CS5;
		databit = 5;
		datamask = 0x1F;
		break;
	case 6:
		printf("using 6 databit\n");
		PortTermios.c_cflag |= CS6;
		databit = 6;
		datamask = 0x3F;
		break;
	case 7:
		printf("using 7 databit\n");
		PortTermios.c_cflag |= CS7;
		databit = 7;
		datamask = 0x7F;
		break;
	case 8:
		printf("using 8 databit\n");
		PortTermios.c_cflag |= CS8;
		databit = 8;
		datamask = 0xFF;
		break;
	default:
		goto ERROR_END;
		break;
	}

	switch(stopbit) {
	case 1:
		printf("using 1 stopbit\n");
		PortTermios.c_cflag &= ~CSTOPB;
		break;
	case 2:
		if(databit == 5)
			printf("using 1.5 stopbit\n");
		else
			printf("using 2 stopbit\n");
		PortTermios.c_cflag |= CSTOPB;
		break;
	default:
		goto ERROR_END;
		break;
	}

	switch(f_flowcontrol) {
	case FC_NONE:
		printf("using no flow control\n");
		PortTermios.c_cflag &= ~CRTSCTS;
		PortTermios.c_cflag &= ~CDTRDSR;
		PortTermios.c_iflag &= ~(IXON | IXOFF);      /* input/output flow */
		PortTermios.c_iflag &= ~IXANY;
		break;
	case FC_XONXOFF:
		printf("using XOn/XOff software flow control\n");
		PortTermios.c_cflag &= ~CRTSCTS;
		PortTermios.c_cflag &= ~CDTRDSR;
		PortTermios.c_iflag |= (IXON | IXOFF);      /* input/output flow */
		PortTermios.c_iflag |= IXANY;
		break;
	case FC_RTSCTS:
		printf("using RTS/CTS hardware flow control\n");
		PortTermios.c_cflag |= CRTSCTS;
		PortTermios.c_cflag &= ~CDTRDSR;
		PortTermios.c_iflag &= ~(IXON | IXOFF);     /* no sw flow ctrl */
		PortTermios.c_iflag |= IXANY;
		break;
	default:
		goto ERROR_END;
		break;
	}

	switch(f_mode) {
	case MODE_RS232:
		printf("using RS-232 transmittion mode\n");
		PortTermios.c_iflag &= ~IRS422;
		PortTermios.c_iflag &= ~IRS485;
		break;
	case MODE_RS422:
		printf("using RS-422 transmittion mode\n");
		PortTermios.c_iflag |= IRS422;
		PortTermios.c_iflag &= ~IRS485;
		break;
	case MODE_RS485:
		printf("using RS-485 transmittion mode\n");
		PortTermios.c_iflag &= ~IRS422;
		PortTermios.c_iflag |= IRS485;
		break;
	default:
		goto ERROR_END;
		break;
	}

	PortTermios.c_lflag &= ~ECHO;
	PortTermios.c_lflag &= ~(ICANON);
	PortTermios.c_iflag &= ~ICRNL;

	PortTermios.c_oflag = 0;

	PortTermios.c_cc[VMIN] = 1;
	PortTermios.c_cc[VTIME] = 0;
	PortTermios.c_cc[VINTR] = 0;
	PortTermios.c_cc[VQUIT] = 0;
	PortTermios.c_cc[VSUSP] = 0;

	cfsetispeed(&PortTermios, baudrate);
	cfsetospeed(&PortTermios, baudrate);
	tcsetattr(PortHandle, TCSANOW, &PortTermios);

	if(f_debug)
		printTermios(&PortTermios);

	if(f_send || f_recv)
		temp = strdup(argv[optind]);  
	if(f_send) {
		sprintf(send_file_name, "./%s_%s.log", basename(temp), "send");
		send_file = fopen(send_file_name, "w");
	}
	if(f_recv) {
		sprintf(recv_file_name, "./%s_%s.log", basename(temp), "receive");
		recv_file = fopen(recv_file_name, "w");
	}
	if(f_send || f_recv)
		free(temp);
/*
	int status;
	if (ioctl(PortHandle, TIOCMGET, &status) == -1) {
		printf("TIOCMGET fail\n");
		goto ERROR_END;
	}
	if (PortTermios.c_cflag & CRTSCTS)
		status &= ~TIOCM_RTS;
	if (ioctl(PortHandle, TIOCMSET, &status) == -1) {
		printf("TIOCMSET fail\n");
		goto ERROR_END;
	}
*/

	pthread_attr_init(&ThreadAttr);
	pthread_attr_setstacksize(&ThreadAttr, 0x10000); // 4KB
	bRunning = 1;

	clock_gettime(CLOCK_REALTIME, &starttime);

	if(f_send || f_recv)
		printf("Press ENTER key to exit...\n");

	if(f_mode == MODE_RS485 && f_send && f_recv) {
		if(f_interaction == IA_LOOP) {
			printf("RS-485 mode can't use loop-back test\n");
			goto ERROR_END;
		}
		ThreadParam.sched_priority = 1;
		pthread_attr_setschedparam(&ThreadAttr, &ThreadParam);
		CreateThreadResult = pthread_create(&RS485ThreadHandle,
			&ThreadAttr, &RS485Thread, (void *)PortHandle);
		if(CreateThreadResult != 0) {
			printf("Create Recv Thread Failed!\n");
			goto ERROR_END;
		}
		if(f_output) {
			ThreadParam.sched_priority = 5;
			pthread_attr_setschedparam(&ThreadAttr, &ThreadParam);
			CreateThreadResult = pthread_create(&ShowThreadHandle,
				&ThreadAttr, &ShowThread, (void *)PortHandle);
			if(CreateThreadResult != 0) {
				printf("Create Show Thread Failed!\n");
				goto ERROR_END;
			}
		}
		if(f_daemon) {
			goto DAEMON;
		}
		//getchar();
		StopTime();
		if(!f_output) {
			if (ioctl(PortHandle, TIOCGICOUNT, &icount) < 0) {
				printf("Cannot get interrupt counters");
				goto END;
			}
			clock_gettime(CLOCK_REALTIME, &nowtime);
			printf(	"R %u, %u BPS<= (%02u:%02u:%02u) "
					"=>S %u, %u BPS OE:%u, %d%5s\n",
				RecvTotal, RecvThroughput,
				(nowtime.tv_sec - starttime.tv_sec) / (60 * 60),
				((nowtime.tv_sec - starttime.tv_sec) % (60 * 60)) / 60,
				(nowtime.tv_sec - starttime.tv_sec) % 60,
				SendTotal, SendThroughput,
				icount.overrun,
				timeoutcount,
				" ");
			fflush(stdout);
		}
	} else {
		if(f_interaction != IA_LOOP) {
			if(f_recv) {
				ThreadParam.sched_priority = 1;
				pthread_attr_setschedparam(&ThreadAttr, &ThreadParam);
				CreateThreadResult = pthread_create(&RecvThreadHandle,
					&ThreadAttr, &RecvThread, (void *)PortHandle);
				if(CreateThreadResult != 0) {
					printf("Create Recv Thread Failed!\n");
					goto ERROR_END;
				}
			}

			if(f_send) {
				ThreadParam.sched_priority = 5;
				pthread_attr_setschedparam(&ThreadAttr, &ThreadParam);
				CreateThreadResult = pthread_create(&SendThreadHandle,
					&ThreadAttr, &SendThread, (void *)PortHandle);
				if(CreateThreadResult != 0) {
					printf("Create Send Thread Failed!\n");
					goto ERROR_END;
				}
			}

			if((f_send || f_recv) && f_output) {
				ThreadParam.sched_priority = 5;
				pthread_attr_setschedparam(&ThreadAttr, &ThreadParam);
				CreateThreadResult = pthread_create(&ShowThreadHandle,
					&ThreadAttr, &ShowThread, (void *)PortHandle);
				if(CreateThreadResult != 0) {
					printf("Create Show Thread Failed!\n");
					goto ERROR_END;
				}
			}
			if(f_daemon) {
				goto DAEMON;
			}
			if(f_send || f_recv) {
				//getchar();
				StopTime();
			}

			if(!f_output) {
				if (ioctl(PortHandle, TIOCGICOUNT, &icount) < 0) {
					printf("Cannot get interrupt counters");
					goto END;
				}
				clock_gettime(CLOCK_REALTIME, &nowtime);
				printf("R %llu, %llu BPS<= (%02u:%02u:%02u) =>S %llu, %llu BPS OE:%u, %d%5s\n",
					RecvTotal, RecvThroughput,
					(nowtime.tv_sec - starttime.tv_sec) / (60 * 60),
					((nowtime.tv_sec - starttime.tv_sec) % (60 * 60)) / 60,
					(nowtime.tv_sec - starttime.tv_sec) % 60,
					SendTotal, SendThroughput,
					icount.overrun,
					timeoutcount,
					" ");
				fflush(stdout);
			}
		}
		else
		{
			ThreadParam.sched_priority = 1;
			pthread_attr_setschedparam(&ThreadAttr, &ThreadParam);
			CreateThreadResult = pthread_create(&LoopThreadHandle,
				&ThreadAttr, &LoopThread, (void *)PortHandle);
			if(CreateThreadResult != 0) {
				printf("Create Loop Thread Failed!\n");
				goto ERROR_END;
			}
			if(f_daemon) {
				goto DAEMON;
			}
			//getchar();
			StopTime();
		}
	}

	bRunning = 0;
END:
	while(f_interaction != IA_LOOP) {
		if(f_send || f_recv || f_output) {
			usleep(10000); // wait until threads terminated.
			exitcount++;
			if(exitcount > 100)
				break;  // to avoid dead lock
		} else
			break;
	}
	if (ioctl(PortHandle, TIOCGICOUNT, &icount) < 0) {
		printf("Cannot get interrupt counters");
		goto ERROR_END;
	}
	printf("total rx:%u\n", icount.rx);
	printf("total tx:%u\n", icount.tx);
	printf("frame error:%u\n", icount.frame);
	printf("parity error:%u\n", icount.parity);
	printf("break error:%u\n", icount.brk);
	printf("overrun:%u\n", icount.overrun);

ERROR_END:
	close(PortHandle);
	if(send_file) {
		fflush(send_file);
		fclose(send_file);
	}
	if(recv_file) {
		fflush(recv_file);
		fclose(recv_file);
	}
	printf("Program Terminated!\n");
	return 0;
	
DAEMON:
	while(1) {
		sleep(100);
	}
	return 0;
}
