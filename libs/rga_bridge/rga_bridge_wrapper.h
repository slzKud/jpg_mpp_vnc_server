// myclass_wrapper.h

#ifndef RGA_WRAPPER_H
#define RGA_WRAPPER_H

#ifdef __cplusplus
extern "C" {
#endif

int c_rga_cvtcolor(char *data_vir, int src_size, int src_width, int src_height, int src_format, int dst_width, int dst_height, int dst_format, char **dst_buf,int *dst_buf_size);
int c_rga_cvtcolor_fd(int src_fd, int src_size, int src_width, int src_height, int src_format, int dst_width, int dst_height, int dst_format, char **dst_buf_ptr,int *dst_buf_size_ptr);
#ifdef __cplusplus
}
#endif


#endif
