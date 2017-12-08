#include <linux/fb.h>
#include <stdio.h>
#include <stdint.h> 
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/ioctl.h> 
#include <getopt.h>
#include <strings.h>
#include <unistd.h>
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

int output_file;
int type_file;

struct fb_fix_screeninfo finfo;
struct fb_var_screeninfo vinfo;

static const struct option long_options[]=  
{  
     {"output",1,NULL,'o'},  
     {"t",1,NULL,'t'},    
     {NULL,0,NULL,0}  
}; 

static void usage(void)  
{  
	fprintf(stderr,  
			"imax6 [option]...\n"  
			"  -o|--output               Output the filename.\n"  
			"  -t|--type               Output the type of thefilename.\n"
			"  -h|--help               help information.\n"
	       );  
};


static void image_bmp( const char *filename)
{
	printf("starting bmp..\n");
	char tmpbufilename[126] = {0};
	if( NULL != filename)
	{
		strcpy(tmpbufilename, filename);				
		strcat(tmpbufilename,".bmp");
	}else
	{
		strcpy(tmpbufilename,"screen.bmp");
	}
	
	FILE *fp;
	BITMAPFILEHEADER    bmfh;
        BITMAPINFOHEADER    bmih;

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

	printf("tmpbufilename = %s\n",tmpbufilename);

        FILE* image_file = fopen(tmpbufilename,"a");
	if( NULL == image_file)
	{
		printf("image fopen fail\n");
	}

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

int main(int argc, char **argv)
{
	int opt = 0;
	int options_index = 0;
	char *tmp = NULL;
	char filename[126] = {0};
	char type[126] = {0};

	if(argc == 1)
	{
		usage();
		return 2;	
	}
	/* 解析命令行参数*/
	while((opt=getopt_long(argc,argv,"o:t:h?",long_options,&options_index))!=EOF )  
	{  
		switch(opt)  
		{    
			case 'o': 
				output_file=1;
				sprintf(filename,"%s",optarg);
				break;  
			case 't': 
				type_file=1;
				sprintf(type,"%s",optarg);
				break;   
			case 'h':  
			case '?': 
				usage();return 2;break;  
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

	long screensize = vinfo.yres_virtual * finfo.line_length;
	FILE *fp = popen("bash ./test.sh","r");	
	pclose(fp);

	//bmp 
	if( output_file && type_file )
	{
		if( !strcmp(type,"bmp"))	
		{
			image_bmp(filename);
		}else if( !strcmp(type,"png"))
		{
			printf("png type = %s\n",type);
		}
		else
		{
			printf("unkown\n");
		}
	
	}	
	else
	{
		usage();return 2;
	}

	close(fb_fd);

	return 0;
}
