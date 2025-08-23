#include <time.h>
#include <stdio.h>
#include <string.h>

#include "../lvgl/lvgl.h"
#include "../inc/main_screen.h"

// 状态栏相关控件
static lv_obj_t * status_bar_label = NULL;
static lv_timer_t * status_timer = NULL;
static lv_obj_t * desktop_win = NULL;
static lv_timer_t *long_press_timer = NULL;

// 黑屏遮罩对象指针（需全局可见）
lv_obj_t *black_screen_mask = NULL;

/**
 * @brief 桌面主界面
 * 
 * 功能说明：
 * 1. 创建桌面窗口
 * 2. 初始化状态栏和应用按钮
 * 3. 添加锁屏按钮
 */
void desktop(void)
{
    // 进入桌面前，确保黑屏遮罩被清理
    if (black_screen_mask) {
        lv_obj_del(black_screen_mask);
        black_screen_mask = NULL;
    }
    // 创建桌面窗口
    desktop_win = lv_obj_create(lv_screen_active());
    lv_obj_set_size(desktop_win, 800, 480);
    lv_obj_set_pos(desktop_win, 0, 0);
    lv_obj_set_style_pad_all(desktop_win, 0, LV_PART_MAIN);
    lv_obj_set_style_border_width(desktop_win, 0, LV_PART_MAIN);
    
    // 设置背景图片
    lv_obj_t * bg_img = lv_image_create(desktop_win);
    lv_obj_set_size(bg_img, 800, 480);
    lv_obj_set_pos(bg_img, 0, 0);
    lv_image_set_src(bg_img, "/home/tyc/work_station/desktop/background.jpg");
    
    // 为桌面添加长按检测（长按3秒息屏）
    lv_obj_add_event_cb(desktop_win, desktop_long_press_cb, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(desktop_win, desktop_long_press_cb, LV_EVENT_RELEASED, NULL);
    lv_obj_add_event_cb(bg_img, desktop_long_press_cb, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(bg_img, desktop_long_press_cb, LV_EVENT_RELEASED, NULL);
    
    // ========== 状态栏 ========== 
    lv_obj_t * status_bar = lv_obj_create(desktop_win);
    lv_obj_set_size(status_bar, 800, 50);
    lv_obj_set_pos(status_bar, 0, 0);
    lv_obj_set_style_bg_color(status_bar, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(status_bar, LV_OPA_70, LV_PART_MAIN);  // 半透明背景
    lv_obj_set_style_border_width(status_bar, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(status_bar, 5, LV_PART_MAIN);
    // 允许手势冒泡，防止遮挡长按
    lv_obj_add_flag(status_bar, LV_OBJ_FLAG_GESTURE_BUBBLE);

    // 创建状态栏标签
    status_bar_label = lv_label_create(status_bar);
    lv_obj_set_style_text_color(status_bar_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_font(status_bar_label, &lv_font_montserrat_16, LV_PART_MAIN);
    lv_obj_align(status_bar_label, LV_ALIGN_CENTER, 0, 0);

    // 初始化时间显示
    init_time_display();

    // 设置状态栏标签和定时器
    extern void set_status_bar_label(lv_obj_t* label);
    set_status_bar_label(status_bar_label);
    status_bar_timer_cb(NULL);
    status_timer = lv_timer_create(status_bar_timer_cb, 1000, NULL);
    
    // ========== 应用区容器 ==========
    lv_obj_t * app_container = lv_obj_create(desktop_win);
    lv_obj_set_size(app_container, 750, 400);
    lv_obj_set_pos(app_container, 25, 70);  // 左上角位置，状态栏下方
    lv_obj_set_style_bg_opa(app_container, LV_OPA_TRANSP, LV_PART_MAIN);  // 透明背景
    lv_obj_set_style_border_width(app_container, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(app_container, 10, LV_PART_MAIN);
    // 允许手势冒泡，防止遮挡长按
    lv_obj_add_flag(app_container, LV_OBJ_FLAG_GESTURE_BUBBLE);

    lv_obj_add_event_cb(app_container, desktop_long_press_cb, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(app_container, desktop_long_press_cb, LV_EVENT_RELEASED, NULL);
    
    
    // ========== 音乐按钮 ==========
    lv_obj_t * music_btn = lv_button_create(app_container);
    lv_obj_set_size(music_btn, 60, 60);
    lv_obj_set_pos(music_btn, 20, 20);  // 左上角第一个位置
    lv_obj_set_style_bg_color(music_btn, lv_color_hex(0xFF6B6B), LV_PART_MAIN);
    lv_obj_set_style_radius(music_btn, 8, LV_PART_MAIN);  // 添加圆角弧度
    
    // 创建音乐图标
    lv_obj_t * music_icon_img = lv_image_create(music_btn);
    lv_obj_set_size(music_icon_img, 52, 52);  // 设置图片尺寸为8的倍数(56=7*8)
    lv_image_set_src(music_icon_img, "/home/tyc/work_station/desktop/music.jpg");
    lv_obj_align(music_icon_img, LV_ALIGN_CENTER, 0, 0);  // 居中显示
    lv_obj_set_style_radius(music_icon_img, 8, LV_PART_MAIN);  // 添加圆角弧度
    
    // 创建音乐标签
    lv_obj_t * music_label = lv_label_create(app_container);
    lv_label_set_text(music_label, "KuGou");
    lv_obj_set_style_text_font(music_label, &lv_font_montserrat_14, LV_PART_MAIN);  // 调大字体
    lv_obj_set_style_text_color(music_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_pos(music_label, 25, 85);  // 位置在按钮下方，左移调整对齐
    
    // 添加音乐按钮事件
    lv_obj_add_event_cb(music_btn, music_btn_event_cb, LV_EVENT_CLICKED, NULL);
    
    // ========== 相册按钮 ==========
    lv_obj_t * gallery_btn = lv_button_create(app_container);
    lv_obj_set_size(gallery_btn, 60, 60);
    lv_obj_set_pos(gallery_btn, 100, 20);  // 第二个位置，酷狗音乐右边
    lv_obj_set_style_bg_color(gallery_btn, lv_color_hex(0x4ECDC4), LV_PART_MAIN);
    lv_obj_set_style_radius(gallery_btn, 8, LV_PART_MAIN);  // 添加圆角弧度
    
    // 创建相册图标
    lv_obj_t * gallery_icon_img = lv_image_create(gallery_btn);
    lv_obj_set_size(gallery_icon_img, 52, 52);  // 设置图片尺寸为8的倍数(56=7*8)
    lv_image_set_src(gallery_icon_img, "/home/tyc/work_station/desktop/dcim.jpg");
    lv_obj_align(gallery_icon_img, LV_ALIGN_CENTER, 0, 0);  // 居中显示
    lv_obj_set_style_radius(gallery_icon_img, 8, LV_PART_MAIN);  // 添加圆角弧度
    
    // 创建相册标签
    lv_obj_t * gallery_label = lv_label_create(app_container);
    lv_label_set_text(gallery_label, "DCIM");
    lv_obj_set_style_text_font(gallery_label, &lv_font_montserrat_14, LV_PART_MAIN);  // 调大字体
    lv_obj_set_style_text_color(gallery_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_pos(gallery_label, 110, 85);  // 位置在按钮下方，左移调整对齐
    
    // 添加相册按钮事件
    lv_obj_add_event_cb(gallery_btn, gallery_btn_event_cb, LV_EVENT_CLICKED, NULL);
    
    // ========== 日历按钮 ========== 
    lv_obj_t * calendar_btn = lv_button_create(app_container);
    lv_obj_set_size(calendar_btn, 60, 60);
    lv_obj_set_pos(calendar_btn, 180, 20);  // 第三个位置，相册右边
    lv_obj_set_style_bg_color(calendar_btn, lv_color_hex(0x9b59b6), LV_PART_MAIN);
    lv_obj_set_style_radius(calendar_btn, 8, LV_PART_MAIN);  // 添加圆角弧度
    
    // 创建日历图标
    lv_obj_t * calendar_icon_img = lv_image_create(calendar_btn);
    lv_obj_set_size(calendar_icon_img, 52, 52);  // 设置图片尺寸为8的倍数(56=7*8)
    lv_image_set_src(calendar_icon_img, "/home/tyc/work_station/desktop/data.jpg");
    lv_obj_align(calendar_icon_img, LV_ALIGN_CENTER, 0, 0);  // 居中显示
    lv_obj_set_style_radius(calendar_icon_img, 8, LV_PART_MAIN);  // 添加圆角弧度
    
    // 创建日历标签
    lv_obj_t * calendar_label = lv_label_create(app_container);
    lv_label_set_text(calendar_label, "Settings");
    lv_obj_set_style_text_font(calendar_label, &lv_font_montserrat_14, LV_PART_MAIN);  // 调大字体
    lv_obj_set_style_text_color(calendar_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_pos(calendar_label, 175, 85);  // 位置在按钮下方，左移调整对齐
    
    // 添加日历按钮事件
    lv_obj_add_event_cb(calendar_btn, calendar_btn_event_cb, LV_EVENT_CLICKED, NULL);
}

/**
 * @brief 释放桌面资源
 * 
 * 功能说明：
 * 1. 删除定时器
 * 2. 删除桌面窗口
 * 3. 清空指针
 */
void release_desktop_resources(void)
{
    // 删除定时器
    if (status_timer != NULL) {
        lv_timer_del(status_timer);
        status_timer = NULL;
    }
    
    // 删除桌面窗口
    if (desktop_win != NULL) {
        lv_obj_del(desktop_win);
        desktop_win = NULL;
    }
    
    // 清空指针
    status_bar_label = NULL;
    // 进入息屏时一并置空黑屏遮罩指针，防止后续误用
    black_screen_mask = NULL;
}

void start_time_settings_app(void);

/**
 * @brief 音乐按钮事件回调
 * 
 * 功能说明：
 * 1. 释放桌面资源
 * 2. 启动音乐应用
 * 
 * @param e 事件指针
 */
void music_btn_event_cb(lv_event_t * e)
{
    // 释放桌面资源
    release_desktop_resources();
    
    // 启动音乐应用
    start_music_app();
}

/**
 * @brief 相册按钮事件回调
 * 
 * 功能说明：
 * 1. 释放桌面资源
 * 2. 启动相册应用
 * 
 * @param e 事件指针
 */
void gallery_btn_event_cb(lv_event_t * e)
{
    // 释放桌面资源
    release_desktop_resources();
    
    // 启动相册应用
    start_gallery_app();
}

/**
 * @brief 日历按钮事件回调
 * 
 * 功能说明：
 * 1. 释放桌面资源
 * 2. 启动时间设置应用
 * 
 * @param e 事件指针
 */
void calendar_btn_event_cb(lv_event_t * e)
{
    // 释放桌面资源
    release_desktop_resources();
    
    // 启动时间设置应用
    start_time_settings_app();
}

// 启动音乐应用
extern void start_music_app(void);

/**
 * @brief 启动相册应用
 * 
 * 功能说明：
 * 初始化相册并显示缩略图界面
 */
void start_gallery_app(void)
{
    // 先初始化相册
    init_gallery();
    
    // 检查是否成功找到图片文件
    extern gallery_manager_t g_gallery;
    if (g_gallery.image_count == 0) {
        lv_obj_t * msg_box = lv_msgbox_create(lv_screen_active());
        lv_msgbox_add_title(msg_box, "Gallery");
        lv_msgbox_add_text(msg_box, "No image files found in /home/tyc/work_station/dcim/");
        lv_msgbox_add_close_button(msg_box);
        return;
    }
    
    // 显示相册缩略图界面
    show_gallery_thumbnail_screen();
}

/**
 * @brief 桌面长按检测回调函数
 * 
 * 长按3秒后返回锁屏界面实现息屏效果
 */
// 桌面长按定时器回调
// 黑屏遮罩对象指针

// 黑屏遮罩点击事件回调
static void black_screen_mask_event_cb(lv_event_t *e)
{
    if (black_screen_mask) {
        lv_obj_del(black_screen_mask);
        black_screen_mask = NULL;
    }
    extern void Screen(void);
    Screen();
}

static void desktop_long_press_timer_cb(lv_timer_t * timer) {
    // 先创建黑屏遮罩，后释放桌面资源
    lv_obj_t *scr = lv_screen_active();
    black_screen_mask = lv_obj_create(scr);
    lv_obj_set_size(black_screen_mask, 800, 480);
    lv_obj_set_pos(black_screen_mask, 0, 0);
    lv_obj_set_style_bg_color(black_screen_mask, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(black_screen_mask, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_width(black_screen_mask, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(black_screen_mask, 0, LV_PART_MAIN);
    lv_obj_add_flag(black_screen_mask, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(black_screen_mask, black_screen_mask_event_cb, LV_EVENT_CLICKED, NULL);
    release_desktop_resources();
    lv_timer_del(timer);
    long_press_timer = NULL;
}

void desktop_long_press_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_PRESSED) {
        // 启动3秒定时器
        if (long_press_timer == NULL) {
            long_press_timer = lv_timer_create(desktop_long_press_timer_cb, 3000, NULL);
        }
    } else if (code == LV_EVENT_RELEASED) {
        // 松开时如果没到3秒就取消定时器
        if (long_press_timer) {
            lv_timer_del(long_press_timer);
            long_press_timer = NULL;
        }
    }
}

