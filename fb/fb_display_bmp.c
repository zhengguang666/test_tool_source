 
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <linux/kd.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <string.h>
#include <errno.h>

struct fb_var_screeninfo vinfo;
struct fb_fix_screeninfo finfo;
char *frameBuffer = 0;

extern unsigned char * bmp_get_data(char *filename,int target_bits_width);

//´òÓ¡fbÇý¶¯ÖÐfix½á¹¹ÐÅÏ¢£¬×¢£ºÔÚfbÇý¶¯¼ÓÔØºó£¬fix½á¹¹²»¿É±»ÐÞ¸Ä¡£
void
printFixedInfo ()
{
    printf ("Fixed screen info:\n"
            "\tid: %s\n"
            "\tsmem_start: 0x%lx\n"
            "\tsmem_len: %d\n"
            "\ttype: %d\n"
            "\ttype_aux: %d\n"
            "\tvisual: %d\n"
            "\txpanstep: %d\n"
            "\typanstep: %d\n"
            "\tywrapstep: %d\n"
            "\tline_length: %d\n"
            "\tmmio_start: 0x%lx\n"
            "\tmmio_len: %d\n"
            "\taccel: %d\n"
            "\n",
            finfo.id, finfo.smem_start, finfo.smem_len, finfo.type,
            finfo.type_aux, finfo.visual, finfo.xpanstep, finfo.ypanstep,
            finfo.ywrapstep, finfo.line_length, finfo.mmio_start,
            finfo.mmio_len, finfo.accel);
}

//´òÓ¡fbÇý¶¯ÖÐvar½á¹¹ÐÅÏ¢£¬×¢£ºfbÇý¶¯¼ÓÔØºó£¬var½á¹¹¿É¸ù¾ÝÊµ¼ÊÐèÒª±»ÖØÖÃ
void
printVariableInfo ()
{
    printf ("Variable screen info:\n"
            "\txres: %d\n"
            "\tyres: %d\n"
            "\txres_virtual: %d\n"
            "\tyres_virtual: %d\n"
            "\tyoffset: %d\n"
            "\txoffset: %d\n"
            "\tbits_per_pixel: %d\n"
            "\tgrayscale: %d\n"
            "\tred: offset: %2d, length: %2d, msb_right: %2d\n"
            "\tgreen: offset: %2d, length: %2d, msb_right: %2d\n"
            "\tblue: offset: %2d, length: %2d, msb_right: %2d\n"
            "\ttransp: offset: %2d, length: %2d, msb_right: %2d\n"
            "\tnonstd: %d\n"
            "\tactivate: %d\n"
            "\theight: %d\n"
            "\twidth: %d\n"
            "\taccel_flags: 0x%x\n"
            "\tpixclock: %d\n"
            "\tleft_margin: %d\n"
            "\tright_margin: %d\n"
            "\tupper_margin: %d\n"
            "\tlower_margin: %d\n"
            "\thsync_len: %d\n"
            "\tvsync_len: %d\n"
            "\tsync: %d\n"
            "\tvmode: %d\n"
            "\n",
            vinfo.xres, vinfo.yres, vinfo.xres_virtual, vinfo.yres_virtual,
            vinfo.xoffset, vinfo.yoffset, vinfo.bits_per_pixel,
            vinfo.grayscale, vinfo.red.offset, vinfo.red.length,
            vinfo.red.msb_right, vinfo.green.offset, vinfo.green.length,
            vinfo.green.msb_right, vinfo.blue.offset, vinfo.blue.length,
            vinfo.blue.msb_right, vinfo.transp.offset, vinfo.transp.length,
            vinfo.transp.msb_right, vinfo.nonstd, vinfo.activate,
            vinfo.height, vinfo.width, vinfo.accel_flags, vinfo.pixclock,
            vinfo.left_margin, vinfo.right_margin, vinfo.upper_margin,
            vinfo.lower_margin, vinfo.hsync_len, vinfo.vsync_len,
            vinfo.sync, vinfo.vmode);
}

void drawRect_rgb32 (char *buf,int x0, int y0, int width, int height)
{
    const int bytesPerPixel = 4;
    const int stride = finfo.line_length / bytesPerPixel;

    int *dest = (int *) (frameBuffer)
        + (y0 + vinfo.yoffset) * stride + (x0 + vinfo.xoffset);

    int x, y;
    int act_height,act_width;

    printf("draw rgb32, line len=%d x0=%d %d y0=%d %d stide=%d\n",finfo.line_length,x0,vinfo.xoffset,y0,vinfo.yoffset,stride);
   
    if (x0+width > vinfo.xres)
        act_width = vinfo.xres - x0;
    else
        act_width = width;
    if (y0+height > vinfo.yres)
        act_height = vinfo.yres - y0;
    else
        act_height = height;
    printf("act width=%d height=%d\n",act_width,act_height);
    for (y = 0; y < act_height; ++y)
    {
        for (x = 0; x < act_width; ++x)
        {
                dest[x] = *((long*)buf+x);
        }
        dest += stride;
        buf += width*4;
    }
}

