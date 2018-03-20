#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <time.h>
#include <fcntl.h>

#include <sys/wait.h>
#include <mtd/mtd-user.h>


#define TIME_BUF_LEN	19
#define TIME_LEN	TIME_BUF_LEN - 5
#define SN_BUF_LEN	10
#define MAC_BUF_LEN	36

#define MAC_OFFSET (0x1d000)
#define TEST_FAIL -1
#define TEST_PASS 0

struct boardcfg_t {
    unsigned char mac[12];
    unsigned char sn[SN_BUF_LEN];
    unsigned char Manufacturing_Time[TIME_LEN];
};

typedef struct format_pattern {
    char target;
    int len;
} format_pattern;

format_pattern time_pattern[] = {
    {'-', 4},
    {'-', 2},
    {'_', 2},
    {'-', 2},
    {'-', 2},
    {'\0', 2},
};

format_pattern mac_pattern[] = {
    {':', 2},
    {':', 2},
    {':', 2},
    {':', 2},
    {':', 2},
    {'\0', 2},
};

int check_format(char* start, char value, int len)
{
    char* end;
    end=strchr(start, value);
    if (end == NULL || (end - start) != len) {
        printf("Incorrect format!!\n");
        return TEST_FAIL;
    } else
        return TEST_PASS;
}

int check_pattern(char* input, format_pattern* pattern, int pattern_len, char* output)
{
    int i;
    char* in_start = input;
    char* out_start = output;

    for (i=0 ; i < pattern_len ; i++) {
        if (check_format(in_start, pattern[i].target, pattern[i].len) == TEST_FAIL)
            return TEST_FAIL;
        else if (output != NULL) {
            strncpy(out_start, in_start, pattern[i].len);
            out_start += pattern[i].len;
        }

        in_start += pattern[i].len + 1;
    }

    return TEST_PASS;
}

unsigned long simple_strtoul(const char *cp,char **endp,unsigned int base)
{
    unsigned long result = 0,value;

    if (*cp == '0') {
        cp++;
        if ((*cp == 'x') && isxdigit(cp[1])) {
            base = 16;
            cp++;
        }
        if (!base) {
            base = 8;
        }
    }
    if (!base) {
        base = 10;
    }
    while (isxdigit(*cp) && (value = isdigit(*cp) ? *cp-'0' : (islower(*cp)
                                     ? toupper(*cp) : *cp)-'A'+10) < base) {
        result = result*base + value;
        cp++;
    }
    if (endp)
        *endp = (char *)cp;
    return result;
}

static int write_mac(char write_buf[])
{
    int err = 0,fd;
    int offset= MAC_OFFSET;
    char *start,*end;//write_buf[TIME_BUF_LEN + SN_BUF_LEN + MAC_BUF_LEN + 3];
    struct boardcfg_t boardcfg;
    int i,len,num;
    struct erase_info_user erase;
    char time_buf[TIME_BUF_LEN+1],time[TIME_LEN+1],sn_buf[SN_BUF_LEN+1],mac_buf[MAC_BUF_LEN +1];
	char MAC[2][18] = {0};

	len = (strlen(write_buf)>MAC_BUF_LEN)?MAC_BUF_LEN:strlen(write_buf);
	strncpy(mac_buf, write_buf, len);
	mac_buf[len]='\0';
	//printf("mac len:%d,value:%s\n",len,mac_buf);
	num = len/17;

	for(i=0; i<12; i++) {
        boardcfg.mac[i] = (uint8_t)simple_strtoul(&mac_buf[3*i],NULL,16);
    }

    for(i = 0; i < num; i++) {
        strncpy(MAC[i], mac_buf + ( i * TIME_BUF_LEN - i), TIME_BUF_LEN - 2);
        MAC[i][TIME_BUF_LEN - 2]='\0';
        printf("MAC[%d]: %s\n", i, MAC[i]);

        if (check_pattern(MAC[i], mac_pattern, sizeof(mac_pattern)/sizeof(format_pattern), NULL) == TEST_FAIL) {
            printf("Please check MAC format!!");
            return TEST_FAIL;
        }
	}
    //printf("mac_buf = %s, len = %d\n", mac_buf, strlen(mac_buf));
        
    if ((fd = open("/dev/mtd0",O_SYNC | O_RDWR)) < 0) {
        printf("open fail\n");
        return TEST_FAIL;
    }

    if (offset != lseek(fd,offset,SEEK_SET)) {
        printf("lseek fail\n");
        return TEST_FAIL;
    }

    //printf("write offset = 0x%X\n", offset);

    erase.start = offset;
    erase.length = 4096;
    err = ioctl(fd,MEMERASE,&erase);
    if (err < 0) {
        printf("erase fail\n");
        return TEST_FAIL;
    }

    err = write(fd,&boardcfg,sizeof(boardcfg));
    if (err < 0) {
        printf ("write fail\n");
        return TEST_FAIL;
    }

    /* close device */
    if (close (fd) < 0) {
        printf ("close fail\n");
        return TEST_FAIL;
    }

    return TEST_PASS;
}


int main(int argc,char *argv[])
{
    if(argc != 2)
	{
		printf("Illegal parameter,-h for help\r\n");
		return -1;
 	} else if(strcmp(argv[1],"-h")==0) {
			printf("Illegal parameter,-h for help\r\n");
			printf("For example:./mac_write 00:01:02:03:04:05,00:01:02:03:04:07\n");
			return -1;
	}
   
    if(write_mac(argv[1]) == TEST_FAIL)
        goto RETURN_FAIL;

    printf("\n\n");
    printf("+----------------------+\n");
    printf("| [MAC] Write Success! |\n");
    printf("+----------------------+\n");
   
    return TEST_PASS;

RETURN_FAIL:
    printf("\n\n");
    printf("+-------------------+\n");
    printf("| [MAC] Write Fail! |\n");
    printf("+-------------------+\n");
 
    return TEST_FAIL;
}
