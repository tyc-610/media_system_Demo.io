#ifndef __DATA_H__
#define __DATA_H__

#include <time.h>
#include "../lvgl/lvgl.h"







// RTC硬件时钟相关函数
int rtc_read_time(struct tm *tm_time);
int rtc_set_time(struct tm *tm_time);

// 时间显示相关函数
void init_time_display(void);
void start_time_settings_app(void);
void status_bar_timer_cb(lv_timer_t * timer);
void set_status_bar_label(lv_obj_t* label);

// 时间设置相关函数
void time_settings_confirm_cb(lv_event_t * e);
void time_settings_cancel_cb(lv_event_t * e);
void time_settings_gesture_cb(lv_event_t * e);  // 手势检测回调
void textarea_focus_cb(lv_event_t * e);
void window_click_cb(lv_event_t * e);

// 获取当前显示时间
struct tm* get_current_display_time(void);

#endif // __DATA_H__
