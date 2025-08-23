#include "../inc/dcim.h"
#include "../inc/main_screen.h"

// ============ 相册全局变量 ============
// 相册管理器全局变量，保存图片列表和状态
gallery_manager_t g_gallery = {0};

/**
 * @brief 返回按钮事件回调
 * 
 * 功能说明：
 * 1. 释放相册资源
 * 2. 返回桌面
 * 
 * @param e 事件指针
 */
void dcim_back_btn_event_cb(lv_event_t * e)
{
    // 释放相册资源
    release_gallery_resources();
    
    // 返回桌面
    desktop();
}

/**
 * @brief 释放相册资源
 * 
 * 功能说明：
 * 1. 删除隐藏信息定时器
 * 2. 删除缩略图和查看器界面
 * 3. 清空指针和状态
 */
void release_gallery_resources(void)
{
    // 删除隐藏信息定时器
    if (g_gallery.hide_timer != NULL) {
        lv_timer_delete(g_gallery.hide_timer);
        g_gallery.hide_timer = NULL;
    }
    
    // 删除缩略图界面
    if (g_gallery.thumbnail_screen != NULL) {
        lv_obj_del(g_gallery.thumbnail_screen);
        g_gallery.thumbnail_screen = NULL;
    }
    
    if (g_gallery.viewer_screen != NULL) {
        lv_obj_del(g_gallery.viewer_screen);
        g_gallery.viewer_screen = NULL;
    }
    
    // 清空对象指针
    g_gallery.current_img = NULL;
    g_gallery.info_container = NULL;
    g_gallery.info_label = NULL;
    
    // 清空触摸状态
    g_gallery.touch_started = false;
    g_gallery.swipe_detected = false;
}

/**
 * @brief 初始化相册
 * 
 * 功能说明：
 * 1. 清零相册管理器结构体，确保初始状态干净
 * 2. 设置当前查看的图片索引为0
 * 3. 扫描指定目录下的所有图片文件
 * 
 * 这个函数在程序启动时调用，用于准备相册功能所需的所有数据
 */
void init_gallery(void)
{
    // 使用memset清零整个结构体，确保所有成员都是初始值
    memset(&g_gallery, 0, sizeof(gallery_manager_t));
    
    // 设置当前查看的图片索引为第一张（索引0）
    g_gallery.current_index = 0;
    
    // 初始化触摸状态
    g_gallery.touch_started = false;
    g_gallery.swipe_detected = false;
    g_gallery.hide_timer = NULL;
    
    // 扫描图片文件：从指定目录加载所有支持格式的图片文件
    scan_image_files("/home/tyc/work_station/dcim");
}

/**
 * @brief 扫描图片文件
 * 
 * 功能说明：
 * 1. 打开指定目录
 * 2. 遍历目录中的所有文件
 * 3. 筛选出支持的图片格式文件（jpg、jpeg、png、bmp、gif）
 * 4. 将图片文件信息添加到图片列表中
 * 5. 构建每个图片文件的完整路径
 * 
 * @param directory 要扫描的目录路径
 */
void scan_image_files(const char* directory)
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
    
    // 初始化图片文件计数器
    g_gallery.image_count = 0;
    
    // 遍历目录中的所有文件，直到没有更多文件或达到最大文件数限制
    while ((entry = readdir(dir)) != NULL && g_gallery.image_count < MAX_IMAGE_COUNT) {
        // 跳过隐藏文件和目录（以'.'开头的文件）
        if (entry->d_name[0] == '.') continue;
        
        // 检查文件是否为支持的图片格式
        if (is_image_file(entry->d_name)) {
            // 获取当前图片文件信息结构体的指针
            image_info_t *info = &g_gallery.image_list[g_gallery.image_count];
            
            // 复制文件名到结构体中，确保字符串结束符
            strncpy(info->filename, entry->d_name, MAX_NAME_LEN - 1);
            info->filename[MAX_NAME_LEN - 1] = '\0';
            
            // 构建完整的文件路径：目录路径 + "/" + 文件名 (Linux路径)
            snprintf(full_path, sizeof(full_path), "%s/%s", directory, entry->d_name);
            strncpy(info->filepath, full_path, MAX_PATH_LEN - 1);
            info->filepath[MAX_PATH_LEN - 1] = '\0';
            
            // 图片文件计数增加
            g_gallery.image_count++;
        }
    }
    
    // 关闭目录句柄，释放系统资源
    closedir(dir);
}

