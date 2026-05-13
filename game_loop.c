#include "game_loop.h"
#include "role.h"
#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
#define CLEAR_CMD "cls"
#else
#define CLEAR_CMD "clear"
#endif

// 執行黑夜流程
void run_night_phase(Player* players, GameState* state) {
    // 1. 每晚都要清空暫時標記
    for (int i = 1; i <= 12; i++) {
        players[i].is_knifed = false;
        players[i].is_saved = false;
        players[i].is_poisoned = false;
        players[i].is_guarded = false;
    }

    // 2. 重新計算存活狼人數 (存入 state 供後續邏輯使用)
    state->alive_wolf_count = 0;
    for (int i = 1; i <= 12; i++) {
        if (players[i].faction == FACTION_WOLF && players[i].is_alive) {
            state->alive_wolf_count++;
        }
    }

    // 狀態機設定為黑夜，並清空昨晚刀口紀錄
    state->is_night = true;
    state->killed_last_night = 0; 

    system(CLEAR_CMD);
    printf("\n================ 第 %d 天 ================\n", state->day_count);
    printf("天黑請閉眼\n");

    int dummy = 0; // 用來暫停畫面的變數

    // 3. 根據不同版子，執行不同的黑夜喚醒順序
    if (state->current_board == 1) {
        // ==========================================
        //         【白狼王騎士局黑夜】
        // ==========================================

        // --- 1. 守衛行動 ---
        printf("\n守衛請睜眼\n");
        int guard_id = -1;
        for (int i = 1; i <= 12; i++) {
            if (players[i].role == ROLE_GUARD) {
                guard_id = i;
                break;
            }
        }
        if (guard_id != -1 && players[guard_id].is_alive) {
            printf("請選擇要守護的玩家號碼 (空守請輸入 0)：");
            int guard_target;
            scanf("%d", &guard_target);

            // 防呆與技能發動 (使用 state 紀錄 last_guarded_id)
            if (guard_target >= 1 && guard_target <= 12 && players[guard_target].is_alive) {
                bool success = guard_protect(&players[guard_target], state);
                if (!success) {
                    printf("[系統提示] 無效操作：不能連續兩晚守護同一名玩家！視為空守。\n");
                }
            } else {
                state->last_guarded_id = -1; // 空守則重置守護紀錄
                if (guard_target != 0) printf("[系統提示] 無效目標，視為空守。\n");
            }
        } else {
            printf("(守衛已死亡，請等待幾秒後輸入 0 繼續以混淆視聽)\n");
        }
        printf("\n確認後請輸入 0 並按 Enter 閉眼：");
        scanf("%d", &dummy);
        system(CLEAR_CMD);

        // --- 2. 狼人行動 ---
        printf("\n狼人請睜眼\n");
        printf("狼人請睜眼，白狼請比大拇指（不殺人請輸入0）：");
        int wolf_target;
        scanf("%d", &wolf_target);

        if (wolf_target >= 1 && wolf_target <= 12 && players[wolf_target].is_alive) {
            werewolf_kill(&players[wolf_target]);
            state->killed_last_night = wolf_target; // 紀錄刀口給女巫看
        }
        printf("\n確認後請輸入 0 並按 Enter 閉眼：");
        scanf("%d", &dummy);
        system(CLEAR_CMD);

        // --- 3. 女巫行動 ---
        printf("\n女巫請睜眼\n");
        int witch_id = -1;
        for (int i = 1; i <= 12; i++) {
            if (players[i].role == ROLE_WITCH) {
                witch_id = i;
                break;
            }
        }

        if (witch_id != -1 && players[witch_id].is_alive) {
            if (!state->witch_has_antidote && !state->witch_has_poison) {
                printf("你的解藥與毒藥皆已用盡。\n");
            } else {
                // 播報刀口 (有解藥才看得到)
                if (state->witch_has_antidote) {
                    if (state->killed_last_night == 0) {
                        printf("今晚平安夜，沒有人被殺死。\n");
                    } else {
                        printf("今晚被殺死的是 【 %d 號玩家 】。\n", state->killed_last_night);
                    }
                } else {
                    printf("你的解藥已用盡，無法得知今晚誰被殺死。\n");
                }

                // 動態顯示選單
                printf("\n請選擇你要進行的操作：\n");
                if (state->witch_has_antidote) printf("[1] 使用解藥\n");
                if (state->witch_has_poison)   printf("[2] 使用毒藥\n");
                printf("[0] 什麼都不做\n");
                printf("你的選擇是：");

                int choice;
                scanf("%d", &choice);

                if (choice == 1 && state->witch_has_antidote) {
                    if (state->killed_last_night == 0) {
                        printf("今晚無人死亡，無法使用解藥。\n");
                    } else {
                        bool success = witch_save(players, witch_id, state->killed_last_night, state);
                        if (success) printf("成功使用解藥，救活了 %d 號玩家。\n", state->killed_last_night);
                        else printf("使用解藥失敗 (可能為自救或條件不符)。\n");
                    }
                } else if (choice == 2 && state->witch_has_poison) {
                    printf("你要毒誰呢？(請輸入玩家編號)：");
                    int poison_target;
                    scanf("%d", &poison_target);
                    if (poison_target >= 1 && poison_target <= 12 && players[poison_target].is_alive) {
                        bool success = witch_poison(&players[poison_target], state);
                        if (success) printf("成功對 %d 號玩家使用毒藥。\n", poison_target);
                    } else {
                        printf("無效的玩家編號或目標已死亡。\n");
                    }
                } else {
                    printf("[系統提示] 選擇保留藥水，什麼都不做。\n");
                }
            }
        } else {
            printf("(女巫已死亡，請等待幾秒後輸入 0 繼續以混淆視聽)\n");
        }
        printf("\n確認後請輸入 0 並按 Enter 閉眼：");
        scanf("%d", &dummy);
        system(CLEAR_CMD);

        // --- 4. 預言家行動 ---
        printf("\n預言家請睜眼\n");
        int seer_id = -1;
        for (int i = 1; i <= 12; i++) {
            if (players[i].role == ROLE_SEER) {
                seer_id = i;
                break;
            }
        }

        if (seer_id != -1 && players[seer_id].is_alive) {
            printf("請輸入你要查驗的玩家號碼 (不查驗請輸入 0)：");
            int seer_target;
            scanf("%d", &seer_target);

            if (seer_target >= 1 && seer_target <= 12) {
                int investigate_result = seer_investigate(players, seer_target, state);
                if (investigate_result == -1) printf("查驗失敗！\n");
                else if (investigate_result == FACTION_GOOD) printf("這個人是 【 好人 】。\n");
                else printf("這個人是 【 狼人 】！\n");
            } else {
                printf("預言家選擇不查驗或目標無效。\n");
            }
        } else {
            printf("(預言家已死亡，請等待幾秒後輸入 0 繼續以混淆視聽)\n");
        }
        printf("\n預言家請閉眼。確認後請輸入 0 並按 Enter 閉眼：");
        scanf("%d", &dummy);
        system(CLEAR_CMD);

    } else if (state->current_board == 2) {
        // ==========================================
        //           【熊隱狼局黑夜】
        // ==========================================

        // --- 1-1. 隱狼行動 ---
        printf("\n隱狼請睜眼\n");
        int hw_id = -1;
        for (int i = 1; i <= 12; i++) {
            if (players[i].role == ROLE_HIDDEN_WOLF) {
                hw_id = i;
                break;
            }
        }

        if (hw_id != -1 && players[hw_id].is_alive) {
            if (state->day_count == 1) {
                printf("你的狼隊友是：（ ");
                for (int i = 1; i <= 12; i++) {
                    if (players[i].faction == FACTION_WOLF && i != hw_id) printf("%d號 ", i);
                }
                printf("）\n");
            }
            bool awakened = hidden_wolf_awakens(state, &players[hw_id]);
            if (awakened) printf("今晚的回歸狼隊情況：（ 已回歸狼隊 ）\n");
            else printf("今晚的回歸狼隊情況：（ 無 ）\n");
        } else {
            printf("(隱狼已死亡，請等待幾秒後輸入 0 繼續以混淆視聽)\n");
        }
        printf("隱狼請閉眼。確認後請輸入 0 並按 Enter 閉眼：");
        scanf("%d", &dummy);
        system(CLEAR_CMD);

        // --- 1-2. 狼人行動 ---
        printf("\n狼人請睜眼\n");
        printf("請選擇要擊殺的玩家號碼 (不殺人請輸入0)：");
        int wolf_target;
        scanf("%d", &wolf_target);

        if (wolf_target >= 1 && wolf_target <= 12 && players[wolf_target].is_alive) {
            werewolf_kill(&players[wolf_target]);
            state->killed_last_night = wolf_target;
        }
        printf("狼人請閉眼。確認後請輸入 0 並按 Enter 閉眼：");
        scanf("%d", &dummy);
        system(CLEAR_CMD);

        // --- 2. 女巫行動 ---
        printf("\n女巫請睜眼\n");
        int witch_id = -1;
        for (int i = 1; i <= 12; i++) {
            if (players[i].role == ROLE_WITCH) {
                witch_id = i;
                break;
            }
        }

        if (witch_id != -1 && players[witch_id].is_alive) {
            if (!state->witch_has_antidote && !state->witch_has_poison) {
                printf("你的解藥與毒藥皆已用盡。\n");
            } else {
                if (state->witch_has_antidote) {
                    if (state->killed_last_night == 0) printf("今晚平安夜，沒有人被殺死。\n");
                    else printf("今晚被殺死的是 【 %d 號玩家 】。\n", state->killed_last_night);
                } else {
                    printf("你的解藥已用盡，無法得知今晚誰被殺死。\n");
                }

                printf("\n請選擇你要進行的操作：\n");
                if (state->witch_has_antidote) printf("[1] 使用解藥\n");
                if (state->witch_has_poison)   printf("[2] 使用毒藥\n");
                printf("[0] 什麼都不做\n");
                printf("你的選擇是：");

                int choice;
                scanf("%d", &choice);

                if (choice == 1 && state->witch_has_antidote) {
                    if (state->killed_last_night == 0) {
                        printf("今晚無人死亡，無法使用解藥。\n");
                    } else {
                        bool success = witch_save(players, witch_id, state->killed_last_night, state);
                        if (success) printf("成功使用解藥，救活了 %d 號玩家。\n", state->killed_last_night);
                        else printf("使用解藥失敗 (可能為自救或條件不符)。\n");
                    }
                } else if (choice == 2 && state->witch_has_poison) {
                    printf("你要毒誰呢？(請輸入玩家編號)：");
                    int poison_target;
                    scanf("%d", &poison_target);
                    if (poison_target >= 1 && poison_target <= 12 && players[poison_target].is_alive) {
                        bool success = witch_poison(&players[poison_target], state);
                        if (success) printf("成功對 %d 號玩家使用毒藥。\n", poison_target);
                    } else {
                        printf("無效的玩家編號或玩家已死亡。\n");
                    }
                } else {
                    printf("[系統提示] 選擇保留藥水，什麼都不做。\n");
                }
            }
        } else {
            printf("(女巫已死亡，法官請等待幾秒後輸入 0 繼續以混淆視聽)\n");
        }
        printf("\n女巫請閉眼。確認後請輸入 0 並按 Enter 閉眼：");
        scanf("%d", &dummy);
        system(CLEAR_CMD);

        // --- 3. 獵人行動 ---
        printf("\n獵人請睜眼\n");
        int hunter_id = -1;
        for (int i = 1; i <= 12; i++) {
            if (players[i].role == ROLE_HUNTER) {
                hunter_id = i;
                break;
            }
        }

        if (hunter_id != -1 && players[hunter_id].is_alive) {
            bool can_shoot = hunter_can_shoot(&players[hunter_id]);
            if (can_shoot) printf("今晚的開槍狀態是 ( 可以開槍 )\n");
            else printf("今晚的開槍狀態是 ( 不能開槍 )\n");
        } else {
            printf("(獵人已死亡，法官請等待幾秒後輸入 0 繼續以混淆視聽)\n");
        }
        printf("\n獵人請閉眼。確認後請輸入 0 並按 Enter 閉眼：");
        scanf("%d", &dummy);
        system(CLEAR_CMD);
    }
}

