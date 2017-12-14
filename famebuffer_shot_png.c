#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <getopt.h>
#include <sys/vt.h>
#include <png.h>
#include <zlib.h>
#include <linux/fb.h>

static int Blue = 0;
static int Green = 1;
static int Red = 2;
static int Alpha = 3;

int main(int argc, char**argv)
{
	struct fb_var_screeninfo fb_varinfo;
	int fd;		
	int bitdepth, height, width;
	size_t buf_size;
	size_t buf_size1;
	unsigned char *inbuffer;
	unsigned char *outbuffer;

	/*fb_fix_screeninfo信息的设置*/
	memset(&fb_varinfo, 0, sizeof(struct fb_var_screeninfo));

	/*打开fb0设备节点*/
	fd = open("/dev/fb0",O_RDWR);
	if( fd == -1)
	{
		printf("open dev fb0 error\n");	
		return -1;
	}

	ioctl(fd,FBIOGET_VSCREENINFO, &fb_varinfo);
	printf("xres = %u yres = %u\n",fb_varinfo.xres,fb_varinfo.yres);

	bitdepth = (int)fb_varinfo.bits_per_pixel;
	width = (int)fb_varinfo.xres;
	height = (int)fb_varinfo.yres;

	/*存储raw文件*/
	Blue = fb_varinfo.blue.offset >> 3;
	Green = fb_varinfo.green.offset >> 3;
	Red = fb_varinfo.red.offset >> 3;
	Alpha = fb_varinfo.transp.offset >> 3;			

	/*计算缓存大小*/
	buf_size = width * height * (bitdepth / 8);

	/*分配空间大小*/
	inbuffer = malloc(buf_size);	
	if(inbuffer == NULL)
	{
		printf("malloc error\n");
		return -1;
	}
	
	outbuffer = malloc(buf_size);
	if( outbuffer == NULL)
	{
		printf("malloc error\n");	
		return -1;
	}
	
	memset(inbuffer,0, buf_size);
	/*将原始数据存储到buf中*/
	if( read(fd, inbuffer, buf_size) != (ssize_t)buf_size)
	{
		printf("not enough memory or data\n");	
		return -1;
	}

	unsigned int i;
	for( i = 0; i < (unsigned int) height*width; i++)
	{
		/* BLUE  = 0 */
		outbuffer[(i<<2)+Blue] = inbuffer[i*4+Blue];
		/* GREEN = 1 */
		outbuffer[(i<<2)+Green] = inbuffer[i*4+Green];
		/* RED   = 2 */
		outbuffer[(i<<2)+Red] = inbuffer[i*4+Red];
		/* ALPHA */
		outbuffer[(i<<2)+Alpha] = inbuffer[i*4+Alpha];	
	}

	FILE *outfile = NULL;	
	png_bytep row_pointers[height];
	outfile = fopen("screen.png","wb");
	
	for( i = 0; i < height; i++)	
		row_pointers[i] = outbuffer + i*4*width;

	png_structp png_ptr;
	/**/	
	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, 
		(png_voidp) NULL, (png_error_ptr) NULL, (png_error_ptr) NULL);

	if( !png_ptr)
	{
		printf("png_create_write_struct error\n");	
		return -1;
	}
	
	png_infop info_ptr;
	/*分配内存并初始化图像信息数据*/	
	info_ptr  = png_create_info_struct(png_ptr);
	if(!info_ptr)
	{
		printf("png_create_info_struct\n");		
		png_destroy_write_struct(&png_ptr,&info_ptr);
		return -1;
	}
	
	if(setjmp(png_jmpbuf(png_ptr)))
	{
		fclose(fd);	
		png_destroy_write_struct(&png_ptr,&info_ptr);
		return -1;
	}
	
	/*设置输出控制,如果使用的是c的标准I/O流*/	
	png_init_io(png_ptr,outfile);	
	
	//png_set_compression_level(png_ptr, Z_BEST_COMPRESSION);
	
	//bit_depth = 8;
	
	//color_type = PNG_COLOR_TYPE_RGB_ALPHA;
	//png_set_invert_alpha(png_ptr);
	//png_set_bgr(png_ptr);	
	/*
			
	*/	
	
	//png_set_IHDR(png_ptr,info_ptr,width,height,bit_depth,color_type, 
	//	PNG_INTERLACE_NONE,PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
	png_set_IHDR(png_ptr,info_ptr,width,height,8,PNG_COLOR_TYPE_RGB_ALPHA,
			PNG_INTERLACE_NONE,PNG_COMPRESSION_TYPE_DEFAULT,PNG_FILTER_TYPE_DEFAULT);
	
	/*设置调色板的颜色集*/
	//png_set_PLTE(png_ptr,info_ptr,palette,PNG_MAX_PALETTE_LENGTH);
	/*标注位(sBIT块)*/
	//png_color_8 sig_bit;	

	/*处理彩色图像*/	
	//sig_bit.red = true_red_bit_depth;
	//sig_bit.green = true_green_depth;
	//sig_bit.blue = true_blue_depth;
	/*如果这个图像有alpha通道*/
	//sig_bit.alpha = true_alpha_bit_depth;
	
	//png_set_sBIT(png_str,info_ptr,&sig_bit);
	/*写入文件头信息(必需)*/
	png_write_info(png_ptr,info_ptr);
	
	

	png_write_image(png_ptr,row_pointers);
	
	png_write_end(png_ptr,info_ptr);

	//png_free(png_ptr,palette);
	//palette = NULL;
	
	png_destroy_write_struct(&png_ptr,&info_ptr);

	if( outfile != NULL)
	{	
		(void)fclose(outfile);	
	}

	return 0;
}
