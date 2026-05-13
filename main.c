#include "player.h"
#include "role.h"
#include "state.h"
#include "setup.h"
#include "game_loop.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
#include <windows.h>
#define CLEAR_CMD "cls"
#else
#define CLEAR_CMD "clear"
#endif

int main()
{
#ifdef _WIN32
    // 設定 Windows 終端機為 UTF-8 編碼，避免中文亂碼
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);
#endif

    int play_again = 0;
    
    do {
        system(CLEAR_CMD);
        Player players[13]; // 宣告 13 個空間 (1~12 號玩家，0 號不用)
        GameState state;    // 宣告掌管全局的狀態機
        int version_choice = 0;

        // --- 遊戲起始選單 ---
        printf("==========================================\n");
        printf("         🐺 歡迎來到狼人殺 🐺         \n");
        printf("==========================================\n");
        printf("請選擇您今天要遊玩的版本：\n");
        printf("[1] 白狼王騎士局 (白狼王/3狼 vs 預言家/女巫/守衛/騎士 vs 4平民)\n");
        printf("[2] 熊隱狼局     (隱狼/3狼   vs 熊/女巫/獵人/白痴     vs 4平民)\n");
        printf("------------------------------------------\n");
        printf("請輸入數字 (1-2): ");
        
        scanf("%d", &version_choice);

        if (version_choice == 1 || version_choice == 2) {
            
            // 🌟 1. 初始化遊戲狀態與發牌 (封裝在 setup.c)
            initialize_game(players, &state, version_choice);
            
            // 🌟 2. 核心遊戲主迴圈 (黑夜與白天交替)
            // 只要遊戲還沒結束 (state.is_game_over == false)，就一直循環
            while (!state.is_game_over) {
                
                // --- 黑夜階段 ---
                run_night_phase(players, &state);
                
                // 防呆：如果夜晚發生異常導致遊戲提早結束，直接跳出迴圈
                if (state.is_game_over) break; 
                
                // --- 白天階段 (包含結算死訊、發言、突發技能、投票) ---
                run_day_phase(players, &state);
            }

            // 🌟 3. 遊戲結束，播報獲勝陣營
            system(CLEAR_CMD);
            printf("\n==========================================\n");
            if (state.game_result == GAME_OVER_GOOD) {
                printf("          🏆 遊戲結束：好人陣營獲勝！ 🏆          \n");
            } else if (state.game_result == GAME_OVER_WOLF) {
                printf("          🏆 遊戲結束：狼人陣營獲勝！ 🏆          \n");
            } else {
                printf("          🏆 遊戲結束：平局或異常結束！ 🏆          \n");
            }
            printf("==========================================\n\n");

        } else {
            // 防呆：如果玩家輸入 1 和 2 以外的數字
            printf("無效的版本選擇。\n");
        }

        // --- 4. 詢問是否再來一局 ---
        printf("\n==========================================\n");
        printf("遊戲結束！要再玩一局嗎？\n");
        printf("[1] 再來一局 (重新選版本/發牌)\n");
        printf("[0] 結束程式\n");
        printf("請選擇：");
        scanf("%d", &play_again);
        
    } while (play_again == 1);

    printf("\n感謝遊玩，下次見！\n");
    return 0;
}