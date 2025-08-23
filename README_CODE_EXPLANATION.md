# ��Ŀ������ϸ˵��

## ��Ŀ����
����һ������LVGLͼ�ο��Ƕ��ʽ��ý�岥������Ŀ��������GEC6818�������ϡ���Ŀ�������ֲ�������ͼƬ���������Ҫ����ģ�顣

## ��Ŀ�ṹ
```
Demo/
������ src/                    # Դ����Ŀ¼
��   ������ main.c             # ���������
��   ������ main_screen.c      # ����������
��   ������ kugou_music.c      # ���ֲ�����ʵ��
��   ������ dcim.c             # ͼƬ���ʵ��
��   ������ data.c             # ʱ�����ù���
������ inc/                    # ͷ�ļ�Ŀ¼
��   ������ main_screen.h      # �������ͷ�ļ�
��   ������ kugou_music.h      # ���ֲ�����ͷ�ļ�
��   ������ dcim.h             # ͼƬ���ͷ�ļ�
��   ������ ...
������ lvgl/                   # LVGLͼ�ο�Դ��
������ build/                  # �������Ŀ¼
������ Makefile               # ���������ļ�
```

## ���Ĺ���ģ��

### 1. ���ֲ����� (kugou_music.c)

#### ��Ҫ����
- **�ļ�ɨ��**: �Զ�ɨ��ָ��Ŀ¼�µ�MP3�ļ�
- **���ſ���**: ���š���ͣ��ֹͣ����һ�ס���һ��
- **����ģʽ**: ˳�򲥷ź��������
- **������ʾ**: �����б����Ͳ��ſ��ƽ���
- **���Ʋ���**: ֧���һ��˳�������

#### �ؼ�������
```c
// 1. ���̹��� - ʹ��fork()�����ӽ��̲�������
pid_t pid = fork();
if (pid == 0) {
    // �ӽ���ִ�����ֲ�����
    execl("/usr/bin/madplay", "madplay", "-q", filepath, NULL);
} else if (pid > 0) {
    // �����̼�¼PID���������
    g_music_player.player_pid = pid;
}

// 2. ���̿��� - ��ͣ/�ָ����ֲ���
kill(g_music_player.player_pid, SIGSTOP);  // ��ͣ
kill(g_music_player.player_pid, SIGCONT);  // �ָ�
kill(g_music_player.player_pid, SIGTERM);  // ��ֹ
```

#### ���ݽṹ
```c
// ������Ϣ�ṹ��
typedef struct {
    char filename[MAX_NAME_LEN];    // �ļ���
    char filepath[MAX_PATH_LEN];    // ����·��
    char title[MAX_NAME_LEN];       // ��������
    char artist[MAX_NAME_LEN];      // ������
    char album[MAX_NAME_LEN];       // ר��
} music_info_t;

// ���ֲ��������ṹ��
typedef struct {
    music_info_t music_list[MAX_MUSIC_COUNT];  // �����б�
    int music_count;                           // ��������
    int current_index;                         // ��ǰ��������
    play_mode_t play_mode;                     // ����ģʽ
    music_state_t state;                       // ����״̬
    pid_t player_pid;                          // ����������ID
    int current_time;                          // ��ǰ����ʱ��
    int total_time;                            // �ܲ���ʱ��
} music_player_t;
```

### 2. ͼƬ��� (dcim.c)

#### ��Ҫ����
- **ͼƬɨ��**: �Զ�ɨ��֧�ֵ�ͼƬ��ʽ(jpg, png, bmp, gif)
- **����ͼ��ʾ**: ���񲼾���ʾ����ͼƬ������ͼ
- **ͼƬ�鿴**: �������ͼ�鿴��ͼ
- **���Ʋ���**: ֧�����һ����л�ͼƬ���һ��˳�

#### �ؼ�������
```c
// 1. �ļ���ʽ���
int is_image_file(const char* filename) {
    const char* ext = strrchr(filename, '.');
    if (!ext) return 0;
    
    return (strcasecmp(ext, ".jpg") == 0 || 
            strcasecmp(ext, ".jpeg") == 0 ||
            strcasecmp(ext, ".png") == 0 || 
            strcasecmp(ext, ".bmp") == 0 ||
            strcasecmp(ext, ".gif") == 0);
}

// 2. ����ͼ���񲼾ּ���
for (int i = 0; i < g_gallery.image_count; i++) {
    int row = i / THUMBNAILS_PER_ROW;  // �����к�
    int col = i % THUMBNAILS_PER_ROW;  // �����к�
    
    // ��������Ļ�ϵ�λ��
    int x = x_offset + col * (THUMBNAIL_WIDTH + spacing);
    int y = y_offset + row * (THUMBNAIL_HEIGHT + spacing);
}
```

