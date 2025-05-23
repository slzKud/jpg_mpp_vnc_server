#ifndef __GET_IMG_SIZE_H__
    #define __GET_IMG_SIZE_H__
    #include <stdbool.h>
    bool get_image_size_without_decode_image(const char* file_path, int*width, int*height, int * file_size);
#endif