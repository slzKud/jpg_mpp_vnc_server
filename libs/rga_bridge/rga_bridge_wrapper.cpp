// myclass_wrapper.cpp

#include "rga_bridge_wrapper.h"
#include "rga_bridge.h"

int c_rga_cvtcolor(char *data_vir, int src_size, int src_width, int src_height, int src_format, int dst_width, int dst_height, int dst_format, char **dst_buf,int *dst_buf_size){
    return rga_cvtcolor(data_vir,src_size,src_width,src_height,src_format,dst_width,dst_height,dst_format,dst_buf,dst_buf_size);
}
int c_rga_cvtcolor_fd(int src_fd, int src_size, int src_width, int src_height, int src_format, int dst_width, int dst_height, int dst_format, char **dst_buf_ptr,int *dst_buf_size_ptr){
    return rga_cvtcolor_fd(src_fd,src_size,src_width,src_height,src_format,dst_width,dst_height,dst_format,dst_buf_ptr,dst_buf_size_ptr);
}