void drawRect_rgb16 (char *buf,int x0, int y0, int width, int height)
{
    const int bytesPerPixel = 2;
    const int stride = finfo.line_length / bytesPerPixel;
    const short color16 = 0xF800; //red color

    short *dest = (short *) (frameBuffer)
        + (y0 + vinfo.yoffset) * stride + (x0 + vinfo.xoffset);

    int x, y;
    int act_height,act_width;
    printf("draw rgb16, line len=%d x0=%d %d y0=%d %d stide=%d\n",finfo.line_length,x0,vinfo.xoffset,y0,vinfo.yoffset,stride);
    if (x0+width > vinfo.xres)
        act_width = vinfo.xres - x0;
    else
        act_width = width;
    if (y0+height > vinfo.yres)
        act_height = vinfo.yres - y0;
    else
        act_height = height;
    printf("act width=%d height=%d\n",act_width,act_height);
    for (y = 0; y < act_height; ++y)
    {
        for (x = 0; x < act_width; ++x)
        {
                //dest[x] = color16;
		dest[x] = *((short *)buf+x);
        }
        dest += stride;
	buf += width*2;
    }
}

void
drawRect (char* buf,int x0, int y0, int width, int height)
{
    switch (vinfo.bits_per_pixel)
    {
    case 32:
        drawRect_rgb32 (buf,x0, y0, width, height);
        break;
    case 16:
        drawRect_rgb16 (buf,x0, y0, width, height);
        break;
//    case 15:
  //      drawRect_rgb15 (x0, y0, width, height, color);
    //    break;
    default:
        printf ("Warning: drawRect() not implemented for color depth %i\n",
                vinfo.bits_per_pixel);
        break;
    }
}

int
main (int argc, char **argv)
{
    const char *devfile = "/dev/fb0";
    long int screensize = 0;
    int fbFd = 0;
    unsigned char * buf = NULL;
    int width;
    int height;
    int bpp;
    int startx,starty;
     
    if (argc <3)
    {
	printf("framebuffer display bmp test\n");
	printf("usage: %s devpath filename [xstart] [ystart] \n",argv[0]);
	printf("sample: %s /dev/fb0 img1.bmp 10 20\n",argv[0]);
	exit (1);
    }
    if (argc >=4)
	startx = atoi(argv[3]);
    else
	startx = 0;
    if (argc >=5)
	starty = atoi(argv[4]);
    else
	starty = 0;

    printf("framebuffer display using device %s\n",argv[1]);
    /* Open the file for reading and writing */
    fbFd = open (argv[1], O_RDWR);
    if (fbFd == -1)
    {
        perror ("Error: cannot open framebuffer device");
        exit (1);
    }

    //»ñÈ¡finfoÐÅÏ¢²¢ÏÔÊ¾
    if (ioctl (fbFd, FBIOGET_FSCREENINFO, &finfo) == -1)
    {
        perror ("Error reading fixed information");
	close (fbFd);
        exit (2);
    }
    printFixedInfo ();
    //»ñÈ¡vinfoÐÅÏ¢²¢ÏÔÊ¾
    if (ioctl (fbFd, FBIOGET_VSCREENINFO, &vinfo) == -1)
    {
        perror ("Error reading variable information");
	close (fbFd);
        exit (3);
    }
    printVariableInfo ();

    if ((vinfo.bits_per_pixel!=16) && (vinfo.bits_per_pixel!=32)) 
    {
	printf("this test only support 16bits/32bits framebuffer\n");
	close (fbFd);	
	exit(4);
    }
    /* Figure out the size of the screen in bytes */
    screensize = finfo.smem_len;

    /* Map the device to memory */
    frameBuffer =
        (char *) mmap (0, screensize, PROT_READ | PROT_WRITE, MAP_SHARED,
                     fbFd, 0);
    if (frameBuffer == MAP_FAILED)
    {
        perror ("Error: Failed to map framebuffer device to memory");
	close (fbFd);
        exit (4);
    }

    buf = bmp_get_data(argv[2],vinfo.bits_per_pixel);
    if (buf!=NULL)
    {
	width = bmp_get_width();
	height = bmp_get_height();
	bpp = bmp_get_pixel_width();

	printf("start %d.%d width=%d height=%d bpp=%d\n",startx,starty,width,height,bpp);
	//copy to framebuffer 
	drawRect((char*)buf,startx,starty,width,height);			

	bmp_free(buf);
    }
    else
	printf("get bmp data fail\n");
    
    
    //sleep (5);
    printf (" Done.\n");

    munmap (frameBuffer, screensize);    //½â³ýÄÚ´æÓ³Éä£¬Óëmmap¶ÔÓ¦

    close (fbFd);
    return 0;
}
