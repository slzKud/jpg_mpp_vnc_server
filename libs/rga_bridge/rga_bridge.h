#ifndef RGA_BRIDGE_H
#define RGA_BRIDGE_H
int rga_cvtcolor(char *data_vir, int src_size, int src_width, int src_height, int src_format, int dst_width, int dst_height, int dst_format, char **dst_buf_ptr,int *dst_buf_size_ptr);
int rga_cvtcolor_fd(int src_fd, int src_size, int src_width, int src_height, int src_format, int dst_width, int dst_height, int dst_format, char **dst_buf_ptr,int *dst_buf_size_ptr);
#endif