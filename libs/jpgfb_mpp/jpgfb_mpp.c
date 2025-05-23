#include "neatvnc.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>
#include <libdrm/drm_fourcc.h>

#include "rk_mpi.h"

#include "mpp_mem.h"
#include "mpp_env.h"
#include "mpp_time.h"
#include "mpp_common.h"
#include "mpi_dec_utils.h"

#include "../rga_bridge/rga_bridge_wrapper.h"

#include "get_img_size.h"
#include <stdbool.h>
#define ERROR_ENABLE
//#define DEBUG_ENABLE
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
int img_w=0;
int img_h=0;
int img_size=0;
struct nvnc_fb* read_jpeg_file(const char* filename);
struct nvnc_fb* read_jpeg_file_no_init_mpp(const char* filename);
bool init_mpp_flag=false;
bool deinit_mpp();
bool init_mpp();
bool get_mpp_flag();
MppCtx ctx = NULL;
MppApi *mpi = NULL;
MppDecCfg cfg = NULL;
DecBufMgr buf_mgr=NULL;
MppFrame frame = NULL;
MppBufferGroup buf_grp = NULL;
bool get_mpp_flag(){
    return init_mpp_flag;
}
bool init_mpp(){
    int ret=0;
    ret = mpp_create(&ctx, &mpi);
    if(ret != MPP_OK){
        ERROR_LOG("mpp_create failed ret=%d",ret);
        goto _EXIT;
    }
    ret = mpp_init(ctx, MPP_CTX_DEC, MPP_VIDEO_CodingMJPEG);
    if(ret != MPP_OK){
        ERROR_LOG("mpp_init failed ret=%d",ret);
        goto _EXIT;
    }
    
    mpp_dec_cfg_init(&cfg);
    ret = mpi->control(ctx, MPP_DEC_GET_CFG, cfg);
    if (ret != MPP_OK){
        ERROR_LOG("%p failed to get decoder cfg ret %d ", ctx, ret);
        goto _EXIT;
    }
    ret = mpp_dec_cfg_set_u32(cfg, "base:split_parse", 1);
    if (ret != MPP_OK){
        ERROR_LOG("%p failed to set split_parse ret %d ", ctx, ret);
        goto _EXIT;
    }
    ret = dec_buf_mgr_init(&buf_mgr);
    if(ret != MPP_OK){
        ERROR_LOG("dec_buf_mgr_init failed ret=%d",ret);
        goto _EXIT;
    }
    init_mpp_flag=true;
    return true;
_EXIT:
    deinit_mpp();
    return false;
}
bool deinit_mpp(){
    DEBUG_LOG("11");
    if (frame) {
        mpp_frame_deinit(&frame);
        frame = NULL;
    }
    DEBUG_LOG("12");
    if (ctx) {
        mpp_destroy(ctx);
        ctx = NULL;
    }
    if (mpi) {      // 通常通过ctx销毁自动释放
        // 注意：根据MPP文档，mpi不需要单独释放
        mpi = NULL;
    }
    if (buf_grp) {
        mpp_buffer_group_put(buf_grp);
        buf_grp = NULL;
    }
    init_mpp_flag=false;
    return true;
}
void reset_decoder_state(){
    if (ctx && mpi) {
        mpi->reset(ctx);
    }
}
struct nvnc_fb* read_jpeg_file_no_init_mpp(const char* filename)
{
    // INIT MPP
    //reset_decoder_state();
    MppFrameFormat dstFormat = MPP_FMT_YUV420SP;
    static MppBuffer frm_buf = NULL;
    int ret = 0;
    const char *input_file = filename;
    size_t file_size;
    void *file_data = NULL;
    FILE *fp_input = NULL, *fp_output = NULL;
    MppBuffer input_buf = NULL;
    MppPacket packet = NULL;
    MppTask task = NULL;
    struct nvnc_fb* fb=NULL;
    DEBUG_LOG("read jpg size");
    if(img_w==0 && img_h==0){
        if(get_image_size_without_decode_image(filename,&img_w,&img_h,&img_size)==false){
            return NULL;
        }
    }
    DEBUG_LOG("read jpg size out,img_w=%d,img_h=%d,img_size=%x",img_w,img_h,img_size);
    
    
    ret = mpi->control(ctx, MPP_DEC_SET_OUTPUT_FORMAT, &(dstFormat));
    if (ret != MPP_OK)
    {
        ERROR_LOG("Failed to set output format 0x%x", dstFormat);
        return NULL;
    }
    ret = mpi->control(ctx, MPP_DEC_SET_CFG, cfg);
    if (ret != MPP_OK){
        ERROR_LOG("%p failed to set cfg %p ret %d ", ctx, cfg, ret);
        goto _EXIT;
    }
    //mpp_dec_cfg_deinit(cfg);
    if(!frame){
        ret = mpp_frame_init(&frame);
        if(ret != MPP_OK){
            ERROR_LOG("mpp_frame_init failed ret=%d",ret);
            goto _EXIT;
        }
    }
    if(!buf_grp){
        buf_grp = dec_buf_mgr_setup(buf_mgr, MPP_ALIGN(img_w,16) * MPP_ALIGN(img_h,16) * 4, 4, MPP_DEC_BUF_HALF_INT);
        if(!buf_grp){
            ERROR_LOG("failed to get buffer group for input frame ret %d", ret);
            ret=MPP_NOK;
            goto _EXIT;
        }
    }
    if (frm_buf) {
        mpp_buffer_put(frm_buf);
        frm_buf = NULL;
    }
    ret = mpp_buffer_get(buf_grp, &frm_buf, MPP_ALIGN(img_w, 16) * MPP_ALIGN(img_h, 16) * 4);
    if(ret != MPP_OK){
        ERROR_LOG("failed to get buffer for input frame ret %d", ret);
        goto _EXIT;
    }
    mpp_frame_set_buffer(frame, frm_buf);

