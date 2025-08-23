#include "../inc/dcim.h"
#include "../inc/main_screen.h"

// ============ ���ȫ�ֱ��� ============
// ��������ȫ�ֱ���������ͼƬ�б��״̬
gallery_manager_t g_gallery = {0};

/**
 * @brief ���ذ�ť�¼��ص�
 * 
 * ����˵����
 * 1. �ͷ������Դ
 * 2. ��������
 * 
 * @param e �¼�ָ��
 */
void dcim_back_btn_event_cb(lv_event_t * e)
{
    // �ͷ������Դ
    release_gallery_resources();
    
    // ��������
    desktop();
}

/**
 * @brief �ͷ������Դ
 * 
 * ����˵����
 * 1. ɾ��������Ϣ��ʱ��
 * 2. ɾ������ͼ�Ͳ鿴������
 * 3. ���ָ���״̬
 */
void release_gallery_resources(void)
{
    // ɾ��������Ϣ��ʱ��
    if (g_gallery.hide_timer != NULL) {
        lv_timer_delete(g_gallery.hide_timer);
        g_gallery.hide_timer = NULL;
    }
    
    // ɾ������ͼ����
    if (g_gallery.thumbnail_screen != NULL) {
        lv_obj_del(g_gallery.thumbnail_screen);
        g_gallery.thumbnail_screen = NULL;
    }
    
    if (g_gallery.viewer_screen != NULL) {
        lv_obj_del(g_gallery.viewer_screen);
        g_gallery.viewer_screen = NULL;
    }
    
    // ��ն���ָ��
    g_gallery.current_img = NULL;
    g_gallery.info_container = NULL;
    g_gallery.info_label = NULL;
    
    // ��մ���״̬
    g_gallery.touch_started = false;
    g_gallery.swipe_detected = false;
}

/**
 * @brief ��ʼ�����
 * 
 * ����˵����
 * 1. �������������ṹ�壬ȷ����ʼ״̬�ɾ�
 * 2. ���õ�ǰ�鿴��ͼƬ����Ϊ0
 * 3. ɨ��ָ��Ŀ¼�µ�����ͼƬ�ļ�
 * 
 * ��������ڳ�������ʱ���ã�����׼����Ṧ���������������
 */
void init_gallery(void)
{
    // ʹ��memset���������ṹ�壬ȷ�����г�Ա���ǳ�ʼֵ
    memset(&g_gallery, 0, sizeof(gallery_manager_t));
    
    // ���õ�ǰ�鿴��ͼƬ����Ϊ��һ�ţ�����0��
    g_gallery.current_index = 0;
    
    // ��ʼ������״̬
    g_gallery.touch_started = false;
    g_gallery.swipe_detected = false;
    g_gallery.hide_timer = NULL;
    
    // ɨ��ͼƬ�ļ�����ָ��Ŀ¼��������֧�ָ�ʽ��ͼƬ�ļ�
    scan_image_files("/home/tyc/work_station/dcim");
}

/**
 * @brief ɨ��ͼƬ�ļ�
 * 
 * ����˵����
 * 1. ��ָ��Ŀ¼
 * 2. ����Ŀ¼�е������ļ�
 * 3. ɸѡ��֧�ֵ�ͼƬ��ʽ�ļ���jpg��jpeg��png��bmp��gif��
 * 4. ��ͼƬ�ļ���Ϣ��ӵ�ͼƬ�б���
 * 5. ����ÿ��ͼƬ�ļ�������·��
 * 
 * @param directory Ҫɨ���Ŀ¼·��
 */
