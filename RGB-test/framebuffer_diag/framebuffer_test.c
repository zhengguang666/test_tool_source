#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>

#include <linux/fb.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include "framebuffer_test.h"

#define VIDEO_PATH "ESS_VIDEO_PATH"
#define FRAME_BUFFER_LCD "ESS_FRAME_BUFFER_LCD"
#define FRAME_BUFFER_LDB0 "ESS_FRAME_BUFFER_LDB0"
#define FRAME_BUFFER_LDB1 "ESS_FRAME_BUFFER_LDB1"
#define DELAY_TIME "ESS_FRAME_BUFFER_DELAY"
#define RGB_32_888(r,g,b) ( ((r) <<16) | ((g) << 8) | ((b) << 0) )//for 16bpp framebuffer RGB_16_565

static char *fbp = 0;
static char *fbp_xy = 0;
char video_path[128];
int fb_lcd = 0;
int fb_ldb0 = 0;
int fb_ldb1 = 0;

int fill_screen_16bpp(unsigned char * fb_buf, char r, char g, char b, struct fb_var_screeninfo * pvar)
{
  unsigned int x,y;
  unsigned int * p = (unsigned int *)fb_buf;

  if(pvar->bits_per_pixel != 32)
  	return -1;
  
  for(x = 0; x < pvar->xres * pvar->yres; x++) {
  	*p = RGB_32_888(r, g, b);
    p++;
  }
    return 0;
}

int show_color(int fb_num, char r,char g, char b)
{
	int fbfd = 0;
	struct fb_var_screeninfo vinfo;
	struct fb_fix_screeninfo finfo;
	long int screensize = 0;
	int xres, yres,x,y;
	int bits_per_pixel;
	char tmp[10];

	sprintf(tmp, "/dev/graphics/fb%d", fb_num);
	fbfd = open(tmp, O_RDWR);
	if (!fbfd)
	{
		return -1;
	}

	if (ioctl(fbfd, FBIOGET_FSCREENINFO, &finfo))
	{
		return -1;
	}

	if (ioctl(fbfd, FBIOGET_VSCREENINFO, &vinfo))
	{
		return -1;
	}
	screensize = vinfo.xres * vinfo.yres * vinfo.bits_per_pixel / 8;

	fbp = (char *)mmap(0, screensize, PROT_READ | PROT_WRITE, MAP_SHARED, fbfd, 0);
	xres = (((vinfo.xres+31)/32)*32);
	//  xres = vinfo.xres;
	yres = vinfo.yres;
	bits_per_pixel = vinfo.bits_per_pixel;

	if ((int)fbp == -1)
	{
		return -1;
	}

/*
	fbp_xy = fbp;
	for (y = 0; y < yres; y++)
	{
		for(x = 0; x < vinfo.xres; x++)
		{
			*(fbp_xy + 0) = r;
			*(fbp_xy + 1) = g;
			*(fbp_xy + 2) = b;
			*(fbp_xy + 3) = 0xff;
			fbp_xy += 4;
		}
	}
*/
	fill_screen_16bpp(fbp, r, g, b, &vinfo);

	munmap(fbp, screensize);
	close(fbfd);
	return 0;
}

int check_framebuffer(void)
{
	int getAnser=99;
	//int delayS=atoi(getenv(DELAY_TIME));
	//int delayK=atoi(getenv(SET_DELAY));
	char cmd[256];
	char board[20], item_name[20];

	/* Show Colors */
	system("echo 0 > /sys/class/graphics/fb0/blank");
	sleep(3);

	show_color(0,0xFF,0xFF,0xFF);
	sleep(3);

	show_color(0,0x00,0x00,0xFF);
	sleep(3);

	show_color(0,0x00,0xFF,0x00);
	sleep(3);

	show_color(0,0xFF,0x00,0x00);
	sleep(3);

	show_color(0,0x00,0x00,0x00);

	/* Video Playback */

	return 0;
}

int framebuffer_test(void) {

	printf("\ntest framebuffer!\n");


	if(check_framebuffer() == -1)
		goto RETURN_FAIL;

	printf("\n\n");
	printf("+---------------------------+\n");
	printf("| [Frame Buffer] Test Finished! |\n");
	printf("+---------------------------+\n");
	//print_pass();
	return 0;

RETURN_FAIL:
	printf("\n\n");
	printf("+---------------------------+\n");
	printf("| [Frame Buffer] Test Fail! |\n");
	printf("+---------------------------+\n");

	//print_fail();
	return -1;
}

int main(int argc, char **argv)
{
    framebuffer_test();
    return 0;
}
