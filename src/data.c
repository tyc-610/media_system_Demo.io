#include <time.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <linux/rtc.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>

#include "../lvgl/lvgl.h"
#include "../inc/data.h"

// 全局变量
static lv_obj_t * status_bar_label = NULL;
static lv_timer_t * status_timer = NULL;

// 当前显示的时间（用于每秒更新）
static struct tm current_display_time = {
    .tm_year = 125,  // 2025年 (2025-1900)
    .tm_mon = 6,     // 7月 (0-11)
    .tm_mday = 18,   // 18日
    .tm_hour = 12,   // 12时
    .tm_min = 30,    // 30分
    .tm_sec = 45     // 45秒
};

// 键盘数据结构
typedef struct {
    lv_obj_t * keyboard;
} kb_data_t;

static kb_data_t g_kb_data;

/**
 * @brief 初始化时间显示
 */
void init_time_display(void)
{
    // 立即从RTC读取当前时间并显示，如果失败则使用默认时间
    if (rtc_read_time(&current_display_time) != 0) {
        // 使用默认时间（保持原有的默认设置）
        current_display_time.tm_year = 125;  // 2025年 (2025-1900)
        current_display_time.tm_mon = 6;     // 7月 (0-11)
        current_display_time.tm_mday = 18;   // 18日
        current_display_time.tm_hour = 12;   // 12时
        current_display_time.tm_min = 30;    // 30分
        current_display_time.tm_sec = 45;    // 45秒
    }
}

/**
 * @brief 获取当前显示时间
 */
struct tm* get_current_display_time(void)
{
    return &current_display_time;
}

/**
 * @brief 状态栏定时器回调函数
 * @param timer 定时器对象
 * 
 * 功能说明：
 * 从RTC硬件时钟读取真实时间并更新显示
 */
void status_bar_timer_cb(lv_timer_t * timer)
{
    if (status_bar_label == NULL) return;
    
    char time_str[100];
    char weekday_str[7][10] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
    char month_str[12][4] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", 
                             "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
    
    struct tm rtc_time;
    
    // 尝试从RTC读取时间
    if (rtc_read_time(&rtc_time) == 0) {
        // 成功读取RTC时间，更新全局时间变量
        current_display_time = rtc_time;
    } else {
        // RTC读取失败，使用软件模拟时间（向前推进一秒）
        
        current_display_time.tm_sec++;
        
        // 处理秒数溢出
        if (current_display_time.tm_sec >= 60) {
            current_display_time.tm_sec = 0;
            current_display_time.tm_min++;
            
            // 处理分钟溢出
            if (current_display_time.tm_min >= 60) {
                current_display_time.tm_min = 0;
                current_display_time.tm_hour++;
                
                // 处理小时溢出
                if (current_display_time.tm_hour >= 24) {
                    current_display_time.tm_hour = 0;
                    current_display_time.tm_mday++;
                    
                    // 简单处理日期溢出（这里简化处理，不考虑月份天数差异）
                    if (current_display_time.tm_mday > 31) {
                        current_display_time.tm_mday = 1;
                        current_display_time.tm_mon++;
                        
                        // 处理月份溢出
                        if (current_display_time.tm_mon >= 12) {
                            current_display_time.tm_mon = 0;
                            current_display_time.tm_year++;
                        }
                    }
                }
            }
        }
    }
    
    // 格式化时间字符串
    snprintf(time_str, sizeof(time_str), "%s %s %02d, %04d  %02d:%02d:%02d",
             weekday_str[current_display_time.tm_wday % 7],  // 使用正确的星期计算
             month_str[current_display_time.tm_mon % 12],
             current_display_time.tm_mday,
             current_display_time.tm_year + 1900,
             current_display_time.tm_hour,
             current_display_time.tm_min,
             current_display_time.tm_sec);
    
    lv_obj_set_style_text_color(status_bar_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN); // 白色
    lv_label_set_text(status_bar_label, time_str);
}

/**
 * @brief 设置状态栏标签对象（由main_screen.c调用）
 */
void set_status_bar_label(lv_obj_t* label)
{
    status_bar_label = label;
}

