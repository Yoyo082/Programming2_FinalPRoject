#ifndef UI_H
#define UI_H

#include "raylib.h"
#include <stdarg.h>

void UI_Init(const char *window_title);
void UI_Close(void);

// 取代 printf：把訊息加到畫面 log
void UI_Log(const char *fmt, ...);

// 清空 log（換階段時用）
void UI_ClearLog(void);

// 取代 scanf：顯示按鈕列，回傳 0-based 選項索引
// 例：UI_WaitChoice((const char*[]){"[1] 再來一局","[0] 結束"}, 2)
int  UI_WaitChoice(const char *options[], int count);

// 取代 scanf：顯示 1~max 的數字按鈕，回傳選擇的數字
int  UI_WaitNumber(const char *prompt, int min, int max);

// 顯示訊息後等玩家點「繼續」
void UI_WaitContinue(void);

#endif