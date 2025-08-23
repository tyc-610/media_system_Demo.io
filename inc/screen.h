#ifndef __SCREEN_H__
#define __SCREEN_H__

#define PASSWORD_FILE "/home/tyc/work_station/start_app/password.txt"
#define PASSWORD_LENGTH 4

void Screen(void);
void desktop(void);
void wallpaper_scroll_event_cb(lv_event_t * e);
void UnlockScreen(void);
void SetPasswordScreen(void);
void load_password(char *password);
void save_password(const char *password);
int verify_password(const char *input);
void unlock_keypad_event_cb(lv_event_t * e);
void cancel_btn_event_cb(lv_event_t * e);
void forgot_btn_event_cb(lv_event_t * e);
void set_password_keypad_event_cb(lv_event_t * e);
void confirm_set_password_btn_event_cb(lv_event_t * e);



// ������������ȫ�ֱ��������δ��������
extern lv_obj_t *black_screen_mask;

// ���������������ּ����¼��ص�����ֹ���Ӵ���
void unlock_keypad_event_cb(lv_event_t * e);

// �����һ����ػص�����ֹδ��������
void set_password_swipe_event_cb(lv_event_t * e);

// ���������������Back��ť�ص�����ֹδ��������
void set_password_back_btn_event_cb(lv_event_t * e);
#endif