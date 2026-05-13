#include "ui.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SCREEN_W 960
#define SCREEN_H 620
#define MAX_LINES 60
#define MAX_LEN 300
#define FONT_SIZE 20
#define LINE_H 26
#define BTN_AREA_H 80

static char g_lines[MAX_LINES][MAX_LEN];
static int g_line_count = 0;
static Font g_font;
static bool g_font_is_default = true;

static void push_line(const char *s) {
    if (g_line_count >= MAX_LINES) {
        memmove(g_lines[0], g_lines[1], (MAX_LINES - 1) * MAX_LEN);
        g_line_count = MAX_LINES - 1;
    }

    strncpy(g_lines[g_line_count], s, MAX_LEN - 1);
    g_lines[g_line_count][MAX_LEN - 1] = '\0';
    g_line_count++;
}

static void draw_log(void) {
    int log_h = SCREEN_H - BTN_AREA_H - 10;
    int visible = log_h / LINE_H;
    int start = (g_line_count > visible) ? g_line_count - visible : 0;

    for (int i = start; i < g_line_count; i++) {
        int y = 10 + (i - start) * LINE_H;
        DrawTextEx(g_font, g_lines[i], (Vector2){14, (float)y}, FONT_SIZE, 1, RAYWHITE);
    }

    DrawLine(0, SCREEN_H - BTN_AREA_H, SCREEN_W, SCREEN_H - BTN_AREA_H,
             (Color){70, 70, 100, 255});
}

static void draw_buttons(const char *options[], int count, int hover_idx) {
    int pad = 12;
    int total = SCREEN_W - pad * 2 - (count - 1) * 8;
    int bw = total / count;
    int bh = 44;
    int by = SCREEN_H - BTN_AREA_H + (BTN_AREA_H - bh) / 2;

    for (int i = 0; i < count; i++) {
        int bx = pad + i * (bw + 8);
        Rectangle r = {(float)bx, (float)by, (float)bw, (float)bh};

        Color bg = (i == hover_idx) ? (Color){80, 90, 160, 255}
                                    : (Color){40, 44, 80, 255};
        Color border = (i == hover_idx) ? (Color){160, 170, 255, 255}
                                        : (Color){80, 86, 130, 255};

        DrawRectangleRounded(r, 0.2f, 6, bg);
        DrawRectangleRoundedLines(r, 0.2f, 6, border);

        Vector2 ts = MeasureTextEx(g_font, options[i], FONT_SIZE, 1);
        DrawTextEx(g_font, options[i],
                   (Vector2){bx + (bw - ts.x) / 2.0f, by + (bh - ts.y) / 2.0f},
                   FONT_SIZE, 1, WHITE);
    }
}

static void wait_for_mouse_release(const char *options[], int count, int hover_idx) {
    while (!WindowShouldClose() && IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
        BeginDrawing();
        ClearBackground((Color){18, 20, 34, 255});
        draw_log();
        draw_buttons(options, count, hover_idx);
        EndDrawing();
    }
}

void UI_Init(const char *title) {
    InitWindow(SCREEN_W, SCREEN_H, title);
    SetTargetFPS(60);

    int total = (0x7E - 0x20 + 1) + (0x9FFF - 0x4E00 + 1);
    int *codepoints = malloc(sizeof(int) * total);

    int idx = 0;

    // ASCII
    for (int i = 0x20; i <= 0x7E; i++) {
        codepoints[idx++] = i;
    }

    // 中文
    for (int i = 0x4E00; i <= 0x9FFF; i++) {
        codepoints[idx++] = i;
    }

    g_font = LoadFontEx(
        "Noto_Sans_TC/static/NotoSansTC-Regular.ttf",
        FONT_SIZE,
        codepoints,
        idx);

    free(codepoints);

    if (g_font.texture.id == 0) {
        g_font = GetFontDefault();
        g_font_is_default = true;
    } else {
        g_font_is_default = false;
        SetTextureFilter(g_font.texture, TEXTURE_FILTER_BILINEAR);
    }
}

void UI_Close(void) {
    if (!g_font_is_default) {
        UnloadFont(g_font);
    }
    CloseWindow();
}

void UI_ClearLog(void) {
    g_line_count = 0;
}

void UI_Log(const char *fmt, ...) {
    char buf[MAX_LEN];
    va_list ap;

    va_start(ap, fmt);
    vsnprintf(buf, MAX_LEN, fmt, ap);
    va_end(ap);

    push_line(buf);
}

int UI_WaitChoice(const char *options[], int count) {
    while (!WindowShouldClose()) {
        Vector2 mouse = GetMousePosition();
        bool clicked = IsMouseButtonPressed(MOUSE_LEFT_BUTTON);

        int pad = 12;
        int bw = (SCREEN_W - pad * 2 - (count - 1) * 8) / count;
        int bh = 44;
        int by = SCREEN_H - BTN_AREA_H + (BTN_AREA_H - bh) / 2;
        int hover = -1;

        for (int i = 0; i < count; i++) {
            int bx = pad + i * (bw + 8);
            Rectangle r = {(float)bx, (float)by, (float)bw, (float)bh};
            if (CheckCollisionPointRec(mouse, r)) {
                hover = i;
                if (clicked) {
                    wait_for_mouse_release(options, count, hover);
                    return i;
                }
            }
        }

        BeginDrawing();
        ClearBackground((Color){18, 20, 34, 255});
        draw_log();
        draw_buttons(options, count, hover);
        EndDrawing();
    }

    return -1;
}

int UI_WaitNumber(const char *prompt, int min, int max) {
    UI_Log(prompt);

    int count = max - min + 1;
    const char **labels = malloc(count * sizeof(char *));
    char **strs = malloc(count * sizeof(char *));

    for (int i = 0; i < count; i++) {
        strs[i] = malloc(8);
        snprintf(strs[i], 8, "%d", min + i);
        labels[i] = strs[i];
    }

    int choice = UI_WaitChoice(labels, count);

    for (int i = 0; i < count; i++) {
        free(strs[i]);
    }
    free(strs);
    free(labels);

    return (choice >= 0) ? min + choice : min;
}

void UI_WaitContinue(void) {
    const char *opts[] = {"繼續"};
    UI_WaitChoice(opts, 1);
}