// 執行白天流程 (包含天亮宣佈死訊、技能發動與投票)
void run_day_phase(Player* players, GameState* state) {
    state->is_night = false;
    printf("\n================ 天亮請睜眼 ================\n");

    int dummy = 0;

    // 1. 呼叫底層引擎進行死亡結算
    finalize_night_results(players, 13);
    
    // 計算昨晚死亡名單
    int death_count = 0;
    int dead_ids[12];
    for (int i = 1; i <= 12; i++) {
        // 抓出「剛剛死掉的」玩家 (沒有存活，但身上帶有昨晚死因標記)
        if (!players[i].is_alive && (players[i].is_knifed || players[i].is_poisoned || (players[i].is_saved && players[i].is_guarded))) {
            dead_ids[death_count] = i;
            death_count++;
        }
    }

    // 2. 特殊板子播報：熊咆哮
    if (state->current_board == 2) {
        int bear_id = -1;
        for (int i = 1; i <= 12; i++) {
            if (players[i].role == ROLE_BEAR) { bear_id = i; break; }
        }
        if (bear_id != -1 && players[bear_id].is_alive) {
            bool roared = bear_roars(players, bear_id, state);
            if (roared) printf("=> 🐻 宣布：【 熊咆哮了！ 】\n\n");
            else printf("=> 🐻 宣布：【 熊沒有咆哮 】\n\n");
        }
    }

    // 3. 播報死訊
    if (death_count == 0) {
        printf("昨晚是平安夜，無人死亡。\n");
    } else {
        printf("昨晚死亡的是：");
        for (int i = 0; i < death_count; i++) printf("【 %d 號 】 ", dead_ids[i]);
        printf("\n");

        // 檢查死者中有沒有獵人要開槍
        if (state->current_board == 2) {
            for (int i = 0; i < death_count; i++) {
                int d_id = dead_ids[i];
                if (players[d_id].role == ROLE_HUNTER) {
                    if (hunter_can_shoot(&players[d_id])) {
                        printf("\n獵人【 %d 號 】昨晚出局，發動技能開槍！\n", d_id);
                        printf("請輸入要帶走的玩家號碼 (放棄開槍請輸入 0)：");
                        int target;
                        scanf("%d", &target);
                        if (target >= 1 && target <= 12 && players[target].is_alive) {
                            hunter_shoot(&players[target]);
                            printf("=> 砰！獵人開槍帶走了【 %d 號 】！\n", target);
                        } else {
                            printf("[系統提示] 無效的目標，獵人放棄開槍。\n");
                        }
                    } else {
                        printf("\n=> 🩸 獵人【 %d 號 】被毒殺封印，無法開槍。\n", d_id);
                    }
                }
            }
        }
    }

    // 4. 死訊播完後，判斷勝負
    state->game_result = check_win_condition(players);
    if (state->game_result != GAME_ONGOING) {
        state->is_game_over = true;
        return; // 直接跳出白天流程，交給 main.c 宣告遊戲結束
    }

    printf("\n請輸入 0 繼續進入發言環節：");
    scanf("%d", &dummy);
    system(CLEAR_CMD);

    // 5. 遺言與發言環節
    bool skip_voting = false;
    
    if (death_count > 0) {
        if (state->day_count == 1) printf("\n請昨晚死亡的玩家發表【遺言】。\n");
        else printf("\n昨晚死亡的玩家【沒有遺言】，請直接離場。\n");
        printf("(請輸入 0 繼續)：");
        scanf("%d", &dummy);
    }

    printf("\n================ 發言環節 ================\n");
    int start_speaker_id = 0;
    while (true) { // 隨機抽一個活人開始
        start_speaker_id = (rand() % 12) + 1;
        if (players[start_speaker_id].is_alive) break;
    }
    printf("=> 由抽籤決定，從【 %d 號玩家 】開始，請自行決定順序發言。\n", start_speaker_id);
    printf("------------------------------------------\n");

    bool daytime_active = true;

    while (daytime_active) {
        printf("\n--- 目前為發言階段 ---\n");
        printf("請選擇接下來發生的事件：\n");
        printf("[1] 繼續下一位玩家發言\n");

        if (state->current_board == 1) {
            printf("[2] ⚡ 騎士發動決鬥\n");
            printf("[3] 🐺 白狼王自爆 (帶走一人)\n");
            printf("[4] 🐺 普通狼人自爆\n");
        } else if (state->current_board == 2) {
            printf("[2] 🐺 狼人自爆\n");
        }
        
        printf("[0] 所有玩家發言完畢，進入投票環節\n");
        printf("請輸入選項: ");

        int day_choice;
        scanf("%d", &day_choice);

        if (day_choice == 1) {
            printf("=> 繼續發言...\n");
        } else if (day_choice == 0) {
            printf("=> 所有玩家發言完畢，準備進入投票。\n");
            daytime_active = false; 
        } else {
            // --- 突發技能邏輯 ---
            if (state->current_board == 1) {
                if (day_choice == 2) { // 騎士
                    int k_id, target_id;
                    printf("請輸入【發動者(騎士)】編號："); scanf("%d", &k_id);
                    printf("請輸入【決鬥目標】編號："); scanf("%d", &target_id);

                    if (k_id >= 1 && k_id <= 12 && target_id >= 1 && target_id <= 12 &&
                        players[k_id].role == ROLE_KNIGHT && players[k_id].is_alive && players[target_id].is_alive) {
                        
                        bool wolf_killed = knight_duel(players, k_id, target_id);
                        if (wolf_killed) {
                            printf("\n=> ⚔️ 騎士決鬥成功！【 %d 號 】是狼人，立刻出局！\n", target_id);
                            daytime_active = false; skip_voting = true;
                        } else {
                            printf("\n=> 🩸 騎士決鬥失敗！【 %d 號 】是好人，騎士以死謝罪。\n", target_id);
                        }
                    } else { printf("[系統提示] 發動失敗。\n"); }

                } else if (day_choice == 3) { // 白狼王
                    int wwk_id, target_id;
                    printf("請輸入【發動者(白狼王)】編號："); scanf("%d", &wwk_id);
                    printf("請輸入【要帶走的目標】編號："); scanf("%d", &target_id);

                    if (wwk_id >= 1 && wwk_id <= 12 && target_id >= 1 && target_id <= 12 &&
                        players[wwk_id].role == ROLE_WHITE_WOLF_KING && players[wwk_id].is_alive && players[target_id].is_alive) {
                        white_wolf_king_explode(players, wwk_id, target_id);
                        printf("\n=> 💥 白狼王自爆！強行帶走【 %d 號 】！\n", target_id);
                        daytime_active = false; skip_voting = true;
                    } else { printf("[系統提示] 發動失敗。\n"); }

                } else if (day_choice == 4) { // 普通狼人
                    int wolf_id;
                    printf("請輸入【自爆的狼人】編號："); scanf("%d", &wolf_id);
                    if (wolf_id >= 1 && wolf_id <= 12 && players[wolf_id].faction == FACTION_WOLF && players[wolf_id].is_alive) {
                        players[wolf_id].is_alive = false; players[wolf_id].can_vote = false; players[wolf_id].can_speak = false;
                        printf("\n=> 💥 【 %d 號 】狼人自爆！\n", wolf_id);
                        daytime_active = false; skip_voting = true;
                    } else { printf("[系統提示] 發動失敗。\n"); }
                }

            } else if (state->current_board == 2) {
                if (day_choice == 2) { // 狼人自爆
                    int wolf_id;
                    printf("請輸入【自爆的狼人】編號："); scanf("%d", &wolf_id);
                    if (wolf_id >= 1 && wolf_id <= 12 && players[wolf_id].faction == FACTION_WOLF && players[wolf_id].is_alive) {
                        players[wolf_id].is_alive = false; players[wolf_id].can_vote = false; players[wolf_id].can_speak = false;
                        printf("\n=> 💥 【 %d 號 】狼人自爆！\n", wolf_id);
                        daytime_active = false; skip_voting = true;
                    } else { printf("[系統提示] 發動失敗。\n"); }
                }
            }
            
            // 每次技能發動後檢查勝負
            state->game_result = check_win_condition(players);
            if (state->game_result != GAME_ONGOING) {
                state->is_game_over = true;
                return;
            }
        }
    }

    // 6. 投票環節
    if (!skip_voting) {
        printf("\n================ 投票環節 ================\n");
        printf("請進行公投，輸入出局者編號 (不投票請輸 0)：");
        int vote_id;
        scanf("%d", &vote_id);

        if (vote_id >= 1 && vote_id <= 12 && players[vote_id].is_alive) {
            
            // 白痴翻牌邏輯
            if (state->current_board == 2 && players[vote_id].role == ROLE_IDIOT) {
                idiot_reveal(&players[vote_id]);
                printf("\n=> 🃏 【 %d 號 】是白痴！翻牌發動技能，狀態視為死亡，但可繼續留在場上發言。\n", vote_id);
            } else {
                players[vote_id].is_alive = false;
                players[vote_id].can_vote = false;
                players[vote_id].can_speak = false;
                printf("\n=> ⚖️ 【 %d 號玩家 】被公投出局。\n", vote_id);

                // 獵人出局開槍邏輯
                if (state->current_board == 2 && players[vote_id].role == ROLE_HUNTER && hunter_can_shoot(&players[vote_id])) {
                    printf("\n=> 🔫 獵人【 %d 號 】被公投出局，發動技能開槍！\n", vote_id);
                    printf("請輸入要帶走的玩家號碼 (放棄開槍請輸入 0)：");
                    int target;
                    scanf("%d", &target);
                    if (target >= 1 && target <= 12 && players[target].is_alive) {
                        hunter_shoot(&players[target]);
                        printf("=> 砰！獵人開槍帶走了【 %d 號 】！\n", target);
                    } else { printf("[系統提示] 放棄開槍。\n"); }
                }
            }

            printf("\n請被公投的【 %d 號玩家 】發表遺言。\n", vote_id);
            printf("(請輸入 0 繼續)：");
            scanf("%d", &dummy);

            state->game_result = check_win_condition(players);
            if (state->game_result != GAME_ONGOING) {
                state->is_game_over = true;
                return;
            }
        } else if (vote_id != 0) {
            printf("[系統提示] 無效的投票目標或該玩家已死亡。\n");
        }
    }

    // 7. 白天順利結束，天數增加
    printf("\n請輸入 0 準備進入黑夜...");
    scanf("%d", &dummy);
    state->day_count++;
}