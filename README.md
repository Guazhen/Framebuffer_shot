# Framebuffer_shot
该文件用于截取framebuffer的图片

#这是用于在嵌入式系统，imax6平台进行的framebuffer的截屏操作
#首先需要搭建交叉编译环境

#这是我使用的交叉编译，测试时，使用的test.c  生成可执行文件时imax6_4
arm-poky-linux-gnueabi-gcc  -march=armv7-a -mfloat-abi=hard -mfpu=neon -mtune=cortex-a9 --sysroot=/opt/fsl-imx-wayland/3.14.52-1.1.1/sysroots/cortexa9hf-vfp-neon-poky-linux-gnueabi -DSAL_PLATFORM_IMX6  -O2 -pipe -g -feliminate-unused-debug-types  -DSAL_PLATFORM_IMX6  -g -O2 -fno-strict-aliasing -pipe -Wall -W -Wold-style-definition -g -Wall -O1 -fomit-frame-pointer  -Wl,-O1 -Wl,--hash-style=gnu -Wl,--as-needed  -g  test4.c -o imax6_4




#生成可执行程序后，要指定输出的文件名，和文件格式（本例只实现了bmp格式的）
./imax6_4 -o filaname -t type
example: ./imax6_4 -o imax6_4 -t bmp

#文件输出的文件名:imax6_4  文件格式: bmp



framebuffer_shot_png.c
#该文件实现了对嵌入式平台imx平台的/dev/fb0的文件进行读取,保存未png图片的
arm-poky-linux-gnueabi-gcc  -march=armv7-a -mfloat-abi=hard -mfpu=neon -mtune=cortex-a9 --sysroot=/opt/fsl-imx-wayland/3.14.52-1.1.1/sysroots/cortexa9hf-vfp-neon-poky-linux-gnueabi -DSAL_PLATFORM_IMX6  -O2 -pipe -g -feliminate-unused-debug-types  -DSAL_PLATFORM_IMX6  -g -O2 -fno-strict-aliasing -pipe -Wall -W -Wold-style-definition -g -Wall -O1 -fomit-frame-pointer  -Wl,-O1 -Wl,--hash-style=gnu -Wl,--as-needed  -g  demo_2.c -o imx6png -lpng

交叉编译的环境需要根据实际的情况进行配置


#framework_shot.c 将实现bmp和png的代码整合在一起编译的时候需要 指定 -lpng动态库
#example:
  usage: ./imx6_shot [-hpb] [FILENAME]
imx6 [option]...
  -p|--png                Save the file as a png.
  -b|--bmp                Save the file as a bmp
  -h|--help               help information.
If FILENAME ends with .png it will be saved as a png.
If FILENAME ends with .bmp it will be saved as a bmp.
If FILENAME is not given, the default will be saved as a png.

默认保存为png图片
-p 保存为png格式图片
-b 保存为bmp格式图片
