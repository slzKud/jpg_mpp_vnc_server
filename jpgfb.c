/*
 * This is not a part of the library. It's only used in tests, examples and
 * benchmarks.
 */

#include "neatvnc.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "jpeglib.h"
#include <stdint.h>
#include <assert.h>
#include <libdrm/drm_fourcc.h>

struct nvnc_fb* read_jpeg_file(const char* filename);

struct nvnc_fb* read_jpeg_file(const char* filename)
{

	FILE* fp = fopen(filename, "rb");

	struct jpeg_decompress_struct cinfo;

	struct jpeg_error_mgr jerr;

	cinfo.err = jpeg_std_error(&jerr);

	jpeg_create_decompress(&cinfo);

	jpeg_stdio_src(&cinfo,fp);//指定解压缩数据源

    jpeg_read_header(&cinfo,TRUE);//获取文件信息

    jpeg_start_decompress(&cinfo);//开始解压缩

	unsigned long width = cinfo.output_width;//图像宽度

    unsigned long height = cinfo.output_height;//图像高度

    unsigned short depth = cinfo.output_components;//图像深度

    //printf("width:%ld,height:%ld,depth:%d\n",width,height,depth);

	// unsigned char *src_buff;

	struct nvnc_fb* fb = nvnc_fb_new(width, height, DRM_FORMAT_BGR888,
			width);

	assert(fb);

	uint8_t* addr = nvnc_fb_get_addr(fb);
    //printf("nvnc_fb created.\n");
	// src_buff = (unsigned char *)malloc(width * height * depth);//分配位图数据空间
	
    //memset(addr, 0, sizeof(unsigned char) * width * height * depth);//清0
	
	JSAMPARRAY buffer;

	buffer = (*cinfo.mem->alloc_sarray)((j_common_ptr)&cinfo, JPOOL_IMAGE, width*depth, 1);

	unsigned char *point = addr;
    //printf("nvnc_fb read.\n");
	while(cinfo.output_scanline < height){
		jpeg_read_scanlines(&cinfo, buffer, 1);
		memcpy(point, *buffer, width*depth);
		point += width * depth;
	}
    //printf("nvnc_fb read ok.\n");
	jpeg_finish_decompress(&cinfo);//解压缩完毕

    jpeg_destroy_decompress(&cinfo);// 释放资源

    //free(src_buff);//释放资源

    fclose(fp);

	return fb;
}
/*

#include "jpeglib.h"

#include <stdio.h>

#include <stdlib.h>

#include <string.h>


int main()

{

FILE *input_file;

input_file=fopen("test.jpg","rb");
 

struct jpeg_decompress_struct cinfo;//JPEG图像在解码过程中，使用jpeg_decompress_struct类型的结构体来表示，图像的所有信息都存储在结构体中
    struct jpeg_error_mgr jerr;//定义一个标准的错误结构体，一旦程序出现错误就会调用exit()函数退出进程

   

cinfo.err = jpeg_std_error(&jerr);//绑定错误处理结构对象

    jpeg_create_decompress(&cinfo);//初始化cinfo结构
    jpeg_stdio_src(&cinfo,input_file);//指定解压缩数据源
    jpeg_read_header(&cinfo,TRUE);//获取文件信息
    jpeg_start_decompress(&cinfo);//开始解压缩

    unsigned long width = cinfo.output_width;//图像宽度
    unsigned long height = cinfo.output_height;//图像高度
    unsigned short depth = cinfo.output_components;//图像深度

     unsigned char *src_buff;//用于存取解码之后的位图数据(RGB格式)
	src_buff = (unsigned char *)malloc(width * height * depth);//分配位图数据空间
    memset(src_buff, 0, sizeof(unsigned char) * width * height * depth);//清0

     JSAMPARRAY buffer;//用于存取一行数据
    buffer = (*cinfo.mem->alloc_sarray)((j_common_ptr)&cinfo, JPOOL_IMAGE, width*depth, 1);//分配一行数据空间

unsigned char *point = src_buff;
    while(cinfo.output_scanline < height)//逐行读取位图数据
    {
        jpeg_read_scanlines(&cinfo, buffer, 1);    //读取一行jpg图像数据到buffer
        memcpy(point, *buffer, width*depth);    //将buffer中的数据逐行给src_buff
        point += width * depth;            //指针偏移一行
    }

    jpeg_finish_decompress(&cinfo);//解压缩完毕
    jpeg_destroy_decompress(&cinfo);// 释放资源
    free(src_buff);//释放资源

    fclose(input_file);

}
————————————————

                            版权声明：本文为博主原创文章，遵循 CC 4.0 BY-SA 版权协议，转载请附上原文出处链接和本声明。
                        
原文链接：https://blog.csdn.net/lushoumin/article/details/70140576
*/