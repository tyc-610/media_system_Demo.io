#ifndef __VIDEO_H__
#define __VIDEO_H__



void start_video_app(void);
void init_video(void);


//-------------------------------------------=-------------------

void video_test(void);


//-------------------------------------------------------------------



//�����Ƶ�ļ��� 
#define MAX_VIDEO_COUNT 20
#define MAX_VIDEO_NAME_LEN 128
#define MAX_VIDEO_PATH_LEN 256

//��Ƶ���·��
#define VIDEO_PATH "/home/tyc/work_station/video"
//mplayer ����
#define MPLAYER_CMD "./work_station/tools/mplayer -fs -loop 0 -quiet -geometry 1024x768 -zoom -x 800 -y 480 "


//��Ƶ�ļ��ṹ��
typedef struct
{
    char name[MAX_VIDEO_NAME_LEN];
    char path[MAX_VIDEO_PATH_LEN];
}video_info_t;




#endif
