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
