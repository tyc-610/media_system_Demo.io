#ifndef __MAIN_SCREEN_H__
#define __MAIN_SCREEN_H__

#include "../lvgl/lvgl.h"
#include "kugou_music.h"
#include "dcim.h"
#include "data.h"
#include "video.h"





// 桌面函数声明
void desktop(void);

// 应用启动函数声明
void start_music_app(void);
void start_gallery_app(void);

// 事件回调函数声明
void music_btn_event_cb(lv_event_t * e);
void gallery_btn_event_cb(lv_event_t * e);
void calendar_btn_event_cb(lv_event_t * e);
void desktop_long_press_cb(lv_event_t * e);  // 桌面长按回调

// 设置状态栏标签
void set_status_bar_label(lv_obj_t* label);

#endif
