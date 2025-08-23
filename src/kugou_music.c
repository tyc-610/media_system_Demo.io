#include "../inc/kugou_music.h"
#include "../inc/main_screen.h"

// ============ 全局变量定义 ============// 全局音乐播放器实例，存储所有音乐播放相关的状态和数据
music_player_t g_music_player = {0};

// ============ 界面对象指针 ============

/**
 * @brief 启动音乐应用
 * 
 * 功能说明：
 * 1. 初始化音乐播放器
 * 2. 检查音乐文件数量
 * 3. 如果没有音乐文件，显示提示信息
 * 4. 否则显示音乐列表界面
 */
void start_music_app(void)
{
    // 初始化音乐播放器
    init_music_player();
    
    // 检查音乐文件数量
    if (g_music_player.music_count == 0) {
        lv_obj_t * msg_box = lv_msgbox_create(lv_screen_active());
        lv_msgbox_add_title(msg_box, "KuGou Music");
        lv_msgbox_add_text(msg_box, "No music files found in /home/tyc/work_station/music/");
        lv_msgbox_add_close_button(msg_box);
        return;
    }
    
    // 显示音乐列表界面
    show_music_list_screen();
}
// 这些静态变量用于存储LVGL界面对象的指针，方便在不同函数间访问和更新界面
static lv_obj_t* music_list_screen = NULL;    // 音乐列表界面的根对象
static lv_obj_t* music_player_screen = NULL;  // 音乐播放界面的根对象
static lv_obj_t* progress_bar = NULL;         // 进度条控件
static lv_obj_t* progress_label = NULL;       // 时间显示标签
static lv_obj_t* song_title_label = NULL;     // 歌曲标题标签
static lv_obj_t* song_artist_label = NULL;    // 艺术家标签
static lv_obj_t* play_pause_btn = NULL;       // 播放/暂停按钮
static lv_obj_t* mode_btn = NULL;             // 播放模式按钮
static lv_timer_t* progress_timer = NULL;     // 进度更新定时器
static int current_progress = 0;              // 当前歌曲进度（秒）

// 手势检测变量
static bool touch_started = false;
static lv_point_t touch_start_point = {0, 0};
static uint32_t touch_start_time = 0;

// 函数声明
void release_music_resources(void);

/**
 * @brief 返回按钮事件处理函数
 * 
 * 功能说明：
 * 1. 释放音乐应用资源
 * 2. 创建桌面界面
 * 
 * @param e 事件对象
 */
void back_btn_event_cb(lv_event_t * e)
{
    // 释放音乐应用资源
    release_music_resources();
    
    // 创建桌面界面
    desktop();
}

/**
 * @brief 释放音乐应用资源
 * 
 * 功能说明：
 * 1. 停止进度定时器
 * 2. 清理音乐播放相关对象
 * 3. 重置全局变量
 */
void release_music_resources(void)
{
    // 停止进度定时器
    if (progress_timer != NULL) {
        lv_timer_del(progress_timer);
        progress_timer = NULL;
    }
    
    // 清理音乐播放相关对象
    if (music_list_screen != NULL) {
        lv_obj_del(music_list_screen);
        music_list_screen = NULL;
    }
    
    if (music_player_screen != NULL) {
        lv_obj_del(music_player_screen);
        music_player_screen = NULL;
    }
    
    // 重置其他界面对象指针
    progress_bar = NULL;
    progress_label = NULL;
    song_title_label = NULL;
    song_artist_label = NULL;
    play_pause_btn = NULL;
    mode_btn = NULL;
    
    // 重置进度
    current_progress = 0;
}

/**
 * @brief 初始化音乐播放器
 * 
 * 功能说明：
 * 1. 清零音乐播放器结构体，确保初始状态干净
 * 2. 设置默认播放模式为顺序播放
 * 3. 设置初始播放状态为停止
 * 4. 初始化当前播放索引为0
 * 5. 初始化播放器进程ID为-1（表示没有播放进程）
 * 6. 扫描指定目录下的音乐文件
 */
void init_music_player(void)
{
    // 使用memset清零整个结构体，确保所有成员都是初始值
    memset(&g_music_player, 0, sizeof(music_player_t));
    
    // 设置播放器的基本参数
    g_music_player.play_mode = PLAY_MODE_SEQUENCE;  // 默认顺序播放
    g_music_player.state = MUSIC_STATE_STOP;        // 初始状态为停止
    g_music_player.current_index = 0;               // 当前播放的音乐索引
    g_music_player.player_pid = -1;                 // 播放器进程ID，-1表示无进程
    
    // 扫描音乐文件：从指定目录加载所有MP3文件到播放列表
    scan_music_files("/home/tyc/work_station/music");
}

/**
 * @brief 扫描音乐文件
 * 
 * 功能说明：
 * 1. 打开指定目录
 * 2. 遍历目录中的所有文件
 * 3. 筛选出MP3格式的音乐文件
 * 4. 将音乐文件信息添加到播放列表中
 * 5. 解析每个音乐文件的基本信息（标题、艺术家等）
 * 
 * @param directory 要扫描的目录路径
 */
