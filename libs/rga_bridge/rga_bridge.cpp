/*
 * Copyright (C) 2022  Rockchip Electronics Co., Ltd.
 * Authors:
 *     YuQiaowei <cerf.yu@rock-chips.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define LOG_NDEBUG 0
#undef LOG_TAG
#define LOG_TAG "rga_cvtcolor_demo"

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <linux/stddef.h>

#include "rga_bridge.h"

#include "RgaUtils.h"
#include "im2d.hpp"
#define ERROR_ENABLE
// DEBUG_LOG 的开关：DEBUG_ENABLE
#ifdef DEBUG_ENABLE
#define DEBUG_LOG(fmt, ...) fprintf(stderr, "[DEBUG] " fmt "\n", ##__VA_ARGS__)
#else
#define DEBUG_LOG(fmt, ...) ((void)0) // 关闭时替换为空操作
#endif

// ERROR_LOG 的开关：ERROR_ENABLE
#ifdef ERROR_ENABLE
#define ERROR_LOG(fmt, ...) fprintf(stderr, "[ERROR] %s:%d: " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#else
#define ERROR_LOG(fmt, ...) ((void)0) // 关闭时替换为空操作
#endif

int rga_cvtcolor(char *data_vir, int src_size, int src_width, int src_height, int src_format, int dst_width, int dst_height, int dst_format, char **dst_buf_ptr,int *dst_buf_size_ptr)
{
    DEBUG_LOG("rga_cvtcolor");
    int ret = 0;
    int src_buf_size, dst_buf_size;
    char *dst_buf;
    rga_buffer_t src_img, dst_img;
    rga_buffer_handle_t src_handle, dst_handle;
    // FILE *file=NULL;
    memset(&src_img, 0, sizeof(src_img));
    memset(&dst_img, 0, sizeof(dst_img));
    DEBUG_LOG("rga_cvtcolor 1");
    src_buf_size = src_width * src_height * get_bpp_from_format(src_format);
    dst_buf_size = dst_width * dst_height * get_bpp_from_format(dst_format);
    DEBUG_LOG("rga_cvtcolor 2");
    dst_buf=*dst_buf_ptr;

    //dst_buf = (char *)malloc(dst_buf_size);
    //memset(dst_buf, 0x80, dst_buf_size);
    DEBUG_LOG("rga_cvtcolor 3");
    src_handle = importbuffer_virtualaddr(data_vir, src_size);
    dst_handle = importbuffer_virtualaddr(dst_buf, dst_buf_size);
    src_img = wrapbuffer_handle(src_handle, src_width, src_height, src_format);
    dst_img = wrapbuffer_handle(dst_handle, dst_width, dst_height, dst_format);
    DEBUG_LOG("rga_cvtcolor 4");
    ret = imcheck(src_img, dst_img, {}, {});
    if (IM_STATUS_NOERROR != ret)
    {
        ERROR_LOG("%d, check error! %s", __LINE__, imStrError((IM_STATUS)ret));
        return 0;
    }

    ret = imcvtcolor(src_img, dst_img, src_format, dst_format);
    if (ret == IM_STATUS_SUCCESS)
    {
        DEBUG_LOG("convert color success! buf_size=%x\n",dst_buf_size);
        *dst_buf_ptr=dst_buf;
        goto release_buffer;
    }
    else
    {
        DEBUG_LOG("convert color failed, %s\n", imStrError((IM_STATUS)ret));
        goto release_buffer;
    }
    DEBUG_LOG("rga_cvtcolor 5");
    // file=fopen("test0510_v4_bgr888.bin","wb");
    // fwrite(dst_buf, dst_buf_size, 1, file);
    // fclose(file);
release_buffer:
    if (src_handle)
        releasebuffer_handle(src_handle);
    if (dst_handle)
        releasebuffer_handle(dst_handle);

    return ret;
}

int rga_cvtcolor_fd(int src_fd, int src_size, int src_width, int src_height, int src_format, int dst_width, int dst_height, int dst_format, char **dst_buf_ptr,int *dst_buf_size_ptr)
{
    int ret = 0;
    int src_buf_size, dst_buf_size;
    char *dst_buf;
    rga_buffer_t src_img, dst_img;
    rga_buffer_handle_t src_handle, dst_handle;
    // FILE *file=NULL;
    memset(&src_img, 0, sizeof(src_img));
    memset(&dst_img, 0, sizeof(dst_img));

    src_buf_size = src_width * src_height * get_bpp_from_format(src_format);
    dst_buf_size = dst_width * dst_height * get_bpp_from_format(dst_format);

    

    dst_buf = (char *)dst_buf_ptr;
    memset(dst_buf, 0x80, dst_buf_size);

    src_handle = importbuffer_fd(src_fd, src_size);
    if (src_handle == 0) {
        ERROR_LOG("import drm fd error!");
        return 0;
    }
    dst_handle = importbuffer_virtualaddr(dst_buf, dst_buf_size);
    src_img = wrapbuffer_handle(src_handle, src_width, src_height, src_format);
    dst_img = wrapbuffer_handle(dst_handle, dst_width, dst_height, dst_format);

    ret = imcheck(src_img, dst_img, {}, {});
    if (IM_STATUS_NOERROR != ret)
    {
        ERROR_LOG("%d, check error! %s", __LINE__, imStrError((IM_STATUS)ret));
        return 0;
    }

    ret = imcvtcolor(src_img, dst_img, src_format, dst_format);
    if (ret == IM_STATUS_SUCCESS)
    {
        DEBUG_LOG("convert color success! buf_size=%x\n",dst_buf_size);
        *dst_buf_size_ptr=dst_buf_size;
        *dst_buf_ptr=dst_buf;
    }
    else
    {
        DEBUG_LOG("convert color failed, %s\n", imStrError((IM_STATUS)ret));
        goto release_buffer;
    }
    // file=fopen("test0510_v4_bgr888.bin","wb");
    // fwrite(dst_buf, dst_buf_size, 1, file);
    // fclose(file);
release_buffer:
    if (src_handle)
        releasebuffer_handle(src_handle);
    if (dst_handle)
        releasebuffer_handle(dst_handle);

    return ret;
}