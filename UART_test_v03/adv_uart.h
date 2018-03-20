#include<stdio.h>      /*标准输入输出定义*/  
#include<unistd.h>     /*Unix 标准函数定义*/  
#include<stdlib.h>     /*标准函数库定义*/  
#include<sys/types.h>   
#include<sys/stat.h>     
#include<fcntl.h>      /*文件控制定义*/  
#include<termios.h>    /*PPSIX 终端控制定义*/  
#include<errno.h>      /*错误号定义*/  
#include<string.h>  

#define DEBUG 1
//宏定义  
#define FALSE  -1  
#define TRUE   0  

int UART_Open(char* port) ; 

void UART_Close(int fd);  

int UART_Set(int fd,int speed,int flow_ctrl,int databits,int stopbits,int parity);

int UART_Recv(int fd, char *rcv_buf,int data_len);
int UART_Send(int fd, char *send_buf,int data_len);


