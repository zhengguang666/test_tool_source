#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;

typedef struct _bmp_header{
    u8 id[2];       /* byte 0x00-0x01 */
    u32 filesize;       /* byte 0x02-0x05 */
    u32 rev1;       /* byte 0x06-0x09 */
    u32 data_off;       /* byte 0x0a-0x0d */
    u32 header_size;    /* byte 0x0e-0x11 */
    u32 width;      /* byte 0x12-0x15 */
    u32 height;     /* byte 0x16-0x19 */
    u16 plane;      /* byte 0x1a-0x1b */
    u16 bpp;        /* byte 0x1c-0x1d */
    u32 compression;    /* byte 0x1e-0x21 */
    u32 bmpdata_size;   /* byte 0x22-0x25 */
    u32 hresolution;    /* byte 0x26-0x29 */
    u32 vresolution;    /* byte 0x2a-0x2d */
    u32 colors;     /* byte 0x2e-0x31 */
    u32 important;      /* byte 0x32-0x35 */
} __attribute__((packed)) bmp_header;

bmp_header bh;

static void dump_bmpheader(bmp_header *bh)
{
    printf("Dump bmp header information : \n");
    printf("\tID %c%c\n", bh->id[0], bh->id[1]);
    printf("\tfilesize %d\n", bh->filesize);
    printf("\trev1 0x%x\n", bh->rev1);
    printf("\tdata_off %d\n", bh->data_off);
    printf("\theader_size 0x%x\n", bh->header_size);
    printf("\twidth %d\n", bh->width);
    printf("\theight %d\n", bh->height);
    printf("\tplane %d\n", bh->plane);
    printf("\tbpp %d\n", bh->bpp);
    printf("\tcompression %d\n", bh->compression);
    printf("\tbmpdata_size 0x%x\n", bh->bmpdata_size);
    printf("\thresolution %d\n", bh->hresolution);
    printf("\tvresolution %d\n", bh->vresolution);
    printf("\tcolors %d\n", bh->colors);
    printf("\timportant %d\n", bh->important);
    return;
}

unsigned char * bmp_get_data(char *filename,int target_bits_width)
{
  FILE *fp = NULL;
  unsigned char *buf = NULL;
  int buflen = 0;
  int offset =0;
  unsigned char *bufptr = NULL;
  int len;
  int i,j;
  int pixel_data;

  if (!(fp = fopen(filename, "rb"))){
        printf("Fail to open input file : %s\n",filename);
        return NULL;
   }

   len = fread(&bh, sizeof(bh), 1, fp);
   if (len != 1){
        printf("Fail to read header data : %d\n", len);
        goto fail_out;
   }
   //debug out msg
   dump_bmpheader(&bh);

   printf("bpp=%d width=%d height=%d\n",bh.bpp,bh.width,bh.height);
   if ((bh.bpp!=16) && (bh.bpp!=24))// && (bh.bpp!=32)) 
   {
	printf("not a normal RGB16/24/32 bmp\n");
	goto fail_out;
   }
   if (bh.compression!=0)
   {
	printf("compress bmp not support!\n");
	goto fail_out;
   }
   if ((target_bits_width!=16) && (target_bits_width!=32))
   {
        printf("can conver to RGB565 AND RGB888 only!\n");
        goto fail_out;
   }
   buflen = bh.width * bh.height * target_bits_width/8;
   buf = malloc(buflen);
   if (buf == NULL)
   {
	printf("can't allocate buffer,size=%d\n",buflen);
	goto fail_out;
   }
   bufptr = buf;
   for (i=bh.height-1;i>=0;i--)
   {
	fseek(fp,bh.data_off + bh.width*i*bh.bpp/8,SEEK_SET);
	for (j=0;j<bh.width;j++)
	{
	  len = fread(&pixel_data,1,bh.bpp/8,fp);
	  //split r/g/b
	  int r,g,b;
	  short color16=0;
	  long color32=0;
	  if (bh.bpp ==16)
          {
	    r = (pixel_data & 0xf800) >> 8;
	    g = ((pixel_data & 0x7e0) >> 5)<<2;
    	    b = (pixel_data & 0x1f)<<3 ;
 	  }else if ((bh.bpp == 24) || (bh.bpp == 32))
	 {
	    r = (pixel_data & 0xff0000)>>16;
	    g = (pixel_data & 0xff00)>>8;
            b = (pixel_data &0xff);
	 }
	
	  //convert to target width and save
	  if (target_bits_width == 16)//RGB565
	  {
	   color16 = (b>>3) | ((g>>2) << 5) | ((r>>3) << (5 + 6)); 
	   memcpy(bufptr,&color16,2);
	   bufptr +=2;
          }else
	  {
	   color32 = b| (g<<8) | (r<<16) | (0xff<<24);
	   memcpy(bufptr,&color32,4);
	   bufptr +=4;
   	  }
	  if ((j==0) && ((i==bh.height-1)|| (i==0)))
	  {
		printf("line %d:r=%x g=%x b=%x,target=%x %x\n",i,r,g,b,color16,color32);
	  } 
	}
   }
   return buf;     
fail_out:
  fclose(fp);
  return NULL;
}
void bmp_free(unsigned char * buf)
{
  free(buf);
}
int bmp_get_width()
{
  return bh.width;
}
int bmp_get_height()
{
  return bh.height;
}
int bmp_get_pixel_width()
{
  return bh.bpp;
}


