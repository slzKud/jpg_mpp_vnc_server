/*
 * Copyright (c) 2019 - 2021 Andri Yngvason
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
 * INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE
 * OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

#include <neatvnc.h>
#define AML_UNSTABLE_API 1
#include <stdio.h>
#include <aml.h>
#include <signal.h>
#include <assert.h>
#include <pixman.h>
#include <pthread.h>
#include <string.h>
#include <stdbool.h>
#include "jpegset.h"

struct nvnc_fb* read_jpeg_file(const char* filename);
struct nvnc_fb* read_jpeg_file_no_init_mpp(const char* filename);
bool deinit_mpp();
bool init_mpp();
bool get_mpp_flag();
static int jpeg_index = 0;
static int jpeg_count = MJPEG_COUNT;
int prev_jpeg_index=0;
int drop_frames=0;
static char jpeg_frame_set[128];
int play_flag=0;
struct nvnc* server = NULL;
struct nvnc_display* display = NULL;
pthread_t pthread_id;

static void on_sigint()
{
	aml_exit(aml_get_default());
}
static void update_vnc_buffer(struct nvnc_display* display)
{
	static char jpeg_frame[128];
	static struct nvnc_fb* fb = NULL;
	int len = snprintf(jpeg_frame, sizeof(jpeg_frame), jpeg_frame_set, jpeg_index);
    if (len < 0 || len >= sizeof(jpeg_frame)) {
        printf("Filename too long: %s\n", jpeg_frame);
        return;
    }
	fb = read_jpeg_file_no_init_mpp(jpeg_frame);
	// if(fb_prev==NULL){
	// 	fb_prev = read_jpeg_file_no_init_mpp(jpeg_frame);
	// }
	if (!fb) {
		printf("Failed to read jpg file\n");
		return;
	}
	//printf("1\n");
	uint8_t* addr = nvnc_fb_get_addr(fb);
	//uint8_t* addr1 = nvnc_fb_get_addr(fb_prev);
	//printf("4\n");
	struct pixman_region16 damage;
	const int BLOCK_SIZE = 2048; // 建议值：1024或2048
	int width = nvnc_fb_get_width(fb);
    int height = nvnc_fb_get_height(fb);
	// pixman_region_init_rect(&damage, 0, 0, nvnc_fb_get_width(fb),
	// nvnc_fb_get_height(fb));
	pixman_region_init(&damage);

    // 分块生成区域
    for (int y = 0; y < height; y += BLOCK_SIZE) {
        int h = (y + BLOCK_SIZE > height) ? (height - y) : BLOCK_SIZE;
        for (int x = 0; x < width; x += BLOCK_SIZE) {
            int w = (x + BLOCK_SIZE > width) ? (width - x) : BLOCK_SIZE;
            pixman_region_union_rect(&damage, &damage, x, y, w, h);
        }
    }
//printf("5\n");
	nvnc_display_feed_buffer(display, fb, &damage);
//printf("6\n");
	pixman_region_fini(&damage);
	//printf("3\n");
	prev_jpeg_index=jpeg_index;
	jpeg_index++;
	//printf("2\n");
	nvnc_fb_unref(fb);
}
static void on_pointer_event(struct nvnc_client* client, uint16_t x, uint16_t y,
		enum nvnc_button_mask buttons)
{
	if (!(buttons & NVNC_BUTTON_LEFT))
		return;
	printf("on_pointer_event:start play.\n");
	play_flag=1;
}
void* play_thread(){
	printf("play_thread\n");
	while(1){
		if(play_flag==1){
			if(jpeg_index>=jpeg_count){
				jpeg_index=0;
				play_flag=0;
				printf("play over!\n");
				continue;
			}
			update_vnc_buffer(display);
			usleep((int)(1000*1000/MJPEG_FPS));
		}else{
			usleep((int)(1000*1000/MJPEG_FPS));
		}
	}
}
static void init_play_thread(){
	pthread_create(&pthread_id, NULL, play_thread, NULL);
    pthread_detach(pthread_id);
}
int main(int argc, char* argv[])
{
	if(argc>=3){
		sprintf(jpeg_frame_set,"%s",argv[1]);
		jpeg_count = atoi(argv[2]);
	}else{
		sprintf(jpeg_frame_set,"%s",MJPEG_FILE_SET);
		jpeg_count = MJPEG_COUNT;
	}
	printf("play set :%s,play count:%d\n",jpeg_frame_set,jpeg_count);
	if(init_mpp()==false){
		printf("mpp init failed.\n");
		return 1;
	}
	struct aml* aml = aml_new();
	aml_set_default(aml);

	server = nvnc_open("0.0.0.0", 5900);
	assert(server);

	display = nvnc_display_new(0, 0);
	assert(display);

	nvnc_add_display(server, display);
	nvnc_set_name(server, "Test Display");
	nvnc_set_pointer_fn(server, on_pointer_event);
	nvnc_set_userdata(server, &display, NULL);
	init_play_thread();
	update_vnc_buffer(display);
	

	struct aml_signal* sig = aml_signal_new(SIGINT, on_sigint, NULL, NULL);
	aml_start(aml_get_default(), sig);
	aml_unref(sig);

	aml_run(aml);

	nvnc_close(server);
	nvnc_display_unref(display);
	aml_unref(aml);
	deinit_mpp();
}
