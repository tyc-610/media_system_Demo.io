#include "../inc/kugou_music.h"
#include "../inc/main_screen.h"

// ============ ȫ�ֱ������� ============// ȫ�����ֲ�����ʵ�����洢�������ֲ�����ص�״̬������
music_player_t g_music_player = {0};

// ============ �������ָ�� ============

/**
 * @brief ��������Ӧ��
 * 
 * ����˵����
 * 1. ��ʼ�����ֲ�����
 * 2. ��������ļ�����
 * 3. ���û�������ļ�����ʾ��ʾ��Ϣ
 * 4. ������ʾ�����б����
 */
void start_music_app(void)
{
    // ��ʼ�����ֲ�����
    init_music_player();
    
    // ��������ļ�����
    if (g_music_player.music_count == 0) {
        lv_obj_t * msg_box = lv_msgbox_create(lv_screen_active());
        lv_msgbox_add_title(msg_box, "KuGou Music");
        lv_msgbox_add_text(msg_box, "No music files found in /home/tyc/work_station/music/");
        lv_msgbox_add_close_button(msg_box);
        return;
    }
    
    // ��ʾ�����б����
    show_music_list_screen();
}
// ��Щ��̬�������ڴ洢LVGL��������ָ�룬�����ڲ�ͬ��������ʺ͸��½���
static lv_obj_t* music_list_screen = NULL;    // �����б����ĸ�����
static lv_obj_t* music_player_screen = NULL;  // ���ֲ��Ž���ĸ�����
static lv_obj_t* progress_bar = NULL;         // �������ؼ�
static lv_obj_t* progress_label = NULL;       // ʱ����ʾ��ǩ
static lv_obj_t* song_title_label = NULL;     // ���������ǩ
static lv_obj_t* song_artist_label = NULL;    // �����ұ�ǩ
static lv_obj_t* play_pause_btn = NULL;       // ����/��ͣ��ť
static lv_obj_t* mode_btn = NULL;             // ����ģʽ��ť
static lv_timer_t* progress_timer = NULL;     // ���ȸ��¶�ʱ��
static int current_progress = 0;              // ��ǰ�������ȣ��룩

// ���Ƽ�����
static bool touch_started = false;
static lv_point_t touch_start_point = {0, 0};
static uint32_t touch_start_time = 0;

// ��������
void release_music_resources(void);

/**
 * @brief ���ذ�ť�¼�������
 * 
 * ����˵����
 * 1. �ͷ�����Ӧ����Դ
 * 2. �����������
 * 
 * @param e �¼�����
 */
void back_btn_event_cb(lv_event_t * e)
{
    // �ͷ�����Ӧ����Դ
    release_music_resources();
    
    // �����������
    desktop();
}

/**
 * @brief �ͷ�����Ӧ����Դ
 * 
 * ����˵����
 * 1. ֹͣ���ȶ�ʱ��
 * 2. �������ֲ�����ض���
 * 3. ����ȫ�ֱ���
 */
void release_music_resources(void)
{
    // ֹͣ���ȶ�ʱ��
    if (progress_timer != NULL) {
        lv_timer_del(progress_timer);
        progress_timer = NULL;
    }
    
    // �������ֲ�����ض���
    if (music_list_screen != NULL) {
        lv_obj_del(music_list_screen);
        music_list_screen = NULL;
    }
    
    if (music_player_screen != NULL) {
        lv_obj_del(music_player_screen);
        music_player_screen = NULL;
    }
    
    // ���������������ָ��
    progress_bar = NULL;
    progress_label = NULL;
    song_title_label = NULL;
    song_artist_label = NULL;
    play_pause_btn = NULL;
    mode_btn = NULL;
    
    // ���ý���
    current_progress = 0;
}

/**
 * @brief ��ʼ�����ֲ�����
 * 
 * ����˵����
 * 1. �������ֲ������ṹ�壬ȷ����ʼ״̬�ɾ�
 * 2. ����Ĭ�ϲ���ģʽΪ˳�򲥷�
 * 3. ���ó�ʼ����״̬Ϊֹͣ
 * 4. ��ʼ����ǰ��������Ϊ0
 * 5. ��ʼ������������IDΪ-1����ʾû�в��Ž��̣�
 * 6. ɨ��ָ��Ŀ¼�µ������ļ�
 */
void init_music_player(void)
{
    // ʹ��memset���������ṹ�壬ȷ�����г�Ա���ǳ�ʼֵ
    memset(&g_music_player, 0, sizeof(music_player_t));
    
    // ���ò������Ļ�������
    g_music_player.play_mode = PLAY_MODE_SEQUENCE;  // Ĭ��˳�򲥷�
    g_music_player.state = MUSIC_STATE_STOP;        // ��ʼ״̬Ϊֹͣ
    g_music_player.current_index = 0;               // ��ǰ���ŵ���������
    g_music_player.player_pid = -1;                 // ����������ID��-1��ʾ�޽���
    
    // ɨ�������ļ�����ָ��Ŀ¼��������MP3�ļ��������б�
    scan_music_files("/home/tyc/work_station/music");
}

/**
 * @brief ɨ�������ļ�
 * 
 * ����˵����
 * 1. ��ָ��Ŀ¼
 * 2. ����Ŀ¼�е������ļ�
 * 3. ɸѡ��MP3��ʽ�������ļ�
 * 4. �������ļ���Ϣ��ӵ������б���
 * 5. ����ÿ�������ļ��Ļ�����Ϣ�����⡢�����ҵȣ�
 * 
 * @param directory Ҫɨ���Ŀ¼·��
 */
