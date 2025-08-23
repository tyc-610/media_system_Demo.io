#ifndef __DCIM_H__
#define __DCIM_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <ctype.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include "../lvgl/lvgl.h"

// ���ͼƬ�ļ���
#define MAX_IMAGE_COUNT 100
#define MAX_PATH_LEN 256
#define MAX_NAME_LEN 128

// ����ͼ��С
#define THUMBNAIL_WIDTH 140
#define THUMBNAIL_HEIGHT 105
#define THUMBNAILS_PER_ROW 4

// ͼƬ��Ϣ�ṹ��
typedef struct {
    char filename[MAX_NAME_LEN];      // �ļ���
    char filepath[MAX_PATH_LEN];      // ����·��
} image_info_t;

// ���������ṹ��
typedef struct {
    image_info_t image_list[MAX_IMAGE_COUNT];  // ͼƬ�б�
    int image_count;                           // ͼƬ�ļ�����
    int current_index;                         // ��ǰ�鿴����
    lv_obj_t* thumbnail_screen;                // ����ͼ����
    lv_obj_t* viewer_screen;                   // �鿴������
    lv_obj_t* current_img;                     // ��ǰ��ʾ��ͼƬ����
    lv_obj_t* info_container;                  // ��Ϣ��ʾ����
    lv_obj_t* info_label;                      // ��Ϣ��ʾ��ǩ
    lv_timer_t* hide_timer;                    // ���ض�ʱ��ָ��
    // ���Ƽ�����
    bool touch_started;                        // �Ƿ�ʼ����
    bool swipe_detected;                       // �Ƿ��⵽����
    lv_point_t touch_start_point;              // ������ʼ��
    lv_point_t touch_end_point;                // ����������
} gallery_manager_t;

// ȫ����������ʵ��
extern gallery_manager_t g_gallery;

// ��Ṧ�ܺ���
void init_gallery(void);
void show_gallery_thumbnail_screen(void);
void show_image_viewer_screen(int index);
void update_viewer_image(void);
void load_next_image(void);
void load_prev_image(void);
void back_to_thumbnail(void);

// ����ص�����
void thumbnail_click_cb(lv_event_t * e);
void gallery_gesture_cb(lv_event_t * e);
void gallery_touch_cb(lv_event_t * e);  // �ֶ����Ƽ��
void image_touch_cb(lv_event_t * e);
void hide_info_timer_cb(lv_timer_t * timer);

// ���ߺ���
void scan_image_files(const char* directory);
int is_image_file(const char* filename);
void create_thumbnail_grid(lv_obj_t* parent);

#endif // __DCIM_H__
