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

// ȫ�ֱ���
static lv_obj_t * status_bar_label = NULL;
static lv_timer_t * status_timer = NULL;

// ��ǰ��ʾ��ʱ�䣨����ÿ����£�
static struct tm current_display_time = {
    .tm_year = 125,  // 2025�� (2025-1900)
    .tm_mon = 6,     // 7�� (0-11)
    .tm_mday = 18,   // 18��
    .tm_hour = 12,   // 12ʱ
    .tm_min = 30,    // 30��
    .tm_sec = 45     // 45��
};

// �������ݽṹ
typedef struct {
    lv_obj_t * keyboard;
} kb_data_t;

static kb_data_t g_kb_data;

/**
 * @brief ��ʼ��ʱ����ʾ
 */
void init_time_display(void)
{
    // ������RTC��ȡ��ǰʱ�䲢��ʾ�����ʧ����ʹ��Ĭ��ʱ��
    if (rtc_read_time(&current_display_time) != 0) {
        // ʹ��Ĭ��ʱ�䣨����ԭ�е�Ĭ�����ã�
        current_display_time.tm_year = 125;  // 2025�� (2025-1900)
        current_display_time.tm_mon = 6;     // 7�� (0-11)
        current_display_time.tm_mday = 18;   // 18��
        current_display_time.tm_hour = 12;   // 12ʱ
        current_display_time.tm_min = 30;    // 30��
        current_display_time.tm_sec = 45;    // 45��
    }
}

/**
 * @brief ��ȡ��ǰ��ʾʱ��
 */
struct tm* get_current_display_time(void)
{
    return &current_display_time;
}

/**
 * @brief ״̬����ʱ���ص�����
 * @param timer ��ʱ������
 * 
 * ����˵����
 * ��RTCӲ��ʱ�Ӷ�ȡ��ʵʱ�䲢������ʾ
 */
void status_bar_timer_cb(lv_timer_t * timer)
{
    if (status_bar_label == NULL) return;
    
    char time_str[100];
    char weekday_str[7][10] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
    char month_str[12][4] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", 
                             "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
    
    struct tm rtc_time;
    
    // ���Դ�RTC��ȡʱ��
    if (rtc_read_time(&rtc_time) == 0) {
        // �ɹ���ȡRTCʱ�䣬����ȫ��ʱ�����
        current_display_time = rtc_time;
    } else {
        // RTC��ȡʧ�ܣ�ʹ�����ģ��ʱ�䣨��ǰ�ƽ�һ�룩
        
        current_display_time.tm_sec++;
        
        // �����������
        if (current_display_time.tm_sec >= 60) {
            current_display_time.tm_sec = 0;
            current_display_time.tm_min++;
            
            // ����������
            if (current_display_time.tm_min >= 60) {
                current_display_time.tm_min = 0;
                current_display_time.tm_hour++;
                
                // ����Сʱ���
                if (current_display_time.tm_hour >= 24) {
                    current_display_time.tm_hour = 0;
                    current_display_time.tm_mday++;
                    
                    // �򵥴����������������򻯴����������·��������죩
                    if (current_display_time.tm_mday > 31) {
                        current_display_time.tm_mday = 1;
                        current_display_time.tm_mon++;
                        
                        // �����·����
                        if (current_display_time.tm_mon >= 12) {
                            current_display_time.tm_mon = 0;
                            current_display_time.tm_year++;
                        }
                    }
                }
            }
        }
    }
    
    // ��ʽ��ʱ���ַ���
    snprintf(time_str, sizeof(time_str), "%s %s %02d, %04d  %02d:%02d:%02d",
             weekday_str[current_display_time.tm_wday % 7],  // ʹ����ȷ�����ڼ���
             month_str[current_display_time.tm_mon % 12],
             current_display_time.tm_mday,
             current_display_time.tm_year + 1900,
             current_display_time.tm_hour,
             current_display_time.tm_min,
             current_display_time.tm_sec);
    
    lv_obj_set_style_text_color(status_bar_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN); // ��ɫ
    lv_label_set_text(status_bar_label, time_str);
}

