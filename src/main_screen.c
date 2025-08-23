#include <time.h>
#include <stdio.h>
#include <string.h>

#include "../lvgl/lvgl.h"
#include "../inc/main_screen.h"

// ״̬����ؿؼ�
static lv_obj_t * status_bar_label = NULL;
static lv_timer_t * status_timer = NULL;
static lv_obj_t * desktop_win = NULL;
static lv_timer_t *long_press_timer = NULL;

// �������ֶ���ָ�루��ȫ�ֿɼ���
lv_obj_t *black_screen_mask = NULL;

/**
 * @brief ����������
 * 
 * ����˵����
 * 1. �������洰��
 * 2. ��ʼ��״̬����Ӧ�ð�ť
 * 3. ���������ť
 */
void desktop(void)
{
    // ��������ǰ��ȷ���������ֱ�����
    if (black_screen_mask) {
        lv_obj_del(black_screen_mask);
        black_screen_mask = NULL;
    }
    // �������洰��
    desktop_win = lv_obj_create(lv_screen_active());
    lv_obj_set_size(desktop_win, 800, 480);
    lv_obj_set_pos(desktop_win, 0, 0);
    lv_obj_set_style_pad_all(desktop_win, 0, LV_PART_MAIN);
    lv_obj_set_style_border_width(desktop_win, 0, LV_PART_MAIN);
    
    // ���ñ���ͼƬ
    lv_obj_t * bg_img = lv_image_create(desktop_win);
    lv_obj_set_size(bg_img, 800, 480);
    lv_obj_set_pos(bg_img, 0, 0);
    lv_image_set_src(bg_img, "/home/tyc/work_station/desktop/background.jpg");
    
    // Ϊ������ӳ�����⣨����3��Ϣ����
    lv_obj_add_event_cb(desktop_win, desktop_long_press_cb, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(desktop_win, desktop_long_press_cb, LV_EVENT_RELEASED, NULL);
    lv_obj_add_event_cb(bg_img, desktop_long_press_cb, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(bg_img, desktop_long_press_cb, LV_EVENT_RELEASED, NULL);
    
    // ========== ״̬�� ========== 
    lv_obj_t * status_bar = lv_obj_create(desktop_win);
    lv_obj_set_size(status_bar, 800, 50);
    lv_obj_set_pos(status_bar, 0, 0);
    lv_obj_set_style_bg_color(status_bar, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(status_bar, LV_OPA_70, LV_PART_MAIN);  // ��͸������
    lv_obj_set_style_border_width(status_bar, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(status_bar, 5, LV_PART_MAIN);
    // ��������ð�ݣ���ֹ�ڵ�����
    lv_obj_add_flag(status_bar, LV_OBJ_FLAG_GESTURE_BUBBLE);

    // ����״̬����ǩ
    status_bar_label = lv_label_create(status_bar);
    lv_obj_set_style_text_color(status_bar_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_font(status_bar_label, &lv_font_montserrat_16, LV_PART_MAIN);
    lv_obj_align(status_bar_label, LV_ALIGN_CENTER, 0, 0);

    // ��ʼ��ʱ����ʾ
    init_time_display();

    // ����״̬����ǩ�Ͷ�ʱ��
    extern void set_status_bar_label(lv_obj_t* label);
    set_status_bar_label(status_bar_label);
    status_bar_timer_cb(NULL);
    status_timer = lv_timer_create(status_bar_timer_cb, 1000, NULL);
    
    // ========== Ӧ�������� ==========
    lv_obj_t * app_container = lv_obj_create(desktop_win);
    lv_obj_set_size(app_container, 750, 400);
    lv_obj_set_pos(app_container, 25, 70);  // ���Ͻ�λ�ã�״̬���·�
    lv_obj_set_style_bg_opa(app_container, LV_OPA_TRANSP, LV_PART_MAIN);  // ͸������
    lv_obj_set_style_border_width(app_container, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(app_container, 10, LV_PART_MAIN);
    // ��������ð�ݣ���ֹ�ڵ�����
    lv_obj_add_flag(app_container, LV_OBJ_FLAG_GESTURE_BUBBLE);

    lv_obj_add_event_cb(app_container, desktop_long_press_cb, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(app_container, desktop_long_press_cb, LV_EVENT_RELEASED, NULL);
    
    
    // ========== ���ְ�ť ==========
    lv_obj_t * music_btn = lv_button_create(app_container);
    lv_obj_set_size(music_btn, 60, 60);
    lv_obj_set_pos(music_btn, 20, 20);  // ���Ͻǵ�һ��λ��
    lv_obj_set_style_bg_color(music_btn, lv_color_hex(0xFF6B6B), LV_PART_MAIN);
    lv_obj_set_style_radius(music_btn, 8, LV_PART_MAIN);  // ���Բ�ǻ���
    
    // ��������ͼ��
    lv_obj_t * music_icon_img = lv_image_create(music_btn);
    lv_obj_set_size(music_icon_img, 52, 52);  // ����ͼƬ�ߴ�Ϊ8�ı���(56=7*8)
    lv_image_set_src(music_icon_img, "/home/tyc/work_station/desktop/music.jpg");
    lv_obj_align(music_icon_img, LV_ALIGN_CENTER, 0, 0);  // ������ʾ
    lv_obj_set_style_radius(music_icon_img, 8, LV_PART_MAIN);  // ���Բ�ǻ���
    
    // �������ֱ�ǩ
    lv_obj_t * music_label = lv_label_create(app_container);
    lv_label_set_text(music_label, "KuGou");
    lv_obj_set_style_text_font(music_label, &lv_font_montserrat_14, LV_PART_MAIN);  // ��������
    lv_obj_set_style_text_color(music_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_pos(music_label, 25, 85);  // λ���ڰ�ť�·������Ƶ�������
    
    // ������ְ�ť�¼�
    lv_obj_add_event_cb(music_btn, music_btn_event_cb, LV_EVENT_CLICKED, NULL);
    
    // ========== ��ᰴť ==========
    lv_obj_t * gallery_btn = lv_button_create(app_container);
    lv_obj_set_size(gallery_btn, 60, 60);
    lv_obj_set_pos(gallery_btn, 100, 20);  // �ڶ���λ�ã��ṷ�����ұ�
    lv_obj_set_style_bg_color(gallery_btn, lv_color_hex(0x4ECDC4), LV_PART_MAIN);
    lv_obj_set_style_radius(gallery_btn, 8, LV_PART_MAIN);  // ���Բ�ǻ���
    
    // �������ͼ��
    lv_obj_t * gallery_icon_img = lv_image_create(gallery_btn);
    lv_obj_set_size(gallery_icon_img, 52, 52);  // ����ͼƬ�ߴ�Ϊ8�ı���(56=7*8)
    lv_image_set_src(gallery_icon_img, "/home/tyc/work_station/desktop/dcim.jpg");
    lv_obj_align(gallery_icon_img, LV_ALIGN_CENTER, 0, 0);  // ������ʾ
    lv_obj_set_style_radius(gallery_icon_img, 8, LV_PART_MAIN);  // ���Բ�ǻ���
    
    // ��������ǩ
    lv_obj_t * gallery_label = lv_label_create(app_container);
    lv_label_set_text(gallery_label, "DCIM");
    lv_obj_set_style_text_font(gallery_label, &lv_font_montserrat_14, LV_PART_MAIN);  // ��������
    lv_obj_set_style_text_color(gallery_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_pos(gallery_label, 110, 85);  // λ���ڰ�ť�·������Ƶ�������
    
    // �����ᰴť�¼�
    lv_obj_add_event_cb(gallery_btn, gallery_btn_event_cb, LV_EVENT_CLICKED, NULL);
    
    // ========== ������ť ========== 
    lv_obj_t * calendar_btn = lv_button_create(app_container);
    lv_obj_set_size(calendar_btn, 60, 60);
    lv_obj_set_pos(calendar_btn, 180, 20);  // ������λ�ã�����ұ�
    lv_obj_set_style_bg_color(calendar_btn, lv_color_hex(0x9b59b6), LV_PART_MAIN);
    lv_obj_set_style_radius(calendar_btn, 8, LV_PART_MAIN);  // ���Բ�ǻ���
    
    // ��������ͼ��
    lv_obj_t * calendar_icon_img = lv_image_create(calendar_btn);
    lv_obj_set_size(calendar_icon_img, 52, 52);  // ����ͼƬ�ߴ�Ϊ8�ı���(56=7*8)
    lv_image_set_src(calendar_icon_img, "/home/tyc/work_station/desktop/data.jpg");
    lv_obj_align(calendar_icon_img, LV_ALIGN_CENTER, 0, 0);  // ������ʾ
    lv_obj_set_style_radius(calendar_icon_img, 8, LV_PART_MAIN);  // ���Բ�ǻ���
    
    // ����������ǩ
    lv_obj_t * calendar_label = lv_label_create(app_container);
    lv_label_set_text(calendar_label, "Settings");
    lv_obj_set_style_text_font(calendar_label, &lv_font_montserrat_14, LV_PART_MAIN);  // ��������
    lv_obj_set_style_text_color(calendar_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_pos(calendar_label, 175, 85);  // λ���ڰ�ť�·������Ƶ�������
    
    // ���������ť�¼�
    lv_obj_add_event_cb(calendar_btn, calendar_btn_event_cb, LV_EVENT_CLICKED, NULL);
}

/**
 * @brief �ͷ�������Դ
 * 
 * ����˵����
 * 1. ɾ����ʱ��
 * 2. ɾ�����洰��
 * 3. ���ָ��
 */
void release_desktop_resources(void)
{
    // ɾ����ʱ��
    if (status_timer != NULL) {
        lv_timer_del(status_timer);
        status_timer = NULL;
    }
    
    // ɾ�����洰��
    if (desktop_win != NULL) {
        lv_obj_del(desktop_win);
        desktop_win = NULL;
    }
    
    // ���ָ��
    status_bar_label = NULL;
    // ����Ϣ��ʱһ���ÿպ�������ָ�룬��ֹ��������
    black_screen_mask = NULL;
}

void start_time_settings_app(void);

/**
 * @brief ���ְ�ť�¼��ص�
 * 
 * ����˵����
 * 1. �ͷ�������Դ
 * 2. ��������Ӧ��
 * 
 * @param e �¼�ָ��
 */
void music_btn_event_cb(lv_event_t * e)
{
    // �ͷ�������Դ
    release_desktop_resources();
    
    // ��������Ӧ��
    start_music_app();
}

/**
 * @brief ��ᰴť�¼��ص�
 * 
 * ����˵����
 * 1. �ͷ�������Դ
 * 2. �������Ӧ��
 * 
 * @param e �¼�ָ��
 */
void gallery_btn_event_cb(lv_event_t * e)
{
    // �ͷ�������Դ
    release_desktop_resources();
    
    // �������Ӧ��
    start_gallery_app();
}

/**
 * @brief ������ť�¼��ص�
 * 
 * ����˵����
 * 1. �ͷ�������Դ
 * 2. ����ʱ������Ӧ��
 * 
 * @param e �¼�ָ��
 */
void calendar_btn_event_cb(lv_event_t * e)
{
    // �ͷ�������Դ
    release_desktop_resources();
    
    // ����ʱ������Ӧ��
    start_time_settings_app();
}

// ��������Ӧ��
extern void start_music_app(void);

/**
 * @brief �������Ӧ��
 * 
 * ����˵����
 * ��ʼ����Ტ��ʾ����ͼ����
 */
void start_gallery_app(void)
{
    // �ȳ�ʼ�����
    init_gallery();
    
    // ����Ƿ�ɹ��ҵ�ͼƬ�ļ�
    extern gallery_manager_t g_gallery;
    if (g_gallery.image_count == 0) {
        lv_obj_t * msg_box = lv_msgbox_create(lv_screen_active());
        lv_msgbox_add_title(msg_box, "Gallery");
        lv_msgbox_add_text(msg_box, "No image files found in /home/tyc/work_station/dcim/");
        lv_msgbox_add_close_button(msg_box);
        return;
    }
    
    // ��ʾ�������ͼ����
    show_gallery_thumbnail_screen();
}

/**
 * @brief ���泤�����ص�����
 * 
 * ����3��󷵻���������ʵ��Ϣ��Ч��
 */
// ���泤����ʱ���ص�
// �������ֶ���ָ��

// �������ֵ���¼��ص�
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
    // �ȴ����������֣����ͷ�������Դ
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
        // ����3�붨ʱ��
        if (long_press_timer == NULL) {
            long_press_timer = lv_timer_create(desktop_long_press_timer_cb, 3000, NULL);
        }
    } else if (code == LV_EVENT_RELEASED) {
        // �ɿ�ʱ���û��3���ȡ����ʱ��
        if (long_press_timer) {
            lv_timer_del(long_press_timer);
            long_press_timer = NULL;
        }
    }
}

