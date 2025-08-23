#ifndef __KUGOU_MUSIC_H__
#define __KUGOU_MUSIC_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <ctype.h>
#include <stdint.h>
#include <math.h>
#include "../lvgl/lvgl.h"

// 最大音乐文件数
#define MAX_MUSIC_COUNT 100
#define MAX_PATH_LEN 256
#define MAX_NAME_LEN 128

// 音乐信息结构体
typedef struct {
    char filename[MAX_NAME_LEN];      // 文件名
    char filepath[MAX_PATH_LEN];      // 完整路径
    char title[MAX_NAME_LEN];         // 歌名
    char artist[MAX_NAME_LEN];        // 歌手
    char album[MAX_NAME_LEN];         // 专辑
} music_info_t;

// 播放模式
typedef enum {
    PLAY_MODE_SEQUENCE = 0,  // 顺序播放
    PLAY_MODE_RANDOM         // 随机播放
} play_mode_t;

// 播放状态
typedef enum {
    MUSIC_STATE_STOP = 0,    // 停止
    MUSIC_STATE_PLAYING,     // 播放中
    MUSIC_STATE_PAUSED       // 暂停
} music_state_t;

// 音乐播放器结构体
typedef struct {
    music_info_t music_list[MAX_MUSIC_COUNT];  // 音乐列表
    int music_count;                           // 音乐文件数量
    int current_index;                         // 当前播放索引
    play_mode_t play_mode;                     // 播放模式
    music_state_t state;                       // 播放状态
    pid_t player_pid;                          // FFplay进程ID
    int total_time;                            // 总时长(秒)
    int current_time;                          // 当前时间(秒)
} music_player_t;

// 全局音乐播放器实例
extern music_player_t g_music_player;

// 音乐播放器功能函数
void init_music_player(void);
void show_music_list_screen(void);
void show_music_player_screen(void);
void play_music(int index);
void pause_music(void);
void resume_music(void);
void stop_music(void);
void next_music(void);
void prev_music(void);
void toggle_play_mode(void);
void set_music_position(int position);
void cleanup_music_player(void);  // 清理音乐播放器资源

// 界面回调函数
void music_list_item_cb(lv_event_t * e);
void music_list_gesture_cb(lv_event_t * e);
void music_list_touch_cb(lv_event_t * e);  // 手动手势检测
void music_play_pause_cb(lv_event_t * e);
void music_next_cb(lv_event_t * e);
void music_prev_cb(lv_event_t * e);
void music_mode_cb(lv_event_t * e);
void music_back_cb(lv_event_t * e);
void music_player_back_cb(lv_event_t * e);
void music_progress_cb(lv_event_t * e);

// 工具函数
void scan_music_files(const char* directory);
void parse_music_info(music_info_t* info);
void update_music_progress(lv_timer_t * timer);

#endif // __KUGOU_MUSIC_H__
