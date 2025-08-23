#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>

#include "../lvgl/lvgl.h"
#include "../lvgl/demos/lv_demos.h"
#include "../inc/screen.h"

#include "../inc/video.h"


void start_video_app()
{
    //先初始化视频应用
    init_video();
}

void init_video()
{
    //初始化视频应用
    //添加背景图
    
}


void video_test()
{
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "%s%s", MPLAYER_CMD, "/home/tyc/work_station/video/11111.mp4");
    system(cmd);
}