#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <fcntl.h>
#include <linux/watchdog.h>

static const char default_wdt[] = "/dev/watchdog";

int main(int argc,char *argv[])
{
	int fd = -1;
	unsigned int time = 0;
	int retval = 0;
	int tmp = 0;

	fd = open(default_wdt, O_RDWR);
	if (fd < 0)
	{
		printf("Fail to open watchdog device!\n");
		retval = -1;
		goto out;
	}

	retval = ioctl(fd, WDIOC_GETTIMEOUT, &time);
	if (retval < 0){
		printf("failed to call WDIOC_GETTIMEOUT ioctl,error:%d\n",retval);
		goto out;
	}
	printf("WDIOC_GETTIMEOUT:0x%x\n",time);

	time = 200;
	retval = ioctl(fd, WDIOC_SETTIMEOUT, &time);
	if (retval < 0){
		printf("failed to call WDIOC_SETTIMEOUT ioctl,error:%d\n",retval);
		goto out;
	}

	time = 0;
	retval = ioctl(fd, WDIOC_GETTIMEOUT, &time);
	if (retval < 0){
		printf("failed to call WDIOC_GETTIMEOUT ioctl,error:%d\n",retval);
		goto out;
	}
	printf("Second WDIOC_GETTIMEOUT:0x%x\n",time);

	for(time=0;time<50;time++)
	{
		retval = ioctl(fd, WDIOC_KEEPALIVE, NULL);
		if (retval < 0){
			printf("failed to call WDIOC_KEEPALIVE ioctl,error:%d\n",retval);
			goto out;
		}
		for(tmp=0;tmp<0x1000000;tmp++);
	}
	retval = 0;

	//wait for reboot
	while(1);
out:
	if(fd > 0)
		close(fd);
	return retval;
}
