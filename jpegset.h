//#define TEST_BA
#ifdef TEST_BA
#define MJPEG_COUNT 6574 
#define MJPEG_FILE_SET ("../../vnc_mjpeg_server/ba/ba%04d.jpg")
#else
#define MJPEG_COUNT 2375 
#define MJPEG_FILE_SET ("../../1080test/1080test%04d.jpg")
#endif
#define MJPEG_FPS 30