void scan_music_files(const char* directory)
{
    DIR *dir;                    // 目录句柄
    struct dirent *entry;        // 目录项结构体，包含文件信息
    char full_path[MAX_PATH_LEN]; // 存储完整文件路径的缓冲区
    
    // 尝试打开指定目录
    dir = opendir(directory);
    if (dir == NULL) {
        // 如果目录打开失败，返回
        return;
    }
    
    // 初始化音乐文件计数器
    g_music_player.music_count = 0;
    
    // 遍历目录中的所有文件，直到没有更多文件或达到最大文件数限制
    while ((entry = readdir(dir)) != NULL && g_music_player.music_count < MAX_MUSIC_COUNT) {
        // 跳过隐藏文件和目录（以'.'开头的文件）
        if (entry->d_name[0] == '.') continue;
        
        // 检查文件扩展名，只处理MP3文件
        char *ext = strrchr(entry->d_name, '.'); // 查找文件名中最后一个'.'
        if (ext && strcasecmp(ext, ".mp3") == 0) { // 不区分大小写比较扩展名
            
            // 获取当前音乐文件信息结构体的指针
            music_info_t *info = &g_music_player.music_list[g_music_player.music_count];
            
            // 复制文件名到结构体中，确保字符串结束符
            strncpy(info->filename, entry->d_name, MAX_NAME_LEN - 1);
            info->filename[MAX_NAME_LEN - 1] = '\0';
            
            // 构建完整的文件路径：目录路径 + "/" + 文件名
            snprintf(full_path, sizeof(full_path), "%s/%s", directory, entry->d_name);
            strncpy(info->filepath, full_path, MAX_PATH_LEN - 1);
            info->filepath[MAX_PATH_LEN - 1] = '\0';
            
            // 解析音乐信息：从文件名提取标题、设置默认艺术家等
            parse_music_info(info);
            
            // 音乐文件计数增加
            g_music_player.music_count++;
        }
    }
    
    // 关闭目录句柄，释放系统资源
    closedir(dir);
}

/**
 * @brief 解析音乐信息
 * 
 * 功能说明：
 * 1. 从文件名中提取歌曲标题（去掉.mp3扩展名）
 * 2. 设置默认的艺术家和专辑信息
 * 
 * 注意：这是简化版本的解析函数，实际应用中可以使用ID3标签解析库
 * 来读取MP3文件内嵌的元数据信息
 * 
 * @param info 指向音乐信息结构体的指针
 */
void parse_music_info(music_info_t* info)
{
    // 复制文件名到临时变量，因为需要修改（去掉扩展名）
    char *name_copy = strdup(info->filename); // strdup分配内存并复制字符串
    
    // 查找文件名中最后一个'.'的位置（扩展名分隔符）
    char *dot = strrchr(name_copy, '.');
    if (dot) {
        *dot = '\0';  // 将'.'替换为字符串结束符，去掉扩展名
    }
    
    // 将处理后的文件名（不含扩展名）作为歌曲标题
    strncpy(info->title, name_copy, MAX_NAME_LEN - 1);
    info->title[MAX_NAME_LEN - 1] = '\0'; // 确保字符串以\0结尾
    
    // 设置默认值：由于这是简化版本，没有解析MP3标签，所以使用默认值
    strcpy(info->artist, "Unknown Artist"); // 默认艺术家
    strcpy(info->album, "Unknown Album");   // 默认专辑
    
    // 释放临时分配的内存，避免内存泄露
    free(name_copy);
}

/**
 * @brief 显示音乐列表界面
 * 
 * 功能说明：
 * 1. 清理当前屏幕
 * 2. 创建音乐列表界面
 * 3. 设置背景图片
 * 4. 创建返回按钮
 * 5. 创建音乐列表
 * 6. 如果没有音乐文件，显示提示信息
 * 7. 否则添加音乐列表项
 * 8. 添加手势事件处理
 */

