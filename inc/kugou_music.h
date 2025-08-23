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

// ��������ļ���
#define MAX_MUSIC_COUNT 100
#define MAX_PATH_LEN 256
#define MAX_NAME_LEN 128

// ������Ϣ�ṹ��
typedef struct {
    char filename[MAX_NAME_LEN];      // �ļ���
    char filepath[MAX_PATH_LEN];      // ����·��
    char title[MAX_NAME_LEN];         // ����
    char artist[MAX_NAME_LEN];        // ����
    char album[MAX_NAME_LEN];         // ר��
} music_info_t;

// ����ģʽ
typedef enum {
    PLAY_MODE_SEQUENCE = 0,  // ˳�򲥷�
    PLAY_MODE_RANDOM         // �������
} play_mode_t;

// ����״̬
typedef enum {
    MUSIC_STATE_STOP = 0,    // ֹͣ
    MUSIC_STATE_PLAYING,     // ������
    MUSIC_STATE_PAUSED       // ��ͣ
} music_state_t;

// ���ֲ������ṹ��
typedef struct {
    music_info_t music_list[MAX_MUSIC_COUNT];  // �����б�
    int music_count;                           // �����ļ�����
    int current_index;                         // ��ǰ��������
    play_mode_t play_mode;                     // ����ģʽ
    music_state_t state;                       // ����״̬
    pid_t player_pid;                          // FFplay����ID
    int total_time;                            // ��ʱ��(��)
    int current_time;                          // ��ǰʱ��(��)
} music_player_t;

// ȫ�����ֲ�����ʵ��
extern music_player_t g_music_player;

// ���ֲ��������ܺ���
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
void cleanup_music_player(void);  // �������ֲ�������Դ

// ����ص�����
void music_list_item_cb(lv_event_t * e);
void music_list_gesture_cb(lv_event_t * e);
void music_list_touch_cb(lv_event_t * e);  // �ֶ����Ƽ��
void music_play_pause_cb(lv_event_t * e);
void music_next_cb(lv_event_t * e);
void music_prev_cb(lv_event_t * e);
void music_mode_cb(lv_event_t * e);
void music_back_cb(lv_event_t * e);
void music_player_back_cb(lv_event_t * e);
void music_progress_cb(lv_event_t * e);

// ���ߺ���
void scan_music_files(const char* directory);
void parse_music_info(music_info_t* info);
void update_music_progress(lv_timer_t * timer);

#endif // __KUGOU_MUSIC_H__