void scan_image_files(const char* directory)
{
    DIR *dir;                    // Ŀ¼���
    struct dirent *entry;        // Ŀ¼��ṹ�壬�����ļ���Ϣ
    char full_path[MAX_PATH_LEN]; // �洢�����ļ�·���Ļ�����
    
    // ���Դ�ָ��Ŀ¼
    dir = opendir(directory);
    if (dir == NULL) {
        // ���Ŀ¼��ʧ�ܣ�����
        return;
    }
    
    // ��ʼ��ͼƬ�ļ�������
    g_gallery.image_count = 0;
    
    // ����Ŀ¼�е������ļ���ֱ��û�и����ļ���ﵽ����ļ�������
    while ((entry = readdir(dir)) != NULL && g_gallery.image_count < MAX_IMAGE_COUNT) {
        // ���������ļ���Ŀ¼����'.'��ͷ���ļ���
        if (entry->d_name[0] == '.') continue;
        
        // ����ļ��Ƿ�Ϊ֧�ֵ�ͼƬ��ʽ
        if (is_image_file(entry->d_name)) {
            // ��ȡ��ǰͼƬ�ļ���Ϣ�ṹ���ָ��
            image_info_t *info = &g_gallery.image_list[g_gallery.image_count];
            
            // �����ļ������ṹ���У�ȷ���ַ���������
            strncpy(info->filename, entry->d_name, MAX_NAME_LEN - 1);
            info->filename[MAX_NAME_LEN - 1] = '\0';
            
            // �����������ļ�·����Ŀ¼·�� + "/" + �ļ��� (Linux·��)
            snprintf(full_path, sizeof(full_path), "%s/%s", directory, entry->d_name);
            strncpy(info->filepath, full_path, MAX_PATH_LEN - 1);
            info->filepath[MAX_PATH_LEN - 1] = '\0';
            
            // ͼƬ�ļ���������
            g_gallery.image_count++;
        }
    }
    
    // �ر�Ŀ¼������ͷ�ϵͳ��Դ
    closedir(dir);
}

/**
 * @brief ����Ƿ�ΪͼƬ�ļ�
 * 
 * ����˵����
 * 1. ��ȡ�ļ�����չ��
 * 2. ��֧�ֵ�ͼƬ��ʽ�б���бȽ�
 * 3. �����Ƿ�Ϊ֧�ֵ�ͼƬ��ʽ
 * 
 * ֧�ֵĸ�ʽ��.jpg, .jpeg, .png, .bmp, .gif�������ִ�Сд��
 * 
 * @param filename Ҫ�����ļ���
 * @return 1��ʾ��ͼƬ�ļ���0��ʾ����ͼƬ�ļ�
 */
int is_image_file(const char* filename)
{
    // �����ļ��������һ��'.'��λ�ã���չ���ָ�����
    const char* ext = strrchr(filename, '.');
    if (!ext) return 0; // ���û����չ��������ͼƬ�ļ�
    
    // �����ִ�Сд�رȽ���չ����֧�ֵ�ͼƬ��ʽ
    // strcasecmp�������д�Сд�����е��ַ����Ƚ�
    return (strcasecmp(ext, ".jpg") == 0 ||   // JPEG��ʽ
            strcasecmp(ext, ".jpeg") == 0 ||  // JPEG��ʽ��������չ����
            strcasecmp(ext, ".png") == 0 ||   // PNG��ʽ
            strcasecmp(ext, ".bmp") == 0 ||   // BMP��ʽ
            strcasecmp(ext, ".gif") == 0);    // GIF��ʽ
}

/**
 * @brief ��ʾ�������ͼ����
 * 
 * ����˵����
 * 1. ����ǰ��Ļ�ϵ����ж���
 * 2. ���ý������ָ�룬��ֹ������Ч�ڴ�
 * 3. ��������ͼ����ĸ�����
 * 4. ���ñ���ͼƬ
 * 5. �������֧�֣��һ��˳���
 * 6. ��������ͼ��������
 * 7. ��������ͼ����
 * 
 * ����ṹ��
 * - thumbnail_screen (������)
 *   - bg_img (����ͼƬ)
 *   - container (��͸����ɫ����)
 *     - scroll_container (�ɹ�������)
 *       - ����ͼ��ť����
 */