/**
 * @brief 启动时间设置应用
 * 
 * 功能说明：
 * 创建时间设置界面，允许用户设置系统时间
 */
void start_time_settings_app(void)
{
    // 创建时间设置窗口
    lv_obj_t * time_win = lv_obj_create(lv_screen_active());
    lv_obj_set_size(time_win, 800, 480);  // 使用全屏尺寸
    lv_obj_set_pos(time_win, 0, 0);       // 位置设为屏幕左上角
    lv_obj_set_style_bg_color(time_win, lv_color_hex(0x2c3e50), LV_PART_MAIN);
    lv_obj_set_style_radius(time_win, 0, LV_PART_MAIN);  // 去掉圆角以适应全屏
    lv_obj_set_style_pad_all(time_win, 0, LV_PART_MAIN);
    lv_obj_set_style_border_width(time_win, 0, LV_PART_MAIN);
    
    // 设置背景图片
    lv_obj_t * bg_img = lv_image_create(time_win);
    lv_obj_set_size(bg_img, 800, 480);
    lv_obj_set_pos(bg_img, 0, 0);
    lv_image_set_src(bg_img, "/home/tyc/work_station/data/data_back.jpg");
    
    // 标题
    lv_obj_t * title = lv_label_create(time_win);
    lv_label_set_text(title, "Time Settings");
    lv_obj_set_style_text_font(title, &lv_font_montserrat_20, LV_PART_MAIN);
    lv_obj_set_style_text_color(title, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 20);
    
    // 年份设置
    lv_obj_t * year_label = lv_label_create(time_win);
    lv_label_set_text(year_label, "Year:");
    lv_obj_set_style_text_color(year_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_font(year_label, &lv_font_montserrat_16, LV_PART_MAIN);
    lv_obj_set_pos(year_label, 115, 50);
    
    lv_obj_t * year_ta = lv_textarea_create(time_win);
    lv_obj_set_size(year_ta, 120, 40);  // 稍微增大输入框
    lv_obj_set_pos(year_ta, 95, 75);
    lv_obj_set_style_bg_opa(year_ta, LV_OPA_TRANSP, LV_PART_MAIN);  // 设置透明背景
    lv_textarea_set_one_line(year_ta, true);
    lv_textarea_set_accepted_chars(year_ta, "0123456789");
    lv_textarea_set_max_length(year_ta, 4);
    lv_textarea_set_text(year_ta, "2025");  // 设置默认年份
    
    // 月份设置
    lv_obj_t * month_label = lv_label_create(time_win);
    lv_label_set_text(month_label, "Month:");
    lv_obj_set_style_text_color(month_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_font(month_label, &lv_font_montserrat_16, LV_PART_MAIN);
    lv_obj_set_pos(month_label, 265, 50);
    
    lv_obj_t * month_ta = lv_textarea_create(time_win);
    lv_obj_set_size(month_ta, 100, 40);
    lv_obj_set_pos(month_ta, 255, 75);
    lv_obj_set_style_bg_opa(month_ta, LV_OPA_TRANSP, LV_PART_MAIN);  // 设置透明背景
    lv_textarea_set_one_line(month_ta, true);
    lv_textarea_set_accepted_chars(month_ta, "0123456789");
    lv_textarea_set_max_length(month_ta, 2);
    lv_textarea_set_text(month_ta, "07");  // 设置默认月份为7月
    
    // 日期设置
    lv_obj_t * day_label = lv_label_create(time_win);
    lv_label_set_text(day_label, "Day:");
    lv_obj_set_style_text_color(day_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_font(day_label, &lv_font_montserrat_16, LV_PART_MAIN);
    lv_obj_set_pos(day_label, 405, 50);
    
    lv_obj_t * day_ta = lv_textarea_create(time_win);
    lv_obj_set_size(day_ta, 100, 40);
    lv_obj_set_pos(day_ta, 385, 75);
    lv_obj_set_style_bg_opa(day_ta, LV_OPA_TRANSP, LV_PART_MAIN);  // 设置透明背景
    lv_textarea_set_one_line(day_ta, true);
    lv_textarea_set_accepted_chars(day_ta, "0123456789");
    lv_textarea_set_max_length(day_ta, 2);
    lv_textarea_set_text(day_ta, "18");  // 设置默认日期为18日
    
    // 小时设置
    lv_obj_t * hour_label = lv_label_create(time_win);
    lv_label_set_text(hour_label, "Hour:");
    lv_obj_set_style_text_color(hour_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_font(hour_label, &lv_font_montserrat_16, LV_PART_MAIN);
    lv_obj_set_pos(hour_label, 545, 50);
    
    lv_obj_t * hour_ta = lv_textarea_create(time_win);
    lv_obj_set_size(hour_ta, 100, 40);
    lv_obj_set_pos(hour_ta, 525, 75);
    lv_obj_set_style_bg_opa(hour_ta, LV_OPA_TRANSP, LV_PART_MAIN);  // 设置透明背景
    lv_textarea_set_one_line(hour_ta, true);
    lv_textarea_set_accepted_chars(hour_ta, "0123456789");
    lv_textarea_set_max_length(hour_ta, 2);
    lv_textarea_set_text(hour_ta, "12");  // 设置默认小时为12时
    
    // 分钟设置
    lv_obj_t * min_label = lv_label_create(time_win);
    lv_label_set_text(min_label, "Min:");
    lv_obj_set_style_text_color(min_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_font(min_label, &lv_font_montserrat_16, LV_PART_MAIN);
    lv_obj_set_pos(min_label, 685, 50);
    
    lv_obj_t * min_ta = lv_textarea_create(time_win);
    lv_obj_set_size(min_ta, 100, 40);
    lv_obj_set_pos(min_ta, 665, 75);
    lv_obj_set_style_bg_opa(min_ta, LV_OPA_TRANSP, LV_PART_MAIN);  // 设置透明背景
    lv_textarea_set_one_line(min_ta, true);
    lv_textarea_set_accepted_chars(min_ta, "0123456789");
    lv_textarea_set_max_length(min_ta, 2);
    lv_textarea_set_text(min_ta, "30");  // 设置默认分钟为30分
    
    // 创建LVGL内置键盘
    lv_obj_t * kb = lv_keyboard_create(time_win);
    lv_obj_set_size(kb, 750, 200);  // 增大键盘尺寸
    lv_obj_set_pos(kb, 25, -35);    // 键盘位置调整到合适位置
    lv_keyboard_set_mode(kb, LV_KEYBOARD_MODE_NUMBER);
    lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);  // 默认隐藏键盘
    
    // 将键盘指针保存到全局数据结构中
    g_kb_data.keyboard = kb;
    
    // 为时间设置窗口添加点击事件，用于隐藏键盘
    lv_obj_add_event_cb(time_win, window_click_cb, LV_EVENT_CLICKED, &g_kb_data);
    
    // 为每个文本区域添加焦点事件，传递键盘数据结构
    lv_obj_add_event_cb(year_ta, textarea_focus_cb, LV_EVENT_FOCUSED, &g_kb_data);
    lv_obj_add_event_cb(month_ta, textarea_focus_cb, LV_EVENT_FOCUSED, &g_kb_data);
    lv_obj_add_event_cb(day_ta, textarea_focus_cb, LV_EVENT_FOCUSED, &g_kb_data);
    lv_obj_add_event_cb(hour_ta, textarea_focus_cb, LV_EVENT_FOCUSED, &g_kb_data);
    lv_obj_add_event_cb(min_ta, textarea_focus_cb, LV_EVENT_FOCUSED, &g_kb_data);
    
    // 确认按钮（居中显示）
    lv_obj_t * confirm_btn = lv_button_create(time_win);
    lv_obj_set_size(confirm_btn, 150, 50);  // 增大按钮尺寸
    lv_obj_set_pos(confirm_btn, 325, 390);  // 居中位置 (800-150)/2 = 325
    lv_obj_set_style_bg_opa(confirm_btn, LV_OPA_TRANSP, LV_PART_MAIN);  // 设置透明背景
    lv_obj_set_style_border_width(confirm_btn, 2, LV_PART_MAIN);  // 添加2像素边框
    lv_obj_set_style_border_color(confirm_btn, lv_color_hex(0xFFFFFF), LV_PART_MAIN);  // 白色边框
    lv_obj_set_style_border_opa(confirm_btn, LV_OPA_50, LV_PART_MAIN);  // 50%透明度，不会太显眼
    
    lv_obj_t * confirm_label = lv_label_create(confirm_btn);
    lv_label_set_text(confirm_label, "Confirm");
    lv_obj_set_style_text_color(confirm_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_font(confirm_label, &lv_font_montserrat_18, LV_PART_MAIN);  // 增大字体
    lv_obj_center(confirm_label);
    
    // 绑定按钮事件（将窗口指针作为用户数据传递）
    lv_obj_add_event_cb(confirm_btn, time_settings_confirm_cb, LV_EVENT_CLICKED, time_win);
    
    // 为时间设置窗口添加手势检测
    lv_obj_add_event_cb(time_win, time_settings_gesture_cb, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(time_win, time_settings_gesture_cb, LV_EVENT_RELEASED, NULL);
}

/**
 * @brief 文本输入框焦点事件回调函数
 * 
 * 功能说明：
 * 1. 当文本输入框获得焦点时显示键盘
 * 2. 连接键盘到当前文本输入框
 * 3. 确保键盘显示在最前面
 * 
 * @param e 事件对象，包含事件相关信息
 */
void textarea_focus_cb(lv_event_t * e)
{
    lv_obj_t * ta = lv_event_get_target(e);
    kb_data_t * kb_data = (kb_data_t *)lv_event_get_user_data(e);
    
    // 显示键盘
    lv_obj_remove_flag(kb_data->keyboard, LV_OBJ_FLAG_HIDDEN);
    
    // 连接键盘到文本框
    lv_keyboard_set_textarea(kb_data->keyboard, ta);
    
    // 让键盘移到最前面
    lv_obj_move_foreground(kb_data->keyboard);
}

/**
 * @brief 窗口点击事件回调函数 - 用于隐藏键盘
 * 
 * 功能说明：
 * 1. 检测键盘是否可见
 * 2. 检查点击的是否是键盘或文本框
 * 3. 如果点击的是其他区域，则隐藏键盘并断开连接
 * 
 * @param e 事件对象，包含事件相关信息
 */
void window_click_cb(lv_event_t * e)
{
    lv_obj_t * clicked_obj = lv_event_get_target(e);
    kb_data_t * kb_data = (kb_data_t *)lv_event_get_user_data(e);
    
    // 检查键盘是否当前可见
    if (lv_obj_has_flag(kb_data->keyboard, LV_OBJ_FLAG_HIDDEN)) {
        return; // 键盘已隐藏，无需处理
    }
    
    // 检查点击的是否是键盘本身或文本框
    if (clicked_obj == kb_data->keyboard) {
        return; // 点击的是键盘，不隐藏
    }
    
    // 检查是否点击了文本框
    if (lv_obj_has_class(clicked_obj, &lv_textarea_class)) {
        return; // 点击的是文本框，不隐藏
    }
    
    // 隐藏键盘
    lv_obj_add_flag(kb_data->keyboard, LV_OBJ_FLAG_HIDDEN);
    
    // 断开键盘连接
    lv_keyboard_set_textarea(kb_data->keyboard, NULL);
}

/**
 * @brief 时间设置确认按钮回调函数
 * 
 * 功能说明：
 * 1. 获取用户设置的时间值
 * 2. 验证时间值的有效性
 * 3. 设置RTC硬件时钟
 * 4. 更新显示时间
 * 5. 关闭时间设置窗口
 * 
 * @param e 事件对象，包含事件相关信息
 */
void time_settings_confirm_cb(lv_event_t * e)
{
    lv_obj_t * time_win = (lv_obj_t *)lv_event_get_user_data(e);
    
    // 查找所有文本输入框控件以获取设置的时间值
    lv_obj_t * year_ta = NULL;
    lv_obj_t * month_ta = NULL;
    lv_obj_t * day_ta = NULL;
    lv_obj_t * hour_ta = NULL;
    lv_obj_t * min_ta = NULL;
    
    // 遍历窗口的子控件找到文本输入框
        uint32_t child_count = lv_obj_get_child_count(time_win);
        for(uint32_t i = 0; i < child_count; i++) {
            lv_obj_t * child = lv_obj_get_child(time_win, i);
            if(lv_obj_has_class(child, &lv_textarea_class)) {
                // 根据位置判断是哪个文本输入框
                lv_coord_t x = lv_obj_get_x(child);
                lv_coord_t y = lv_obj_get_y(child);
                
                if(y < 120) {  // 所有输入框都在同一行
                    if(x < 150) year_ta = child;           // Year: x=80
                    else if(x < 300) month_ta = child;     // Month: x=240
                    else if(x < 450) day_ta = child;       // Day: x=370
                    else if(x < 600) hour_ta = child;      // Hour: x=510
                    else min_ta = child;                   // Min: x=650
                }
            }
        }    // 获取设置的时间值
    if(year_ta && month_ta && day_ta && hour_ta && min_ta) {
        const char * year_text = lv_textarea_get_text(year_ta);
        const char * month_text = lv_textarea_get_text(month_ta);
        const char * day_text = lv_textarea_get_text(day_ta);
        const char * hour_text = lv_textarea_get_text(hour_ta);
        const char * min_text = lv_textarea_get_text(min_ta);
        
        int year = atoi(year_text);
        int month = atoi(month_text);
        int day = atoi(day_text);
        int hour = atoi(hour_text);
        int minute = atoi(min_text);
        
        // 验证输入范围
        if(year < 2020 || year > 2030 || 
           month < 1 || month > 12 ||
           day < 1 || day > 31 ||
           hour < 0 || hour > 23 ||
           minute < 0 || minute > 59) {
            
            // 显示错误消息
            lv_obj_t * msg_box = lv_msgbox_create(lv_screen_active());
            lv_msgbox_add_title(msg_box, "Error");
            lv_msgbox_add_text(msg_box, "Invalid time values! Please check input.");
            lv_msgbox_add_close_button(msg_box);
            return;
        }
        
        // 构造 tm 结构体
        struct tm new_time = {0};
        new_time.tm_year = year - 1900;  // tm_year 是从1900年开始计算
        new_time.tm_mon = month - 1;     // tm_mon 范围是0-11
        new_time.tm_mday = day;
        new_time.tm_hour = hour;
        new_time.tm_min = minute;
        new_time.tm_sec = 0;
        new_time.tm_isdst = -1;          // 让系统自动处理夏令时
        
        // 计算星期几
        time_t timestamp = mktime(&new_time);
        if (timestamp != -1) {
            struct tm *temp_tm = localtime(&timestamp);
            new_time.tm_wday = temp_tm->tm_wday;
            new_time.tm_yday = temp_tm->tm_yday;
        }
        
        // 设置RTC硬件时钟
        if (rtc_set_time(&new_time) == 0) {
            // 更新当前显示时间
            current_display_time = new_time;
            
            // 关闭窗口，返回桌面
            lv_obj_del(time_win);
            extern void desktop(void);
            desktop();
        } else {
            // RTC设置失败，但仍然更新显示时间
            current_display_time = new_time;
            
            // 关闭窗口，返回桌面
            lv_obj_del(time_win);
            extern void desktop(void);
            desktop();
        }
    } else {
        // 关闭窗口，返回桌面
        lv_obj_del(time_win);
        extern void desktop(void);
        desktop();
    }
}

/**
 * @brief 时间设置界面手势检测回调函数
 * 
 * 通过监听按下和释放事件手动检测手势
 */
void time_settings_gesture_cb(lv_event_t * e)
{
    static bool touch_started = false;
    static lv_point_t touch_start_point = {0, 0};
    
    lv_event_code_t code = lv_event_get_code(e);
    
    if (code == LV_EVENT_PRESSED) {
        // 记录触摸开始点
        lv_indev_t * indev = lv_indev_get_act();
        if (indev) {
            lv_indev_get_point(indev, &touch_start_point);
            touch_started = true;
        }
    }
    else if (code == LV_EVENT_RELEASED) {
        if (touch_started) {
            // 计算手势
            lv_indev_t * indev = lv_indev_get_act();
            if (indev) {
                lv_point_t touch_end_point;
                lv_indev_get_point(indev, &touch_end_point);
                
                int dx = touch_end_point.x - touch_start_point.x;
                int dy = touch_end_point.y - touch_start_point.y;
                
                // 判断是否为有效的右滑手势
                if (dx > 50 && abs(dy) < 100) {  // 右滑超过50像素，垂直偏移小于100像素
                    // 右滑退出到桌面
                    lv_obj_t * time_win = lv_event_get_target(e);
                    lv_obj_del(time_win);
                    extern void desktop(void);
                    desktop();
                }
            }
            touch_started = false;
        }
    }
}

/**
 * @brief 时间设置取消按钮回调函数
 * 
 * 功能说明：
 * 关闭时间设置窗口，不保存任何更改
 * 
 * @param e 事件对象，包含事件相关信息
 */
void time_settings_cancel_cb(lv_event_t * e)
{
    lv_obj_t * time_win = (lv_obj_t *)lv_event_get_user_data(e);
    
    // 关闭窗口
    lv_obj_del(time_win);
}

/**
 * @brief 从RTC硬件时钟读取时间
 * 
 * 功能说明：
 * 1. 打开RTC设备
 * 2. 读取RTC时间
 * 3. 转换为标准tm结构
 * 4. 关闭设备并返回结果
 * 
 * @param tm_time 用于存储读取时间的结构体指针
 * @return 0表示成功，-1表示失败
 */
int rtc_read_time(struct tm *tm_time)
{
    int rtc_fd;
    struct rtc_time rtc_tm;
    
    // 打开RTC设备
    rtc_fd = open("/dev/rtc0", O_RDONLY);
    if (rtc_fd < 0) {
        return -1;
    }
    
    // 读取RTC时间
    if (ioctl(rtc_fd, RTC_RD_TIME, &rtc_tm) < 0) {
        close(rtc_fd);
        return -1;
    }
    
    // 转换为标准tm结构
    tm_time->tm_sec = rtc_tm.tm_sec;
    tm_time->tm_min = rtc_tm.tm_min;
    tm_time->tm_hour = rtc_tm.tm_hour;
    tm_time->tm_mday = rtc_tm.tm_mday;
    tm_time->tm_mon = rtc_tm.tm_mon;
    tm_time->tm_year = rtc_tm.tm_year;
    tm_time->tm_wday = rtc_tm.tm_wday;
    tm_time->tm_yday = rtc_tm.tm_yday;
    tm_time->tm_isdst = rtc_tm.tm_isdst;
    
    close(rtc_fd);
    return 0;
}

/**
 * @brief 设置RTC硬件时钟时间
 * 
 * 功能说明：
 * 1. 打开RTC设备
 * 2. 转换为RTC时间结构
 * 3. 设置RTC时间
 * 4. 关闭设备并返回结果
 * 
 * @param tm_time 要设置的时间结构体指针
 * @return 0表示成功，-1表示失败
 */
int rtc_set_time(struct tm *tm_time)
{
    int rtc_fd;
    struct rtc_time rtc_tm;
    
    // 打开RTC设备
    rtc_fd = open("/dev/rtc0", O_WRONLY);
    if (rtc_fd < 0) {
        return -1;
    }
    
    // 转换为RTC时间结构
    rtc_tm.tm_sec = tm_time->tm_sec;
    rtc_tm.tm_min = tm_time->tm_min;
    rtc_tm.tm_hour = tm_time->tm_hour;
    rtc_tm.tm_mday = tm_time->tm_mday;
    rtc_tm.tm_mon = tm_time->tm_mon;
    rtc_tm.tm_year = tm_time->tm_year;
    rtc_tm.tm_wday = tm_time->tm_wday;
    rtc_tm.tm_yday = tm_time->tm_yday;
    rtc_tm.tm_isdst = tm_time->tm_isdst;
    
    // 设置RTC时间
    if (ioctl(rtc_fd, RTC_SET_TIME, &rtc_tm) < 0) {
        close(rtc_fd);
        return -1;
    }
    
    close(rtc_fd);
    return 0;
}