void scan_music_files(const char* directory)
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
    
    // ��ʼ�������ļ�������
    g_music_player.music_count = 0;
    
    // ����Ŀ¼�е������ļ���ֱ��û�и����ļ���ﵽ����ļ�������
    while ((entry = readdir(dir)) != NULL && g_music_player.music_count < MAX_MUSIC_COUNT) {
        // ���������ļ���Ŀ¼����'.'��ͷ���ļ���
        if (entry->d_name[0] == '.') continue;
        
        // ����ļ���չ����ֻ����MP3�ļ�
        char *ext = strrchr(entry->d_name, '.'); // �����ļ��������һ��'.'
        if (ext && strcasecmp(ext, ".mp3") == 0) { // �����ִ�Сд�Ƚ���չ��
            
            // ��ȡ��ǰ�����ļ���Ϣ�ṹ���ָ��
            music_info_t *info = &g_music_player.music_list[g_music_player.music_count];
            
            // �����ļ������ṹ���У�ȷ���ַ���������
            strncpy(info->filename, entry->d_name, MAX_NAME_LEN - 1);
            info->filename[MAX_NAME_LEN - 1] = '\0';
            
            // �����������ļ�·����Ŀ¼·�� + "/" + �ļ���
            snprintf(full_path, sizeof(full_path), "%s/%s", directory, entry->d_name);
            strncpy(info->filepath, full_path, MAX_PATH_LEN - 1);
            info->filepath[MAX_PATH_LEN - 1] = '\0';
            
            // ����������Ϣ�����ļ�����ȡ���⡢����Ĭ�������ҵ�
            parse_music_info(info);
            
            // �����ļ���������
            g_music_player.music_count++;
        }
    }
    
    // �ر�Ŀ¼������ͷ�ϵͳ��Դ
    closedir(dir);
}

/**
 * @brief ����������Ϣ
 * 
 * ����˵����
 * 1. ���ļ�������ȡ�������⣨ȥ��.mp3��չ����
 * 2. ����Ĭ�ϵ������Һ�ר����Ϣ
 * 
 * ע�⣺���Ǽ򻯰汾�Ľ���������ʵ��Ӧ���п���ʹ��ID3��ǩ������
 * ����ȡMP3�ļ���Ƕ��Ԫ������Ϣ
 * 
 * @param info ָ��������Ϣ�ṹ���ָ��
 */
void parse_music_info(music_info_t* info)
{
    // �����ļ�������ʱ��������Ϊ��Ҫ�޸ģ�ȥ����չ����
    char *name_copy = strdup(info->filename); // strdup�����ڴ沢�����ַ���
    
    // �����ļ��������һ��'.'��λ�ã���չ���ָ�����
    char *dot = strrchr(name_copy, '.');
    if (dot) {
        *dot = '\0';  // ��'.'�滻Ϊ�ַ�����������ȥ����չ��
    }
    
    // ���������ļ�����������չ������Ϊ��������
    strncpy(info->title, name_copy, MAX_NAME_LEN - 1);
    info->title[MAX_NAME_LEN - 1] = '\0'; // ȷ���ַ�����\0��β
    
    // ����Ĭ��ֵ���������Ǽ򻯰汾��û�н���MP3��ǩ������ʹ��Ĭ��ֵ
    strcpy(info->artist, "Unknown Artist"); // Ĭ��������
    strcpy(info->album, "Unknown Album");   // Ĭ��ר��
    
    // �ͷ���ʱ������ڴ棬�����ڴ�й¶
    free(name_copy);
}

/**
 * @brief ��ʾ�����б����
 * 
 * ����˵����
 * 1. ����ǰ��Ļ
 * 2. ���������б����
 * 3. ���ñ���ͼƬ
 * 4. �������ذ�ť
 * 5. ���������б�
 * 6. ���û�������ļ�����ʾ��ʾ��Ϣ
 * 7. ������������б���
 * 8. ��������¼�����
 */

