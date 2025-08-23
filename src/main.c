#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "../lvgl/lvgl.h"
#include "../lvgl/demos/lv_demos.h"
#include "../my_font_decl.h"

#include "../inc/start_video.h"
#include "../inc/main.h"
#include "../inc/screen.h"
#include "../inc/main_screen.h"

//测试video所用，记得删除------------------------------------------

#include "../inc/video.h"

//--------------------------------------------------


#define BEEP_LED_ON 1
#define BEEP_LED_OFF 0


extern const lv_font_t lv_myself_font;
/**
 * @brief 主函数
 * 
 * 功能说明：
 * 1. 初始化LVGL图形库
 * 2. 注册显示设备和输入设备
 * 3. 提供调试选项
 * 4. 进入主循环处理LVGL任务
 * 
 * @return 返回0表示程序正常退出
 */
int main(void)
{
    //初始化lvgl的内核文件和配置文件
    lv_init();

    /*Linux frame buffer device init*/
    //创建屏幕(frame buffer)，用disp进行接收
    lv_display_t * disp = lv_linux_fbdev_create();

    //设置屏幕对应的设备文件，并与disp进行绑定映射
    lv_linux_fbdev_set_file(disp, "/dev/fb0");
    
    //触摸屏的初始化
    //创建一个输入事件，与输入设备关联
    lv_indev_t * indev = lv_evdev_create(LV_INDEV_TYPE_POINTER, "/dev/input/event0");







    // /*Create a Demo*/

    // // 播放开机动画视频
    // lv_start_video();
    
    // // 创建定时器，12秒后切换到主界面
    // lv_timer_t * boot_timer = lv_timer_create(boot_animation_timer_cb, 12000, NULL);
    // lv_timer_set_repeat_count(boot_timer, 1);  // 只执行一次
    video_test();
    






    

    
    /*Handle LVGL tasks*/
    while(1) {
        lv_timer_handler();
        usleep(5000);
    }

    return 0;
}


/**
 * @brief 开机动画定时器回调函数
 * 
 * 功能说明：
 * 1. 打印切换到主界面的信息
 * 2. 停止视频播放
 * 3. 清除屏幕
 * 4. 显示主界面
 * 5. 删除定时器
 * 
 * @param timer 定时器对象
 */
void boot_animation_timer_cb(lv_timer_t * timer)
{
    // 开机动画时间到，切换到主界面
    
    // 停止视频播放
    lv_stop_video();
    
    // 清除屏幕
    lv_obj_clean(lv_screen_active());
    
    // 显示主界面
    Screen();
    
    // 删除定时器（只执行一次）
    lv_timer_del(timer);
}



