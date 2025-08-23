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
    // 息屏/桌面资源彻底释放，防止界面叠加和指针悬挂
    extern void release_desktop_resources(void);
    release_desktop_resources();
    // 黑屏遮罩指针全局唯一，防止悬挂
    extern lv_obj_t *black_screen_mask;
    if (black_screen_mask) {
        lv_obj_del(black_screen_mask);
        black_screen_mask = NULL;
    }

    //创建一个窗口
    lv_obj_t * win = lv_obj_create(lv_screen_active());
    //设置窗口的大小
    lv_obj_set_size(win, 800, 480);
    //设置窗口的位置
    lv_obj_set_pos(win, 0, 0);
    //设置窗口背景颜色为黑色
    lv_obj_set_style_bg_color(win, lv_color_hex(0x000000), LV_STATE_DEFAULT);

    //设置渐变色背景
    lv_obj_set_style_bg_color(win, lv_color_hex(0x3498db), LV_PART_MAIN);
    lv_obj_set_style_bg_grad_color(win, lv_color_hex(0x9b59b6), LV_PART_MAIN);
    lv_obj_set_style_bg_grad_dir(win, LV_GRAD_DIR_VER, LV_PART_MAIN);
    
    //启用滑动检测但限制滑动范围，防止界面移动
    lv_obj_add_flag(win, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scroll_dir(win, LV_DIR_ALL);
    lv_obj_set_scrollbar_mode(win, LV_SCROLLBAR_MODE_OFF);
    //设置滑动边界，防止实际滑动
    lv_obj_set_scroll_snap_x(win, LV_SCROLL_SNAP_CENTER);
    lv_obj_set_scroll_snap_y(win, LV_SCROLL_SNAP_CENTER);

    //创建一个图片控件,作为锁屏图片
    lv_obj_t * img = lv_image_create(win);
    //设置图片控件的大小
    lv_obj_set_size(img, 800, 480);
    //设置图片控件的位置
    lv_obj_set_pos(img, 0, 0);
    //设置图片控件的图片
    lv_image_set_src(img, "/home/tyc/work_station/start_app/main_screen.jpg");
    //将图片控件居中
    lv_obj_center(img);

    //限制图片只能左右和向上滑动（禁止下滑）
    lv_obj_set_scroll_dir(img, LV_DIR_ALL);
    //设置图片控件的滚动条不可见
    lv_obj_set_scrollbar_mode(img, LV_SCROLLBAR_MODE_OFF);
    //禁用图片的滚动，让窗口处理所有滑动事件
    lv_obj_clear_flag(img, LV_OBJ_FLAG_SCROLLABLE);

    //将回调函数绑定到窗口上，捕获所有触摸和滑动事件
    lv_obj_add_event_cb(win, wallpaper_scroll_event_cb, LV_EVENT_ALL, NULL);
    
    
}

