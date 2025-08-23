#ifndef __MY_FONT_H__
#define __MY_FONT_H__

#ifdef LV_LVGL_H_INCLUDE_SIMPLE
#include "lvgl.h"
#else
#include "lvgl/lvgl.h"
#endif

// 只声明字体，不包含定义
extern const lv_font_t lv_myself_font;

#endif // __MY_FONT_H__