#### ���ݽṹ
```c
// ͼƬ��Ϣ�ṹ��
typedef struct {
    char filename[MAX_NAME_LEN];    // �ļ���
    char filepath[MAX_PATH_LEN];    // ����·��
} image_info_t;

// ���������ṹ��
typedef struct {
    image_info_t image_list[MAX_IMAGE_COUNT];  // ͼƬ�б�
    int image_count;                           // ͼƬ����
    int current_index;                         // ��ǰ�鿴����
    lv_obj_t* thumbnail_screen;                // ����ͼ����
    lv_obj_t* viewer_screen;                   // �鿴������
    lv_obj_t* current_img;                     // ��ǰ��ʾ��ͼƬ����
} gallery_manager_t;
```

### 3. ������� (main_screen.c)

#### ��Ҫ����
- **�������**: ��ʾӦ��ͼ���״̬��
- **ʱ����ʾ**: ʵʱ��ʾ��ǰʱ�������
- **Ӧ������**: ��Ӧ�û����������ӦӦ��

#### ����ṹ
```
�������
������ ״̬�� (��ʾʱ������)
������ Ӧ��ͼ������
��   ������ ���ֲ�����ͼ��
��   ������ ���ͼ��
��   ������ ��������ͼ��
��   ������ ʱ������ͼ��
������ ����ͼƬ
```

## LVGL������

### ��������νṹ
```
lv_screen_active()          // ��ǰ���Ļ
������ Ӧ�ø����� (��music_list_screen)
    ������ ����ͼƬ (bg_img)
    ������ �������� (container)
    ��   ������ �б�/��������
    ��   ��   ������ ��ť/��Ŀ
    ��   ������ ���ư�ť����
    ������ ���ƴ�������
```

### �¼��������
```c
// 1. ����¼��ص�
lv_obj_add_event_cb(obj, callback_function, LV_EVENT_CLICKED, NULL);

// 2. �¼��ص������ṹ
void callback_function(lv_event_t * e) {
    lv_obj_t * target = lv_event_get_target(e);  // ��ȡ�����¼��Ķ���
    // �����¼��߼�
}

// 3. �����¼�����
lv_dir_t gesture = lv_indev_get_gesture_dir(lv_indev_get_act());
switch(gesture) {
    case LV_DIR_LEFT:  // ��
    case LV_DIR_RIGHT: // �һ�
    case LV_DIR_UP:    // �ϻ�
    case LV_DIR_DOWN:  // �»�
}
```

## ���������

### �����������
��Ŀʹ��ARM�������������Makefile�����ã�
```makefile
CC = arm-linux-gnueabihf-gcc
CFLAGS = -I./inc -I./lvgl -Wall -O2
```

### �ļ�·������
- �����ļ�ɨ��·��: `/home/tyc/work_station/`
- ͼƬ�ļ�ɨ��·��: `/home/tyc/work_station/`
- ����ͼƬ·��: 
  - ����: `/home/tyc/work_station/kugou_back.jpg`
  - ���: `/home/tyc/work_station/dicm.jpg`

### ���л���
- Ŀ��Ӳ��: GEC6818������
- ����ϵͳ: Ƕ��ʽLinux
- ���ֲ�����: madplay �� mpg123
- ��ʾ�ֱ���: 800x480

## ��������ͽ������

### 1. �δ��� (Segmentation Fault)
**ԭ��**: LVGL�����ڴ��������
**���**: 
- �����л�ǰ�������: `lv_obj_clean(lv_screen_active())`
- ���ö���ָ��: `obj_ptr = NULL`
- ���ʶ���ǰ�����Ч��: `lv_obj_is_valid(obj)`

### 2. ͼƬ����ʧ��
**ԭ��**: ͼƬ�ļ�·��������ʽ��֧��
**���**: 
- ����ļ�·���Ƿ���ȷ
- ȷ��ͼƬ��ʽ��֧��
- ��Ϊ��ɫ����: `lv_obj_set_style_bg_color()`

### 3. ���ֲ���ʧ��
**ԭ��**: ���������򲻴��ڻ�Ȩ������
**���**: 
- ȷ��madplay��mpg123�Ѱ�װ
- ����ִ���ļ�·��: `/usr/bin/madplay`
- ȷ�������ļ���ʽ��ȷ

## ��չ����

### ������չ
1. **�����б����**: ֧���Զ��岥���б�
2. **������Ϣ��ʾ**: ����ID3��ǩ��ʾ��ϸ��Ϣ
3. **ͼƬ�༭**: ֧�ּ򵥵�ͼƬ����
4. **���繦��**: ֧���������ֲ��ź�����ͼƬ

### �����Ż�
1. **�������**: ʵ������ͼ����
2. **�첽����**: ʹ�ú�̨�̼߳���ý���ļ�
3. **�ڴ��Ż�**: ��ʱ�ͷŲ��õ�LVGL����
4. **�����Ż�**: �ӳټ��طǱ�Ҫģ��

�����Ŀչʾ��Ƕ��ʽGUIӦ�õĻ����ܹ����������ļ�ϵͳ���������̹����û�������Ƶȶ���������򡣴���ṹ����������ģ�黯����ѧϰǶ��ʽӦ�ÿ���������ʾ����
