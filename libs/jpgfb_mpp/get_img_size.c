#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

bool get_image_size_without_decode_image(const char* file_path, int* width, int* height, int* file_size) {
    FILE* fp = fopen(file_path, "rb");
    if (!fp) {
        fprintf(stderr, "无法打开文件: %s\n", file_path);
        return false;
    }

    // 获取文件大小
    fseek(fp, 0, SEEK_END);
    *file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    // 读取整个文件到内存
    unsigned char* data = (unsigned char*)malloc(*file_size);
    if (!data) {
        fclose(fp);
        fprintf(stderr, "内存分配失败\n");
        return false;
    }

    if (fread(data, 1, *file_size, fp) != *file_size) {
        free(data);
        fclose(fp);
        fprintf(stderr, "文件读取失败\n");
        return false;
    }
    fclose(fp);

    // 检查JPEG文件头
    if (*file_size < 2 || data[0] != 0xFF || data[1] != 0xD8) {
        free(data);
        fprintf(stderr, "无效的JPEG文件\n");
        return false;
    }

    size_t pos = 2;  // 跳过文件头FFD8
    bool found = false;

    while (pos < *file_size) {
        // 寻找标记前缀0xFF
        if (data[pos] != 0xFF) {
            pos++;
            continue;
        }

        unsigned char marker = data[pos + 1];
        pos += 2;  // 跳过FF和标记字节

        // 处理结束标记
        if (marker == 0xD9) break;  // EOI

        // 处理SOF标记（0xC0-0xCF，排除特定标记）
        if ((marker >= 0xC0 && marker <= 0xCF) && 
            !(marker == 0xC4 || marker == 0xCC || marker == 0xDA)) {
            
            if (pos + 4 > *file_size) {
                free(data);
                fprintf(stderr, "无效的JPEG结构\n");
                return false;
            }

            // 读取段长度
            int length = (data[pos] << 8) | data[pos + 1];
            if (length < 8 || (pos + length) > *file_size) {
                free(data);
                fprintf(stderr, "无效的SOF段长度\n");
                return false;
            }

            // 提取分辨率（大端序）
            *height = (data[pos + 3] << 8) | data[pos + 4];
            *width = (data[pos + 5] << 8) | data[pos + 6];
            
            found = true;
            break;
        }
        // 处理其他段
        else if (marker == 0xDA) {  // SOS标记
            break;
        } else {
            if (pos + 2 > *file_size) break;
            int segment_length = (data[pos] << 8) | data[pos + 1];
            pos += segment_length;
        }
    }

    free(data);
    return found;
}