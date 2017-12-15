#include <linux/fb.h>
#include <stdio.h>
#include <stdint.h> 
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/ioctl.h> 
#include <getopt.h>
#include <strings.h>
#include <unistd.h>


#include <stdlib.h>
#include <sys/vt.h>
#include <png.h>
#include <zlib.h>


static int Blue = 0;
static int Green = 1;
static int Red = 2;
static int Alpha = 3;


typedef unsigned short WORD;
typedef unsigned int DWORD;
typedef unsigned long LONG;

typedef struct tagBITMAPFILEHEADER {
	WORD bfType;
	DWORD bfSize;
	WORD bfReserved1;
	WORD bfReserved2;
	DWORD bfOffBits; 
} __attribute__((packed)) BITMAPFILEHEADER;


typedef struct tagBITMAPINFOHEADER
{
	DWORD biSize; /*info header size in bytes*/
	DWORD biWidth; /*widht of image*/
	DWORD biHeight;/*height of image*/
	WORD biPlanes;/*number of colour planes*/ 
	WORD biBitCount;/*bits per pixel*/
	DWORD biCompression;/*compression type*/
	DWORD biSizeImage;/*image size meter*/
	DWORD biXPelsPerMeter;/*pixels per meter*/
	DWORD biYPelsPerMeter;/*pexels per meter*/
	DWORD biClrUsed;/*number of colour*/
	DWORD biClrImportant;/*important colour*/
} __attribute__((packed)) BITMAPINFOHEADER;

int png_file;
int bmp_file;

struct fb_fix_screeninfo finfo;
struct fb_var_screeninfo vinfo;

static const struct option long_options[]=  
{  
     {"png",1,NULL,'p'},  
     {"bmp",1,NULL,'b'},
     {"help",1,NULL,'h'},    
     {NULL,0,NULL,0}  
}; 

static void usage(const char* pname)  
{  
	fprintf(stderr,  
			"usage: %s [-hpb] [FILENAME]\n"
			"imx6 [option]...\n"  
			"  -p|--png                Save the file as a png.\n"
			"  -b|--bmp                Save the file as a bmp\n"
			"  -h|--help               help information.\n"
			"If FILENAME ends with .png it will be saved as a png.\n"
			"If FILENAME ends with .bmp it will be saved as a bmp.\n"
			"If FILENAME is not given, the default will be saved as a png.\n",
			pname
	       );  
};


static void image_bmp( const char *filename)
{
	printf("starting bmp..\n");
	
	FILE *fp;
	BITMAPFILEHEADER    bmfh;
        BITMAPINFOHEADER    bmih;
	/*bmp图片的开头--BM开头*/
        ((unsigned char *)&bmfh.bfType)[0] = 'B';
        ((unsigned char *)&bmfh.bfType)[1] = 'M';

        bmfh.bfSize =  sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + vinfo.yres * vinfo.xres * 4;
        bmfh.bfReserved1 = 0;
        bmfh.bfReserved2 = 0;
        bmfh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

        bmih.biSize = sizeof(BITMAPINFOHEADER);

        bmih.biWidth = vinfo.xres;
        bmih.biHeight = vinfo.yres;
        bmih.biPlanes = 1;
        bmih.biBitCount = 32;
        bmih.biCompression = 0;
        bmih.biSizeImage = 0; /*说明图像的大小，以字节为单位。当用BI_RGB格式时，总设置为0*/
        bmih.biXPelsPerMeter = 0; /*缺省值*/
        bmih.biYPelsPerMeter = 0;
        bmih.biClrUsed = 0; /*说明位图实际使用的调色板索引数，0：使用所有的调色板索引*/
        bmih.biClrImportant = 0; /*说明对图像显示有重要影响的颜色索引的数目，如果是0，表示都重要*/

	printf("filename = %s\n",filename);

	/*打开文件要存储的文件*/
        FILE* image_file = fopen(filename,"a");
	if( NULL == image_file)
	{
		printf("image fopen fail\n");
	}
	/*写图片文件的头部格式信息*/
        fwrite(&bmfh, sizeof(BITMAPFILEHEADER),1,image_file);
	
        fwrite(&bmih, sizeof(BITMAPINFOHEADER),1,image_file);

        FILE *raw_file = fopen( "test.raw","rb");
	if( NULL == raw_file)	
	{
		printf("rawfile fopen fail..\n");
	}
	
	int ch = getc(raw_file);

	int x, y;
   	for( y = vinfo.yres - 1; y >= 0; y--)
	{
		for(x = 0; x < vinfo.xres; x++)
		{
			/*字节数*/
			long location = (x + vinfo.xoffset) * (vinfo.bits_per_pixel/8) + (y + vinfo.yoffset) * finfo.line_length;
			fseek(raw_file, location, SEEK_SET);
			ch = fgetc(raw_file);
			fputc(ch,image_file);

			ch = fgetc(raw_file);
			fputc(ch,image_file);

			ch = fgetc(raw_file);
			fputc(ch,image_file);

			ch = fgetc(raw_file);
			fputc(ch,image_file); 
		}
	}

        fp = popen("rm ./test.raw","r");
        pclose(fp);

	fclose(raw_file);
	fclose(image_file);
	printf("ending bmp\n");
}

