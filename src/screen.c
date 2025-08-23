#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>

#include "../lvgl/lvgl.h"
#include "../lvgl/demos/lv_demos.h"

#include "../inc/screen.h"

#include "../inc/main_screen.h"



void Screen(void)
{
    // Ϣ��/������Դ�����ͷţ���ֹ������Ӻ�ָ������
    extern void release_desktop_resources(void);
    release_desktop_resources();
    // ��������ָ��ȫ��Ψһ����ֹ����
    extern lv_obj_t *black_screen_mask;
    if (black_screen_mask) {
        lv_obj_del(black_screen_mask);
        black_screen_mask = NULL;
    }

    //����һ������
    lv_obj_t * win = lv_obj_create(lv_screen_active());
    //���ô��ڵĴ�С
    lv_obj_set_size(win, 800, 480);
    //���ô��ڵ�λ��
    lv_obj_set_pos(win, 0, 0);
    //���ô��ڱ�����ɫΪ��ɫ
    lv_obj_set_style_bg_color(win, lv_color_hex(0x000000), LV_STATE_DEFAULT);

    //���ý���ɫ����
    lv_obj_set_style_bg_color(win, lv_color_hex(0x3498db), LV_PART_MAIN);
    lv_obj_set_style_bg_grad_color(win, lv_color_hex(0x9b59b6), LV_PART_MAIN);
    lv_obj_set_style_bg_grad_dir(win, LV_GRAD_DIR_VER, LV_PART_MAIN);
    
    //���û�����⵫���ƻ�����Χ����ֹ�����ƶ�
    lv_obj_add_flag(win, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scroll_dir(win, LV_DIR_ALL);
    lv_obj_set_scrollbar_mode(win, LV_SCROLLBAR_MODE_OFF);
    //���û����߽磬��ֹʵ�ʻ���
    lv_obj_set_scroll_snap_x(win, LV_SCROLL_SNAP_CENTER);
    lv_obj_set_scroll_snap_y(win, LV_SCROLL_SNAP_CENTER);

    //����һ��ͼƬ�ؼ�,��Ϊ����ͼƬ
    lv_obj_t * img = lv_image_create(win);
    //����ͼƬ�ؼ��Ĵ�С
    lv_obj_set_size(img, 800, 480);
    //����ͼƬ�ؼ���λ��
    lv_obj_set_pos(img, 0, 0);
    //����ͼƬ�ؼ���ͼƬ
    lv_image_set_src(img, "/home/tyc/work_station/start_app/main_screen.jpg");
    //��ͼƬ�ؼ�����
    lv_obj_center(img);

    //����ͼƬֻ�����Һ����ϻ�������ֹ�»���
    lv_obj_set_scroll_dir(img, LV_DIR_ALL);
    //����ͼƬ�ؼ��Ĺ��������ɼ�
    lv_obj_set_scrollbar_mode(img, LV_SCROLLBAR_MODE_OFF);
    //����ͼƬ�Ĺ������ô��ڴ������л����¼�
    lv_obj_clear_flag(img, LV_OBJ_FLAG_SCROLLABLE);

    //���ص������󶨵������ϣ��������д����ͻ����¼�
    lv_obj_add_event_cb(win, wallpaper_scroll_event_cb, LV_EVENT_ALL, NULL);
    
    
}

void wallpaper_scroll_event_cb(lv_event_t * e)
{  
    
    //��ȡ�����¼��Ķ���������win���ڣ�
    lv_obj_t * win = lv_event_get_target(e);
    //��ȡ�¼�����
    lv_event_code_t code = lv_event_get_code(e);
    
    
    //��ȡ������ƫ����
    lv_coord_t scroll_x = lv_obj_get_scroll_x(win);
    lv_coord_t scroll_y = lv_obj_get_scroll_y(win);

    // ʹ�þ�̬�������渲�ǲ㣬ֻ����һ��
    static lv_obj_t * cover = NULL;
    static lv_obj_t * label = NULL;
    
    // ֻ�ڵ�һ�δ������ǲ�
    if(cover == NULL) {
        //����һ����͸�����ǲ㣬����������Ļ
        cover = lv_obj_create(lv_screen_active());
        lv_obj_set_size(cover, 800, 480);
        lv_obj_set_pos(cover, 0, 0);
        //���ñ�����ɫΪ��ɫ
        lv_obj_set_style_bg_color(cover, lv_color_hex(0x000000), LV_PART_MAIN);
        //�Ƴ��߿���ڱ߾�
        lv_obj_set_style_border_width(cover, 0, LV_PART_MAIN);
        lv_obj_set_style_pad_all(cover, 0, LV_PART_MAIN);
        //ȷ�����ǲ������ϲ�
        lv_obj_move_foreground(cover);
        //���ø��ǲ�Ľ������ô����¼���͸���²�
        lv_obj_clear_flag(cover, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_clear_flag(cover, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_add_flag(cover, LV_OBJ_FLAG_IGNORE_LAYOUT);
        //��ʼʱ���ظ��ǲ�
        lv_obj_add_flag(cover, LV_OBJ_FLAG_HIDDEN);
    }

    //�ж��Ƿ��ǻ��������¼�
    if(code == LV_EVENT_SCROLL || code == LV_EVENT_GESTURE || code == LV_EVENT_PRESSING) 
    {
        int abs_scroll_y = abs(scroll_y);
        int abs_scroll_x = abs(scroll_x);
        
        // ��ֹ���»���������λ��
        if(scroll_y < 0) {
            lv_obj_scroll_to_y(win, 0, LV_ANIM_ON);
        }
        
        // ��ʾ���ǲ�
        lv_obj_clear_flag(cover, LV_OBJ_FLAG_HIDDEN);
        
        //���ݻ����������ò�ͬ��͸����
        if(abs_scroll_y > 150 || abs_scroll_x > 150) {
            lv_obj_set_style_bg_opa(cover, LV_OPA_60, LV_PART_MAIN);
            // ���ģ��Ч�� - ʹ�ý���
            lv_obj_set_style_bg_grad_color(cover, lv_color_hex(0x333333), LV_PART_MAIN);
            lv_obj_set_style_bg_grad_dir(cover, LV_GRAD_DIR_VER, LV_PART_MAIN);
        }
        else if(abs_scroll_y > 120 || abs_scroll_x > 120) {
            lv_obj_set_style_bg_opa(cover, LV_OPA_50, LV_PART_MAIN);
        }
        else if(abs_scroll_y > 90 || abs_scroll_x > 90) {
            lv_obj_set_style_bg_opa(cover, LV_OPA_40, LV_PART_MAIN);
        }
        else if(abs_scroll_y > 60 || abs_scroll_x > 60) {
            lv_obj_set_style_bg_opa(cover, LV_OPA_30, LV_PART_MAIN);
            
        }
        else if(abs_scroll_y > 30 || abs_scroll_x > 30) {
            lv_obj_set_style_bg_opa(cover, LV_OPA_20, LV_PART_MAIN);
        }
        else {
            lv_obj_set_style_bg_opa(cover, LV_OPA_10, LV_PART_MAIN);
        }
        
        //��������ϻ����Ҵ���100���أ����"�ɿ�����"����ʾ
        if(scroll_y > 100)
        {
            if(label == NULL) {
                label = lv_label_create(cover);
                lv_label_set_text(label, "Release to unlock");
                //����������ɫΪ��ɫ�������׿���
                lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
                //���������С
                lv_obj_set_style_text_font(label, &lv_font_montserrat_20, LV_PART_MAIN);
                //���ñ߿�Ϊ͸��
                lv_obj_set_style_bg_opa(label, LV_OPA_TRANSP, LV_PART_MAIN);
                //���þ���
                lv_obj_center(label);
            }
            lv_obj_clear_flag(label, LV_OBJ_FLAG_HIDDEN);
        }
        else {
            if(label != NULL) {
                lv_obj_add_flag(label, LV_OBJ_FLAG_HIDDEN);
            }
        }
    }
    
    // �����ͷ��¼�������λ�ò����ظ��ǲ�
    if(code == LV_EVENT_RELEASED) {
        // ���û���λ�õ�ԭ��
        lv_obj_scroll_to(win, 0, 0, LV_ANIM_ON);
        
        if(scroll_y > 100)
        {
            //�����������
            UnlockScreen();
        }

        // ���ظ��ǲ�ͱ�ǩ��������ɾ��
        if(cover != NULL) {
            lv_obj_add_flag(cover, LV_OBJ_FLAG_HIDDEN);
        }
        if(label != NULL) {
            lv_obj_add_flag(label, LV_OBJ_FLAG_HIDDEN);
        }
    }
}

// =========================== �����������ȫ�ֱ��� ===========================
static char stored_password[PASSWORD_LENGTH + 1] = "1234"; // �洢�����룬Ĭ��Ϊ1234
static char input_password[PASSWORD_LENGTH + 1] = "";      // �û���������뻺��
static char new_password[PASSWORD_LENGTH + 1] = "";        // ����������ʱ����ʱ�洢
static char confirm_password[PASSWORD_LENGTH + 1] = "";    // ȷ������ʱ����ʱ�洢
static lv_obj_t *unlock_screen = NULL;                     // �����������Ļ����
static lv_obj_t *password_label = NULL;                    // ��ʾ��������״̬�ı�ǩ
static lv_obj_t *set_password_screen = NULL;               // ��������������Ļ����
static lv_obj_t *new_password_label = NULL;                // ��ʾ����������״̬�ı�ǩ
static lv_obj_t *confirm_password_label = NULL;            // ��ʾȷ����������״̬�ı�ǩ
static int password_step = 0;                              // �������ò��裺0=���������룬1=ȷ������
static lv_obj_t *unlock_cover = NULL;                      // ���������͸�����ǲ�
static lv_obj_t *setpw_cover = NULL;                       // ������������͸�����ǲ�

// =========================== �����ļ��������� ===========================

/**
 * @brief ���ļ���������
 * 
 * ����˵����
 * 1. ���Դ������ļ����ж�ȡ
 * 2. ����ļ����ڣ���ȡ���벢�Ƴ����з�
 * 3. ����ļ������ڣ�ʹ��Ĭ������"1234"�����浽�ļ�
 * 4. ȷ����ʹ���������壬����Ҳ�ܱ�����
 * 
 * @param password ���ڴ洢��ȡ����������ַ�����
 */
void load_password(char *password) {
    FILE *file = fopen(PASSWORD_FILE, "r");    // ��ֻ��ģʽ�������ļ�
    if (file != NULL) {
        // �ļ����ڣ���ȡ����
        if (fgets(password, PASSWORD_LENGTH + 1, file) != NULL) {
            // �Ƴ��ַ���ĩβ�Ļ��з���fgets��������з���
            password[strcspn(password, "\n")] = 0;
        }
        fclose(file);   // �ر��ļ�
    } else {
        // �ļ������ڣ�ʹ��Ĭ�����벢����
        strcpy(password, "1234");       // ����Ĭ������
        save_password(password);        // ��Ĭ�����뱣�浽�ļ�
    }
}

/**
 * @brief �������뵽�ļ�
 * 
 * ����˵����
 * 1. ��д��ģʽ�������ļ�
 * 2. ������д���ļ�
 * 3. ȷ������־û����棬�����󲻻ᶪʧ
 * 
 * @param password Ҫ����������ַ���
 */
void save_password(const char *password) {
    FILE *file = fopen(PASSWORD_FILE, "w");    // ��д��ģʽ���ļ�
    if (file != NULL) {
        fprintf(file, "%s", password);          // ������д���ļ�
        fclose(file);                           // �ر��ļ�
    }
}

/**
 * @brief ��֤����������Ƿ���ȷ
 * 
 * ����˵����
 * �Ƚ�����������洢�����Ƿ���ͬ
 * 
 * @param input �û����������
 * @return ����1��ʾ������ȷ������0��ʾ�������
 */
int verify_password(const char *input) {
    return strcmp(input, stored_password) == 0; // �Ƚ�����������洢�����Ƿ���ͬ
}

// =========================== �������洴������ ===========================

/**
 * @brief ��������ʾ��������
 * 
 * ����˵����
 * 1. ���ر��������
 * 2. ����������뻺��
 * 3. ����ģ����������������Ļ
 * 4. ������������������������
 *    - ��������
 *    - ������ʾ������*����ʾ��
 *    - ���ּ��̣�0-9��
 *    - ȡ����ť������������
 *    - �������밴ť�����������룩
 */
void UnlockScreen(void) {
    // Ϣ��/������Դ�����ͷţ���ֹ������Ӻ�ָ������
    extern void release_desktop_resources(void);
    release_desktop_resources();
    extern lv_obj_t *black_screen_mask;
    if (black_screen_mask) {
        lv_obj_del(black_screen_mask);
        black_screen_mask = NULL;
    }

    // ���ļ�����֮ǰ���������
    load_password(stored_password);
    
    // ����������뻺�棬׼�������µ�����
    memset(input_password, 0, sizeof(input_password));
    
    // ========== �������������͸�����ǲ� ========== 
    if(unlock_cover) {
        lv_obj_del(unlock_cover);
        unlock_cover = NULL;
    }
    unlock_cover = lv_obj_create(lv_screen_active());
    lv_obj_set_size(unlock_cover, 800, 480);
    lv_obj_set_pos(unlock_cover, 0, 0);
    lv_obj_set_style_bg_color(unlock_cover, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(unlock_cover, LV_OPA_80, LV_PART_MAIN); // ��ǿģ��
    lv_obj_set_style_border_width(unlock_cover, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(unlock_cover, 0, LV_PART_MAIN);
    lv_obj_clear_flag(unlock_cover, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(unlock_cover, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(unlock_cover, LV_OBJ_FLAG_IGNORE_LAYOUT);
    // ========== ������������������ ========== 
    unlock_screen = lv_obj_create(lv_screen_active());
    lv_obj_set_size(unlock_screen, 800, 480);
    lv_obj_set_pos(unlock_screen, 0, 0);
    // ֻ���������߿��������ޱ߿�ȫ͸��
    lv_obj_set_style_bg_opa(unlock_screen, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(unlock_screen, 0, LV_PART_MAIN);
    
    // ========== ���������������� ========== 
    lv_obj_t *password_container = lv_obj_create(unlock_screen);
    lv_obj_set_size(password_container, 500, 430);
    lv_obj_center(password_container);
    // ����ȫ͸�����ޱ߿�
    lv_obj_set_style_bg_opa(password_container, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(password_container, 0, LV_PART_MAIN);
    
    // ========== ɾ������ ========== 
    // ����������label
    
    // ========== ����������ʾ���� ========== 
    password_label = lv_label_create(password_container);
    lv_label_set_text(password_label, "________");             // ��ʼ��ʾ8���»���
    lv_obj_set_style_text_color(password_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN); // ��ɫ����
    lv_obj_set_style_text_font(password_label, &lv_font_montserrat_32, LV_PART_MAIN);  // 32������
    lv_obj_set_pos(password_label, 160, 10); // ����һ�㣬����̫����
    // ����͸����label�����ޱ�����������͸����

    
    // ========== �������ּ������� ========== 
    lv_obj_t *keypad = lv_obj_create(password_container);
    lv_obj_set_size(keypad, 350, 250);
    lv_obj_set_pos(keypad, 85, 80);
    lv_obj_set_style_bg_opa(keypad, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(keypad, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(keypad, 5, LV_PART_MAIN);
    
    // ========== ����Բ�����ְ�ť1-9��3x3���֣�==========
    int btn_size = 50;
    int btn_h_space = 70; // ˮƽ���
    int btn_v_space = 60; // ��ֱ���
    int keypad_offset_x = 40; // ʹ�������
    int keypad_offset_y = 10;
    for (int i = 1; i <= 9; i++) {
        lv_obj_t *btn = lv_button_create(keypad);
        lv_obj_set_size(btn, btn_size, btn_size);
        int row = (i - 1) / 3;
        int col = (i - 1) % 3;
        lv_obj_set_pos(btn, keypad_offset_x + col * btn_h_space, keypad_offset_y + row * btn_v_space);
        lv_obj_set_style_bg_opa(btn, LV_OPA_TRANSP, LV_PART_MAIN);
        lv_obj_set_style_border_width(btn, 2, LV_PART_MAIN);
        lv_obj_set_style_border_color(btn, lv_color_hex(0xe74c3c), LV_PART_MAIN);
        lv_obj_set_style_border_opa(btn, LV_OPA_80, LV_PART_MAIN);
        lv_obj_set_style_radius(btn, LV_RADIUS_CIRCLE, LV_PART_MAIN);
        lv_obj_t *label = lv_label_create(btn);
        lv_label_set_text_fmt(label, "%d", i);
        lv_obj_center(label);
        lv_obj_set_style_text_font(label, &lv_font_montserrat_20, LV_PART_MAIN);
        lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
        lv_obj_add_event_cb(btn, unlock_keypad_event_cb, LV_EVENT_CLICKED, (void*)(intptr_t)i);
    }
    // ========== ����Բ������0��ť ========== 
    lv_obj_t *btn0 = lv_button_create(keypad);
    lv_obj_set_size(btn0, btn_size, btn_size);
    lv_obj_set_pos(btn0, keypad_offset_x + btn_h_space, keypad_offset_y + 3 * btn_v_space);
    lv_obj_set_style_bg_opa(btn0, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(btn0, 2, LV_PART_MAIN);
    lv_obj_set_style_border_color(btn0, lv_color_hex(0xe74c3c), LV_PART_MAIN);
    lv_obj_set_style_border_opa(btn0, LV_OPA_80, LV_PART_MAIN);
    lv_obj_set_style_radius(btn0, LV_RADIUS_CIRCLE, LV_PART_MAIN);
    lv_obj_t *label0 = lv_label_create(btn0);
    lv_label_set_text(label0, "0");
    lv_obj_center(label0);
    lv_obj_set_style_text_font(label0, &lv_font_montserrat_20, LV_PART_MAIN);
    lv_obj_set_style_text_color(label0, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_add_event_cb(btn0, unlock_keypad_event_cb, LV_EVENT_CLICKED, (void*)(intptr_t)0);
    
    // ========== ����ȡ����ť ========== 
    lv_obj_t *cancel_btn = lv_button_create(password_container);
    lv_obj_set_size(cancel_btn, 120, 45);
    lv_obj_set_pos(cancel_btn, 90, 330);
    lv_obj_set_style_bg_opa(cancel_btn, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(cancel_btn, 1, LV_PART_MAIN);
    lv_obj_set_style_border_color(cancel_btn, lv_color_hex(0xf39c12), LV_PART_MAIN); // ��ɫ
    lv_obj_set_style_border_opa(cancel_btn, LV_OPA_80, LV_PART_MAIN);
    lv_obj_set_style_radius(cancel_btn, 15, LV_PART_MAIN);
    lv_obj_t *cancel_label = lv_label_create(cancel_btn);
    lv_label_set_text(cancel_label, "Cancel");
    lv_obj_center(cancel_label);
    lv_obj_set_style_text_color(cancel_label, lv_color_hex(0xf39c12), LV_PART_MAIN); // ��ɫ
    lv_obj_add_event_cb(cancel_btn, cancel_btn_event_cb, LV_EVENT_CLICKED, NULL);
    // ========== �����������밴ť ========== 
    lv_obj_t *forgot_btn = lv_button_create(password_container);
    lv_obj_set_size(forgot_btn, 120, 45);
    lv_obj_set_pos(forgot_btn, 290, 330);
    lv_obj_set_style_bg_opa(forgot_btn, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(forgot_btn, 1, LV_PART_MAIN);
    lv_obj_set_style_border_color(forgot_btn, lv_color_hex(0xf39c12), LV_PART_MAIN);
    lv_obj_set_style_border_opa(forgot_btn, LV_OPA_80, LV_PART_MAIN);
    lv_obj_set_style_radius(forgot_btn, 15, LV_PART_MAIN);
    lv_obj_t *forgot_label = lv_label_create(forgot_btn);
    lv_label_set_text(forgot_label, "Forgot");
    lv_obj_center(forgot_label);
    lv_obj_set_style_text_color(forgot_label, lv_color_hex(0xf39c12), LV_PART_MAIN);
    lv_obj_add_event_cb(forgot_btn, forgot_btn_event_cb, LV_EVENT_CLICKED, NULL);
}

// =========================== �¼������� ===========================

/**
 * @brief �����������ּ����¼�������
 * 
 * ����˵����
 * 1. ��ȡ�û���������֣�0-9��
 * 2. �����������뻺��
 * 3. ���½�����ʾ����*����ʾ�������λ����
 * 4. ������4λ������Զ���֤
 * 5. ������ȷ��رս������棬��������ʾ��ʾ���������
 * 
 * @param e �¼����󣬰����¼������Ϣ
 */
void unlock_keypad_event_cb(lv_event_t * e) {
    int num = (int)(intptr_t)lv_event_get_user_data(e);
    int len = strlen(input_password);
    if (len < PASSWORD_LENGTH) {
        input_password[len] = '0' + num;
        input_password[len + 1] = '\0';
    }
    char display[9];
    int input_len = strlen(input_password);
    int underlines = 8 - (input_len * 2);
    int i = 0;
    for (i = 0; i < input_len; i++) {
        display[i] = '*';
    }
    for (int j = 0; j < underlines && i < 8; j++, i++) {
        display[i] = '_';
    }
    display[i] = '\0';
    lv_label_set_text(password_label, display);
    if (strlen(input_password) == PASSWORD_LENGTH) {
        if (verify_password(input_password)) {
            if (unlock_screen) {
                lv_obj_del(unlock_screen);
                unlock_screen = NULL;
            }
            if (unlock_cover) {
                lv_obj_del(unlock_cover);
                unlock_cover = NULL;
            }
            desktop();
        } else {
            memset(input_password, 0, sizeof(input_password));
            lv_label_set_text(password_label, "Wrong!");
        }
    }
}

/**
 * @brief ȡ����ť�¼�������
 * 
 * ����˵����
 * �رս������棬��������״̬
 * 
 * @param e �¼����󣬰����¼������Ϣ
 */
void cancel_btn_event_cb(lv_event_t * e) {
    if (unlock_screen != NULL) {
        lv_obj_del(unlock_screen);                  // ɾ����������
        unlock_screen = NULL;                       // ���ָ��
    }
    if (unlock_cover != NULL) {
        lv_obj_del(unlock_cover);
        unlock_cover = NULL;
    }
}

/**
 * @brief �������밴ť�¼�������
 * @param e �¼�����
 * 
 * ����˵����
 * 1. �رյ�ǰ��������
 * 2. ���������������
 */
void forgot_btn_event_cb(lv_event_t * e) {
    // ɾ����������
    if (unlock_screen != NULL) {
        lv_obj_del(unlock_screen);
        unlock_screen = NULL;
    }
    if (unlock_cover != NULL) {
        lv_obj_del(unlock_cover);
        unlock_cover = NULL;
    }
    // �������������
    SetPasswordScreen();
}

// =========================== ����������� ===========================

/**
 * @brief �����������������
 * 
 * ����˵����
 * 1. ��������������Ľ���
 * 2. ���沼��������������ƣ������ܲ�ͬ
 * 3. �û����������µ�4λ��������
 * 4. ���ȷ�Ϻ�ᱣ�������뵽�ļ�
 * 5. �������������Ч�����������󱣳�
 */
void SetPasswordScreen(void) {
    // ������������뻺��
    memset(new_password, 0, sizeof(new_password));
    memset(confirm_password, 0, sizeof(confirm_password));
    password_step = 0;  // ���ò���Ϊ����������
    
    // ========== ���������������ģ���� ========== 
    if(setpw_cover) {
        lv_obj_del(setpw_cover);
        setpw_cover = NULL;
    }
    setpw_cover = lv_obj_create(lv_screen_active());
    lv_obj_set_size(setpw_cover, 800, 480);
    lv_obj_set_pos(setpw_cover, 0, 0);
    lv_obj_set_style_bg_color(setpw_cover, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(setpw_cover, LV_OPA_80, LV_PART_MAIN); // ��ɫ��80%��͸��
    lv_obj_set_style_border_width(setpw_cover, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(setpw_cover, 0, LV_PART_MAIN);
    lv_obj_clear_flag(setpw_cover, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(setpw_cover, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(setpw_cover, LV_OBJ_FLAG_IGNORE_LAYOUT);
    // ���ӽ���ɫ
    // �ŵ����ϲ㣬������������
    lv_obj_move_foreground(setpw_cover);
    // ========== ��������������������� ===========
    set_password_screen = lv_obj_create(lv_screen_active());
    lv_obj_set_size(set_password_screen, 800, 480);
    lv_obj_set_pos(set_password_screen, 0, 0);
    // ������ȫ͸��
    lv_obj_set_style_bg_opa(set_password_screen, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(set_password_screen, 0, LV_PART_MAIN);
    lv_obj_add_flag(set_password_screen, LV_OBJ_FLAG_GESTURE_BUBBLE); // ��������������ð��
    // ��֤�������ڸ��ǲ�֮��
    lv_obj_move_foreground(set_password_screen);
    
    // ========== ���������������� ========== 
    lv_obj_t *password_container = lv_obj_create(set_password_screen);
    lv_obj_set_size(password_container, 500, 430);
    lv_obj_center(password_container);
    // ��������ȫ͸��
    lv_obj_set_style_bg_opa(password_container, LV_OPA_TRANSP, LV_PART_MAIN);
    // ��������������͸���ȣ�ֻȥ���߿�
    lv_obj_set_style_border_width(password_container, 0, LV_PART_MAIN);
    lv_obj_add_flag(password_container, LV_OBJ_FLAG_GESTURE_BUBBLE); // ������������ð��
    
    // ========== �������� ==========
    lv_obj_t *title = lv_label_create(password_container);
    lv_label_set_text(title, "Enter New Password");          // �������������
    lv_obj_set_style_text_color(title, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_24, LV_PART_MAIN);
    lv_obj_set_pos(title, 100, 0);                          // ��������λ��
    
    // ========== ������������ʾ���� ==========
    new_password_label = lv_label_create(password_container);
    lv_label_set_text(new_password_label, "________");         // ��ʼ��ʾ8���»���
    lv_obj_set_style_text_color(new_password_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_font(new_password_label, &lv_font_montserrat_32, LV_PART_MAIN);
    lv_obj_set_pos(new_password_label, 155, 30); // ���Ʋ����ƣ�������
    
    // ========== ����ȷ��������ʾ���� ==========
    confirm_password_label = lv_label_create(password_container);
    lv_label_set_text(confirm_password_label, "________");     // ��ʼ��ʾ8���»���
    lv_obj_set_style_text_color(confirm_password_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_font(confirm_password_label, &lv_font_montserrat_32, LV_PART_MAIN);
    lv_obj_set_pos(confirm_password_label, 155, 55); // ���Ʋ����ƣ���������label����
    lv_obj_add_flag(confirm_password_label, LV_OBJ_FLAG_HIDDEN); // ��ʼ����
    
    // ========== �������ּ��� ========== 
    lv_obj_t *keypad = lv_obj_create(password_container);
    lv_obj_set_size(keypad, 350, 240);
    lv_obj_set_pos(keypad, 85, 95);
    lv_obj_set_style_bg_opa(keypad, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(keypad, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(keypad, 5, LV_PART_MAIN);
    // ========== ����Բ�����ְ�ť1-9 ========== 
    int btn_size = 50;
    int btn_h_space = 70;
    int btn_v_space = 60;
    int keypad_offset_x = 40;
    int keypad_offset_y = 0;
    for (int i = 1; i <= 9; i++) {
        lv_obj_t *btn = lv_button_create(keypad);
        lv_obj_set_size(btn, btn_size, btn_size);
        int row = (i - 1) / 3;
        int col = (i - 1) % 3;
        lv_obj_set_pos(btn, keypad_offset_x + col * btn_h_space, keypad_offset_y + row * btn_v_space);
        lv_obj_set_style_bg_opa(btn, LV_OPA_TRANSP, LV_PART_MAIN);
        lv_obj_set_style_border_width(btn, 2, LV_PART_MAIN);
        lv_obj_set_style_border_color(btn, lv_color_hex(0xe74c3c), LV_PART_MAIN);
        lv_obj_set_style_border_opa(btn, LV_OPA_80, LV_PART_MAIN);
        lv_obj_set_style_radius(btn, LV_RADIUS_CIRCLE, LV_PART_MAIN);
        lv_obj_t *label = lv_label_create(btn);
        lv_label_set_text_fmt(label, "%d", i);
        lv_obj_center(label);
        lv_obj_set_style_text_font(label, &lv_font_montserrat_20, LV_PART_MAIN);
        lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
        lv_obj_add_event_cb(btn, set_password_keypad_event_cb, LV_EVENT_CLICKED, (void*)(intptr_t)i);
    }
    // ========== ����Back��ť ========== 
    lv_obj_t *back_btn = lv_button_create(keypad);
    lv_obj_set_size(back_btn, btn_size, btn_size);
    lv_obj_set_pos(back_btn, keypad_offset_x + 0 * btn_h_space, keypad_offset_y + 3 * btn_v_space);
    lv_obj_set_style_bg_opa(back_btn, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(back_btn, 2, LV_PART_MAIN);
    lv_obj_set_style_border_color(back_btn, lv_color_hex(0xe74c3c), LV_PART_MAIN);
    lv_obj_set_style_border_opa(back_btn, LV_OPA_80, LV_PART_MAIN);
    lv_obj_set_style_radius(back_btn, LV_RADIUS_CIRCLE, LV_PART_MAIN);
    lv_obj_t *back_label = lv_label_create(back_btn);
    lv_label_set_text(back_label, "Back");
    lv_obj_center(back_label);
    lv_obj_set_style_text_font(back_label, &lv_font_montserrat_16, LV_PART_MAIN);
    lv_obj_set_style_text_color(back_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_add_event_cb(back_btn, set_password_back_btn_event_cb, LV_EVENT_CLICKED, NULL);
    // ========== ����Բ������0��ť ========== 
    lv_obj_t *btn0 = lv_button_create(keypad);
    lv_obj_set_size(btn0, btn_size, btn_size);
    lv_obj_set_pos(btn0, keypad_offset_x + 1 * btn_h_space, keypad_offset_y + 3 * btn_v_space);
    lv_obj_set_style_bg_opa(btn0, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(btn0, 2, LV_PART_MAIN);
    lv_obj_set_style_border_color(btn0, lv_color_hex(0xe74c3c), LV_PART_MAIN);
    lv_obj_set_style_border_opa(btn0, LV_OPA_80, LV_PART_MAIN);
    lv_obj_set_style_radius(btn0, LV_RADIUS_CIRCLE, LV_PART_MAIN);
    lv_obj_t *label0 = lv_label_create(btn0);
    lv_label_set_text(label0, "0");
    lv_obj_center(label0);
    lv_obj_set_style_text_font(label0, &lv_font_montserrat_20, LV_PART_MAIN);
    lv_obj_set_style_text_color(label0, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_add_event_cb(btn0, set_password_keypad_event_cb, LV_EVENT_CLICKED, (void*)(intptr_t)0);
    // ========== ����Բ��ɾ����ť ========== 
    lv_obj_t *del_btn = lv_button_create(keypad);
    lv_obj_set_size(del_btn, btn_size, btn_size);
    lv_obj_set_pos(del_btn, keypad_offset_x + 2 * btn_h_space, keypad_offset_y + 3 * btn_v_space);
    lv_obj_set_style_bg_opa(del_btn, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(del_btn, 2, LV_PART_MAIN);
    lv_obj_set_style_border_color(del_btn, lv_color_hex(0xe74c3c), LV_PART_MAIN);
    lv_obj_set_style_border_opa(del_btn, LV_OPA_80, LV_PART_MAIN);
    lv_obj_set_style_radius(del_btn, LV_RADIUS_CIRCLE, LV_PART_MAIN);
    lv_obj_t *del_label = lv_label_create(del_btn);
    lv_label_set_text(del_label, "DEL");
    lv_obj_center(del_label);
    lv_obj_set_style_text_font(del_label, &lv_font_montserrat_16, LV_PART_MAIN);
    lv_obj_set_style_text_color(del_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_add_event_cb(del_btn, set_password_keypad_event_cb, LV_EVENT_CLICKED, (void*)(intptr_t)-1);
    
    // ========== ����ȷ�����ð�ť ========== 
    lv_obj_t *confirm_btn = lv_button_create(password_container);
    lv_obj_set_size(confirm_btn, 200, 45);
    lv_obj_set_pos(confirm_btn, 137, 340);
    lv_obj_set_style_bg_opa(confirm_btn, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(confirm_btn, 1, LV_PART_MAIN);
    lv_obj_set_style_border_color(confirm_btn, lv_color_hex(0xf39c12), LV_PART_MAIN); // ��ɫ
    lv_obj_set_style_border_opa(confirm_btn, LV_OPA_80, LV_PART_MAIN);
    lv_obj_t *confirm_label = lv_label_create(confirm_btn);
    lv_label_set_text(confirm_label, "Save");
    lv_obj_center(confirm_label);
    lv_obj_set_style_text_color(confirm_label, lv_color_hex(0xf39c12), LV_PART_MAIN); // ��ɫ
    lv_obj_add_event_cb(confirm_btn, confirm_set_password_btn_event_cb, LV_EVENT_CLICKED, NULL);
    // ��������һ����Ʒ���

// ========== Back��ť�¼����� ==========
}

void set_password_back_btn_event_cb(lv_event_t * e) {
    if (set_password_screen) {
        lv_obj_del(set_password_screen);
        set_password_screen = NULL;
    }
    if (setpw_cover) {
        lv_obj_del(setpw_cover);
        setpw_cover = NULL;
    }
    Screen();
}

/**
 * @brief ���������������ּ����¼�������
 * @param e �¼�����
 * 
 * ����˵����
 * 1. �����û�����������������������
 * 2. ��������������뻺��
 * 3. ���½�����ʾ
 * 4. ��������治ͬ�����ﲻ���Զ���֤����Ҫ�û����ȷ�ϰ�ť
 */
void set_password_keypad_event_cb(lv_event_t * e) {
    int num = (int)(intptr_t)lv_event_get_user_data(e);    // ��ȡ�û����������
    
    if (password_step == 0) {
        // ��һ��������������
        int len = strlen(new_password);
        
        if (num == -1) { 
            // ɾ������
            if (len > 0) {
                new_password[len - 1] = '\0';
            }
        } else if (len < PASSWORD_LENGTH) { 
            // ��������
            new_password[len] = '0' + num;
            new_password[len + 1] = '\0';
        }
        
        // ������������ʾ
        char display[9];
        int input_len = strlen(new_password);
        int underlines = 8 - (input_len * 2);
        
        int i = 0;
        for (i = 0; i < input_len; i++) {
            display[i] = '*';
        }
        for (int j = 0; j < underlines && i < 8; j++, i++) {
            display[i] = '_';
        }
        display[i] = '\0';
        lv_label_set_text(new_password_label, display);
        
        // ����Ƿ�������ɣ�����ȷ�ϲ���
        if (strlen(new_password) == PASSWORD_LENGTH) {
            password_step = 1;
            // ��ʾȷ����������
            lv_obj_clear_flag(confirm_password_label, LV_OBJ_FLAG_HIDDEN);
            // ���±���
            lv_obj_t *title = lv_obj_get_child(lv_obj_get_parent(new_password_label), 0);
            lv_label_set_text(title, "Confirm Password");
            lv_obj_set_pos(title, 125, 0);
        }
    } else {
        // �ڶ�����ȷ������
        int len = strlen(confirm_password);
        
        if (num == -1) { 
            // ɾ������
            if (len > 0) {
                confirm_password[len - 1] = '\0';
            }
        } else if (len < PASSWORD_LENGTH) { 
            // ��������
            confirm_password[len] = '0' + num;
            confirm_password[len + 1] = '\0';
        }
        
        // ����ȷ��������ʾ
        char display[9];
        int input_len = strlen(confirm_password);
        int underlines = 8 - (input_len * 2);
        
        int i = 0;
        for (i = 0; i < input_len; i++) {
            display[i] = '*';
        }
        for (int j = 0; j < underlines && i < 8; j++, i++) {
            display[i] = '_';
        }
        display[i] = '\0';
        lv_label_set_text(confirm_password_label, display);
    }
}

/**
 * @brief ȷ���������밴ť�¼�������
 * @param e �¼�����
 * 
 * ����˵����
 * 1. ����������Ƿ�����������4λ��
 * 2. ����������򱣴������뵽ȫ�ֱ������ļ�
 * 3. �ر������������
 * 4. ������������Ч��������Ҳ�ᱣ��
 */
void confirm_set_password_btn_event_cb(lv_event_t * e) {
    if (password_step == 0) {
        // ��������������׶Σ�������ȷ��
        return;
    }
    
    if (strlen(confirm_password) == PASSWORD_LENGTH) {
        // ������������Ƿ�һ��
        if (strcmp(new_password, confirm_password) == 0) {
            // ����һ�£���������
            strcpy(stored_password, new_password);             // ����ȫ���������
            save_password(stored_password);                    // ���浽�ļ���ȷ�������󲻶�ʧ
            // �ر������������
            if (set_password_screen != NULL) {
                lv_obj_del(set_password_screen);
                set_password_screen = NULL;
            }
            if (setpw_cover != NULL) {
                lv_obj_del(setpw_cover);
                setpw_cover = NULL;
            }
            // �������ѱ���
        } else {
            // ���벻һ�£����¿�ʼ
            memset(new_password, 0, sizeof(new_password));
            memset(confirm_password, 0, sizeof(confirm_password));
            password_step = 0;
            // ������ʾ
            lv_label_set_text(new_password_label, "________");
            lv_label_set_text(confirm_password_label, "________");
            lv_obj_add_flag(confirm_password_label, LV_OBJ_FLAG_HIDDEN);
            // ���±���
            lv_obj_t *title = lv_obj_get_child(lv_obj_get_parent(new_password_label), 0);
            lv_label_set_text(title, "Passwords don't match! Try again");
            lv_obj_set_pos(title, 30, 0);                  // �������ƶ�����50��Ϊ30
            // ���벻ƥ�䣬�û���Ҫ��������
        }
    }
    // ���ȷ�����벻��4λ����ִ���κβ������û���Ҫ��������
}

// ������������һ����ؽ�������
void set_password_swipe_event_cb(lv_event_t * e) {
    lv_indev_t * indev = lv_indev_get_act();
    if (!indev) return;
    lv_dir_t gesture = lv_indev_get_gesture_dir(indev);
    if (gesture == LV_DIR_RIGHT) {
        if (set_password_screen) {
            lv_obj_del(set_password_screen);
            set_password_screen = NULL;
        }
        if (setpw_cover) {
            lv_obj_del(setpw_cover);
            setpw_cover = NULL;
        }
        Screen(); // �ص�����������
    }
}