void wallpaper_scroll_event_cb(lv_event_t * e)
{  
    
    //获取触发事件的对象（现在是win窗口）
    lv_obj_t * win = lv_event_get_target(e);
    //获取事件代码
    lv_event_code_t code = lv_event_get_code(e);
    
    
    //获取滑动的偏移量
    lv_coord_t scroll_x = lv_obj_get_scroll_x(win);
    lv_coord_t scroll_y = lv_obj_get_scroll_y(win);

    // 使用静态变量保存覆盖层，只创建一次
    static lv_obj_t * cover = NULL;
    static lv_obj_t * label = NULL;
    
    // 只在第一次创建覆盖层
    if(cover == NULL) {
        //创建一个半透明覆盖层，覆盖整个屏幕
        cover = lv_obj_create(lv_screen_active());
        lv_obj_set_size(cover, 800, 480);
        lv_obj_set_pos(cover, 0, 0);
        //设置背景颜色为黑色
        lv_obj_set_style_bg_color(cover, lv_color_hex(0x000000), LV_PART_MAIN);
        //移除边框和内边距
        lv_obj_set_style_border_width(cover, 0, LV_PART_MAIN);
        lv_obj_set_style_pad_all(cover, 0, LV_PART_MAIN);
        //确保覆盖层在最上层
        lv_obj_move_foreground(cover);
        //禁用覆盖层的交互，让触摸事件穿透到下层
        lv_obj_clear_flag(cover, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_clear_flag(cover, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_add_flag(cover, LV_OBJ_FLAG_IGNORE_LAYOUT);
        //初始时隐藏覆盖层
        lv_obj_add_flag(cover, LV_OBJ_FLAG_HIDDEN);
    }

    //判断是否是滑动或触摸事件
    if(code == LV_EVENT_SCROLL || code == LV_EVENT_GESTURE || code == LV_EVENT_PRESSING) 
    {
        int abs_scroll_y = abs(scroll_y);
        int abs_scroll_x = abs(scroll_x);
        
        // 防止向下滑动，重置位置
        if(scroll_y < 0) {
            lv_obj_scroll_to_y(win, 0, LV_ANIM_ON);
        }
        
        // 显示覆盖层
        lv_obj_clear_flag(cover, LV_OBJ_FLAG_HIDDEN);
        
        //根据滑动距离设置不同的透明度
        if(abs_scroll_y > 150 || abs_scroll_x > 150) {
            lv_obj_set_style_bg_opa(cover, LV_OPA_60, LV_PART_MAIN);
            // 添加模糊效果 - 使用渐变
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
        
        //如果是向上滑动且大于100像素，添加"松开解锁"的提示
        if(scroll_y > 100)
        {
            if(label == NULL) {
                label = lv_label_create(cover);
                lv_label_set_text(label, "Release to unlock");
                //设置字体颜色为白色，更容易看到
                lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
                //设置字体大小
                lv_obj_set_style_text_font(label, &lv_font_montserrat_20, LV_PART_MAIN);
                //设置边框为透明
                lv_obj_set_style_bg_opa(label, LV_OPA_TRANSP, LV_PART_MAIN);
                //设置居中
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
    
    // 处理释放事件，重置位置并隐藏覆盖层
    if(code == LV_EVENT_RELEASED) {
        // 重置滑动位置到原点
        lv_obj_scroll_to(win, 0, 0, LV_ANIM_ON);
        
        if(scroll_y > 100)
        {
            //进入解锁界面
            UnlockScreen();
        }

        // 隐藏覆盖层和标签，而不是删除
        if(cover != NULL) {
            lv_obj_add_flag(cover, LV_OBJ_FLAG_HIDDEN);
        }
        if(label != NULL) {
            lv_obj_add_flag(label, LV_OBJ_FLAG_HIDDEN);
        }
    }
}

// =========================== 解锁功能相关全局变量 ===========================
static char stored_password[PASSWORD_LENGTH + 1] = "1234"; // 存储的密码，默认为1234
static char input_password[PASSWORD_LENGTH + 1] = "";      // 用户输入的密码缓存
static char new_password[PASSWORD_LENGTH + 1] = "";        // 设置新密码时的临时存储
static char confirm_password[PASSWORD_LENGTH + 1] = "";    // 确认密码时的临时存储
static lv_obj_t *unlock_screen = NULL;                     // 解锁界面的屏幕对象
static lv_obj_t *password_label = NULL;                    // 显示密码输入状态的标签
static lv_obj_t *set_password_screen = NULL;               // 设置密码界面的屏幕对象
static lv_obj_t *new_password_label = NULL;                // 显示新密码输入状态的标签
static lv_obj_t *confirm_password_label = NULL;            // 显示确认密码输入状态的标签
static int password_step = 0;                              // 密码设置步骤：0=输入新密码，1=确认密码
static lv_obj_t *unlock_cover = NULL;                      // 解锁界面半透明覆盖层
static lv_obj_t *setpw_cover = NULL;                       // 设置密码界面半透明覆盖层

// =========================== 密码文件操作函数 ===========================

/**
 * @brief 从文件加载密码
 * 
 * 功能说明：
 * 1. 尝试打开密码文件进行读取
 * 2. 如果文件存在，读取密码并移除换行符
 * 3. 如果文件不存在，使用默认密码"1234"并保存到文件
 * 4. 确保即使重启开发板，密码也能被保存
 * 
 * @param password 用于存储读取到的密码的字符数组
 */
void load_password(char *password) {
    FILE *file = fopen(PASSWORD_FILE, "r");    // 以只读模式打开密码文件
    if (file != NULL) {
        // 文件存在，读取密码
        if (fgets(password, PASSWORD_LENGTH + 1, file) != NULL) {
            // 移除字符串末尾的换行符（fgets会包含换行符）
            password[strcspn(password, "\n")] = 0;
        }
        fclose(file);   // 关闭文件
    } else {
        // 文件不存在，使用默认密码并保存
        strcpy(password, "1234");       // 设置默认密码
        save_password(password);        // 将默认密码保存到文件
    }
}

/**
 * @brief 保存密码到文件
 * 
 * 功能说明：
 * 1. 以写入模式打开密码文件
 * 2. 将密码写入文件
 * 3. 确保密码持久化保存，重启后不会丢失
 * 
 * @param password 要保存的密码字符串
 */
void save_password(const char *password) {
    FILE *file = fopen(PASSWORD_FILE, "w");    // 以写入模式打开文件
    if (file != NULL) {
        fprintf(file, "%s", password);          // 将密码写入文件
        fclose(file);                           // 关闭文件
    }
}

/**
 * @brief 验证输入的密码是否正确
 * 
 * 功能说明：
 * 比较输入密码与存储密码是否相同
 * 
 * @param input 用户输入的密码
 * @return 返回1表示密码正确，返回0表示密码错误
 */
int verify_password(const char *input) {
    return strcmp(input, stored_password) == 0; // 比较输入密码与存储密码是否相同
}

// =========================== 解锁界面创建函数 ===========================

/**
 * @brief 创建并显示解锁界面
 * 
 * 功能说明：
 * 1. 加载保存的密码
 * 2. 清空输入密码缓存
 * 3. 创建模糊背景覆盖整个屏幕
 * 4. 创建密码输入容器，包含：
 *    - 标题文字
 *    - 密码显示区域（用*号显示）
 *    - 数字键盘（0-9）
 *    - 取消按钮（返回锁屏）
 *    - 忘记密码按钮（设置新密码）
 */
void UnlockScreen(void) {
    // 息屏/桌面资源彻底释放，防止界面叠加和指针悬挂
    extern void release_desktop_resources(void);
    release_desktop_resources();
    extern lv_obj_t *black_screen_mask;
    if (black_screen_mask) {
        lv_obj_del(black_screen_mask);
        black_screen_mask = NULL;
    }

    // 从文件加载之前保存的密码
    load_password(stored_password);
    
    // 清空输入密码缓存，准备接收新的输入
    memset(input_password, 0, sizeof(input_password));
    
    // ========== 创建解锁界面半透明覆盖层 ========== 
    if(unlock_cover) {
        lv_obj_del(unlock_cover);
        unlock_cover = NULL;
    }
    unlock_cover = lv_obj_create(lv_screen_active());
    lv_obj_set_size(unlock_cover, 800, 480);
    lv_obj_set_pos(unlock_cover, 0, 0);
    lv_obj_set_style_bg_color(unlock_cover, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(unlock_cover, LV_OPA_80, LV_PART_MAIN); // 更强模糊
    lv_obj_set_style_border_width(unlock_cover, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(unlock_cover, 0, LV_PART_MAIN);
    lv_obj_clear_flag(unlock_cover, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(unlock_cover, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(unlock_cover, LV_OBJ_FLAG_IGNORE_LAYOUT);
    // ========== 创建解锁界面主容器 ========== 
    unlock_screen = lv_obj_create(lv_screen_active());
    lv_obj_set_size(unlock_screen, 800, 480);
    lv_obj_set_pos(unlock_screen, 0, 0);
    // 只保留按键边框，主容器无边框、全透明
    lv_obj_set_style_bg_opa(unlock_screen, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(unlock_screen, 0, LV_PART_MAIN);
    
    // ========== 创建密码输入容器 ========== 
    lv_obj_t *password_container = lv_obj_create(unlock_screen);
    lv_obj_set_size(password_container, 500, 430);
    lv_obj_center(password_container);
    // 容器全透明且无边框
    lv_obj_set_style_bg_opa(password_container, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(password_container, 0, LV_PART_MAIN);
    
    // ========== 删除标题 ========== 
    // 不创建标题label
    
    // ========== 创建密码显示区域 ========== 
    password_label = lv_label_create(password_container);
    lv_label_set_text(password_label, "________");             // 初始显示8个下划线
    lv_obj_set_style_text_color(password_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN); // 白色文字
    lv_obj_set_style_text_font(password_label, &lv_font_montserrat_32, LV_PART_MAIN);  // 32号字体
    lv_obj_set_pos(password_label, 160, 10); // 右移一点，避免太靠左
    // 背景透明（label本身无背景，容器已透明）

    
    // ========== 创建数字键盘容器 ========== 
    lv_obj_t *keypad = lv_obj_create(password_container);
    lv_obj_set_size(keypad, 350, 250);
    lv_obj_set_pos(keypad, 85, 80);
    lv_obj_set_style_bg_opa(keypad, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(keypad, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(keypad, 5, LV_PART_MAIN);
    
    // ========== 创建圆形数字按钮1-9（3x3布局）==========
    int btn_size = 50;
    int btn_h_space = 70; // 水平间距
    int btn_v_space = 60; // 垂直间距
    int keypad_offset_x = 40; // 使整体居中
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
    // ========== 创建圆形数字0按钮 ========== 
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
    
    // ========== 创建取消按钮 ========== 
    lv_obj_t *cancel_btn = lv_button_create(password_container);
    lv_obj_set_size(cancel_btn, 120, 45);
    lv_obj_set_pos(cancel_btn, 90, 330);
    lv_obj_set_style_bg_opa(cancel_btn, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(cancel_btn, 1, LV_PART_MAIN);
    lv_obj_set_style_border_color(cancel_btn, lv_color_hex(0xf39c12), LV_PART_MAIN); // 金色
    lv_obj_set_style_border_opa(cancel_btn, LV_OPA_80, LV_PART_MAIN);
    lv_obj_set_style_radius(cancel_btn, 15, LV_PART_MAIN);
    lv_obj_t *cancel_label = lv_label_create(cancel_btn);
    lv_label_set_text(cancel_label, "Cancel");
    lv_obj_center(cancel_label);
    lv_obj_set_style_text_color(cancel_label, lv_color_hex(0xf39c12), LV_PART_MAIN); // 金色
    lv_obj_add_event_cb(cancel_btn, cancel_btn_event_cb, LV_EVENT_CLICKED, NULL);
    // ========== 创建忘记密码按钮 ========== 
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

// =========================== 事件处理函数 ===========================

/**
 * @brief 解锁界面数字键盘事件处理函数
 * 
 * 功能说明：
 * 1. 获取用户点击的数字（0-9）
 * 2. 更新密码输入缓存
 * 3. 更新界面显示（用*号显示已输入的位数）
 * 4. 当输入4位密码后自动验证
 * 5. 密码正确则关闭解锁界面，错误则显示提示并清空输入
 * 
 * @param e 事件对象，包含事件相关信息
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
 * @brief 取消按钮事件处理函数
 * 
 * 功能说明：
 * 关闭解锁界面，返回锁屏状态
 * 
 * @param e 事件对象，包含事件相关信息
 */
void cancel_btn_event_cb(lv_event_t * e) {
    if (unlock_screen != NULL) {
        lv_obj_del(unlock_screen);                  // 删除解锁界面
        unlock_screen = NULL;                       // 清空指针
    }
    if (unlock_cover != NULL) {
        lv_obj_del(unlock_cover);
        unlock_cover = NULL;
    }
}

/**
 * @brief 忘记密码按钮事件处理函数
 * @param e 事件对象
 * 
 * 功能说明：
 * 1. 关闭当前解锁界面
 * 2. 打开设置新密码界面
 */
void forgot_btn_event_cb(lv_event_t * e) {
    // 删除解锁界面
    if (unlock_screen != NULL) {
        lv_obj_del(unlock_screen);
        unlock_screen = NULL;
    }
    if (unlock_cover != NULL) {
        lv_obj_del(unlock_cover);
        unlock_cover = NULL;
    }
    // 打开设置密码界面
    SetPasswordScreen();
}

// =========================== 设置密码界面 ===========================

/**
 * @brief 创建设置新密码界面
 * 
 * 功能说明：
 * 1. 创建设置新密码的界面
 * 2. 界面布局与解锁界面类似，但功能不同
 * 3. 用户可以输入新的4位数字密码
 * 4. 点击确认后会保存新密码到文件
 * 5. 新密码会立即生效，并在重启后保持
 */
void SetPasswordScreen(void) {
    // 清空新密码输入缓存
    memset(new_password, 0, sizeof(new_password));
    memset(confirm_password, 0, sizeof(confirm_password));
    password_step = 0;  // 重置步骤为输入新密码
    
    // ========== 创建设置密码界面模糊层 ========== 
    if(setpw_cover) {
        lv_obj_del(setpw_cover);
        setpw_cover = NULL;
    }
    setpw_cover = lv_obj_create(lv_screen_active());
    lv_obj_set_size(setpw_cover, 800, 480);
    lv_obj_set_pos(setpw_cover, 0, 0);
    lv_obj_set_style_bg_color(setpw_cover, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(setpw_cover, LV_OPA_80, LV_PART_MAIN); // 黑色，80%不透明
    lv_obj_set_style_border_width(setpw_cover, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(setpw_cover, 0, LV_PART_MAIN);
    lv_obj_clear_flag(setpw_cover, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(setpw_cover, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(setpw_cover, LV_OBJ_FLAG_IGNORE_LAYOUT);
    // 不加渐变色
    // 放到最上层，覆盖所有内容
    lv_obj_move_foreground(setpw_cover);
    // ========== 创建设置密码界面主容器 ===========
    set_password_screen = lv_obj_create(lv_screen_active());
    lv_obj_set_size(set_password_screen, 800, 480);
    lv_obj_set_pos(set_password_screen, 0, 0);
    // 主容器全透明
    lv_obj_set_style_bg_opa(set_password_screen, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(set_password_screen, 0, LV_PART_MAIN);
    lv_obj_add_flag(set_password_screen, LV_OBJ_FLAG_GESTURE_BUBBLE); // 主容器允许手势冒泡
    // 保证主容器在覆盖层之上
    lv_obj_move_foreground(set_password_screen);
    
    // ========== 创建密码设置容器 ========== 
    lv_obj_t *password_container = lv_obj_create(set_password_screen);
    lv_obj_set_size(password_container, 500, 430);
    lv_obj_center(password_container);
    // 密码容器全透明
    lv_obj_set_style_bg_opa(password_container, LV_OPA_TRANSP, LV_PART_MAIN);
    // 密码容器不设置透明度，只去掉边框
    lv_obj_set_style_border_width(password_container, 0, LV_PART_MAIN);
    lv_obj_add_flag(password_container, LV_OBJ_FLAG_GESTURE_BUBBLE); // 容器允许手势冒泡
    
    // ========== 创建标题 ==========
    lv_obj_t *title = lv_label_create(password_container);
    lv_label_set_text(title, "Enter New Password");          // 设置新密码标题
    lv_obj_set_style_text_color(title, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_24, LV_PART_MAIN);
    lv_obj_set_pos(title, 100, 0);                          // 调整标题位置
    
    // ========== 创建新密码显示区域 ==========
    new_password_label = lv_label_create(password_container);
    lv_label_set_text(new_password_label, "________");         // 初始显示8个下划线
    lv_obj_set_style_text_color(new_password_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_font(new_password_label, &lv_font_montserrat_32, LV_PART_MAIN);
    lv_obj_set_pos(new_password_label, 155, 30); // 右移并下移，更居中
    
    // ========== 创建确认密码显示区域 ==========
    confirm_password_label = lv_label_create(password_container);
    lv_label_set_text(confirm_password_label, "________");     // 初始显示8个下划线
    lv_obj_set_style_text_color(confirm_password_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_font(confirm_password_label, &lv_font_montserrat_32, LV_PART_MAIN);
    lv_obj_set_pos(confirm_password_label, 155, 55); // 右移并下移，与新密码label对齐
    lv_obj_add_flag(confirm_password_label, LV_OBJ_FLAG_HIDDEN); // 初始隐藏
    
    // ========== 创建数字键盘 ========== 
    lv_obj_t *keypad = lv_obj_create(password_container);
    lv_obj_set_size(keypad, 350, 240);
    lv_obj_set_pos(keypad, 85, 95);
    lv_obj_set_style_bg_opa(keypad, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(keypad, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(keypad, 5, LV_PART_MAIN);
    // ========== 创建圆形数字按钮1-9 ========== 
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
    // ========== 创建Back按钮 ========== 
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
    // ========== 创建圆形数字0按钮 ========== 
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
    // ========== 创建圆形删除按钮 ========== 
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
    
    // ========== 创建确认设置按钮 ========== 
    lv_obj_t *confirm_btn = lv_button_create(password_container);
    lv_obj_set_size(confirm_btn, 200, 45);
    lv_obj_set_pos(confirm_btn, 137, 340);
    lv_obj_set_style_bg_opa(confirm_btn, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(confirm_btn, 1, LV_PART_MAIN);
    lv_obj_set_style_border_color(confirm_btn, lv_color_hex(0xf39c12), LV_PART_MAIN); // 金色
    lv_obj_set_style_border_opa(confirm_btn, LV_OPA_80, LV_PART_MAIN);
    lv_obj_t *confirm_label = lv_label_create(confirm_btn);
    lv_label_set_text(confirm_label, "Save");
    lv_obj_center(confirm_label);
    lv_obj_set_style_text_color(confirm_label, lv_color_hex(0xf39c12), LV_PART_MAIN); // 金色
    lv_obj_add_event_cb(confirm_btn, confirm_set_password_btn_event_cb, LV_EVENT_CLICKED, NULL);
    // 不再添加右滑手势返回

// ========== Back按钮事件处理 ==========
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
 * @brief 设置密码界面的数字键盘事件处理函数
 * @param e 事件对象
 * 
 * 功能说明：
 * 1. 处理用户在设置密码界面的数字输入
 * 2. 更新新密码的输入缓存
 * 3. 更新界面显示
 * 4. 与解锁界面不同，这里不会自动验证，需要用户点击确认按钮
 */
void set_password_keypad_event_cb(lv_event_t * e) {
    int num = (int)(intptr_t)lv_event_get_user_data(e);    // 获取用户点击的数字
    
    if (password_step == 0) {
        // 第一步：输入新密码
        int len = strlen(new_password);
        
        if (num == -1) { 
            // 删除操作
            if (len > 0) {
                new_password[len - 1] = '\0';
            }
        } else if (len < PASSWORD_LENGTH) { 
            // 输入数字
            new_password[len] = '0' + num;
            new_password[len + 1] = '\0';
        }
        
        // 更新新密码显示
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
        
        // 检查是否输入完成，进入确认步骤
        if (strlen(new_password) == PASSWORD_LENGTH) {
            password_step = 1;
            // 显示确认密码区域
            lv_obj_clear_flag(confirm_password_label, LV_OBJ_FLAG_HIDDEN);
            // 更新标题
            lv_obj_t *title = lv_obj_get_child(lv_obj_get_parent(new_password_label), 0);
            lv_label_set_text(title, "Confirm Password");
            lv_obj_set_pos(title, 125, 0);
        }
    } else {
        // 第二步：确认密码
        int len = strlen(confirm_password);
        
        if (num == -1) { 
            // 删除操作
            if (len > 0) {
                confirm_password[len - 1] = '\0';
            }
        } else if (len < PASSWORD_LENGTH) { 
            // 输入数字
            confirm_password[len] = '0' + num;
            confirm_password[len + 1] = '\0';
        }
        
        // 更新确认密码显示
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
 * @brief 确认设置密码按钮事件处理函数
 * @param e 事件对象
 * 
 * 功能说明：
 * 1. 检查新密码是否输入完整（4位）
 * 2. 如果完整，则保存新密码到全局变量和文件
 * 3. 关闭设置密码界面
 * 4. 新密码立即生效，重启后也会保持
 */
void confirm_set_password_btn_event_cb(lv_event_t * e) {
    if (password_step == 0) {
        // 还在输入新密码阶段，不允许确认
        return;
    }
    
    if (strlen(confirm_password) == PASSWORD_LENGTH) {
        // 检查两次密码是否一致
        if (strcmp(new_password, confirm_password) == 0) {
            // 密码一致，保存密码
            strcpy(stored_password, new_password);             // 更新全局密码变量
            save_password(stored_password);                    // 保存到文件，确保重启后不丢失
            // 关闭设置密码界面
            if (set_password_screen != NULL) {
                lv_obj_del(set_password_screen);
                set_password_screen = NULL;
            }
            if (setpw_cover != NULL) {
                lv_obj_del(setpw_cover);
                setpw_cover = NULL;
            }
            // 新密码已保存
        } else {
            // 密码不一致，重新开始
            memset(new_password, 0, sizeof(new_password));
            memset(confirm_password, 0, sizeof(confirm_password));
            password_step = 0;
            // 重置显示
            lv_label_set_text(new_password_label, "________");
            lv_label_set_text(confirm_password_label, "________");
            lv_obj_add_flag(confirm_password_label, LV_OBJ_FLAG_HIDDEN);
            // 更新标题
            lv_obj_t *title = lv_obj_get_child(lv_obj_get_parent(new_password_label), 0);
            lv_label_set_text(title, "Passwords don't match! Try again");
            lv_obj_set_pos(title, 30, 0);                  // 再往左移动，从50改为30
            // 密码不匹配，用户需要重新输入
        }
    }
    // 如果确认密码不足4位，不执行任何操作，用户需要继续输入
}

// 设置密码界面右滑返回解锁界面
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
        Screen(); // 回到锁屏主界面
    }
}