void show_music_list_screen(void)
{
    // 停止之前的定时器
    if (progress_timer) {
        lv_timer_del(progress_timer);
        progress_timer = NULL;
    }

    // 清理当前屏幕
    lv_obj_clean(lv_screen_active());

    // 重置对象指针
    music_player_screen = NULL;
    progress_bar = NULL;
    progress_label = NULL;
    song_title_label = NULL;
    song_artist_label = NULL;
    play_pause_btn = NULL;
    mode_btn = NULL;

    // 创建音乐列表界面
    music_list_screen = lv_obj_create(lv_screen_active());
    lv_obj_set_size(music_list_screen, 800, 480);
    lv_obj_set_pos(music_list_screen, 0, 0);
    lv_obj_set_style_pad_all(music_list_screen, 0, LV_PART_MAIN);
    lv_obj_set_style_border_width(music_list_screen, 0, LV_PART_MAIN);

    // 设置背景图片
    lv_obj_t * bg_img = lv_image_create(music_list_screen);
    lv_obj_set_size(bg_img, 800, 480);
    lv_obj_set_pos(bg_img, 0, 0);
    lv_image_set_src(bg_img, "/home/tyc/work_station/music/music_choseback.jpg");

    // 添加滑动退出手势 - 确保正确绑定
    lv_obj_add_event_cb(music_list_screen, music_list_gesture_cb, LV_EVENT_GESTURE, NULL);
    lv_obj_add_flag(music_list_screen, LV_OBJ_FLAG_GESTURE_BUBBLE);

    // 为背景图片也添加手势事件，确保手势被捕获
    lv_obj_add_event_cb(bg_img, music_list_gesture_cb, LV_EVENT_GESTURE, NULL);
    lv_obj_add_flag(bg_img, LV_OBJ_FLAG_GESTURE_BUBBLE);
    
    // 添加手动手势检测 - 监听按下和释放事件
    lv_obj_add_event_cb(music_list_screen, music_list_touch_cb, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(music_list_screen, music_list_touch_cb, LV_EVENT_RELEASED, NULL);
    lv_obj_add_event_cb(bg_img, music_list_touch_cb, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(bg_img, music_list_touch_cb, LV_EVENT_RELEASED, NULL);
    
    // 创建音乐列表容器
    lv_obj_t * list_container = lv_obj_create(music_list_screen);
    lv_obj_set_size(list_container, 760, 460);
    lv_obj_set_pos(list_container, 20, 10);
    lv_obj_set_style_bg_opa(list_container, LV_OPA_80, LV_PART_MAIN);
    lv_obj_set_style_bg_color(list_container, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_border_width(list_container, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(list_container, 10, LV_PART_MAIN);
    lv_obj_set_style_pad_all(list_container, 10, LV_PART_MAIN);

    // // 创建背景图并放到 list_container 最底层，设置为不可滑动、不可点击
    // lv_obj_t * list_background = lv_img_create(list_container);
    // lv_obj_set_size(list_background, 740, 440);
    // lv_obj_set_pos(list_background, 0, 0);
    // lv_image_set_src(list_background, "/home/tyc/work_station/music_choseback.jpg");
    // lv_obj_move_background(list_background);
    // lv_obj_clear_flag(list_background, LV_OBJ_FLAG_SCROLLABLE); // 禁止滑动
    // lv_obj_clear_flag(list_background, LV_OBJ_FLAG_CLICKABLE); // 禁止点击
    // lv_obj_clear_flag(list_background, LV_OBJ_FLAG_GESTURE_BUBBLE); // 禁止手势冒泡
    
    // 为列表容器也添加手势检测，确保手势能够传递
    lv_obj_add_event_cb(list_container, music_list_touch_cb, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(list_container, music_list_touch_cb, LV_EVENT_RELEASED, NULL);
    lv_obj_add_flag(list_container, LV_OBJ_FLAG_GESTURE_BUBBLE);  // 确保事件能够冒泡
    
    // 创建滚动列表
    lv_obj_t * list = lv_list_create(list_container);
    lv_obj_set_size(list, 740, 440);
    lv_obj_set_pos(list, 0, 0);
    lv_obj_set_style_bg_opa(list, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(list, 0, LV_PART_MAIN);
    
    // 为列表添加手势检测，确保手势能够传递
    lv_obj_add_event_cb(list, music_list_touch_cb, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(list, music_list_touch_cb, LV_EVENT_RELEASED, NULL);
    lv_obj_add_flag(list, LV_OBJ_FLAG_GESTURE_BUBBLE);  // 确保事件能够冒泡
    
    // 如果没有音乐文件，显示提示信息
    if (g_music_player.music_count == 0) {
        lv_obj_t * no_music_label = lv_label_create(list);
        lv_label_set_text(no_music_label, "No music files found");
        lv_obj_set_style_text_color(no_music_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
        lv_obj_set_style_text_font(no_music_label, &lv_font_montserrat_16, LV_PART_MAIN);
        lv_obj_center(no_music_label);
        return;
    }
    
    // 添加音乐列表项
    for (int i = 0; i < g_music_player.music_count; i++) {
        music_info_t *info = &g_music_player.music_list[i];
        
        // 只显示歌曲标题（去掉扩展名的文件名）
        lv_obj_t * btn = lv_list_add_button(list, LV_SYMBOL_AUDIO, info->title);
        lv_obj_set_style_text_color(btn, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
        lv_obj_set_style_bg_color(btn, lv_color_hex(0x34495e), LV_PART_MAIN);
        lv_obj_set_style_bg_opa(btn, LV_OPA_70, LV_PART_MAIN);
        lv_obj_set_style_text_font(btn, &lv_font_montserrat_14, LV_PART_MAIN);
        
        // 将索引作为用户数据传递
        lv_obj_set_user_data(btn, (void*)(uintptr_t)i);
        lv_obj_add_event_cb(btn, music_list_item_cb, LV_EVENT_CLICKED, NULL);
    }
}

/**
 * @brief 显示音乐播放界面
 * 
 * 功能说明：
 * 1. 停止之前的定时器
 * 2. 清理当前屏幕
 * 3. 重置对象指针
 * 4. 创建播放器界面
 * 5. 设置背景图片
 * 6. 创建歌曲信息区域
 * 7. 创建控制按钮（上一首、播放/暂停、下一首等）
 * 8. 创建播放模式按钮
 * 9. 创建返回按钮
 * 10. 创建进度条区域
 * 11. 更新界面信息
 * 12. 启动进度更新定时器
 */
void show_music_player_screen(void)
{
    // 停止之前的定时器
    if (progress_timer) {
        lv_timer_del(progress_timer);
        progress_timer = NULL;
    }
    
    // 清理当前屏幕
    lv_obj_clean(lv_screen_active());
    
    // 重置对象指针
    music_list_screen = NULL;
    
    // 创建播放器界面
    music_player_screen = lv_obj_create(lv_screen_active());
    lv_obj_set_size(music_player_screen, 800, 480);
    lv_obj_set_pos(music_player_screen, 0, 0);
    lv_obj_set_style_pad_all(music_player_screen, 0, LV_PART_MAIN);
    lv_obj_set_style_border_width(music_player_screen, 0, LV_PART_MAIN);
    
    // 设置背景图片
    lv_obj_t * bg_img = lv_image_create(music_player_screen);
    lv_obj_set_size(bg_img, 800, 480);
    lv_obj_set_pos(bg_img, 0, 0);
    lv_image_set_src(bg_img, "/home/tyc/work_station/music/kugou_back.jpg");
    
    // 歌曲信息区域 - 美化设计
    lv_obj_t * info_area = lv_obj_create(music_player_screen);
    lv_obj_set_size(info_area, 480, 60);  // 增大尺寸
    lv_obj_set_pos(info_area, 160, 25);   // 调整位置使其更居中
    lv_obj_set_style_bg_opa(info_area, LV_OPA_60, LV_PART_MAIN);  // 半透明背景
    lv_obj_set_style_bg_color(info_area, lv_color_hex(0x000000), LV_PART_MAIN);  // 黑色背景
    lv_obj_set_style_border_width(info_area, 2, LV_PART_MAIN);   // 增加边框宽度
    lv_obj_set_style_border_color(info_area, lv_color_hex(0xFFD700), LV_PART_MAIN);  // 金色边框
    lv_obj_set_style_border_opa(info_area, LV_OPA_80, LV_PART_MAIN);  // 边框更明显
    lv_obj_set_style_radius(info_area, 25, LV_PART_MAIN);  // 更大的圆角
    lv_obj_set_style_pad_all(info_area, 20, LV_PART_MAIN);  // 更大的内边距
    
    // 添加渐变效果
    lv_obj_set_style_bg_grad_color(info_area, lv_color_hex(0x333333), LV_PART_MAIN);
    lv_obj_set_style_bg_grad_dir(info_area, LV_GRAD_DIR_VER, LV_PART_MAIN);

    // 歌曲标题 - 美化文字显示
    song_title_label = lv_label_create(info_area);
    lv_obj_set_style_text_color(song_title_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);  // 白色文字
    lv_obj_set_style_text_font(song_title_label, &lv_font_montserrat_20, LV_PART_MAIN);  // 更大字体
    lv_obj_set_style_text_opa(song_title_label, LV_OPA_100, LV_PART_MAIN);  // 完全不透明
    lv_obj_set_style_text_align(song_title_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);  // 居中对齐
    // ...已移除文字阴影效果，兼容低版本LVGL...
    lv_obj_align(song_title_label, LV_ALIGN_CENTER, 0, 0);  // 完全居中
    
    // 直接在界面上放置控制按钮 - 优化布局
    // 上一首按钮
    lv_obj_t * prev_btn = lv_button_create(music_player_screen);
    lv_obj_set_size(prev_btn, 90, 90);//大小
    lv_obj_set_pos(prev_btn, 90, 285);//位置
    lv_obj_set_style_bg_color(prev_btn, lv_color_hex(0x3498db), LV_PART_MAIN);
    lv_obj_set_style_radius(prev_btn, 45, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(prev_btn, LV_OPA_0, LV_PART_MAIN);  // 文字完全不透明
    lv_obj_set_style_border_opa(prev_btn, LV_OPA_0, LV_PART_ANY);
    
    
    
    lv_obj_t * prev_label = lv_label_create(prev_btn);
    lv_label_set_text(prev_label, LV_SYMBOL_PREV);
    //lv_obj_set_style_text_color(prev_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_font(prev_label, &lv_font_montserrat_18, LV_PART_MAIN);
    lv_obj_center(prev_label);
    lv_obj_add_event_cb(prev_btn, music_prev_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_set_style_text_opa(prev_label, LV_OPA_0, LV_PART_MAIN);  // 文字完全不透明
    lv_obj_set_style_border_opa(prev_label, LV_OPA_0, LV_PART_ANY);
    
    // 播放/暂停按钮 - 居中放置
    play_pause_btn = lv_button_create(music_player_screen);
    lv_obj_set_size(play_pause_btn, 300, 200);//大小
    lv_obj_set_pos(play_pause_btn, 245, 115);//位置
    //lv_obj_set_style_bg_color(play_pause_btn, lv_color_hex(0xe74c3c), LV_PART_MAIN);
    lv_obj_set_style_radius(play_pause_btn, 100, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(play_pause_btn, LV_OPA_0, LV_PART_MAIN);  // 文字完全不透明
    lv_obj_set_style_border_opa(play_pause_btn, LV_OPA_0, LV_PART_MAIN);

    
    lv_obj_t * play_pause_label = lv_label_create(play_pause_btn);
    const char* btn_text = (g_music_player.state == MUSIC_STATE_PLAYING) ? LV_SYMBOL_PAUSE : LV_SYMBOL_PLAY;
    lv_label_set_text(play_pause_label, btn_text);
    lv_obj_set_style_text_color(play_pause_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_font(play_pause_label, &lv_font_montserrat_22, LV_PART_MAIN);
    lv_obj_center(play_pause_label);
    lv_obj_add_event_cb(play_pause_btn, music_play_pause_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_set_style_text_opa(play_pause_label, LV_OPA_0, LV_PART_MAIN);  // 文字完全不透明
    lv_obj_set_style_border_opa(play_pause_label, LV_OPA_0, LV_PART_MAIN);
    
    // 下一首按钮
    lv_obj_t * next_btn = lv_button_create(music_player_screen);
    lv_obj_set_size(next_btn, 90, 90);//大小
    lv_obj_set_pos(next_btn, 590, 275);//位置
    //lv_obj_set_style_bg_color(next_btn, lv_color_hex(0x3498db), LV_PART_MAIN);
    lv_obj_set_style_radius(next_btn, 45, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(next_btn, LV_OPA_0, LV_PART_MAIN);  // 文字完全不透明
    lv_obj_set_style_border_opa(next_btn, LV_OPA_0, LV_PART_MAIN);

    
    lv_obj_t * next_label = lv_label_create(next_btn);
    lv_label_set_text(next_label, LV_SYMBOL_NEXT);
    lv_obj_set_style_text_color(next_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_font(next_label, &lv_font_montserrat_18, LV_PART_MAIN);
    lv_obj_center(next_label);
    lv_obj_add_event_cb(next_btn, music_next_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_set_style_text_opa(next_label, LV_OPA_0, LV_PART_MAIN);  // 文字完全不透明
    lv_obj_set_style_border_opa(next_label, LV_OPA_0, LV_PART_MAIN);
    
    // 播放模式按钮 - 右上角
    mode_btn = lv_button_create(music_player_screen);
    lv_obj_set_size(mode_btn, 80, 80);
    lv_obj_set_pos(mode_btn, 80, 120);
    //lv_obj_set_style_bg_color(mode_btn, lv_color_hex(0x9b59b6), LV_PART_MAIN);
    lv_obj_set_style_radius(mode_btn, 40, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(mode_btn, LV_OPA_0, LV_PART_MAIN);  // 文字完全不透明
    lv_obj_set_style_border_opa(mode_btn, LV_OPA_0, LV_PART_MAIN);
    
    lv_obj_t * mode_label = lv_label_create(mode_btn);
    const char* mode_text = (g_music_player.play_mode == PLAY_MODE_SEQUENCE) ? "Sequential" : "Random";
    lv_label_set_text(mode_label, mode_text);
    lv_obj_set_style_text_color(mode_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_font(mode_label, &lv_font_montserrat_10, LV_PART_MAIN);
    lv_obj_center(mode_label);
    lv_obj_add_event_cb(mode_btn, music_mode_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_set_style_text_opa(mode_label, LV_OPA_0, LV_PART_MAIN);  // 文字完全不透明
    lv_obj_set_style_border_opa(mode_label, LV_OPA_0, LV_PART_MAIN);
    
    // 返回按钮 - 左上角
    lv_obj_t * back_btn = lv_button_create(music_player_screen);
    lv_obj_set_size(back_btn, 70, 70);
    lv_obj_set_pos(back_btn, 630, 35);
    //lv_obj_set_style_bg_color(back_btn, lv_color_hex(0x95a5a6), LV_PART_MAIN);
    lv_obj_set_style_radius(back_btn, 35, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(back_btn, LV_OPA_0, LV_PART_MAIN);  // 文字完全不透明
    lv_obj_set_style_border_opa(back_btn, LV_OPA_0, LV_PART_MAIN);
    
    lv_obj_t * back_label = lv_label_create(back_btn);
    lv_label_set_text(back_label, "Back");
    lv_obj_set_style_text_color(back_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_font(back_label, &lv_font_montserrat_10, LV_PART_MAIN);
    lv_obj_center(back_label);
    lv_obj_add_event_cb(back_btn, music_player_back_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_set_style_text_opa(back_label, LV_OPA_0, LV_PART_MAIN);  // 文字完全不透明
    lv_obj_set_style_border_opa(back_label, LV_OPA_0, LV_PART_MAIN);
    
   // 进度条区域 - 透明背景带边框
    lv_obj_t * progress_area = lv_obj_create(music_player_screen);
    lv_obj_set_size(progress_area, 620, 80);//大小

    lv_obj_set_pos(progress_area, 80, 400);
    lv_obj_set_style_bg_opa(progress_area, LV_OPA_0, LV_PART_MAIN);  // 高透明度


    lv_obj_set_style_bg_color(progress_area, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_border_width(progress_area, 1, LV_PART_MAIN);   // 添加边框
    lv_obj_set_style_border_color(progress_area, lv_color_hex(0x555555), LV_PART_MAIN);
    lv_obj_set_style_border_opa(progress_area, LV_OPA_0, LV_PART_MAIN);
    lv_obj_set_style_radius(progress_area, 10, LV_PART_MAIN);
    lv_obj_set_style_pad_all(progress_area, 15, LV_PART_MAIN);
    
    // 进度条 - 移除拖拽功能，只显示进度
    progress_bar = lv_slider_create(progress_area);
    lv_obj_set_size(progress_bar, 560, 15);  // 修正进度条尺寸
    lv_obj_set_pos(progress_bar, 20, 5);      // 相对于progress_area的位置
    lv_obj_set_style_bg_color(progress_bar, lv_color_hex(0x555555), LV_PART_MAIN);
    lv_obj_set_style_bg_color(progress_bar, lv_color_hex(0xFF6B6B), LV_PART_INDICATOR);
    lv_slider_set_range(progress_bar, 0, 100);
    lv_slider_set_value(progress_bar, 0, LV_ANIM_OFF);
    lv_obj_remove_flag(progress_bar, LV_OBJ_FLAG_CLICKABLE);  // 移除点击功能
    
    // 时间标签
    progress_label = lv_label_create(progress_area);
    lv_label_set_text(progress_label, "00:00 / 00:00");
    lv_obj_set_style_text_color(progress_label, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_text_font(progress_label, &lv_font_montserrat_10, LV_PART_MAIN);
    lv_obj_set_style_text_opa(progress_label, LV_OPA_70, LV_PART_MAIN);
    lv_obj_set_pos(progress_label, 260, 25);  // 调整时间标签位置，在进度条下方
    
    // 更新界面信息 - 只显示文件名
    if (g_music_player.music_count > 0) {
        lv_label_set_text(song_title_label, g_music_player.music_list[g_music_player.current_index].filename);
    }
    
    // 启动进度更新定时器 - 每秒更新
    if (progress_timer) {
        lv_timer_del(progress_timer);
    }
    progress_timer = lv_timer_create(update_music_progress, 1000, NULL);
}

/**
 * @brief 播放音乐
 * 
 * 功能说明：
 * 1. 验证音乐索引的有效性
 * 2. 停止当前正在播放的音乐
 * 3. 更新当前播放索引
 * 4. 使用fork()创建子进程来播放音乐
 * 5. 在子进程中执行madplay或mpg123播放器
 * 6. 在父进程中记录播放状态和进程ID
 * 7. 更新播放界面的显示信息
 * 
 * 技术要点：
 * - 使用进程间通信：fork()创建子进程，父子进程分工合作
 * - 子进程负责音乐播放，父进程负责界面管理
 * - 使用execl()在子进程中替换为音乐播放器程序
 * - 支持多种播放器：优先使用madplay，备选mpg123
 * 
 * @param index 要播放的音乐在播放列表中的索引
 */
void play_music(int index)
{
    // ========== 参数验证 ==========
    // 检查音乐索引是否在有效范围内
    if (index < 0 || index >= g_music_player.music_count) {
        return; // 索引无效，直接返回
    }
    
    // ========== 强制停止当前播放 ==========
    // 确保完全清理之前的播放进程，防止资源泄露
    stop_music();
    
    // 额外等待，确保进程完全结束
    usleep(50000); // 等待50ms
    
    // ========== 更新播放状态 ==========
    // 设置当前播放的音乐索引
    g_music_player.current_index = index;
    
    // 重置进度计数器
    current_progress = 0;
    
    // 在控制台输出当前播放的文件路径，便于调试
    // printf("Starting new music: %s\n", g_music_player.music_list[index].filepath);
    
    // ========== 创建播放进程 ==========
    // 使用fork()创建子进程来播放音乐
    // 这样做的好处是：主进程继续处理界面，子进程专门负责音乐播放
    pid_t pid = fork();
    
    if (pid == 0) {
        // ========== 子进程：负责音乐播放 ==========
        
        // 设置子进程信号处理，确保能够正确响应终止信号
        signal(SIGTERM, SIG_DFL);
        signal(SIGINT, SIG_DFL);
        
        // 关闭不需要的文件描述符，减少资源占用
        // close(STDIN_FILENO);  // 可选：关闭标准输入
        
        // 尝试使用madplay播放器播放MP3文件
        // execl()会替换当前进程映像，执行指定的程序
        // 参数说明：程序路径、程序名、选项、文件路径、结束标志
        execl("/usr/bin/madplay", "madplay", "-q", g_music_player.music_list[index].filepath, NULL);
        
        // 如果madplay执行失败（比如程序不存在），尝试使用mpg123
        execl("/usr/bin/mpg123", "mpg123", "-q", g_music_player.music_list[index].filepath, NULL);
        
        // 如果两个播放器都无法执行，子进程退出
        // 注意：如果execl()成功，不会执行到这里
        // 如果到达这里说明execl失败
        exit(1);
        
    } else if (pid > 0) {
        // ========== 父进程：负责界面管理 ==========
        
        // 记录子进程的进程ID，用于后续的进程控制（暂停、停止等）
        g_music_player.player_pid = pid;
        
        // 更新播放器状态为"正在播放"
        g_music_player.state = MUSIC_STATE_PLAYING;
        
        // printf("Music player started with PID: %d\n", pid);
        
        // 更新播放界面 - 只更新文件名
        if (music_player_screen && lv_obj_is_valid(music_player_screen) && 
            song_title_label && lv_obj_is_valid(song_title_label)) {
            lv_label_set_text(song_title_label, g_music_player.music_list[index].filename);
            
            // 更新播放按钮
            if (play_pause_btn && lv_obj_is_valid(play_pause_btn)) {
                lv_obj_t * play_pause_label = lv_obj_get_child(play_pause_btn, 0);
                if (play_pause_label && lv_obj_is_valid(play_pause_label)) {
                    lv_label_set_text(play_pause_label, LV_SYMBOL_PAUSE);
                }
            }
        }
    } else {
        // ========== fork失败 ==========
        // 进程创建失败
        g_music_player.state = MUSIC_STATE_STOP;
    }
}

/**
 * @brief 暂停音乐
 * 
 * 功能说明：
 * 1. 检查播放进程是否存在且正在播放
 * 2. 向播放进程发送暂停信号
 * 3. 更新播放状态
 * 4. 更新播放按钮显示
 */
void pause_music(void)
{
    if (g_music_player.player_pid > 0 && g_music_player.state == MUSIC_STATE_PLAYING) {
        kill(g_music_player.player_pid, SIGSTOP);
        g_music_player.state = MUSIC_STATE_PAUSED;
        
        // 更新播放按钮
        if (play_pause_btn && lv_obj_is_valid(play_pause_btn)) {
            lv_obj_t * play_pause_label = lv_obj_get_child(play_pause_btn, 0);
            if (play_pause_label && lv_obj_is_valid(play_pause_label)) {
                lv_label_set_text(play_pause_label, LV_SYMBOL_PLAY);
            }
        }
    }
}

/**
 * @brief 恢复音乐
 * 
 * 功能说明：
 * 1. 检查播放进程是否存在且已暂停
 * 2. 向播放进程发送继续信号
 * 3. 更新播放状态
 * 4. 更新播放按钮显示
 */
void resume_music(void)
{
    if (g_music_player.player_pid > 0 && g_music_player.state == MUSIC_STATE_PAUSED) {
        kill(g_music_player.player_pid, SIGCONT);
        g_music_player.state = MUSIC_STATE_PLAYING;
        
        // 更新播放按钮
        if (play_pause_btn && lv_obj_is_valid(play_pause_btn)) {
            lv_obj_t * play_pause_label = lv_obj_get_child(play_pause_btn, 0);
            if (play_pause_label && lv_obj_is_valid(play_pause_label)) {
                lv_label_set_text(play_pause_label, LV_SYMBOL_PAUSE);
            }
        }
    }
}

/**
 * @brief 停止音乐 - 优化资源释放
 */
void stop_music(void)
{
    if (g_music_player.player_pid > 0) {
        // 1. 先尝试温和地终止进程
        kill(g_music_player.player_pid, SIGTERM);
        
        // 2. 等待进程结束，设置超时避免无限等待
        int status;
        pid_t result = waitpid(g_music_player.player_pid, &status, WNOHANG);
        
        // 3. 如果进程没有立即结束，强制终止
        if (result == 0) {
            usleep(100000); // 等待100ms
            result = waitpid(g_music_player.player_pid, &status, WNOHANG);
            if (result == 0) {
                kill(g_music_player.player_pid, SIGKILL); // 强制终止
                waitpid(g_music_player.player_pid, NULL, 0);
            }
        }
        
        // 4. 清理进程ID
        g_music_player.player_pid = -1;
        
        // printf("Music process stopped and resources released\n");
    }
    
    g_music_player.state = MUSIC_STATE_STOP;
    g_music_player.current_time = 0;
    
    // 重置进度计数器
    current_progress = 0;
    
    // 重置进度条显示
    if (progress_bar && lv_obj_is_valid(progress_bar)) {
        lv_slider_set_value(progress_bar, 0, LV_ANIM_OFF);
    }
    if (progress_label && lv_obj_is_valid(progress_label)) {
        lv_label_set_text(progress_label, "00:00 / 00:00");
    }
    
    // 更新播放按钮
    if (play_pause_btn && lv_obj_is_valid(play_pause_btn)) {
        lv_obj_t * play_pause_label = lv_obj_get_child(play_pause_btn, 0);
        if (play_pause_label && lv_obj_is_valid(play_pause_label)) {
            lv_label_set_text(play_pause_label, LV_SYMBOL_PLAY);
        }
    }
}

/**
 * @brief 下一首
 * 
 * 功能说明：
 * 1. 检查是否有音乐文件
 * 2. 根据播放模式计算下一首的索引
 * 3. 播放选中的音乐
 */
void next_music(void)
{
    if (g_music_player.music_count == 0) return;
    
    int next_index;
    if (g_music_player.play_mode == PLAY_MODE_RANDOM) {
        next_index = rand() % g_music_player.music_count;
    } else {
        next_index = (g_music_player.current_index + 1) % g_music_player.music_count;
    }
    
    play_music(next_index);
}

/**
 * @brief 上一首
 * 
 * 功能说明：
 * 1. 检查是否有音乐文件
 * 2. 根据播放模式计算上一首的索引
 * 3. 播放选中的音乐
 */
void prev_music(void)
{
    if (g_music_player.music_count == 0) return;
    
    int prev_index;
    if (g_music_player.play_mode == PLAY_MODE_RANDOM) {
        prev_index = rand() % g_music_player.music_count;
    } else {
        prev_index = (g_music_player.current_index - 1 + g_music_player.music_count) % g_music_player.music_count;
    }
    
    play_music(prev_index);
}

/**
 * @brief 切换播放模式
 * 
 * 在顺序播放和随机播放模式之间切换
 * 顺序播放：按列表顺序播放歌曲
 * 随机播放：随机选择列表中的歌曲播放
 */
void toggle_play_mode(void)
{
    g_music_player.play_mode = (g_music_player.play_mode == PLAY_MODE_SEQUENCE) ? 
                               PLAY_MODE_RANDOM : PLAY_MODE_SEQUENCE;
    
    // 更新模式按钮显示
    if (mode_btn) {
        lv_obj_t * label = lv_obj_get_child(mode_btn, 0);
        if (label) {
            const char * mode_text = (g_music_player.play_mode == PLAY_MODE_SEQUENCE) ? 
                                   "Sequential" : "Random";
            lv_label_set_text(label, mode_text);
        }
    }
}

// ============ 事件回调函数 ============
// 以下函数处理用户在音乐播放器中的各种交互操作

/**
 * @brief 音乐列表手势事件回调函数
 * 
 * 功能说明：
 * 在音乐列表界面检测用户的手势操作
 * 当用户向右滑动时，退出音乐播放器回到桌面
 * 
 * 退出时的清理工作：
 * 1. 删除进度更新定时器
 * 2. 停止当前播放的音乐
 * 3. 返回桌面界面
 * 
 * @param e 事件对象，包含手势信息
 */
/**
 * @brief 清理音乐播放器资源
 * 
 * 在退出应用或切换界面时调用，确保所有资源都被正确释放
 * 包括定时器、播放进程、界面元素和状态变量
 */
void cleanup_music_player(void)
{
    // 停止定时器
    if (progress_timer) {
        lv_timer_del(progress_timer);
        progress_timer = NULL;
    }
    
    // 停止音乐播放
    stop_music();
    
    // 重置所有状态
    g_music_player.state = MUSIC_STATE_STOP;
    g_music_player.player_pid = -1;
    current_progress = 0;
    
    // 清理界面指针
    music_list_screen = NULL;
    music_player_screen = NULL;
    progress_bar = NULL;
    progress_label = NULL;
    song_title_label = NULL;
    song_artist_label = NULL;
    play_pause_btn = NULL;
    mode_btn = NULL;
}

/**
 * @brief 手动手势检测回调函数
 * 
 * 通过监听按下和释放事件手动检测手势
 */
void music_list_touch_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    
    if (code == LV_EVENT_PRESSED) {
        // 记录触摸开始点
        lv_indev_t * indev = lv_indev_get_act();
        if (indev) {
            lv_indev_get_point(indev, &touch_start_point);
            touch_start_time = lv_tick_get();
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
                uint32_t touch_end_time = lv_tick_get();
                
                int dx = touch_end_point.x - touch_start_point.x;
                int dy = touch_end_point.y - touch_start_point.y;
                uint32_t duration = touch_end_time - touch_start_time;
                
                // 判断是否为有效的右滑手势
                if (dx > 50 && abs(dy) < 100 && duration < 1000) {  // 右滑超过50像素，垂直偏移小于100像素，时间小于1秒
                    cleanup_music_player();
                    extern void desktop(void);
                    desktop();
                }
            }
            touch_started = false;
        }
    }
}

void music_list_gesture_cb(lv_event_t * e)
{
    // 获取当前活动输入设备
    lv_indev_t * indev = lv_indev_get_act();
    if (indev == NULL) {
        return;
    }

    // 获取当前活动输入设备的手势方向
    lv_dir_t gesture = lv_indev_get_gesture_dir(indev);

    // 检查是否为右滑手势
    if (gesture == LV_DIR_RIGHT) {
        cleanup_music_player();
        extern void desktop(void);
        desktop();
    }
}

// 确保在show_music_list_screen函数中正确添加手势事件
void ensure_gesture_enabled(void)
{
    if (music_list_screen) {
        lv_obj_add_event_cb(music_list_screen, music_list_gesture_cb, LV_EVENT_GESTURE, NULL);
        lv_obj_add_flag(music_list_screen, LV_OBJ_FLAG_GESTURE_BUBBLE);
    }
}

void music_list_item_cb(lv_event_t * e)
{
    lv_obj_t * btn = lv_event_get_target(e);
    int index = (int)(uintptr_t)lv_obj_get_user_data(btn);
    
    g_music_player.current_index = index;
    show_music_player_screen();
    play_music(index);
}

void music_play_pause_cb(lv_event_t * e)
{
    if (g_music_player.state == MUSIC_STATE_PLAYING) {
        pause_music();
    } else if (g_music_player.state == MUSIC_STATE_PAUSED) {
        resume_music();
    } else {
        if (g_music_player.music_count > 0) {
            play_music(g_music_player.current_index);
        }
    }
}

void music_next_cb(lv_event_t * e)
{
    next_music();
}

void music_prev_cb(lv_event_t * e)
{
    prev_music();
}

void music_mode_cb(lv_event_t * e)
{
    toggle_play_mode();
}

void music_player_back_cb(lv_event_t * e)
{
    // 从播放界面返回到列表界面
    show_music_list_screen();
}

void music_back_cb(lv_event_t * e)
{
    // 使用统一的清理函数
    cleanup_music_player();
    
    // 返回桌面
    extern void desktop(void);
    desktop();
}

/**
 * @brief 更新音乐进度
 * 
 * 优化说明：
 * 1. 每首歌都从0开始计时，避免进度混乱
 * 2. 只在播放状态时更新进度
 * 3. 检查子进程状态，清理僵尸进程
 * 4. 减少UI更新频率，避免界面卡顿
 * 5. 简化时间显示逻辑，格式化为MM:SS
 */
void update_music_progress(lv_timer_t * timer)
{
    // 检查播放进程是否还存在，清理僵尸进程
    if (g_music_player.player_pid > 0) {
        int status;
        pid_t result = waitpid(g_music_player.player_pid, &status, WNOHANG);
        
        if (result > 0) {
            // 子进程已结束，自动播放下一首
            g_music_player.player_pid = -1;
            g_music_player.state = MUSIC_STATE_STOP;
            current_progress = 0;
            next_music();
            return;
        }
    }
    
    // 只有在播放状态才更新进度
    if (g_music_player.state != MUSIC_STATE_PLAYING) {
        return;
    }
    
    // 检查进度条是否有效
    if (!progress_bar || !lv_obj_is_valid(progress_bar)) {
        return;
    }
    
    // 增加进度计数器（每秒+1）
    current_progress++;
    
    // 假设每首歌3分30秒 = 210秒，计算百分比
    int total_seconds = 210;
    int progress_percent = (current_progress * 100) / total_seconds;
    if (progress_percent > 100) {
        progress_percent = 100;
    }
    
    // 更新进度条 - 减少动画，提高性能
    lv_slider_set_value(progress_bar, progress_percent, LV_ANIM_OFF);
    
    // 更新时间显示 - 只在必要时更新
    if (progress_label && lv_obj_is_valid(progress_label)) {
        char time_str[16];
        int current_min = current_progress / 60;
        int current_sec = current_progress % 60;
        int total_min = total_seconds / 60;
        int total_sec = total_seconds % 60;
        snprintf(time_str, sizeof(time_str), "%02d:%02d / %02d:%02d", 
                current_min, current_sec, total_min, total_sec);
        lv_label_set_text(progress_label, time_str);
    }
}