    // 打开输入文件并读取数据
    
    if (!(fp_input = fopen(input_file, "rb")))
    {
        ERROR_LOG("Failed to open input file: %s", input_file);
        return NULL;
    }
    fseek(fp_input, 0, SEEK_END);
    file_size = ftell(fp_input);
    fseek(fp_input, 0, SEEK_SET);
    if (!(file_data = malloc(file_size)))
    {
        ERROR_LOG("Failed to allocate memory");
        fclose(fp_input);
        return NULL;
    }
    if (fread(file_data, 1, file_size, fp_input) != file_size)
    {
        ERROR_LOG("Failed to read file");
        goto _EXIT;
    }
    fclose(fp_input);
    DEBUG_LOG("Loaded %zu bytes from %s", file_size, input_file);

    //DECODE
    DEBUG_LOG("mpp_buffer_get");
    ret = mpp_buffer_get(buf_grp, &input_buf, file_size);
    if(ret != MPP_OK){
        ERROR_LOG("allocate input picture buffer failed");
        goto _EXIT;
    }
    DEBUG_LOG("memcpy");
    memcpy((RK_U8 *)mpp_buffer_get_ptr(input_buf), file_data, file_size);
    DEBUG_LOG("mpp_packet_init_with_buffer");
    ret = mpp_packet_init_with_buffer(&packet, input_buf);
    if(ret != MPP_OK){
        ERROR_LOG("mpp_packet_init_with_buffer failed");
        goto _EXIT;
    }
    DEBUG_LOG("mpi->poll 1");
    ret = mpi->poll(ctx, MPP_PORT_INPUT, MPP_POLL_BLOCK);
    if(ret != MPP_OK){
        ERROR_LOG("%p mpp input poll failed", ctx);
        goto _EXIT;
    }
    DEBUG_LOG("mpi->dequeue 1");
    ret = mpi->dequeue(ctx, MPP_PORT_INPUT, &task);
    if(ret != MPP_OK){
        ERROR_LOG("%p mpp task input dequeue failed", ctx);
        goto _EXIT;
    }
    DEBUG_LOG("mpp_task_meta_set_packet");
    ret = mpp_task_meta_set_packet(task, KEY_INPUT_PACKET, packet);
    ret = mpp_task_meta_set_frame(task, KEY_OUTPUT_FRAME, frame);
    ret = mpi->enqueue(ctx, MPP_PORT_INPUT, task);
    if(ret != MPP_OK){
        ERROR_LOG("%p mpp task input enqueue failed", ctx);
        goto _EXIT;
    }
    DEBUG_LOG("mpi->poll 2");
    ret = mpi->poll(ctx, MPP_PORT_OUTPUT, MPP_POLL_BLOCK);
    if(ret != MPP_OK){
        ERROR_LOG("%p mpp output poll failed", ctx);
        goto _EXIT;
    }
    DEBUG_LOG("mpi->dequeue 2");
    ret = mpi->dequeue(ctx, MPP_PORT_OUTPUT, &task); 
    if(ret != MPP_OK){
        ERROR_LOG("%p mpp task output dequeue failed", ctx);
        goto _EXIT;
    }
    DEBUG_LOG("over");
    if(task){
        if(frame){
            DEBUG_LOG("frame decode success...");
            DEBUG_LOG("Frame Info:");
            DEBUG_LOG("width: %d", mpp_frame_get_width(frame));
            DEBUG_LOG("height: %d", mpp_frame_get_height(frame));
            DEBUG_LOG("hor_stride: %d", mpp_frame_get_hor_stride(frame));
            DEBUG_LOG("ver_stride: %d", mpp_frame_get_ver_stride(frame));
            DEBUG_LOG("format: %d", mpp_frame_get_fmt(frame));
            // WRITE YUV OUTPUT
            //RK_U8 *fp_buf =NULL;
            char* rga_convert_buf=NULL;
            int rga_convert_buf_size=0;
            fb = nvnc_fb_new(img_w, img_h, DRM_FORMAT_RGB888,
                img_w);
            assert(fb);
            char *data_vir = (char *)mpp_buffer_get_ptr(mpp_frame_get_buffer(frame)); // 获取解码输出帧地址
            int fd = mpp_buffer_get_fd(mpp_frame_get_buffer(frame));
            size_t buf_size=mpp_buffer_get_size(mpp_frame_get_buffer(frame));
            DEBUG_LOG("fd=%x,buf_size=%x",fd,buf_size);
            DEBUG_LOG("nvnc_fb_get_addr");
            rga_convert_buf = (uint8_t *) nvnc_fb_get_addr(fb);
            DEBUG_LOG("nvnc_fb_get_addr OUT");
            ret = c_rga_cvtcolor(data_vir,buf_size,mpp_frame_get_hor_stride(frame),mpp_frame_get_ver_stride(frame),2560,img_w,img_h,1792,&rga_convert_buf,&rga_convert_buf_size);
            DEBUG_LOG("c_rga_cvtcolor OUT");
            if(ret!=1){
                ERROR_LOG("conver color via rga failed.ret=%d", ret);
                goto _EXIT;
            }
            DEBUG_LOG("convert success");
            //goto _EXIT;
        }else{
            DEBUG_LOG("frame decode success,but frame empty...");
        }
        ret = mpi->enqueue(ctx, MPP_PORT_OUTPUT, task);
        if (ret != MPP_OK){
            ERROR_LOG("%p mpp task output enqueue failed", ctx);
            goto _EXIT;
        }

    }
_EXIT:
    // 释放规则：后申请的资源先释放，NULL检查防止重复释放
    // if (task) {
    //     mpp_task_put(task);
    //     task = NULL;
    // }
    DEBUG_LOG("1");
    if(1){
        if (packet) {
            mpp_packet_deinit(&packet);
            packet = NULL;
        }
    }else{
        ret=mpi->dequeue(ctx,MPP_PORT_INPUT,&task);
        if(ret){
            ERROR_LOG("%p mpp task input enqueue failed", ctx);
        }
        if(task){
            MppPacket packet_out=NULL;
            mpp_task_meta_get_packet(task,KEY_INPUT_PACKET,&packet_out);
            if(!packet_out || packet_out == packet){
                ERROR_LOG("mismatch packet.");
            }
            mpp_packet_deinit(&packet_out);
            ret= mpi->enqueue(ctx,MPP_PORT_INPUT,task);
            if(ret){
                ERROR_LOG("%p mpp task input enqueue failed", ctx);
            }
        }
    }
    
    DEBUG_LOG("2");
    if (input_buf) {
        mpp_buffer_put(input_buf);
        input_buf = NULL;
    }
    DEBUG_LOG("5");
    if (file_data) {
        free(file_data);
        file_data = NULL;
    }
    // DEBUG_LOG("6");
    // if (frm_buf) {
    //     mpp_buffer_put(frm_buf);
    //     frm_buf = NULL;
    // }
    
    // DEBUG_LOG("9");
    // if (cfg) {
    //     mpp_dec_cfg_deinit(cfg);
    //     cfg = NULL;
    // }
    // DEBUG_LOG("10");
    // if (buf_mgr) {  // 假设有对应的释放函数
    //     dec_buf_mgr_deinit(buf_mgr);
    //     buf_mgr = NULL;
    // }
    return fb;
}
struct nvnc_fb* read_jpeg_file(const char* filename){
    struct nvnc_fb* fb=NULL;
    if(!init_mpp_flag){
        if(init_mpp()==false){
            ERROR_LOG("mpp init failed");
            return NULL;
        }else{
            DEBUG_LOG("mpp init success");
        }
    }
    fb=read_jpeg_file_no_init_mpp(filename);
    deinit_mpp();
    return fb;
}