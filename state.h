#ifndef STATE_H
#define STATE_H

#include <stdbool.h>

// 1. 定義勝負狀態常數 (若不放在 role.h 則可放在此處)
#define GAME_ONGOING 0
#define GAME_OVER_GOOD 1
#define GAME_OVER_WOLF 2

// 2. 遊戲進度結構體
typedef struct {
    // --- 基礎流程資訊 ---
    int current_board;      // 1: 白狼王騎士, 2: 熊隱狼
    int day_count;          // 目前是第幾天 (days)
    int game_result;        // 紀錄勝負結果 (0, 1, 2)
    bool is_game_over;      // 遊戲是否結束 (game_over)

    // --- 角色資源與歷史紀錄 ---
    int last_guarded_id;    // 昨晚守護的 ID (預防連守)
    bool witch_has_antidote;// 女巫是否有解藥
    bool witch_has_poison;  // 女巫是否有毒藥
    bool knight_has_dueled; // 紀錄騎士是否已經決鬥過
    
    // --- 動態計數器 (解決妳發現的 alive_wolf_count 更新問題) ---
    int alive_wolf_count;   // 當前存活狼人數
    int alive_god_count;    // 當前存活神職數
    int alive_villager_count; // 當前存活平民數

    // --- 階段資訊 ---
    bool is_night;          // 目前是否為黑夜 (方便 UI 判定背景顏色)
    int killed_last_night;  // 昨晚被刀的玩家 ID (給女巫看)
} GameState;

#endif