/**
 * @brief ����״̬����ǩ������main_screen.c���ã�
 */
void set_status_bar_label(lv_obj_t* label)
{
    status_bar_label = label;
}

/**
 * @brief ����ʱ������Ӧ��
 * 
 * ����˵����
 * ����ʱ�����ý��棬�����û�����ϵͳʱ��
 */
void start_time_settings_app(void)
{
    // ����ʱ�����ô���
    lv_obj_t * time_win = lv_obj_create(lv_screen_active());
    lv_obj_set_size(time_win, 800, 480);  // ʹ��ȫ���ߴ�
    lv_obj_set_pos(time_win, 0, 0);       // λ����Ϊ��Ļ���Ͻ�
    lv_obj_set_style_bg_color(time_win, lv_color_hex(0x2c3e50), LV_PART_MAIN);
    lv_obj_set_style_radius(time_win, 0, LV_PART_MAIN);  // ȥ��Բ������Ӧȫ��
    lv_obj_set_style_pad_all(time_win, 0, LV_PART_MAIN);
    lv_obj_set_style_border_width(time_win, 0, LV_PART_MAIN);
    
    // ���ñ���ͼƬ
    lv_obj_t * bg_img = lv_image_create(time_win);
    lv_obj_set_size(bg_img, 800, 480);
    lv_obj_set_pos(bg_img, 0, 0);
    lv_image_set_src(bg_img, "/home/tyc/work_station/data/data_back.jpg");
    
    // ����
    lv_obj_t * title = lv_label_create(time_win);
    lv_label_set_text(title, "Time Settings");
    lv_obj_set_style_text_font(title, &lv_font_montserrat_20, LV_PART_MAIN);
    lv_obj_set_style_text_color(title, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 20);
    
    // �������
    lv_obj_t * year_label = lv_label_create(time_win);
    lv_label_set_text(year_label, "Year:");
    lv_obj_set_style_text_color(year_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_font(year_label, &lv_font_montserrat_16, LV_PART_MAIN);
    lv_obj_set_pos(year_label, 115, 50);
    
    lv_obj_t * year_ta = lv_textarea_create(time_win);
    lv_obj_set_size(year_ta, 120, 40);  // ��΢���������
    lv_obj_set_pos(year_ta, 95, 75);
    lv_obj_set_style_bg_opa(year_ta, LV_OPA_TRANSP, LV_PART_MAIN);  // ����͸������
    lv_textarea_set_one_line(year_ta, true);
    lv_textarea_set_accepted_chars(year_ta, "0123456789");
    lv_textarea_set_max_length(year_ta, 4);
    lv_textarea_set_text(year_ta, "2025");  // ����Ĭ�����
    
    // �·�����
    lv_obj_t * month_label = lv_label_create(time_win);
    lv_label_set_text(month_label, "Month:");
    lv_obj_set_style_text_color(month_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_font(month_label, &lv_font_montserrat_16, LV_PART_MAIN);
    lv_obj_set_pos(month_label, 265, 50);
    
    lv_obj_t * month_ta = lv_textarea_create(time_win);
    lv_obj_set_size(month_ta, 100, 40);
    lv_obj_set_pos(month_ta, 255, 75);
    lv_obj_set_style_bg_opa(month_ta, LV_OPA_TRANSP, LV_PART_MAIN);  // ����͸������
    lv_textarea_set_one_line(month_ta, true);
    lv_textarea_set_accepted_chars(month_ta, "0123456789");
    lv_textarea_set_max_length(month_ta, 2);
    lv_textarea_set_text(month_ta, "07");  // ����Ĭ���·�Ϊ7��
    
    // ��������
    lv_obj_t * day_label = lv_label_create(time_win);
    lv_label_set_text(day_label, "Day:");
    lv_obj_set_style_text_color(day_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_font(day_label, &lv_font_montserrat_16, LV_PART_MAIN);
    lv_obj_set_pos(day_label, 405, 50);
    
    lv_obj_t * day_ta = lv_textarea_create(time_win);
    lv_obj_set_size(day_ta, 100, 40);
    lv_obj_set_pos(day_ta, 385, 75);
    lv_obj_set_style_bg_opa(day_ta, LV_OPA_TRANSP, LV_PART_MAIN);  // ����͸������
    lv_textarea_set_one_line(day_ta, true);
    lv_textarea_set_accepted_chars(day_ta, "0123456789");
    lv_textarea_set_max_length(day_ta, 2);
    lv_textarea_set_text(day_ta, "18");  // ����Ĭ������Ϊ18��
    
    // Сʱ����
    lv_obj_t * hour_label = lv_label_create(time_win);
    lv_label_set_text(hour_label, "Hour:");
    lv_obj_set_style_text_color(hour_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_font(hour_label, &lv_font_montserrat_16, LV_PART_MAIN);
    lv_obj_set_pos(hour_label, 545, 50);
    
    lv_obj_t * hour_ta = lv_textarea_create(time_win);
    lv_obj_set_size(hour_ta, 100, 40);
    lv_obj_set_pos(hour_ta, 525, 75);
    lv_obj_set_style_bg_opa(hour_ta, LV_OPA_TRANSP, LV_PART_MAIN);  // ����͸������
    lv_textarea_set_one_line(hour_ta, true);
    lv_textarea_set_accepted_chars(hour_ta, "0123456789");
    lv_textarea_set_max_length(hour_ta, 2);
    lv_textarea_set_text(hour_ta, "12");  // ����Ĭ��СʱΪ12ʱ
    
    // ��������
    lv_obj_t * min_label = lv_label_create(time_win);
    lv_label_set_text(min_label, "Min:");
    lv_obj_set_style_text_color(min_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_font(min_label, &lv_font_montserrat_16, LV_PART_MAIN);
    lv_obj_set_pos(min_label, 685, 50);
    
    lv_obj_t * min_ta = lv_textarea_create(time_win);
    lv_obj_set_size(min_ta, 100, 40);
    lv_obj_set_pos(min_ta, 665, 75);
    lv_obj_set_style_bg_opa(min_ta, LV_OPA_TRANSP, LV_PART_MAIN);  // ����͸������
    lv_textarea_set_one_line(min_ta, true);
    lv_textarea_set_accepted_chars(min_ta, "0123456789");
    lv_textarea_set_max_length(min_ta, 2);
    lv_textarea_set_text(min_ta, "30");  // ����Ĭ�Ϸ���Ϊ30��
    
    // ����LVGL���ü���
    lv_obj_t * kb = lv_keyboard_create(time_win);
    lv_obj_set_size(kb, 750, 200);  // ������̳ߴ�
    lv_obj_set_pos(kb, 25, -35);    // ����λ�õ���������λ��
    lv_keyboard_set_mode(kb, LV_KEYBOARD_MODE_NUMBER);
    lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);  // Ĭ�����ؼ���
    
    // ������ָ�뱣�浽ȫ�����ݽṹ��
    g_kb_data.keyboard = kb;
    
    // Ϊʱ�����ô�����ӵ���¼����������ؼ���
    lv_obj_add_event_cb(time_win, window_click_cb, LV_EVENT_CLICKED, &g_kb_data);
    
    // Ϊÿ���ı�������ӽ����¼������ݼ������ݽṹ
    lv_obj_add_event_cb(year_ta, textarea_focus_cb, LV_EVENT_FOCUSED, &g_kb_data);
    lv_obj_add_event_cb(month_ta, textarea_focus_cb, LV_EVENT_FOCUSED, &g_kb_data);
    lv_obj_add_event_cb(day_ta, textarea_focus_cb, LV_EVENT_FOCUSED, &g_kb_data);
    lv_obj_add_event_cb(hour_ta, textarea_focus_cb, LV_EVENT_FOCUSED, &g_kb_data);
    lv_obj_add_event_cb(min_ta, textarea_focus_cb, LV_EVENT_FOCUSED, &g_kb_data);
    
    // ȷ�ϰ�ť��������ʾ��
    lv_obj_t * confirm_btn = lv_button_create(time_win);
    lv_obj_set_size(confirm_btn, 150, 50);  // ����ť�ߴ�
    lv_obj_set_pos(confirm_btn, 325, 390);  // ����λ�� (800-150)/2 = 325
    lv_obj_set_style_bg_opa(confirm_btn, LV_OPA_TRANSP, LV_PART_MAIN);  // ����͸������
    lv_obj_set_style_border_width(confirm_btn, 2, LV_PART_MAIN);  // ���2���ر߿�
    lv_obj_set_style_border_color(confirm_btn, lv_color_hex(0xFFFFFF), LV_PART_MAIN);  // ��ɫ�߿�
    lv_obj_set_style_border_opa(confirm_btn, LV_OPA_50, LV_PART_MAIN);  // 50%͸���ȣ�����̫����
    
    lv_obj_t * confirm_label = lv_label_create(confirm_btn);
    lv_label_set_text(confirm_label, "Confirm");
    lv_obj_set_style_text_color(confirm_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_font(confirm_label, &lv_font_montserrat_18, LV_PART_MAIN);  // ��������
    lv_obj_center(confirm_label);
    
    // �󶨰�ť�¼���������ָ����Ϊ�û����ݴ��ݣ�
    lv_obj_add_event_cb(confirm_btn, time_settings_confirm_cb, LV_EVENT_CLICKED, time_win);
    
    // Ϊʱ�����ô���������Ƽ��
    lv_obj_add_event_cb(time_win, time_settings_gesture_cb, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(time_win, time_settings_gesture_cb, LV_EVENT_RELEASED, NULL);
}

/**
 * @brief �ı�����򽹵��¼��ص�����
 * 
 * ����˵����
 * 1. ���ı�������ý���ʱ��ʾ����
 * 2. ���Ӽ��̵���ǰ�ı������
 * 3. ȷ��������ʾ����ǰ��
 * 
 * @param e �¼����󣬰����¼������Ϣ
 */
void textarea_focus_cb(lv_event_t * e)
{
    lv_obj_t * ta = lv_event_get_target(e);
    kb_data_t * kb_data = (kb_data_t *)lv_event_get_user_data(e);
    
    // ��ʾ����
    lv_obj_remove_flag(kb_data->keyboard, LV_OBJ_FLAG_HIDDEN);
    
    // ���Ӽ��̵��ı���
    lv_keyboard_set_textarea(kb_data->keyboard, ta);
    
    // �ü����Ƶ���ǰ��
    lv_obj_move_foreground(kb_data->keyboard);
}

/**
 * @brief ���ڵ���¼��ص����� - �������ؼ���
 * 
 * ����˵����
 * 1. �������Ƿ�ɼ�
 * 2. ��������Ƿ��Ǽ��̻��ı���
 * 3. �����������������������ؼ��̲��Ͽ�����
 * 
 * @param e �¼����󣬰����¼������Ϣ
 */
void window_click_cb(lv_event_t * e)
{
    lv_obj_t * clicked_obj = lv_event_get_target(e);
    kb_data_t * kb_data = (kb_data_t *)lv_event_get_user_data(e);
    
    // �������Ƿ�ǰ�ɼ�
    if (lv_obj_has_flag(kb_data->keyboard, LV_OBJ_FLAG_HIDDEN)) {
        return; // ���������أ����账��
    }
    
    // ��������Ƿ��Ǽ��̱�����ı���
    if (clicked_obj == kb_data->keyboard) {
        return; // ������Ǽ��̣�������
    }
    
    // ����Ƿ������ı���
    if (lv_obj_has_class(clicked_obj, &lv_textarea_class)) {
        return; // ��������ı��򣬲�����
    }
    
    // ���ؼ���
    lv_obj_add_flag(kb_data->keyboard, LV_OBJ_FLAG_HIDDEN);
    
    // �Ͽ���������
    lv_keyboard_set_textarea(kb_data->keyboard, NULL);
}

/**
 * @brief ʱ������ȷ�ϰ�ť�ص�����
 * 
 * ����˵����
 * 1. ��ȡ�û����õ�ʱ��ֵ
 * 2. ��֤ʱ��ֵ����Ч��
 * 3. ����RTCӲ��ʱ��
 * 4. ������ʾʱ��
 * 5. �ر�ʱ�����ô���
 * 
 * @param e �¼����󣬰����¼������Ϣ
 */
void time_settings_confirm_cb(lv_event_t * e)
{
    lv_obj_t * time_win = (lv_obj_t *)lv_event_get_user_data(e);
    
    // ���������ı������ؼ��Ի�ȡ���õ�ʱ��ֵ
    lv_obj_t * year_ta = NULL;
    lv_obj_t * month_ta = NULL;
    lv_obj_t * day_ta = NULL;
    lv_obj_t * hour_ta = NULL;
    lv_obj_t * min_ta = NULL;
    
    // �������ڵ��ӿؼ��ҵ��ı������
        uint32_t child_count = lv_obj_get_child_count(time_win);
        for(uint32_t i = 0; i < child_count; i++) {
            lv_obj_t * child = lv_obj_get_child(time_win, i);
            if(lv_obj_has_class(child, &lv_textarea_class)) {
                // ����λ���ж����ĸ��ı������
                lv_coord_t x = lv_obj_get_x(child);
                lv_coord_t y = lv_obj_get_y(child);
                
                if(y < 120) {  // �����������ͬһ��
                    if(x < 150) year_ta = child;           // Year: x=80
                    else if(x < 300) month_ta = child;     // Month: x=240
                    else if(x < 450) day_ta = child;       // Day: x=370
                    else if(x < 600) hour_ta = child;      // Hour: x=510
                    else min_ta = child;                   // Min: x=650
                }
            }
        }    // ��ȡ���õ�ʱ��ֵ
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
        
        // ��֤���뷶Χ
        if(year < 2020 || year > 2030 || 
           month < 1 || month > 12 ||
           day < 1 || day > 31 ||
           hour < 0 || hour > 23 ||
           minute < 0 || minute > 59) {
            
            // ��ʾ������Ϣ
            lv_obj_t * msg_box = lv_msgbox_create(lv_screen_active());
            lv_msgbox_add_title(msg_box, "Error");
            lv_msgbox_add_text(msg_box, "Invalid time values! Please check input.");
            lv_msgbox_add_close_button(msg_box);
            return;
        }
        
        // ���� tm �ṹ��
        struct tm new_time = {0};
        new_time.tm_year = year - 1900;  // tm_year �Ǵ�1900�꿪ʼ����
        new_time.tm_mon = month - 1;     // tm_mon ��Χ��0-11
        new_time.tm_mday = day;
        new_time.tm_hour = hour;
        new_time.tm_min = minute;
        new_time.tm_sec = 0;
        new_time.tm_isdst = -1;          // ��ϵͳ�Զ���������ʱ
        
        // �������ڼ�
        time_t timestamp = mktime(&new_time);
        if (timestamp != -1) {
            struct tm *temp_tm = localtime(&timestamp);
            new_time.tm_wday = temp_tm->tm_wday;
            new_time.tm_yday = temp_tm->tm_yday;
        }
        
        // ����RTCӲ��ʱ��
        if (rtc_set_time(&new_time) == 0) {
            // ���µ�ǰ��ʾʱ��
            current_display_time = new_time;
            
            // �رմ��ڣ���������
            lv_obj_del(time_win);
            extern void desktop(void);
            desktop();
        } else {
            // RTC����ʧ�ܣ�����Ȼ������ʾʱ��
            current_display_time = new_time;
            
            // �رմ��ڣ���������
            lv_obj_del(time_win);
            extern void desktop(void);
            desktop();
        }
    } else {
        // �رմ��ڣ���������
        lv_obj_del(time_win);
        extern void desktop(void);
        desktop();
    }
}

/**
 * @brief ʱ�����ý������Ƽ��ص�����
 * 
 * ͨ���������º��ͷ��¼��ֶ��������
 */
void time_settings_gesture_cb(lv_event_t * e)
{
    static bool touch_started = false;
    static lv_point_t touch_start_point = {0, 0};
    
    lv_event_code_t code = lv_event_get_code(e);
    
    if (code == LV_EVENT_PRESSED) {
        // ��¼������ʼ��
        lv_indev_t * indev = lv_indev_get_act();
        if (indev) {
            lv_indev_get_point(indev, &touch_start_point);
            touch_started = true;
        }
    }
    else if (code == LV_EVENT_RELEASED) {
        if (touch_started) {
            // ��������
            lv_indev_t * indev = lv_indev_get_act();
            if (indev) {
                lv_point_t touch_end_point;
                lv_indev_get_point(indev, &touch_end_point);
                
                int dx = touch_end_point.x - touch_start_point.x;
                int dy = touch_end_point.y - touch_start_point.y;
                
                // �ж��Ƿ�Ϊ��Ч���һ�����
                if (dx > 50 && abs(dy) < 100) {  // �һ�����50���أ���ֱƫ��С��100����
                    // �һ��˳�������
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
 * @brief ʱ������ȡ����ť�ص�����
 * 
 * ����˵����
 * �ر�ʱ�����ô��ڣ��������κθ���
 * 
 * @param e �¼����󣬰����¼������Ϣ
 */
void time_settings_cancel_cb(lv_event_t * e)
{
    lv_obj_t * time_win = (lv_obj_t *)lv_event_get_user_data(e);
    
    // �رմ���
    lv_obj_del(time_win);
}

/**
 * @brief ��RTCӲ��ʱ�Ӷ�ȡʱ��
 * 
 * ����˵����
 * 1. ��RTC�豸
 * 2. ��ȡRTCʱ��
 * 3. ת��Ϊ��׼tm�ṹ
 * 4. �ر��豸�����ؽ��
 * 
 * @param tm_time ���ڴ洢��ȡʱ��Ľṹ��ָ��
 * @return 0��ʾ�ɹ���-1��ʾʧ��
 */
int rtc_read_time(struct tm *tm_time)
{
    int rtc_fd;
    struct rtc_time rtc_tm;
    
    // ��RTC�豸
    rtc_fd = open("/dev/rtc0", O_RDONLY);
    if (rtc_fd < 0) {
        return -1;
    }
    
    // ��ȡRTCʱ��
    if (ioctl(rtc_fd, RTC_RD_TIME, &rtc_tm) < 0) {
        close(rtc_fd);
        return -1;
    }
    
    // ת��Ϊ��׼tm�ṹ
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
 * @brief ����RTCӲ��ʱ��ʱ��
 * 
 * ����˵����
 * 1. ��RTC�豸
 * 2. ת��ΪRTCʱ��ṹ
 * 3. ����RTCʱ��
 * 4. �ر��豸�����ؽ��
 * 
 * @param tm_time Ҫ���õ�ʱ��ṹ��ָ��
 * @return 0��ʾ�ɹ���-1��ʾʧ��
 */
int rtc_set_time(struct tm *tm_time)
{
    int rtc_fd;
    struct rtc_time rtc_tm;
    
    // ��RTC�豸
    rtc_fd = open("/dev/rtc0", O_WRONLY);
    if (rtc_fd < 0) {
        return -1;
    }
    
    // ת��ΪRTCʱ��ṹ
    rtc_tm.tm_sec = tm_time->tm_sec;
    rtc_tm.tm_min = tm_time->tm_min;
    rtc_tm.tm_hour = tm_time->tm_hour;
    rtc_tm.tm_mday = tm_time->tm_mday;
    rtc_tm.tm_mon = tm_time->tm_mon;
    rtc_tm.tm_year = tm_time->tm_year;
    rtc_tm.tm_wday = tm_time->tm_wday;
    rtc_tm.tm_yday = tm_time->tm_yday;
    rtc_tm.tm_isdst = tm_time->tm_isdst;
    
    // ����RTCʱ��
    if (ioctl(rtc_fd, RTC_SET_TIME, &rtc_tm) < 0) {
        close(rtc_fd);
        return -1;
    }
    
    close(rtc_fd);
    return 0;
}
