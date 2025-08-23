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

// 最大图片文件数
#define MAX_IMAGE_COUNT 100
#define MAX_PATH_LEN 256
#define MAX_NAME_LEN 128

// 缩略图大小
#define THUMBNAIL_WIDTH 140
#define THUMBNAIL_HEIGHT 105
#define THUMBNAILS_PER_ROW 4

// 图片信息结构体
typedef struct {
    char filename[MAX_NAME_LEN];      // 文件名
    char filepath[MAX_PATH_LEN];      // 完整路径
} image_info_t;

// 相册管理器结构体
typedef struct {
    image_info_t image_list[MAX_IMAGE_COUNT];  // 图片列表
    int image_count;                           // 图片文件数量
    int current_index;                         // 当前查看索引
    lv_obj_t* thumbnail_screen;                // 缩略图界面
    lv_obj_t* viewer_screen;                   // 查看器界面
    lv_obj_t* current_img;                     // 当前显示的图片对象
    lv_obj_t* info_container;                  // 信息显示容器
    lv_obj_t* info_label;                      // 信息显示标签
    lv_timer_t* hide_timer;                    // 隐藏定时器指针
    // 手势检测相关
    bool touch_started;                        // 是否开始触摸
    bool swipe_detected;                       // 是否检测到滑动
    lv_point_t touch_start_point;              // 触摸开始点
    lv_point_t touch_end_point;                // 触摸结束点
} gallery_manager_t;

// 全局相册管理器实例
extern gallery_manager_t g_gallery;

// 相册功能函数
void init_gallery(void);
void show_gallery_thumbnail_screen(void);
void show_image_viewer_screen(int index);
void update_viewer_image(void);
void load_next_image(void);
void load_prev_image(void);
void back_to_thumbnail(void);

// 界面回调函数
void thumbnail_click_cb(lv_event_t * e);
void gallery_gesture_cb(lv_event_t * e);
void gallery_touch_cb(lv_event_t * e);  // 手动手势检测
void image_touch_cb(lv_event_t * e);
void hide_info_timer_cb(lv_timer_t * timer);

// 工具函数
void scan_image_files(const char* directory);
int is_image_file(const char* filename);
void create_thumbnail_grid(lv_obj_t* parent);

#endif // __DCIM_H__
