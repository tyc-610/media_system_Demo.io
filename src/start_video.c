#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>

#include "../lvgl/lvgl.h"
#include "../lvgl/demos/lv_demos.h"

// 全局变量用于同步控制
static pthread_mutex_t sync_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t sync_cond = PTHREAD_COND_INITIALIZER;
static volatile int video_ready = 0;
static lv_obj_t* g_video_player = NULL;  // 保存视频播放器对象


//播放音频的函数
void* audio_thread(void* arg) {
    char* audio_file = (char*)arg;
    
    // 等待视频准备就绪的信号
    pthread_mutex_lock(&sync_mutex);
    while (!video_ready) {
        // 音频线程等待视频准备就绪
        pthread_cond_wait(&sync_cond, &sync_mutex);
    }
    pthread_mutex_unlock(&sync_mutex);
    
    // 给视频一些时间开始播放
    sleep(2);
    
    // 视频开始后再启动音频
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "madplay -Q \"%s\"", audio_file);
    system(cmd);
    return NULL;
}

//播放视频函数 
void lv_start_video(void)
{
    // 创建FFmpeg视频播放器
    
    // 禁用屏幕的所有交互（防止在开机动画期间滑动）
    lv_obj_t * screen = lv_screen_active();
    lv_obj_clear_flag(screen, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(screen, LV_OBJ_FLAG_GESTURE_BUBBLE);
    lv_obj_set_scroll_dir(screen, LV_DIR_NONE);
    
    lv_obj_t * video = lv_ffmpeg_player_create(screen);
    if(video == NULL) {
        // 创建视频播放器失败
        return;
    }
    
    // 保存视频播放器对象到全局变量
    g_video_player = video;
    
    // 设置视频源
    lv_ffmpeg_player_set_src(video , "/home/tyc/work_station/start_app/video.mp4");
    
    // 启用自动重启
    lv_ffmpeg_player_set_auto_restart(video , true);
    
    // 居中显示
    lv_obj_center(video);
    
    // 禁用视频播放器的所有交互功能（防止滑动）
    lv_obj_clear_flag(video, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(video, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(video, LV_OBJ_FLAG_GESTURE_BUBBLE);
    lv_obj_clear_flag(video, LV_OBJ_FLAG_SCROLL_ELASTIC);
    lv_obj_clear_flag(video, LV_OBJ_FLAG_SCROLL_MOMENTUM);
    lv_obj_clear_flag(video, LV_OBJ_FLAG_SCROLL_CHAIN);
    
    // 完全禁用滚动
    lv_obj_set_scroll_dir(video, LV_DIR_NONE);
    
    // 设置为不可交互
    lv_obj_add_flag(video, LV_OBJ_FLAG_IGNORE_LAYOUT);
    lv_obj_add_flag(video, LV_OBJ_FLAG_FLOATING);

    // 开始播放视频
    lv_ffmpeg_player_set_cmd(video , LV_FFMPEG_PLAYER_CMD_START);
    
    // 准备音频播放线程
    pthread_t audio_tid;
    char* audio_file = "/home/tyc/work_station/start_app/start.mp3";
    
    // 检查音频文件是否存在
    if(access(audio_file, F_OK) == 0) {
        // 创建音频播放线程
        pthread_create(&audio_tid, NULL, audio_thread, audio_file);
        pthread_detach(audio_tid);
    } else {
        // 音频文件不存在，只播放视频
    }

    // 立即通知音频线程（不要在主线程中sleep）
    pthread_mutex_lock(&sync_mutex);
    video_ready = 1;
    pthread_cond_signal(&sync_cond);
    pthread_mutex_unlock(&sync_mutex);
    
    // 音视频播放器设置完成
}

// 停止视频播放的函数
void lv_stop_video(void)
{
    if (g_video_player != NULL) {
        // 停止视频播放
        lv_ffmpeg_player_set_cmd(g_video_player, LV_FFMPEG_PLAYER_CMD_STOP);
        
        // 停止音频播放
        system("killall madplay 2>/dev/null");
        
        g_video_player = NULL;
        
        // 重置同步状态
        pthread_mutex_lock(&sync_mutex);
        video_ready = 0;
        pthread_mutex_unlock(&sync_mutex);
        
        // 重新启用屏幕交互（为锁屏界面准备）
        lv_obj_t * screen = lv_screen_active();
        lv_obj_add_flag(screen, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_scroll_dir(screen, LV_DIR_ALL);
        
        // 视频播放已停止
    }
}