/**
 * @brief 检查是否为图片文件
 * 
 * 功能说明：
 * 1. 提取文件的扩展名
 * 2. 与支持的图片格式列表进行比较
 * 3. 返回是否为支持的图片格式
 * 
 * 支持的格式：.jpg, .jpeg, .png, .bmp, .gif（不区分大小写）
 * 
 * @param filename 要检查的文件名
 * @return 1表示是图片文件，0表示不是图片文件
 */
int is_image_file(const char* filename)
{
    // 查找文件名中最后一个'.'的位置（扩展名分隔符）
    const char* ext = strrchr(filename, '.');
    if (!ext) return 0; // 如果没有扩展名，则不是图片文件
    
    // 不区分大小写地比较扩展名与支持的图片格式
    // strcasecmp函数进行大小写不敏感的字符串比较
    return (strcasecmp(ext, ".jpg") == 0 ||   // JPEG格式
            strcasecmp(ext, ".jpeg") == 0 ||  // JPEG格式（完整扩展名）
            strcasecmp(ext, ".png") == 0 ||   // PNG格式
            strcasecmp(ext, ".bmp") == 0 ||   // BMP格式
            strcasecmp(ext, ".gif") == 0);    // GIF格式
}

/**
 * @brief 显示相册缩略图界面
 * 
 * 功能说明：
 * 1. 清理当前屏幕上的所有对象
 * 2. 重置界面对象指针，防止访问无效内存
 * 3. 创建缩略图界面的根容器
 * 4. 设置背景图片
 * 5. 添加手势支持（右滑退出）
 * 6. 创建缩略图网格容器
 * 7. 生成缩略图网格
 * 
 * 界面结构：
 * - thumbnail_screen (根容器)
 *   - bg_img (背景图片)
 *   - container (半透明黑色容器)
 *     - scroll_container (可滚动容器)
 *       - 缩略图按钮网格
 */