void show_gallery_thumbnail_screen(void)
{
    // ========== ��������� ==========
    // ����ǰ��Ļ�ϵ�����LVGL�����ͷ��ڴ�
    lv_obj_clean(lv_screen_active());
    
    // ���ö���ָ��ΪNULL����ֹ�������ͷŵ��ڴ�
    g_gallery.viewer_screen = NULL;
    g_gallery.current_img = NULL;
    g_gallery.info_container = NULL;
    g_gallery.info_label = NULL;
    
    // ɾ����ʱ����������ڣ�
    if (g_gallery.hide_timer != NULL) {
        lv_timer_delete(g_gallery.hide_timer);
        g_gallery.hide_timer = NULL;
    }
    
    // ========== �������������� ==========
    // ��������ͼ����ĸ�������ռ��������Ļ
    g_gallery.thumbnail_screen = lv_obj_create(lv_screen_active());
    lv_obj_set_size(g_gallery.thumbnail_screen, 800, 480);  // ����Ϊ��Ļ��С
    lv_obj_set_pos(g_gallery.thumbnail_screen, 0, 0);       // λ������Ļ���Ͻ�
    lv_obj_set_style_pad_all(g_gallery.thumbnail_screen, 0, LV_PART_MAIN); // ���ڱ߾�
    lv_obj_set_style_border_width(g_gallery.thumbnail_screen, 0, LV_PART_MAIN); // �ޱ߿�
    
    // ========== ���ñ���ͼƬ ==========
    // ��������ͼƬ������ʾ���ı���ͼ
    lv_obj_t * bg_img = lv_image_create(g_gallery.thumbnail_screen);
    lv_obj_set_size(bg_img, 800, 480);  // ����ͼƬ������Ļ
    lv_obj_set_pos(bg_img, 0, 0);       // λ�������Ͻ�
    lv_image_set_src(bg_img, "/home/tyc/work_station/dcim/dcim_back.jpg"); // ����ͼƬԴ
    
    // ========== �������֧�� ==========
    // ����һ������¼��ص����û��һ������˳�������Ļ
    lv_obj_add_event_cb(g_gallery.thumbnail_screen, gallery_gesture_cb, LV_EVENT_GESTURE, NULL);
    lv_obj_add_flag(g_gallery.thumbnail_screen, LV_OBJ_FLAG_GESTURE_BUBBLE); // ���������¼�ð��
    
    lv_obj_add_event_cb(bg_img, gallery_gesture_cb, LV_EVENT_GESTURE, NULL);
    lv_obj_add_flag(bg_img, LV_OBJ_FLAG_GESTURE_BUBBLE);
    
    // ����ֶ����Ƽ�� - �������º��ͷ��¼�
    lv_obj_add_event_cb(g_gallery.thumbnail_screen, gallery_touch_cb, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(g_gallery.thumbnail_screen, gallery_touch_cb, LV_EVENT_RELEASED, NULL);
    lv_obj_add_event_cb(bg_img, gallery_touch_cb, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(bg_img, gallery_touch_cb, LV_EVENT_RELEASED, NULL);
    
    // ========== ��������ͼ���� ==========
    // ����ͼ����
    lv_obj_t * container = lv_obj_create(g_gallery.thumbnail_screen);
    lv_obj_set_size(container, 760, 460);
    lv_obj_set_pos(container, 20, 10);
    lv_obj_set_style_bg_opa(container, LV_OPA_80, LV_PART_MAIN);  // 80%͸����
    lv_obj_set_style_bg_color(container, lv_color_hex(0x000000), LV_PART_MAIN); // ��ɫ����
    lv_obj_set_style_border_width(container, 0, LV_PART_MAIN);    // �ޱ߿�
    lv_obj_set_style_radius(container, 10, LV_PART_MAIN);         // Բ��
    lv_obj_set_style_pad_all(container, 10, LV_PART_MAIN);        // �ڱ߾�
    
    // Ϊ����ͼ����������Ƽ�⣬ȷ�������ܹ�����
    lv_obj_add_event_cb(container, gallery_touch_cb, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(container, gallery_touch_cb, LV_EVENT_RELEASED, NULL);
    lv_obj_add_flag(container, LV_OBJ_FLAG_GESTURE_BUBBLE);  // ȷ���¼��ܹ�ð��
    
    // ========== ������������ ==========
    // �����ɹ��������������ڷ�������ͼ���񣨵�ͼƬ�ܶ�ʱ���Թ�����
    lv_obj_t * scroll_container = lv_obj_create(container);
    lv_obj_set_size(scroll_container, 740, 440);
    lv_obj_set_pos(scroll_container, 0, 0);       // λ���ڸ��������Ͻ�
    lv_obj_set_style_bg_opa(scroll_container, LV_OPA_TRANSP, LV_PART_MAIN); // ͸������
    lv_obj_set_style_border_width(scroll_container, 0, LV_PART_MAIN);       // �ޱ߿�
    lv_obj_set_scroll_dir(scroll_container, LV_DIR_VER);  // ֻ����ֱ����
    
    // Ϊ��������������Ƽ�⣬ȷ�������ܹ�����
    lv_obj_add_event_cb(scroll_container, gallery_touch_cb, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(scroll_container, gallery_touch_cb, LV_EVENT_RELEASED, NULL);
    lv_obj_add_flag(scroll_container, LV_OBJ_FLAG_GESTURE_BUBBLE);  // ȷ���¼��ܹ�ð��
    
    // ========== ��������ͼ���� ==========
    create_thumbnail_grid(scroll_container);
}

/**
 * @brief ��������ͼ����
 * 
 * ����˵����
 * 1. ����ͼƬ�б�
 * 2. ��������ͼ��ť��ͼƬ
 * 3. ���õ���¼�
 * 4. ������ͣЧ��
 * 5. ��������ͼ
 * 
 * @param parent ������
 */
void create_thumbnail_grid(lv_obj_t* parent)
{
    if (g_gallery.image_count == 0) {
        // ��ʾ��ͼƬ��ʾ
        lv_obj_t * no_images_label = lv_label_create(parent);
        lv_label_set_text(no_images_label, "No images found");
        lv_obj_set_style_text_color(no_images_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
        lv_obj_set_style_text_font(no_images_label, &lv_font_montserrat_16, LV_PART_MAIN);
        lv_obj_center(no_images_label);
        return;
    }
    
    int x_offset = 55;  // ��68����Ϊ55�������ƶ�13����
    int y_offset = 10;
    int spacing = 15;
    
    for (int i = 0; i < g_gallery.image_count; i++) {
        int row = i / THUMBNAILS_PER_ROW;
        int col = i % THUMBNAILS_PER_ROW;
        
        // ����λ��
        int x = x_offset + col * (THUMBNAIL_WIDTH + spacing);
        int y = y_offset + row * (THUMBNAIL_HEIGHT + spacing);
        
        // ��������ͼ����
        lv_obj_t * thumb_container = lv_obj_create(parent);
        lv_obj_set_size(thumb_container, THUMBNAIL_WIDTH, THUMBNAIL_HEIGHT);
        lv_obj_set_pos(thumb_container, x, y);
        lv_obj_set_style_bg_color(thumb_container, lv_color_hex(0x34495e), LV_PART_MAIN);
        lv_obj_set_style_radius(thumb_container, 5, LV_PART_MAIN);
        lv_obj_set_style_border_width(thumb_container, 2, LV_PART_MAIN);
        lv_obj_set_style_border_color(thumb_container, lv_color_hex(0x2c3e50), LV_PART_MAIN);
        lv_obj_set_style_pad_all(thumb_container, 2, LV_PART_MAIN);
        
        // ����ͼƬ������ʾ����ͼ
        lv_obj_t * thumb_img = lv_image_create(thumb_container);
        lv_obj_set_size(thumb_img, THUMBNAIL_WIDTH - 8, THUMBNAIL_HEIGHT - 8);
        lv_obj_center(thumb_img);
        
        // ���Լ���ͼƬ�����ʧ������ʾռλ��
        lv_image_set_src(thumb_img, g_gallery.image_list[i].filepath);
        
        // ����ͼƬ����ģʽ�����ֱ���
        lv_obj_set_style_image_opa(thumb_img, LV_OPA_COVER, LV_PART_MAIN);
        
        // ��ӵ���¼�������
        lv_obj_set_user_data(thumb_container, (void*)(uintptr_t)i);
        lv_obj_add_event_cb(thumb_container, thumbnail_click_cb, LV_EVENT_CLICKED, NULL);
        lv_obj_add_flag(thumb_container, LV_OBJ_FLAG_CLICKABLE);
        
        // �����ͣЧ��
        lv_obj_set_style_bg_color(thumb_container, lv_color_hex(0x3498db), LV_PART_MAIN | LV_STATE_PRESSED);
    }
}

/**
 * @brief ��ʾͼƬ�鿴������
 * 
 * ����˵����
 * 1. ����ǰ��Ļ
 * 2. ����ȫ��ͼƬ��ʾ
 * 3. ��ʾͼƬ��Ϣ
 * 4. ������ƺ͵���¼�
 * 5. ��Ϣ���Զ�����
 * 
 * @param index ͼƬ����
 */
void show_image_viewer_screen(int index)
{
    if (index < 0 || index >= g_gallery.image_count) return;
    
    g_gallery.current_index = index;
    
    // ����ǰ��Ļ
    lv_obj_clean(lv_screen_active());
    
    // ɾ����ʱ����������ڣ�
    if (g_gallery.hide_timer != NULL) {
        lv_timer_delete(g_gallery.hide_timer);
        g_gallery.hide_timer = NULL;
    }
    
    // ���ö���ָ��
    g_gallery.thumbnail_screen = NULL;
    
    // ����ȫ���鿴������
    g_gallery.viewer_screen = lv_obj_create(lv_screen_active());
    lv_obj_set_size(g_gallery.viewer_screen, 800, 480);
    lv_obj_set_pos(g_gallery.viewer_screen, 0, 0);
    lv_obj_set_style_bg_color(g_gallery.viewer_screen, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_pad_all(g_gallery.viewer_screen, 0, LV_PART_MAIN);
    lv_obj_set_style_border_width(g_gallery.viewer_screen, 0, LV_PART_MAIN);
    
    // ����ȫ��ͼƬ��ʾ����
    g_gallery.current_img = lv_image_create(g_gallery.viewer_screen);
    lv_obj_set_size(g_gallery.current_img, 800, 480);
    lv_obj_set_pos(g_gallery.current_img, 0, 0);
    
    // ���ز���ʾ��ǰͼƬ
    lv_image_set_src(g_gallery.current_img, g_gallery.image_list[index].filepath);
    
    // ����ͼƬ������ʾ�����ֱ���
    lv_obj_center(g_gallery.current_img);
    
    // ��Ӵ����¼�����
    lv_obj_add_event_cb(g_gallery.current_img, image_touch_cb, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(g_gallery.current_img, image_touch_cb, LV_EVENT_RELEASED, NULL);
    lv_obj_add_event_cb(g_gallery.current_img, image_touch_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_add_flag(g_gallery.current_img, LV_OBJ_FLAG_CLICKABLE);
    
    lv_obj_add_event_cb(g_gallery.viewer_screen, image_touch_cb, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(g_gallery.viewer_screen, image_touch_cb, LV_EVENT_RELEASED, NULL);
    lv_obj_add_event_cb(g_gallery.viewer_screen, image_touch_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_add_flag(g_gallery.viewer_screen, LV_OBJ_FLAG_CLICKABLE);
    
    // ����ͼƬ��Ϣ��ʾ����͸�����ǲ㣬��ѡ��ʾ��
    g_gallery.info_container = lv_obj_create(g_gallery.viewer_screen);
    lv_obj_set_size(g_gallery.info_container, 300, 60);
    lv_obj_align(g_gallery.info_container, LV_ALIGN_BOTTOM_MID, 0, -10);
    lv_obj_set_style_bg_color(g_gallery.info_container, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(g_gallery.info_container, LV_OPA_70, LV_PART_MAIN);
    lv_obj_set_style_border_width(g_gallery.info_container, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(g_gallery.info_container, 10, LV_PART_MAIN);
    lv_obj_set_style_pad_all(g_gallery.info_container, 8, LV_PART_MAIN);
    
    // ��ʾͼƬ��Ϣ
    g_gallery.info_label = lv_label_create(g_gallery.info_container);
    char info_text[256];
    snprintf(info_text, sizeof(info_text), "%d / %d - %s", 
             index + 1, g_gallery.image_count, g_gallery.image_list[index].filename);
    lv_label_set_text(g_gallery.info_label, info_text);
    lv_obj_set_style_text_color(g_gallery.info_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_font(g_gallery.info_label, &lv_font_montserrat_12, LV_PART_MAIN);
    lv_obj_set_style_text_align(g_gallery.info_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_obj_center(g_gallery.info_label);
    
    // ����Ϣ��3����Զ�����
    g_gallery.hide_timer = lv_timer_create(hide_info_timer_cb, 3000, g_gallery.info_container);
    lv_timer_set_repeat_count(g_gallery.hide_timer, 1);  // ִֻ��һ��
}

/**
 * @brief ������һ��ͼƬ
 * 
 * ����˵����
 * 1. �л�����һ��ͼƬ
 * 2. ����ͼƬ��ʾ
 * 3. ѭ���л�
 */
void load_next_image(void)
{
    if (g_gallery.image_count == 0) return;
    
    g_gallery.current_index = (g_gallery.current_index + 1) % g_gallery.image_count;
    update_viewer_image();
}

/**
 * @brief ������һ��ͼƬ
 * 
 * ����˵����
 * 1. �л�����һ��ͼƬ
 * 2. ����ͼƬ��ʾ
 * 3. ѭ���л�
 */
void load_prev_image(void)
{
    if (g_gallery.image_count == 0) return;
    
    g_gallery.current_index = (g_gallery.current_index - 1 + g_gallery.image_count) % g_gallery.image_count;
    update_viewer_image();
}

/**
 * @brief ����ͼƬ�鿴������
 * 
 * ����˵����
 * 1. ����ͼƬ����Ϣ��ʾ
 * 2. ��������Ч��
 * 3. ��ʾ��Ϣ����
 * 4. �л�ͼƬʱ����
 * 
 * @param index ��ǰͼƬ����
 */
void update_viewer_image(void)
{
    // �������Ƿ����
    if (g_gallery.current_img == NULL || g_gallery.info_label == NULL || 
        g_gallery.viewer_screen == NULL || g_gallery.info_container == NULL) {
        show_image_viewer_screen(g_gallery.current_index);
        return;
    }
    
    // ���������Ч��
    if (g_gallery.current_index < 0 || g_gallery.current_index >= g_gallery.image_count) {
        return;
    }
    
    // ����ͼƬ����Ϣ��ʾ
    lv_image_set_src(g_gallery.current_img, g_gallery.image_list[g_gallery.current_index].filepath);
    
    char info_text[256];
    snprintf(info_text, sizeof(info_text), "%d / %d - %s", 
             g_gallery.current_index + 1, g_gallery.image_count, 
             g_gallery.image_list[g_gallery.current_index].filename);
    lv_label_set_text(g_gallery.info_label, info_text);
    
    // ��ʾ��Ϣ����
    if (g_gallery.info_container != NULL) {
        lv_obj_remove_flag(g_gallery.info_container, LV_OBJ_FLAG_HIDDEN);
    }
}

/**
 * @brief ��������ͼ����
 * 
 * ����˵����
 * ֱ����ʾ����ͼ����
 */
void back_to_thumbnail(void)
{
    show_gallery_thumbnail_screen();
}

// ============ �¼��ص����� ============
// ���º��������û��ĸ��ֽ�������

/**
 * @brief ����ͼ����¼��ص�
 * 
 * ����˵����
 * 1. ��ȡ���������ͼ����
 * 2. ��ʾ��ӦͼƬ�鿴��
 * 3. ͨ���û����ݴ�������
 * 
 * @param e �¼�ָ��
 */
void thumbnail_click_cb(lv_event_t * e)
{
    // ��ȡ�����¼����������󣨱����������ͼ������
    lv_obj_t * container = lv_event_get_target(e);
    
    // ���������û�������ȡ��ͼƬ����
    // �ڴ�������ͼʱ�����ǽ������洢���������û�������
    int index = (int)(uintptr_t)lv_obj_get_user_data(container);
    
    // ��ʾ��Ӧ������ͼƬ�鿴������
    show_image_viewer_screen(index);
}

/**
 * @brief �ֶ����Ƽ��ص����� - ����ͼ����
 * 
 * ͨ���������º��ͷ��¼��ֶ��������
 */
void gallery_touch_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    
    if (code == LV_EVENT_PRESSED) {
        // ��¼������ʼ��
        lv_indev_t * indev = lv_indev_get_act();
        if (indev) {
            lv_indev_get_point(indev, &g_gallery.touch_start_point);
            g_gallery.touch_started = true;
        }
    }
    else if (code == LV_EVENT_RELEASED) {
        if (g_gallery.touch_started) {
            // ��������
            lv_indev_t * indev = lv_indev_get_act();
            if (indev) {
                lv_indev_get_point(indev, &g_gallery.touch_end_point);
                
                int dx = g_gallery.touch_end_point.x - g_gallery.touch_start_point.x;
                int dy = g_gallery.touch_end_point.y - g_gallery.touch_start_point.y;
                
                // �ж��Ƿ�Ϊ��Ч���һ�����
                if (dx > 50 && abs(dy) < 100) {  // �һ�����50���أ���ֱƫ��С��100����
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
 * @brief ����ͼ���������¼��ص�
 * 
 * ����˵����
 * 1. ����һ�����
 * 2. �һ���������
 * 
 * @param e �¼�ָ��
 */
void gallery_gesture_cb(lv_event_t * e)
{
    // ��ȡ��ǰ������豸
    lv_indev_t * indev = lv_indev_get_act();
    if (indev == NULL) {
        return;
    }

    // ��ȡ��ǰ������豸�����Ʒ���
    lv_dir_t gesture = lv_indev_get_gesture_dir(indev);

    // ����Ƿ�Ϊ�һ�����
    if (gesture == LV_DIR_RIGHT) {
        // �һ��˳�������������
        extern void desktop(void); // �����ⲿ���溯��
        desktop(); // �������溯��������������
    }
}

/**
 * @brief ͼƬ�鿴�������¼��ص�
 * 
 * ����˵����
 * 1. ��⻬���л�ͼƬ
 * 2. �����������ͼ
 * 3. ��¼������
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
            // ��¼������ʼ��
            g_gallery.touch_started = true;
            g_gallery.swipe_detected = false;
            g_gallery.touch_start_point = point;
            break;
            
        case LV_EVENT_RELEASED:
            if (g_gallery.touch_started) {
                // ��¼���������㲢����λ��
                g_gallery.touch_end_point = point;
                int dx = g_gallery.touch_end_point.x - g_gallery.touch_start_point.x;
                
                // ��⻬�����ƣ�X��λ�ƴ���50���أ�
                if (abs(dx) > 50) {
                    g_gallery.swipe_detected = true;
                    if (dx > 0) {
                        // �һ� - ��һ��ͼƬ
                        load_next_image();
                    } else {
                        // �� - ��һ��ͼƬ
                        load_prev_image();
                    }
                }
                g_gallery.touch_started = false;
            }
            break;
            
        case LV_EVENT_CLICKED:
            // ֻ����û�м�⵽����ʱ�Ŵ������¼�
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
 * @brief ��Ϣ���Զ����ض�ʱ���ص�
 * 
 * ����˵����
 * 3����Զ�����ͼƬ��Ϣ��
 * 
 * @param timer ��ʱ��ָ��
 */
void hide_info_timer_cb(lv_timer_t * timer)
{
    lv_obj_t * info_container = (lv_obj_t *)lv_timer_get_user_data(timer);
    if (info_container != NULL) {
        lv_obj_add_flag(info_container, LV_OBJ_FLAG_HIDDEN);
    }
    
    // ���ȫ�ֶ�ʱ������
    if (g_gallery.hide_timer == timer) {
        g_gallery.hide_timer = NULL;
    }
    
    lv_timer_delete(timer);
}
