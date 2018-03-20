 
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

//»­´óÐ¡Îªwidth*heightµÄÍ¬É«¾ØÕó£¬5reds+6greens+5blues
void
drawRect_rgb16 (int x0, int y0, int width, int height, int color)
{
    const int bytesPerPixel = 2;
    const int stride = finfo.line_length / bytesPerPixel;
    const int red = (color & 0xff0000) >> (16 + 3);
    const int green = (color & 0xff00) >> (8 + 2);
    const int blue = (color & 0xff) >> 3;
    const short color16 = blue | (green << 5) | (red << (5 + 6));
    const short color_bound = 0xF800; //red color

    short *dest = (short *) (frameBuffer)
        + (y0 + vinfo.yoffset) * stride + (x0 + vinfo.xoffset);

    int x, y;
    int act_height,act_width;
    if (x0+width > vinfo.xres)
	act_width = vinfo.xres - x0;
    else
	act_width = width;
    if (y0+height > vinfo.yres)
	act_height = vinfo.yres - y0;
    else
	act_height = height;
    for (y = 0; y < act_height; ++y)
    {
        for (x = 0; x < act_width; ++x)
        {
        	dest[x] = color16;
        }
        dest += stride;
    }
}
void
drawRect_rgb32(int x0, int y0, int width, int height, int color)
{
    const int bytesPerPixel = 4;
    const int stride = finfo.line_length / bytesPerPixel;
    const int red = (color & 0xff0000)>>16;
    const int green = (color & 0xff00)>>8 ;
    const int blue = (color & 0xff) ;
    const int color32 = blue | (green << 8) | (red << 16);

    int *dest = (int *) (frameBuffer)
        + (y0 + vinfo.yoffset) * stride + (x0 + vinfo.xoffset);

    int x, y;
    int act_height,act_width;
    if (x0+width > vinfo.xres)
        act_width = vinfo.xres - x0;
    else
        act_width = width;
    if (y0+height > vinfo.yres)
        act_height = vinfo.yres - y0;
    else
        act_height = height;
    for (y = 0; y < act_height; ++y)
    {
        for (x = 0; x < act_width; ++x)
        {
                dest[x] = color32;
        }
        dest += stride;
    }
}

void
drawRect (int x0, int y0, int width, int height, int color)
{
    switch (vinfo.bits_per_pixel)
    {
    case 32:
        drawRect_rgb32 (x0, y0, width, height, color);
        break;
    case 16:
        drawRect_rgb16 (x0, y0, width, height, color);
        break;
//    case 15:
//        drawRect_rgb15 (x0, y0, width, height, color);
//        break;
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

    int xstart,ystart,width,height;
    int color;

    if (argc <7)
    {
	printf("framebuffer display rect test\n");
	printf("usage: %s devpath xstart ystart width height color \n",argv[0]);
	printf("sample: %s /dev/fb0 10 10  100 100  0xff0000 \n",argv[0]);
	exit (1);
    }
    xstart = atoi(argv[2]);
    ystart = atoi(argv[3]);
    width = atoi(argv[4]);
    height = atoi(argv[5]);
    color = atoi(argv[6]);

    printf("framebuffer test using device %s\n",argv[1]);
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
        printf("this test only support RGB565(16bits) and RGB888(24BIT) framebuffer\n");
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

    printf("draw rect at (%d,%d,%d,%d) with color %x\n",xstart,ystart,xstart+width-1,ystart+height-1,color);
    drawRect(xstart,ystart,width,height,color);
	
    //sleep (5);
    printf (" Done.\n");

    munmap (frameBuffer, screensize);    //½â³ýÄÚ´æÓ³Éä£¬Óëmmap¶ÔÓ¦

    close (fbFd);
    return 0;
}