void show_music_list_screen(void)
{
    // ֹ֮ͣǰ�Ķ�ʱ��
    if (progress_timer) {
        lv_timer_del(progress_timer);
        progress_timer = NULL;
    }

    // ����ǰ��Ļ
    lv_obj_clean(lv_screen_active());

    // ���ö���ָ��
    music_player_screen = NULL;
    progress_bar = NULL;
    progress_label = NULL;
    song_title_label = NULL;
    song_artist_label = NULL;
    play_pause_btn = NULL;
    mode_btn = NULL;

    // ���������б����
    music_list_screen = lv_obj_create(lv_screen_active());
    lv_obj_set_size(music_list_screen, 800, 480);
    lv_obj_set_pos(music_list_screen, 0, 0);
    lv_obj_set_style_pad_all(music_list_screen, 0, LV_PART_MAIN);
    lv_obj_set_style_border_width(music_list_screen, 0, LV_PART_MAIN);

    // ���ñ���ͼƬ
    lv_obj_t * bg_img = lv_image_create(music_list_screen);
    lv_obj_set_size(bg_img, 800, 480);
    lv_obj_set_pos(bg_img, 0, 0);
    lv_image_set_src(bg_img, "/home/tyc/work_station/music/music_choseback.jpg");

    // ��ӻ����˳����� - ȷ����ȷ��
    lv_obj_add_event_cb(music_list_screen, music_list_gesture_cb, LV_EVENT_GESTURE, NULL);
    lv_obj_add_flag(music_list_screen, LV_OBJ_FLAG_GESTURE_BUBBLE);

    // Ϊ����ͼƬҲ��������¼���ȷ�����Ʊ�����
    lv_obj_add_event_cb(bg_img, music_list_gesture_cb, LV_EVENT_GESTURE, NULL);
    lv_obj_add_flag(bg_img, LV_OBJ_FLAG_GESTURE_BUBBLE);
    
    // ����ֶ����Ƽ�� - �������º��ͷ��¼�
    lv_obj_add_event_cb(music_list_screen, music_list_touch_cb, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(music_list_screen, music_list_touch_cb, LV_EVENT_RELEASED, NULL);
    lv_obj_add_event_cb(bg_img, music_list_touch_cb, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(bg_img, music_list_touch_cb, LV_EVENT_RELEASED, NULL);
    
    // ���������б�����
    lv_obj_t * list_container = lv_obj_create(music_list_screen);
    lv_obj_set_size(list_container, 760, 460);
    lv_obj_set_pos(list_container, 20, 10);
    lv_obj_set_style_bg_opa(list_container, LV_OPA_80, LV_PART_MAIN);
    lv_obj_set_style_bg_color(list_container, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_border_width(list_container, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(list_container, 10, LV_PART_MAIN);
    lv_obj_set_style_pad_all(list_container, 10, LV_PART_MAIN);

    // // ��������ͼ���ŵ� list_container ��ײ㣬����Ϊ���ɻ��������ɵ��
    // lv_obj_t * list_background = lv_img_create(list_container);
    // lv_obj_set_size(list_background, 740, 440);
    // lv_obj_set_pos(list_background, 0, 0);
    // lv_image_set_src(list_background, "/home/tyc/work_station/music_choseback.jpg");
    // lv_obj_move_background(list_background);
    // lv_obj_clear_flag(list_background, LV_OBJ_FLAG_SCROLLABLE); // ��ֹ����
    // lv_obj_clear_flag(list_background, LV_OBJ_FLAG_CLICKABLE); // ��ֹ���
    // lv_obj_clear_flag(list_background, LV_OBJ_FLAG_GESTURE_BUBBLE); // ��ֹ����ð��
    
    // Ϊ�б�����Ҳ������Ƽ�⣬ȷ�������ܹ�����
    lv_obj_add_event_cb(list_container, music_list_touch_cb, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(list_container, music_list_touch_cb, LV_EVENT_RELEASED, NULL);
    lv_obj_add_flag(list_container, LV_OBJ_FLAG_GESTURE_BUBBLE);  // ȷ���¼��ܹ�ð��
    
    // ���������б�
    lv_obj_t * list = lv_list_create(list_container);
    lv_obj_set_size(list, 740, 440);
    lv_obj_set_pos(list, 0, 0);
    lv_obj_set_style_bg_opa(list, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(list, 0, LV_PART_MAIN);
    
    // Ϊ�б�������Ƽ�⣬ȷ�������ܹ�����
    lv_obj_add_event_cb(list, music_list_touch_cb, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(list, music_list_touch_cb, LV_EVENT_RELEASED, NULL);
    lv_obj_add_flag(list, LV_OBJ_FLAG_GESTURE_BUBBLE);  // ȷ���¼��ܹ�ð��
    
    // ���û�������ļ�����ʾ��ʾ��Ϣ
    if (g_music_player.music_count == 0) {
        lv_obj_t * no_music_label = lv_label_create(list);
        lv_label_set_text(no_music_label, "No music files found");
        lv_obj_set_style_text_color(no_music_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
        lv_obj_set_style_text_font(no_music_label, &lv_font_montserrat_16, LV_PART_MAIN);
        lv_obj_center(no_music_label);
        return;
    }
    
    // ��������б���
    for (int i = 0; i < g_music_player.music_count; i++) {
        music_info_t *info = &g_music_player.music_list[i];
        
        // ֻ��ʾ�������⣨ȥ����չ�����ļ�����
        lv_obj_t * btn = lv_list_add_button(list, LV_SYMBOL_AUDIO, info->title);
        lv_obj_set_style_text_color(btn, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
        lv_obj_set_style_bg_color(btn, lv_color_hex(0x34495e), LV_PART_MAIN);
        lv_obj_set_style_bg_opa(btn, LV_OPA_70, LV_PART_MAIN);
        lv_obj_set_style_text_font(btn, &lv_font_montserrat_14, LV_PART_MAIN);
        
        // ��������Ϊ�û����ݴ���
        lv_obj_set_user_data(btn, (void*)(uintptr_t)i);
        lv_obj_add_event_cb(btn, music_list_item_cb, LV_EVENT_CLICKED, NULL);
    }
}

/**
 * @brief ��ʾ���ֲ��Ž���
 * 
 * ����˵����
 * 1. ֹ֮ͣǰ�Ķ�ʱ��
 * 2. ����ǰ��Ļ
 * 3. ���ö���ָ��
 * 4. ��������������
 * 5. ���ñ���ͼƬ
 * 6. ����������Ϣ����
 * 7. �������ư�ť����һ�ס�����/��ͣ����һ�׵ȣ�
 * 8. ��������ģʽ��ť
 * 9. �������ذ�ť
 * 10. ��������������
 * 11. ���½�����Ϣ
 * 12. �������ȸ��¶�ʱ��
 */
void show_music_player_screen(void)
{
    // ֹ֮ͣǰ�Ķ�ʱ��
    if (progress_timer) {
        lv_timer_del(progress_timer);
        progress_timer = NULL;
    }
    
    // ����ǰ��Ļ
    lv_obj_clean(lv_screen_active());
    
    // ���ö���ָ��
    music_list_screen = NULL;
    
    // ��������������
    music_player_screen = lv_obj_create(lv_screen_active());
    lv_obj_set_size(music_player_screen, 800, 480);
    lv_obj_set_pos(music_player_screen, 0, 0);
    lv_obj_set_style_pad_all(music_player_screen, 0, LV_PART_MAIN);
    lv_obj_set_style_border_width(music_player_screen, 0, LV_PART_MAIN);
    
    // ���ñ���ͼƬ
    lv_obj_t * bg_img = lv_image_create(music_player_screen);
    lv_obj_set_size(bg_img, 800, 480);
    lv_obj_set_pos(bg_img, 0, 0);
    lv_image_set_src(bg_img, "/home/tyc/work_station/music/kugou_back.jpg");
    
    // ������Ϣ���� - �������
    lv_obj_t * info_area = lv_obj_create(music_player_screen);
    lv_obj_set_size(info_area, 480, 60);  // ����ߴ�
    lv_obj_set_pos(info_area, 160, 25);   // ����λ��ʹ�������
    lv_obj_set_style_bg_opa(info_area, LV_OPA_60, LV_PART_MAIN);  // ��͸������
    lv_obj_set_style_bg_color(info_area, lv_color_hex(0x000000), LV_PART_MAIN);  // ��ɫ����
    lv_obj_set_style_border_width(info_area, 2, LV_PART_MAIN);   // ���ӱ߿���
    lv_obj_set_style_border_color(info_area, lv_color_hex(0xFFD700), LV_PART_MAIN);  // ��ɫ�߿�
    lv_obj_set_style_border_opa(info_area, LV_OPA_80, LV_PART_MAIN);  // �߿������
    lv_obj_set_style_radius(info_area, 25, LV_PART_MAIN);  // �����Բ��
    lv_obj_set_style_pad_all(info_area, 20, LV_PART_MAIN);  // ������ڱ߾�
    
    // ��ӽ���Ч��
    lv_obj_set_style_bg_grad_color(info_area, lv_color_hex(0x333333), LV_PART_MAIN);
    lv_obj_set_style_bg_grad_dir(info_area, LV_GRAD_DIR_VER, LV_PART_MAIN);

    // �������� - ����������ʾ
    song_title_label = lv_label_create(info_area);
    lv_obj_set_style_text_color(song_title_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);  // ��ɫ����
    lv_obj_set_style_text_font(song_title_label, &lv_font_montserrat_20, LV_PART_MAIN);  // ��������
    lv_obj_set_style_text_opa(song_title_label, LV_OPA_100, LV_PART_MAIN);  // ��ȫ��͸��
    lv_obj_set_style_text_align(song_title_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);  // ���ж���
    // ...���Ƴ�������ӰЧ�������ݵͰ汾LVGL...
    lv_obj_align(song_title_label, LV_ALIGN_CENTER, 0, 0);  // ��ȫ����
    
    // ֱ���ڽ����Ϸ��ÿ��ư�ť - �Ż�����
    // ��һ�װ�ť
    lv_obj_t * prev_btn = lv_button_create(music_player_screen);
    lv_obj_set_size(prev_btn, 90, 90);//��С
    lv_obj_set_pos(prev_btn, 90, 285);//λ��
    lv_obj_set_style_bg_color(prev_btn, lv_color_hex(0x3498db), LV_PART_MAIN);
    lv_obj_set_style_radius(prev_btn, 45, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(prev_btn, LV_OPA_0, LV_PART_MAIN);  // ������ȫ��͸��
    lv_obj_set_style_border_opa(prev_btn, LV_OPA_0, LV_PART_ANY);
    
    
    
    lv_obj_t * prev_label = lv_label_create(prev_btn);
    lv_label_set_text(prev_label, LV_SYMBOL_PREV);
    //lv_obj_set_style_text_color(prev_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_font(prev_label, &lv_font_montserrat_18, LV_PART_MAIN);
    lv_obj_center(prev_label);
    lv_obj_add_event_cb(prev_btn, music_prev_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_set_style_text_opa(prev_label, LV_OPA_0, LV_PART_MAIN);  // ������ȫ��͸��
    lv_obj_set_style_border_opa(prev_label, LV_OPA_0, LV_PART_ANY);
    
    // ����/��ͣ��ť - ���з���
    play_pause_btn = lv_button_create(music_player_screen);
    lv_obj_set_size(play_pause_btn, 300, 200);//��С
    lv_obj_set_pos(play_pause_btn, 245, 115);//λ��
    //lv_obj_set_style_bg_color(play_pause_btn, lv_color_hex(0xe74c3c), LV_PART_MAIN);
    lv_obj_set_style_radius(play_pause_btn, 100, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(play_pause_btn, LV_OPA_0, LV_PART_MAIN);  // ������ȫ��͸��
    lv_obj_set_style_border_opa(play_pause_btn, LV_OPA_0, LV_PART_MAIN);

    
    lv_obj_t * play_pause_label = lv_label_create(play_pause_btn);
    const char* btn_text = (g_music_player.state == MUSIC_STATE_PLAYING) ? LV_SYMBOL_PAUSE : LV_SYMBOL_PLAY;
    lv_label_set_text(play_pause_label, btn_text);
    lv_obj_set_style_text_color(play_pause_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_font(play_pause_label, &lv_font_montserrat_22, LV_PART_MAIN);
    lv_obj_center(play_pause_label);
    lv_obj_add_event_cb(play_pause_btn, music_play_pause_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_set_style_text_opa(play_pause_label, LV_OPA_0, LV_PART_MAIN);  // ������ȫ��͸��
    lv_obj_set_style_border_opa(play_pause_label, LV_OPA_0, LV_PART_MAIN);
    
    // ��һ�װ�ť
    lv_obj_t * next_btn = lv_button_create(music_player_screen);
    lv_obj_set_size(next_btn, 90, 90);//��С
    lv_obj_set_pos(next_btn, 590, 275);//λ��
    //lv_obj_set_style_bg_color(next_btn, lv_color_hex(0x3498db), LV_PART_MAIN);
    lv_obj_set_style_radius(next_btn, 45, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(next_btn, LV_OPA_0, LV_PART_MAIN);  // ������ȫ��͸��
    lv_obj_set_style_border_opa(next_btn, LV_OPA_0, LV_PART_MAIN);

    
    lv_obj_t * next_label = lv_label_create(next_btn);
    lv_label_set_text(next_label, LV_SYMBOL_NEXT);
    lv_obj_set_style_text_color(next_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_font(next_label, &lv_font_montserrat_18, LV_PART_MAIN);
    lv_obj_center(next_label);
    lv_obj_add_event_cb(next_btn, music_next_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_set_style_text_opa(next_label, LV_OPA_0, LV_PART_MAIN);  // ������ȫ��͸��
    lv_obj_set_style_border_opa(next_label, LV_OPA_0, LV_PART_MAIN);
    
    // ����ģʽ��ť - ���Ͻ�
    mode_btn = lv_button_create(music_player_screen);
    lv_obj_set_size(mode_btn, 80, 80);
    lv_obj_set_pos(mode_btn, 80, 120);
    //lv_obj_set_style_bg_color(mode_btn, lv_color_hex(0x9b59b6), LV_PART_MAIN);
    lv_obj_set_style_radius(mode_btn, 40, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(mode_btn, LV_OPA_0, LV_PART_MAIN);  // ������ȫ��͸��
    lv_obj_set_style_border_opa(mode_btn, LV_OPA_0, LV_PART_MAIN);
    
    lv_obj_t * mode_label = lv_label_create(mode_btn);
    const char* mode_text = (g_music_player.play_mode == PLAY_MODE_SEQUENCE) ? "Sequential" : "Random";
    lv_label_set_text(mode_label, mode_text);
    lv_obj_set_style_text_color(mode_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_font(mode_label, &lv_font_montserrat_10, LV_PART_MAIN);
    lv_obj_center(mode_label);
    lv_obj_add_event_cb(mode_btn, music_mode_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_set_style_text_opa(mode_label, LV_OPA_0, LV_PART_MAIN);  // ������ȫ��͸��
    lv_obj_set_style_border_opa(mode_label, LV_OPA_0, LV_PART_MAIN);
    
    // ���ذ�ť - ���Ͻ�
    lv_obj_t * back_btn = lv_button_create(music_player_screen);
    lv_obj_set_size(back_btn, 70, 70);
    lv_obj_set_pos(back_btn, 630, 35);
    //lv_obj_set_style_bg_color(back_btn, lv_color_hex(0x95a5a6), LV_PART_MAIN);
    lv_obj_set_style_radius(back_btn, 35, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(back_btn, LV_OPA_0, LV_PART_MAIN);  // ������ȫ��͸��
    lv_obj_set_style_border_opa(back_btn, LV_OPA_0, LV_PART_MAIN);
    
    lv_obj_t * back_label = lv_label_create(back_btn);
    lv_label_set_text(back_label, "Back");
    lv_obj_set_style_text_color(back_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_font(back_label, &lv_font_montserrat_10, LV_PART_MAIN);
    lv_obj_center(back_label);
    lv_obj_add_event_cb(back_btn, music_player_back_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_set_style_text_opa(back_label, LV_OPA_0, LV_PART_MAIN);  // ������ȫ��͸��
    lv_obj_set_style_border_opa(back_label, LV_OPA_0, LV_PART_MAIN);
    
   // ���������� - ͸���������߿�
    lv_obj_t * progress_area = lv_obj_create(music_player_screen);
    lv_obj_set_size(progress_area, 620, 80);//��С

    lv_obj_set_pos(progress_area, 80, 400);
    lv_obj_set_style_bg_opa(progress_area, LV_OPA_0, LV_PART_MAIN);  // ��͸����


    lv_obj_set_style_bg_color(progress_area, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_border_width(progress_area, 1, LV_PART_MAIN);   // ��ӱ߿�
    lv_obj_set_style_border_color(progress_area, lv_color_hex(0x555555), LV_PART_MAIN);
    lv_obj_set_style_border_opa(progress_area, LV_OPA_0, LV_PART_MAIN);
    lv_obj_set_style_radius(progress_area, 10, LV_PART_MAIN);
    lv_obj_set_style_pad_all(progress_area, 15, LV_PART_MAIN);
    
    // ������ - �Ƴ���ק���ܣ�ֻ��ʾ����
    progress_bar = lv_slider_create(progress_area);
    lv_obj_set_size(progress_bar, 560, 15);  // �����������ߴ�
    lv_obj_set_pos(progress_bar, 20, 5);      // �����progress_area��λ��
    lv_obj_set_style_bg_color(progress_bar, lv_color_hex(0x555555), LV_PART_MAIN);
    lv_obj_set_style_bg_color(progress_bar, lv_color_hex(0xFF6B6B), LV_PART_INDICATOR);
    lv_slider_set_range(progress_bar, 0, 100);
    lv_slider_set_value(progress_bar, 0, LV_ANIM_OFF);
    lv_obj_remove_flag(progress_bar, LV_OBJ_FLAG_CLICKABLE);  // �Ƴ��������
    
    // ʱ���ǩ
    progress_label = lv_label_create(progress_area);
    lv_label_set_text(progress_label, "00:00 / 00:00");
    lv_obj_set_style_text_color(progress_label, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_text_font(progress_label, &lv_font_montserrat_10, LV_PART_MAIN);
    lv_obj_set_style_text_opa(progress_label, LV_OPA_70, LV_PART_MAIN);
    lv_obj_set_pos(progress_label, 260, 25);  // ����ʱ���ǩλ�ã��ڽ������·�
    
    // ���½�����Ϣ - ֻ��ʾ�ļ���
    if (g_music_player.music_count > 0) {
        lv_label_set_text(song_title_label, g_music_player.music_list[g_music_player.current_index].filename);
    }
    
    // �������ȸ��¶�ʱ�� - ÿ�����
    if (progress_timer) {
        lv_timer_del(progress_timer);
    }
    progress_timer = lv_timer_create(update_music_progress, 1000, NULL);
}

/**
 * @brief ��������
 * 
 * ����˵����
 * 1. ��֤������������Ч��
 * 2. ֹͣ��ǰ���ڲ��ŵ�����
 * 3. ���µ�ǰ��������
 * 4. ʹ��fork()�����ӽ�������������
 * 5. ���ӽ�����ִ��madplay��mpg123������
 * 6. �ڸ������м�¼����״̬�ͽ���ID
 * 7. ���²��Ž������ʾ��Ϣ
 * 
 * ����Ҫ�㣺
 * - ʹ�ý��̼�ͨ�ţ�fork()�����ӽ��̣����ӽ��̷ֹ�����
 * - �ӽ��̸������ֲ��ţ������̸���������
 * - ʹ��execl()���ӽ������滻Ϊ���ֲ���������
 * - ֧�ֶ��ֲ�����������ʹ��madplay����ѡmpg123
 * 
 * @param index Ҫ���ŵ������ڲ����б��е�����
 */
void play_music(int index)
{
    // ========== ������֤ ==========
    // ������������Ƿ�����Ч��Χ��
    if (index < 0 || index >= g_music_player.music_count) {
        return; // ������Ч��ֱ�ӷ���
    }
    
    // ========== ǿ��ֹͣ��ǰ���� ==========
    // ȷ����ȫ����֮ǰ�Ĳ��Ž��̣���ֹ��Դй¶
    stop_music();
    
    // ����ȴ���ȷ��������ȫ����
    usleep(50000); // �ȴ�50ms
    
    // ========== ���²���״̬ ==========
    // ���õ�ǰ���ŵ���������
    g_music_player.current_index = index;
    
    // ���ý��ȼ�����
    current_progress = 0;
    
    // �ڿ���̨�����ǰ���ŵ��ļ�·�������ڵ���
    // printf("Starting new music: %s\n", g_music_player.music_list[index].filepath);
    
    // ========== �������Ž��� ==========
    // ʹ��fork()�����ӽ�������������
    // �������ĺô��ǣ������̼���������棬�ӽ���ר�Ÿ������ֲ���
    pid_t pid = fork();
    
    if (pid == 0) {
        // ========== �ӽ��̣��������ֲ��� ==========
        
        // �����ӽ����źŴ���ȷ���ܹ���ȷ��Ӧ��ֹ�ź�
        signal(SIGTERM, SIG_DFL);
        signal(SIGINT, SIG_DFL);
        
        // �رղ���Ҫ���ļ���������������Դռ��
        // close(STDIN_FILENO);  // ��ѡ���رձ�׼����
        
        // ����ʹ��madplay����������MP3�ļ�
        // execl()���滻��ǰ����ӳ��ִ��ָ���ĳ���
        // ����˵��������·������������ѡ��ļ�·����������־
        execl("/usr/bin/madplay", "madplay", "-q", g_music_player.music_list[index].filepath, NULL);
        
        // ���madplayִ��ʧ�ܣ�������򲻴��ڣ�������ʹ��mpg123
        execl("/usr/bin/mpg123", "mpg123", "-q", g_music_player.music_list[index].filepath, NULL);
        
        // ����������������޷�ִ�У��ӽ����˳�
        // ע�⣺���execl()�ɹ�������ִ�е�����
        // �����������˵��execlʧ��
        exit(1);
        
    } else if (pid > 0) {
        // ========== �����̣����������� ==========
        
        // ��¼�ӽ��̵Ľ���ID�����ں����Ľ��̿��ƣ���ͣ��ֹͣ�ȣ�
        g_music_player.player_pid = pid;
        
        // ���²�����״̬Ϊ"���ڲ���"
        g_music_player.state = MUSIC_STATE_PLAYING;
        
        // printf("Music player started with PID: %d\n", pid);
        
        // ���²��Ž��� - ֻ�����ļ���
        if (music_player_screen && lv_obj_is_valid(music_player_screen) && 
            song_title_label && lv_obj_is_valid(song_title_label)) {
            lv_label_set_text(song_title_label, g_music_player.music_list[index].filename);
            
            // ���²��Ű�ť
            if (play_pause_btn && lv_obj_is_valid(play_pause_btn)) {
                lv_obj_t * play_pause_label = lv_obj_get_child(play_pause_btn, 0);
                if (play_pause_label && lv_obj_is_valid(play_pause_label)) {
                    lv_label_set_text(play_pause_label, LV_SYMBOL_PAUSE);
                }
            }
        }
    } else {
        // ========== forkʧ�� ==========
        // ���̴���ʧ��
        g_music_player.state = MUSIC_STATE_STOP;
    }
}

/**
 * @brief ��ͣ����
 * 
 * ����˵����
 * 1. ��鲥�Ž����Ƿ���������ڲ���
 * 2. �򲥷Ž��̷�����ͣ�ź�
 * 3. ���²���״̬
 * 4. ���²��Ű�ť��ʾ
 */
void pause_music(void)
{
    if (g_music_player.player_pid > 0 && g_music_player.state == MUSIC_STATE_PLAYING) {
        kill(g_music_player.player_pid, SIGSTOP);
        g_music_player.state = MUSIC_STATE_PAUSED;
        
        // ���²��Ű�ť
        if (play_pause_btn && lv_obj_is_valid(play_pause_btn)) {
            lv_obj_t * play_pause_label = lv_obj_get_child(play_pause_btn, 0);
            if (play_pause_label && lv_obj_is_valid(play_pause_label)) {
                lv_label_set_text(play_pause_label, LV_SYMBOL_PLAY);
            }
        }
    }
}

/**
 * @brief �ָ�����
 * 
 * ����˵����
 * 1. ��鲥�Ž����Ƿ����������ͣ
 * 2. �򲥷Ž��̷��ͼ����ź�
 * 3. ���²���״̬
 * 4. ���²��Ű�ť��ʾ
 */
void resume_music(void)
{
    if (g_music_player.player_pid > 0 && g_music_player.state == MUSIC_STATE_PAUSED) {
        kill(g_music_player.player_pid, SIGCONT);
        g_music_player.state = MUSIC_STATE_PLAYING;
        
        // ���²��Ű�ť
        if (play_pause_btn && lv_obj_is_valid(play_pause_btn)) {
            lv_obj_t * play_pause_label = lv_obj_get_child(play_pause_btn, 0);
            if (play_pause_label && lv_obj_is_valid(play_pause_label)) {
                lv_label_set_text(play_pause_label, LV_SYMBOL_PAUSE);
            }
        }
    }
}

/**
 * @brief ֹͣ���� - �Ż���Դ�ͷ�
 */
void stop_music(void)
{
    if (g_music_player.player_pid > 0) {
        // 1. �ȳ����º͵���ֹ����
        kill(g_music_player.player_pid, SIGTERM);
        
        // 2. �ȴ����̽��������ó�ʱ�������޵ȴ�
        int status;
        pid_t result = waitpid(g_music_player.player_pid, &status, WNOHANG);
        
        // 3. �������û������������ǿ����ֹ
        if (result == 0) {
            usleep(100000); // �ȴ�100ms
            result = waitpid(g_music_player.player_pid, &status, WNOHANG);
            if (result == 0) {
                kill(g_music_player.player_pid, SIGKILL); // ǿ����ֹ
                waitpid(g_music_player.player_pid, NULL, 0);
            }
        }
        
        // 4. �������ID
        g_music_player.player_pid = -1;
        
        // printf("Music process stopped and resources released\n");
    }
    
    g_music_player.state = MUSIC_STATE_STOP;
    g_music_player.current_time = 0;
    
    // ���ý��ȼ�����
    current_progress = 0;
    
    // ���ý�������ʾ
    if (progress_bar && lv_obj_is_valid(progress_bar)) {
        lv_slider_set_value(progress_bar, 0, LV_ANIM_OFF);
    }
    if (progress_label && lv_obj_is_valid(progress_label)) {
        lv_label_set_text(progress_label, "00:00 / 00:00");
    }
    
    // ���²��Ű�ť
    if (play_pause_btn && lv_obj_is_valid(play_pause_btn)) {
        lv_obj_t * play_pause_label = lv_obj_get_child(play_pause_btn, 0);
        if (play_pause_label && lv_obj_is_valid(play_pause_label)) {
            lv_label_set_text(play_pause_label, LV_SYMBOL_PLAY);
        }
    }
}

/**
 * @brief ��һ��
 * 
 * ����˵����
 * 1. ����Ƿ��������ļ�
 * 2. ���ݲ���ģʽ������һ�׵�����
 * 3. ����ѡ�е�����
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
 * @brief ��һ��
 * 
 * ����˵����
 * 1. ����Ƿ��������ļ�
 * 2. ���ݲ���ģʽ������һ�׵�����
 * 3. ����ѡ�е�����
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
 * @brief �л�����ģʽ
 * 
 * ��˳�򲥷ź��������ģʽ֮���л�
 * ˳�򲥷ţ����б�˳�򲥷Ÿ���
 * ������ţ����ѡ���б��еĸ�������
 */
void toggle_play_mode(void)
{
    g_music_player.play_mode = (g_music_player.play_mode == PLAY_MODE_SEQUENCE) ? 
                               PLAY_MODE_RANDOM : PLAY_MODE_SEQUENCE;
    
    // ����ģʽ��ť��ʾ
    if (mode_btn) {
        lv_obj_t * label = lv_obj_get_child(mode_btn, 0);
        if (label) {
            const char * mode_text = (g_music_player.play_mode == PLAY_MODE_SEQUENCE) ? 
                                   "Sequential" : "Random";
            lv_label_set_text(label, mode_text);
        }
    }
}

// ============ �¼��ص����� ============
// ���º��������û������ֲ������еĸ��ֽ�������

/**
 * @brief �����б������¼��ص�����
 * 
 * ����˵����
 * �������б�������û������Ʋ���
 * ���û����һ���ʱ���˳����ֲ������ص�����
 * 
 * �˳�ʱ����������
 * 1. ɾ�����ȸ��¶�ʱ��
 * 2. ֹͣ��ǰ���ŵ�����
 * 3. �����������
 * 
 * @param e �¼����󣬰���������Ϣ
 */
/**
 * @brief �������ֲ�������Դ
 * 
 * ���˳�Ӧ�û��л�����ʱ���ã�ȷ��������Դ������ȷ�ͷ�
 * ������ʱ�������Ž��̡�����Ԫ�غ�״̬����
 */
void cleanup_music_player(void)
{
    // ֹͣ��ʱ��
    if (progress_timer) {
        lv_timer_del(progress_timer);
        progress_timer = NULL;
    }
    
    // ֹͣ���ֲ���
    stop_music();
    
    // ��������״̬
    g_music_player.state = MUSIC_STATE_STOP;
    g_music_player.player_pid = -1;
    current_progress = 0;
    
    // �������ָ��
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
 * @brief �ֶ����Ƽ��ص�����
 * 
 * ͨ���������º��ͷ��¼��ֶ��������
 */
void music_list_touch_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    
    if (code == LV_EVENT_PRESSED) {
        // ��¼������ʼ��
        lv_indev_t * indev = lv_indev_get_act();
        if (indev) {
            lv_indev_get_point(indev, &touch_start_point);
            touch_start_time = lv_tick_get();
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
                uint32_t touch_end_time = lv_tick_get();
                
                int dx = touch_end_point.x - touch_start_point.x;
                int dy = touch_end_point.y - touch_start_point.y;
                uint32_t duration = touch_end_time - touch_start_time;
                
                // �ж��Ƿ�Ϊ��Ч���һ�����
                if (dx > 50 && abs(dy) < 100 && duration < 1000) {  // �һ�����50���أ���ֱƫ��С��100���أ�ʱ��С��1��
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
    // ��ȡ��ǰ������豸
    lv_indev_t * indev = lv_indev_get_act();
    if (indev == NULL) {
        return;
    }

    // ��ȡ��ǰ������豸�����Ʒ���
    lv_dir_t gesture = lv_indev_get_gesture_dir(indev);

    // ����Ƿ�Ϊ�һ�����
    if (gesture == LV_DIR_RIGHT) {
        cleanup_music_player();
        extern void desktop(void);
        desktop();
    }
}

// ȷ����show_music_list_screen��������ȷ��������¼�
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
    // �Ӳ��Ž��淵�ص��б����
    show_music_list_screen();
}

void music_back_cb(lv_event_t * e)
{
    // ʹ��ͳһ��������
    cleanup_music_player();
    
    // ��������
    extern void desktop(void);
    desktop();
}

/**
 * @brief �������ֽ���
 * 
 * �Ż�˵����
 * 1. ÿ�׸趼��0��ʼ��ʱ��������Ȼ���
 * 2. ֻ�ڲ���״̬ʱ���½���
 * 3. ����ӽ���״̬������ʬ����
 * 4. ����UI����Ƶ�ʣ�������濨��
 * 5. ��ʱ����ʾ�߼�����ʽ��ΪMM:SS
 */
void update_music_progress(lv_timer_t * timer)
{
    // ��鲥�Ž����Ƿ񻹴��ڣ�����ʬ����
    if (g_music_player.player_pid > 0) {
        int status;
        pid_t result = waitpid(g_music_player.player_pid, &status, WNOHANG);
        
        if (result > 0) {
            // �ӽ����ѽ������Զ�������һ��
            g_music_player.player_pid = -1;
            g_music_player.state = MUSIC_STATE_STOP;
            current_progress = 0;
            next_music();
            return;
        }
    }
    
    // ֻ���ڲ���״̬�Ÿ��½���
    if (g_music_player.state != MUSIC_STATE_PLAYING) {
        return;
    }
    
    // ���������Ƿ���Ч
    if (!progress_bar || !lv_obj_is_valid(progress_bar)) {
        return;
    }
    
    // ���ӽ��ȼ�������ÿ��+1��
    current_progress++;
    
    // ����ÿ�׸�3��30�� = 210�룬����ٷֱ�
    int total_seconds = 210;
    int progress_percent = (current_progress * 100) / total_seconds;
    if (progress_percent > 100) {
        progress_percent = 100;
    }
    
    // ���½����� - ���ٶ������������
    lv_slider_set_value(progress_bar, progress_percent, LV_ANIM_OFF);
    
    // ����ʱ����ʾ - ֻ�ڱ�Ҫʱ����
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
