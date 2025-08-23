#ifndef __MAIN_H__
#define __MAIN_H__

// º¯ÊýÉùÃ÷

void BEEP_Ctrl( unsigned char sta);
void Event_beep_ctrl_cb(lv_event_t * e);
void Print_Point_XY_cb(lv_event_t * e );
void Print_Mov_Dir_cb(lv_event_t * e);

void test05();
void test07();
void test08();
void boot_animation_timer_cb(lv_timer_t * timer);

#endif