static void image_png( const char *filename)
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
	//png_bytep row_pointers[height];
	png_bytep *row_pointers = malloc(sizeof(png_bytep)*height);
	if( NULL == row_pointers)
	{
		printf("malloc row_pointers\n");		
	}	
	outfile = fopen(filename,"wb");
	
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
	
	/*读取图像宽度(width)，高度(height)，位深(bit_depth)，颜色类型(color_type)，压缩方法(compression_type)*/
    	/*滤波器方法(filter_type),隔行扫描方式(interlace_type)*/
	png_set_IHDR(png_ptr,info_ptr,width,height,8,PNG_COLOR_TYPE_RGB_ALPHA,
			PNG_INTERLACE_NONE,PNG_COMPRESSION_TYPE_DEFAULT,PNG_FILTER_TYPE_DEFAULT);
	
	/*写入文件头信息(必需)*/
	png_write_info(png_ptr,info_ptr);
	
	/*写png文件格式内容*/
	png_write_image(png_ptr,row_pointers);
	
	/*写png的结束标记*/
	png_write_end(png_ptr,info_ptr);


	free(row_pointers);
	/*释放&png_ptr,&info_ptr内存空间*/
	png_destroy_write_struct(&png_ptr,&info_ptr);

	if( outfile != NULL)
	{	
		(void)fclose(outfile);	
	}

	return 0;
}

int main(int argc, char **argv)
{
	int opt = 0;
	int options_index = 0;
	char *tmp = NULL;
	char filename[126] = {0};
	char type[126] = {0};
	const char *fn;
	int len;
	if(argc == 1)
	{
		usage(argv[0]);
		return 2;	
	}
	/* 解析命令行参数*/
	while((opt=getopt_long(argc,argv,"pbh?",long_options,&options_index))!=EOF )  
	{  
		switch(opt)  
		{    
			case 'p': 
				png_file=1;
				//sprintf(filename,"%s",optarg);
				if(argv[optind] == NULL)
				{
					sprintf(filename, "%s.png", argv[0]);
					printf("filename = %s\n", filename);	
				}
				else
				{
					//sprintf(filename, "%s.png", argv[0]);
					fn = argv[optind];
					len = strlen(fn);
					if (len >= 4 && 0 == strcmp(fn+len-4, ".png")) {
						sprintf(filename, "%s", argv[optind]);
						printf("filename = %s\n", filename);
					}
					else
					{
						sprintf(filename, "%s.png", argv[optind]);
						printf("filename = %s\n", filename);
					}
				}
				break;  
			case 'b': 
				bmp_file=1;
				//sprintf(type,"%s",optarg);
				if(argv[optind] == NULL)
				{
					sprintf(filename, "%s.bmp", argv[0]);
					printf("filename = %s\n", filename);	
				}
				else
				{
					fn = argv[optind];
					len = strlen(fn);
					if (len >= 4 && 0 == strcmp(fn+len-4, ".bmp")) {
						sprintf(filename, "%s", argv[optind]);
						printf("filename = %s\n", filename);
					}
					else
					{
						sprintf(filename, "%s.bmp", argv[optind]);
						printf("filename = %s\n", filename);
					}
				}
				break;   
			case 'h':  
			case '?': 
				usage(argv[0]);return 2;break;  
		}  
	}
	

	int fb_fd = open("/dev/fb0",O_RDWR);
	if(fb_fd == -1)
	{
		printf("open fail..\n");
		return -1;
	}


	ioctl(fb_fd, FBIOGET_VSCREENINFO, &vinfo);
	ioctl(fb_fd, FBIOGET_FSCREENINFO, &finfo);
	close(fb_fd);
	long screensize = vinfo.yres_virtual * finfo.line_length;
 
	if( !bmp_file && !png_file)
	{
		png_file = 1;
		sprintf(filename, "%s.png", argv[0]);	
	}
	/*存储为bmp文件*/
	if( bmp_file)
	{
		FILE *fp = popen("cat /dev/fb0 >> test.raw","r");
		pclose(fp);
		printf("bmp_file = %s\n", filename);
		image_bmp(filename);
	}
	/*默认保存为png, 或是直接选择-p 保存为png图片*/
	if( png_file)
	{
		printf("bmp_file = %s\n", filename);
		image_png(filename);
	}
	
	

	return 0;
}