void show_gallery_thumbnail_screen(void)
{
    // ========== 清理和重置 ==========
    // 清理当前屏幕上的所有LVGL对象，释放内存
    lv_obj_clean(lv_screen_active());
    
    // 重置对象指针为NULL，防止访问已释放的内存
    g_gallery.viewer_screen = NULL;
    g_gallery.current_img = NULL;
    g_gallery.info_container = NULL;
    g_gallery.info_label = NULL;
    
    // 删除定时器（如果存在）
    if (g_gallery.hide_timer != NULL) {
        lv_timer_delete(g_gallery.hide_timer);
        g_gallery.hide_timer = NULL;
    }
    
    // ========== 创建根界面容器 ==========
    // 创建缩略图界面的根容器，占满整个屏幕
    g_gallery.thumbnail_screen = lv_obj_create(lv_screen_active());
    lv_obj_set_size(g_gallery.thumbnail_screen, 800, 480);  // 设置为屏幕大小
    lv_obj_set_pos(g_gallery.thumbnail_screen, 0, 0);       // 位置在屏幕左上角
    lv_obj_set_style_pad_all(g_gallery.thumbnail_screen, 0, LV_PART_MAIN); // 无内边距
    lv_obj_set_style_border_width(g_gallery.thumbnail_screen, 0, LV_PART_MAIN); // 无边框
    
    // ========== 设置背景图片 ==========
    // 创建背景图片对象，显示相册的背景图
    lv_obj_t * bg_img = lv_image_create(g_gallery.thumbnail_screen);
    lv_obj_set_size(bg_img, 800, 480);  // 背景图片铺满屏幕
    lv_obj_set_pos(bg_img, 0, 0);       // 位置在左上角
    lv_image_set_src(bg_img, "/home/tyc/work_station/dcim/dcim_back.jpg"); // 设置图片源
    
    // ========== 添加手势支持 ==========
    // 添加右滑手势事件回调，用户右滑可以退出到主屏幕
    lv_obj_add_event_cb(g_gallery.thumbnail_screen, gallery_gesture_cb, LV_EVENT_GESTURE, NULL);
    lv_obj_add_flag(g_gallery.thumbnail_screen, LV_OBJ_FLAG_GESTURE_BUBBLE); // 允许手势事件冒泡
    
    lv_obj_add_event_cb(bg_img, gallery_gesture_cb, LV_EVENT_GESTURE, NULL);
    lv_obj_add_flag(bg_img, LV_OBJ_FLAG_GESTURE_BUBBLE);
    
    // 添加手动手势检测 - 监听按下和释放事件
    lv_obj_add_event_cb(g_gallery.thumbnail_screen, gallery_touch_cb, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(g_gallery.thumbnail_screen, gallery_touch_cb, LV_EVENT_RELEASED, NULL);
    lv_obj_add_event_cb(bg_img, gallery_touch_cb, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(bg_img, gallery_touch_cb, LV_EVENT_RELEASED, NULL);
    
    // ========== 创建缩略图容器 ==========
    // 缩略图容器
    lv_obj_t * container = lv_obj_create(g_gallery.thumbnail_screen);
    lv_obj_set_size(container, 760, 460);
    lv_obj_set_pos(container, 20, 10);
    lv_obj_set_style_bg_opa(container, LV_OPA_80, LV_PART_MAIN);  // 80%透明度
    lv_obj_set_style_bg_color(container, lv_color_hex(0x000000), LV_PART_MAIN); // 黑色背景
    lv_obj_set_style_border_width(container, 0, LV_PART_MAIN);    // 无边框
    lv_obj_set_style_radius(container, 10, LV_PART_MAIN);         // 圆角
    lv_obj_set_style_pad_all(container, 10, LV_PART_MAIN);        // 内边距
    
    // 为缩略图容器添加手势检测，确保手势能够传递
    lv_obj_add_event_cb(container, gallery_touch_cb, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(container, gallery_touch_cb, LV_EVENT_RELEASED, NULL);
    lv_obj_add_flag(container, LV_OBJ_FLAG_GESTURE_BUBBLE);  // 确保事件能够冒泡
    
    // ========== 创建滚动容器 ==========
    // 创建可滚动的容器，用于放置缩略图网格（当图片很多时可以滚动）
    lv_obj_t * scroll_container = lv_obj_create(container);
    lv_obj_set_size(scroll_container, 740, 440);
    lv_obj_set_pos(scroll_container, 0, 0);       // 位置在父容器左上角
    lv_obj_set_style_bg_opa(scroll_container, LV_OPA_TRANSP, LV_PART_MAIN); // 透明背景
    lv_obj_set_style_border_width(scroll_container, 0, LV_PART_MAIN);       // 无边框
    lv_obj_set_scroll_dir(scroll_container, LV_DIR_VER);  // 只允许垂直滚动
    
    // 为滚动容器添加手势检测，确保手势能够传递
    lv_obj_add_event_cb(scroll_container, gallery_touch_cb, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(scroll_container, gallery_touch_cb, LV_EVENT_RELEASED, NULL);
    lv_obj_add_flag(scroll_container, LV_OBJ_FLAG_GESTURE_BUBBLE);  // 确保事件能够冒泡
    
    // ========== 创建缩略图网格 ==========
    create_thumbnail_grid(scroll_container);
}

/**
 * @brief 创建缩略图网格
 * 
 * 功能说明：
 * 1. 遍历图片列表
 * 2. 创建缩略图按钮和图片
 * 3. 设置点击事件
 * 4. 设置悬停效果
 * 5. 布局缩略图
 * 
 * @param parent 父容器
 */
void create_thumbnail_grid(lv_obj_t* parent)
{
    if (g_gallery.image_count == 0) {
        // 显示无图片提示
        lv_obj_t * no_images_label = lv_label_create(parent);
        lv_label_set_text(no_images_label, "No images found");
        lv_obj_set_style_text_color(no_images_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
        lv_obj_set_style_text_font(no_images_label, &lv_font_montserrat_16, LV_PART_MAIN);
        lv_obj_center(no_images_label);
        return;
    }
    
    int x_offset = 55;  // 从68调整为55，向左移动13像素
    int y_offset = 10;
    int spacing = 15;
    
    for (int i = 0; i < g_gallery.image_count; i++) {
        int row = i / THUMBNAILS_PER_ROW;
        int col = i % THUMBNAILS_PER_ROW;
        
        // 计算位置
        int x = x_offset + col * (THUMBNAIL_WIDTH + spacing);
        int y = y_offset + row * (THUMBNAIL_HEIGHT + spacing);
        
        // 创建缩略图容器
        lv_obj_t * thumb_container = lv_obj_create(parent);
        lv_obj_set_size(thumb_container, THUMBNAIL_WIDTH, THUMBNAIL_HEIGHT);
        lv_obj_set_pos(thumb_container, x, y);
        lv_obj_set_style_bg_color(thumb_container, lv_color_hex(0x34495e), LV_PART_MAIN);
        lv_obj_set_style_radius(thumb_container, 5, LV_PART_MAIN);
        lv_obj_set_style_border_width(thumb_container, 2, LV_PART_MAIN);
        lv_obj_set_style_border_color(thumb_container, lv_color_hex(0x2c3e50), LV_PART_MAIN);
        lv_obj_set_style_pad_all(thumb_container, 2, LV_PART_MAIN);
        
        // 创建图片对象显示缩略图
        lv_obj_t * thumb_img = lv_image_create(thumb_container);
        lv_obj_set_size(thumb_img, THUMBNAIL_WIDTH - 8, THUMBNAIL_HEIGHT - 8);
        lv_obj_center(thumb_img);
        
        // 尝试加载图片，如果失败则显示占位符
        lv_image_set_src(thumb_img, g_gallery.image_list[i].filepath);
        
        // 设置图片缩放模式，保持比例
        lv_obj_set_style_image_opa(thumb_img, LV_OPA_COVER, LV_PART_MAIN);
        
        // 添加点击事件到容器
        lv_obj_set_user_data(thumb_container, (void*)(uintptr_t)i);
        lv_obj_add_event_cb(thumb_container, thumbnail_click_cb, LV_EVENT_CLICKED, NULL);
        lv_obj_add_flag(thumb_container, LV_OBJ_FLAG_CLICKABLE);
        
        // 添加悬停效果
        lv_obj_set_style_bg_color(thumb_container, lv_color_hex(0x3498db), LV_PART_MAIN | LV_STATE_PRESSED);
    }
}

/**
 * @brief 显示图片查看器界面
 * 
 * 功能说明：
 * 1. 清理当前屏幕
 * 2. 创建全屏图片显示
 * 3. 显示图片信息
 * 4. 添加手势和点击事件
 * 5. 信息条自动隐藏
 * 
 * @param index 图片索引
 */
void show_image_viewer_screen(int index)
{
    if (index < 0 || index >= g_gallery.image_count) return;
    
    g_gallery.current_index = index;
    
    // 清理当前屏幕
    lv_obj_clean(lv_screen_active());
    
    // 删除定时器（如果存在）
    if (g_gallery.hide_timer != NULL) {
        lv_timer_delete(g_gallery.hide_timer);
        g_gallery.hide_timer = NULL;
    }
    
    // 重置对象指针
    g_gallery.thumbnail_screen = NULL;
    
    // 创建全屏查看器界面
    g_gallery.viewer_screen = lv_obj_create(lv_screen_active());
    lv_obj_set_size(g_gallery.viewer_screen, 800, 480);
    lv_obj_set_pos(g_gallery.viewer_screen, 0, 0);
    lv_obj_set_style_bg_color(g_gallery.viewer_screen, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_pad_all(g_gallery.viewer_screen, 0, LV_PART_MAIN);
    lv_obj_set_style_border_width(g_gallery.viewer_screen, 0, LV_PART_MAIN);
    
    // 创建全屏图片显示对象
    g_gallery.current_img = lv_image_create(g_gallery.viewer_screen);
    lv_obj_set_size(g_gallery.current_img, 800, 480);
    lv_obj_set_pos(g_gallery.current_img, 0, 0);
    
    // 加载并显示当前图片
    lv_image_set_src(g_gallery.current_img, g_gallery.image_list[index].filepath);
    
    // 设置图片居中显示，保持比例
    lv_obj_center(g_gallery.current_img);
    
    // 添加触摸事件处理
    lv_obj_add_event_cb(g_gallery.current_img, image_touch_cb, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(g_gallery.current_img, image_touch_cb, LV_EVENT_RELEASED, NULL);
    lv_obj_add_event_cb(g_gallery.current_img, image_touch_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_add_flag(g_gallery.current_img, LV_OBJ_FLAG_CLICKABLE);
    
    lv_obj_add_event_cb(g_gallery.viewer_screen, image_touch_cb, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(g_gallery.viewer_screen, image_touch_cb, LV_EVENT_RELEASED, NULL);
    lv_obj_add_event_cb(g_gallery.viewer_screen, image_touch_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_add_flag(g_gallery.viewer_screen, LV_OBJ_FLAG_CLICKABLE);
    
    // 创建图片信息显示（半透明覆盖层，可选显示）
    g_gallery.info_container = lv_obj_create(g_gallery.viewer_screen);
    lv_obj_set_size(g_gallery.info_container, 300, 60);
    lv_obj_align(g_gallery.info_container, LV_ALIGN_BOTTOM_MID, 0, -10);
    lv_obj_set_style_bg_color(g_gallery.info_container, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(g_gallery.info_container, LV_OPA_70, LV_PART_MAIN);
    lv_obj_set_style_border_width(g_gallery.info_container, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(g_gallery.info_container, 10, LV_PART_MAIN);
    lv_obj_set_style_pad_all(g_gallery.info_container, 8, LV_PART_MAIN);
    
    // 显示图片信息
    g_gallery.info_label = lv_label_create(g_gallery.info_container);
    char info_text[256];
    snprintf(info_text, sizeof(info_text), "%d / %d - %s", 
             index + 1, g_gallery.image_count, g_gallery.image_list[index].filename);
    lv_label_set_text(g_gallery.info_label, info_text);
    lv_obj_set_style_text_color(g_gallery.info_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_font(g_gallery.info_label, &lv_font_montserrat_12, LV_PART_MAIN);
    lv_obj_set_style_text_align(g_gallery.info_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_obj_center(g_gallery.info_label);
    
    // 让信息条3秒后自动隐藏
    g_gallery.hide_timer = lv_timer_create(hide_info_timer_cb, 3000, g_gallery.info_container);
    lv_timer_set_repeat_count(g_gallery.hide_timer, 1);  // 只执行一次
}

/**
 * @brief 加载下一张图片
 * 
 * 功能说明：
 * 1. 切换到下一张图片
 * 2. 更新图片显示
 * 3. 循环切换
 */
void load_next_image(void)
{
    if (g_gallery.image_count == 0) return;
    
    g_gallery.current_index = (g_gallery.current_index + 1) % g_gallery.image_count;
    update_viewer_image();
}

/**
 * @brief 加载上一张图片
 * 
 * 功能说明：
 * 1. 切换到上一张图片
 * 2. 更新图片显示
 * 3. 循环切换
 */
void load_prev_image(void)
{
    if (g_gallery.image_count == 0) return;
    
    g_gallery.current_index = (g_gallery.current_index - 1 + g_gallery.image_count) % g_gallery.image_count;
    update_viewer_image();
}

/**
 * @brief 更新图片查看器界面
 * 
 * 功能说明：
 * 1. 更新图片和信息显示
 * 2. 检查对象有效性
 * 3. 显示信息容器
 * 4. 切换图片时调用
 * 
 * @param index 当前图片索引
 */
void update_viewer_image(void)
{
    // 检查对象是否存在
    if (g_gallery.current_img == NULL || g_gallery.info_label == NULL || 
        g_gallery.viewer_screen == NULL || g_gallery.info_container == NULL) {
        show_image_viewer_screen(g_gallery.current_index);
        return;
    }
    
    // 检查索引有效性
    if (g_gallery.current_index < 0 || g_gallery.current_index >= g_gallery.image_count) {
        return;
    }
    
    // 更新图片和信息显示
    lv_image_set_src(g_gallery.current_img, g_gallery.image_list[g_gallery.current_index].filepath);
    
    char info_text[256];
    snprintf(info_text, sizeof(info_text), "%d / %d - %s", 
             g_gallery.current_index + 1, g_gallery.image_count, 
             g_gallery.image_list[g_gallery.current_index].filename);
    lv_label_set_text(g_gallery.info_label, info_text);
    
    // 显示信息容器
    if (g_gallery.info_container != NULL) {
        lv_obj_remove_flag(g_gallery.info_container, LV_OBJ_FLAG_HIDDEN);
    }
}

/**
 * @brief 返回缩略图界面
 * 
 * 功能说明：
 * 直接显示缩略图界面
 */
void back_to_thumbnail(void)
{
    show_gallery_thumbnail_screen();
}

// ============ 事件回调函数 ============
// 以下函数处理用户的各种交互操作

/**
 * @brief 缩略图点击事件回调
 * 
 * 功能说明：
 * 1. 获取点击的缩略图索引
 * 2. 显示对应图片查看器
 * 3. 通过用户数据传递索引
 * 
 * @param e 事件指针
 */
void thumbnail_click_cb(lv_event_t * e)
{
    // 获取触发事件的容器对象（被点击的缩略图容器）
    lv_obj_t * container = lv_event_get_target(e);
    
    // 从容器的用户数据中取出图片索引
    // 在创建缩略图时，我们将索引存储在容器的用户数据中
    int index = (int)(uintptr_t)lv_obj_get_user_data(container);
    
    // 显示对应索引的图片查看器界面
    show_image_viewer_screen(index);
}

/**
 * @brief 手动手势检测回调函数 - 缩略图界面
 * 
 * 通过监听按下和释放事件手动检测手势
 */
void gallery_touch_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    
    if (code == LV_EVENT_PRESSED) {
        // 记录触摸开始点
        lv_indev_t * indev = lv_indev_get_act();
        if (indev) {
            lv_indev_get_point(indev, &g_gallery.touch_start_point);
            g_gallery.touch_started = true;
        }
    }
    else if (code == LV_EVENT_RELEASED) {
        if (g_gallery.touch_started) {
            // 计算手势
            lv_indev_t * indev = lv_indev_get_act();
            if (indev) {
                lv_indev_get_point(indev, &g_gallery.touch_end_point);
                
                int dx = g_gallery.touch_end_point.x - g_gallery.touch_start_point.x;
                int dy = g_gallery.touch_end_point.y - g_gallery.touch_start_point.y;
                
                // 判断是否为有效的右滑手势
                if (dx > 50 && abs(dy) < 100) {  // 右滑超过50像素，垂直偏移小于100像素
                    release_gallery_resources();
                    extern void desktop(void);
                    desktop();
                }
            }
            g_gallery.touch_started = false;
        }
    }
}

/**
 * @brief 缩略图界面手势事件回调
 * 
 * 功能说明：
 * 1. 检测右滑手势
 * 2. 右滑返回桌面
 * 
 * @param e 事件指针
 */
void gallery_gesture_cb(lv_event_t * e)
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
        // 右滑退出到桌面主界面
        extern void desktop(void); // 声明外部桌面函数
        desktop(); // 调用桌面函数，返回主界面
    }
}

/**
 * @brief 图片查看器触摸事件回调
 * 
 * 功能说明：
 * 1. 检测滑动切换图片
 * 2. 点击返回缩略图
 * 3. 记录触摸点
 */
void image_touch_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_indev_t * indev = lv_indev_get_act();
    
    if (indev == NULL) return;
    
    lv_point_t point;
    lv_indev_get_point(indev, &point);
    
    switch(code) {
        case LV_EVENT_PRESSED:
            // 记录触摸开始点
            g_gallery.touch_started = true;
            g_gallery.swipe_detected = false;
            g_gallery.touch_start_point = point;
            break;
            
        case LV_EVENT_RELEASED:
            if (g_gallery.touch_started) {
                // 记录触摸结束点并计算位移
                g_gallery.touch_end_point = point;
                int dx = g_gallery.touch_end_point.x - g_gallery.touch_start_point.x;
                
                // 检测滑动手势（X轴位移大于50像素）
                if (abs(dx) > 50) {
                    g_gallery.swipe_detected = true;
                    if (dx > 0) {
                        // 右滑 - 下一张图片
                        load_next_image();
                    } else {
                        // 左滑 - 上一张图片
                        load_prev_image();
                    }
                }
                g_gallery.touch_started = false;
            }
            break;
            
        case LV_EVENT_CLICKED:
            // 只有在没有检测到滑动时才处理点击事件
            if (!g_gallery.swipe_detected) {
                show_gallery_thumbnail_screen();
            }
            g_gallery.swipe_detected = false;
            break;
            
        default:
            break;
    }
}

/**
 * @brief 信息条自动隐藏定时器回调
 * 
 * 功能说明：
 * 3秒后自动隐藏图片信息条
 * 
 * @param timer 定时器指针
 */
void hide_info_timer_cb(lv_timer_t * timer)
{
    lv_obj_t * info_container = (lv_obj_t *)lv_timer_get_user_data(timer);
    if (info_container != NULL) {
        lv_obj_add_flag(info_container, LV_OBJ_FLAG_HIDDEN);
    }
    
    // 清除全局定时器引用
    if (g_gallery.hide_timer == timer) {
        g_gallery.hide_timer = NULL;
    }
    
    lv_timer_delete(timer);
}
