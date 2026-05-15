#include "game_loop.h"
#include "role.h"
#include "ui.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ─────────────────────────────────────────────
//  內部輔助：找出角色的玩家編號，找不到回傳 -1
// ─────────────────────────────────────────────
static int find_player(Player *players, RoleType role) {
    for (int i = 1; i <= 12; i++)
        if (players[i].role == role)
            return i;
    return -1;
}

// ─────────────────────────────────────────────
//  內部輔助：動態生成「合法目標」的按鈕選單 (升級版)
// ─────────────────────────────────────────────
static int get_valid_target(const char *prompt, Player *players, int exclude_id_1, int exclude_id_2, bool allow_zero) {
    UI_Log(prompt);
    char *strs[13];
    int val_map[13];
    int count = 0;

    // 是否允許選擇 0 (放棄/空守/不查驗)
    if (allow_zero) {
        strs[count] = malloc(2);
        strcpy(strs[count], "0");
        val_map[count] = 0;
        count++;
    }

    // 遍歷所有玩家，過濾無效選項
    for (int i = 1; i <= 12; i++) {
        if (!players[i].is_alive) continue; // 🌟 絕對不能對死人發動技能
        if (i == exclude_id_1 || i == exclude_id_2) continue; // 🌟 排除指定對象

        strs[count] = malloc(8);
        snprintf(strs[count], 8, "%d", i);
        val_map[count] = i;
        count++;
    }

    if (count == 0) {
        UI_Log("[系統提示] 目前沒有符合條件的目標。");
        return 0; 
    }

    int sel = UI_WaitChoice((const char **)strs, count);
    int result = val_map[sel];

    for (int i = 0; i < count; i++) free(strs[i]);
    return result;
}

// ─────────────────────────────────────────────
//  獵人開槍通用輔助
// ─────────────────────────────────────────────
static void do_hunter_shoot(Player *players, int hunter_id) {
    char buf[128];
    snprintf(buf, sizeof(buf), "獵人【 %d 號 】發動技能開槍！", hunter_id);
    UI_Log(buf);
    
    int target = get_valid_target("請選擇要帶走的玩家號碼 (放棄請選 0)：", players, hunter_id, -1, true);
    
    if (target >= 1 && target <= 12) {
        hunter_shoot(&players[target]);
        snprintf(buf, sizeof(buf), "砰！獵人帶走了【 %d 號 】！請【 %d 號 】發表遺言。", target, target);
        UI_Log(buf);
    } else {
        UI_Log("[系統提示] 獵人放棄開槍。");
    }
}

// ─────────────────────────────────────────────
//  🌟 核心修正：所有出局者通用技能判定 (獵人/白痴 假象系統)
// ─────────────────────────────────────────────
static void prompt_death_skill(Player *players, GameState *state, int dead_id, bool is_voted) {
    char buf[128];
    snprintf(buf, sizeof(buf), "【 %d 號 】玩家是否發動技能？", dead_id);
    UI_Log(buf);
    
    const char *opts[] = {"是", "否"};
    int choice = UI_WaitChoice(opts, 2);
    
    if (choice == 0) { // 選擇「是」
        bool skill_success = false;
        
        // 1. 真實獵人開槍判定
        if (players[dead_id].role == ROLE_HUNTER && hunter_can_shoot(&players[dead_id])) {
            do_hunter_shoot(players, dead_id);
            skill_success = true;
        } 
        // 2. 真實白痴翻牌判定 (只有白天被公投才能發動)
        else if (is_voted && players[dead_id].role == ROLE_IDIOT) {
            idiot_reveal(&players[dead_id]);
            snprintf(buf, sizeof(buf), "【 %d 號 】是白痴！翻牌發動技能，留在場上。", dead_id);
            UI_Log(buf);
            skill_success = true;
        }
        
        // 3. 假冒發動、平民點錯，或被毒殺的獵人：一律顯示無法發動！
        if (!skill_success) {
            snprintf(buf, sizeof(buf), "【 %d 號 】玩家無法發動技能!!!", dead_id);
            UI_Log(buf);
            UI_WaitContinue();
        }
    }
}

// ─────────────────────────────────────────────
//  執行黑夜流程
// ─────────────────────────────────────────────
void run_night_phase(Player *players, GameState *state) {

    for (int i = 1; i <= 12; i++) {
        players[i].is_knifed = false;
        players[i].is_saved = false;
        players[i].is_poisoned = false;
        players[i].is_guarded = false;
    }

    state->alive_wolf_count = 0;
    for (int i = 1; i <= 12; i++)
        if (players[i].faction == FACTION_WOLF && players[i].is_alive)
            state->alive_wolf_count++;

    state->is_night = true;
    state->killed_last_night = 0;

    UI_ClearLog();
    char buf[64];
    snprintf(buf, sizeof(buf), "第 %d 天　天黑請閉眼", state->day_count);
    UI_Log("==========================================");
    UI_Log(buf);
    UI_Log("==========================================");

    // ==========================================
    //  板子 1：白狼王騎士局
    // ==========================================
    if (state->current_board == 1) {

        // ── 守衛 ──
        UI_ClearLog();
        UI_Log("守衛請睜眼");
        int guard_id = find_player(players, ROLE_GUARD);

        if (guard_id != -1 && players[guard_id].is_alive) {
            int guard_target = get_valid_target("請選擇要守護的玩家號碼 (空守請選 0)：", players, state->last_guarded_id, -1, true);
            
            if (guard_target >= 1 && guard_target <= 12) {
                guard_protect(&players[guard_target], state);
            } else {
                state->last_guarded_id = -1; 
            }
        } else {
            UI_Log("(守衛已死亡，請死者協助混淆視聽)");
        }
        UI_WaitContinue();

        // ── 狼人 ──
        UI_ClearLog();
        if (state->day_count == 1) {
            UI_Log("狼人請睜眼，白狼請比大拇指");
        } else {
            UI_Log("狼人請睜眼");
        }
        int wolf_target = get_valid_target("請選擇要擊殺的玩家號碼 (不殺請選 0)：", players, -1, -1, true);
        if (wolf_target >= 1 && wolf_target <= 12) {
            werewolf_kill(&players[wolf_target]);
            state->killed_last_night = wolf_target;
        }
        UI_WaitContinue();

        // ── 女巫 ──
        UI_ClearLog();
        UI_Log("女巫請睜眼");
        int witch_id = find_player(players, ROLE_WITCH);

        if (witch_id != -1 && players[witch_id].is_alive) {
            if (!state->witch_has_antidote && !state->witch_has_poison) {
                UI_Log("你的解藥與毒藥皆已用盡。");
                UI_WaitContinue();
            } else {
                if (state->witch_has_antidote) {
                    if (state->killed_last_night == 0)
                        UI_Log("今晚平安夜，沒有人被殺死。");
                    else {
                        snprintf(buf, sizeof(buf), "今晚被殺死的是 【 %d 號玩家 】。", state->killed_last_night);
                        UI_Log(buf);
                    }
                } else {
                    UI_Log("你的解藥已用盡，無法得知今晚誰被殺死。");
                }

                const char *opts[3];
                int opt_map[3]; 
                int opt_cnt = 0;
                
                if (state->witch_has_antidote && state->killed_last_night != 0 && state->killed_last_night != witch_id) {
                    opts[opt_cnt] = "使用解藥";
                    opt_map[opt_cnt++] = 1;
                }
                
                if (state->witch_has_poison) {
                    opts[opt_cnt] = "使用毒藥";
                    opt_map[opt_cnt++] = 2;
                }
                
                opts[opt_cnt] = "什麼都不做";
                opt_map[opt_cnt++] = 0;

                int sel = UI_WaitChoice(opts, opt_cnt);
                int choice = opt_map[sel];

                if (choice == 1) {
                    bool ok = witch_save(players, witch_id, state->killed_last_night, state);
                    if (ok) {
                        snprintf(buf, sizeof(buf), "成功使用解藥，救活了 %d 號玩家。", state->killed_last_night);
                        UI_Log(buf);
                    }
                } else if (choice == 2) {
                    int pt = get_valid_target("你要毒誰？(請選擇玩家編號)", players, witch_id, -1, false);
                    if (pt >= 1 && pt <= 12) {
                        witch_poison(&players[pt], state);
                        snprintf(buf, sizeof(buf), "成功對 %d 號玩家使用毒藥。", pt);
                        UI_Log(buf);
                    }
                } else {
                    UI_Log("[系統提示] 選擇保留藥水，什麼都不做。");
                }
                UI_WaitContinue();
            }
        } else {
            UI_Log("(女巫已死亡，請死者協助混淆視聽)");
            UI_WaitContinue();
        }

        // ── 預言家 ──
        UI_ClearLog();
        UI_Log("預言家請睜眼");
        int seer_id = find_player(players, ROLE_SEER);

        if (seer_id != -1 && players[seer_id].is_alive) {
            int seer_target = get_valid_target("請選擇要查驗的玩家號碼 (不查驗請選 0)：", players, seer_id, -1, true);
            if (seer_target >= 1 && seer_target <= 12) {
                int result = seer_investigate(players, seer_target, state);
                if (result == -1)
                    UI_Log("查驗失敗！");
                else if (result == FACTION_GOOD)
                    UI_Log("這個人是 【 好人 】。");
                else
                    UI_Log("這個人是 【 狼人 】！");
            } else {
                UI_Log("預言家選擇不查驗。");
            }
        } else {
            UI_Log("(預言家已死亡，請死者協助混淆視聽)"); 
        }
        UI_WaitContinue();
    }

    // ==========================================
    //  板子 2：熊隱狼局
    // ==========================================
    else if (state->current_board == 2) {

        // ── 隱狼 ──
        UI_ClearLog();
        UI_Log("隱狼請睜眼");
        int hw_id = find_player(players, ROLE_HIDDEN_WOLF);

        if (hw_id != -1 && players[hw_id].is_alive) {
            if (state->day_count == 1) {
                char line[128] = "你的狼隊友是：（ ";
                for (int i = 1; i <= 12; i++) {
                    if (players[i].faction == FACTION_WOLF && i != hw_id) {
                        char tmp[8];
                        snprintf(tmp, sizeof(tmp), "%d號 ", i);
                        strcat(line, tmp);
                    }
                }
                strcat(line, "）");
                UI_Log(line);
            }
            bool awakened = hidden_wolf_awakens(state, &players[hw_id]);
            UI_Log(awakened ? "回歸狼隊情況：（ 已回歸 ）" : "回歸狼隊情況：（ 無 ）");
        } else {
            UI_Log("(隱狼已死亡，請死者協助混淆視聽)");
        }
        UI_WaitContinue();

        // ── 狼人 ──
        UI_ClearLog();
        UI_Log("狼人請睜眼");
        int wolf_target = get_valid_target("請選擇要擊殺的玩家號碼 (不殺請選 0)：", players, -1, -1, true);
        if (wolf_target >= 1 && wolf_target <= 12) {
            werewolf_kill(&players[wolf_target]);
            state->killed_last_night = wolf_target;
        }
        UI_WaitContinue();

        // ── 女巫 ──
        UI_ClearLog();
        UI_Log("女巫請睜眼");
        int witch_id = find_player(players, ROLE_WITCH);

        if (witch_id != -1 && players[witch_id].is_alive) {
            if (!state->witch_has_antidote && !state->witch_has_poison) {
                UI_Log("你的解藥與毒藥皆已用盡。");
            } else {
                if (state->witch_has_antidote) {
                    if (state->killed_last_night == 0)
                        UI_Log("今晚平安夜，沒有人被殺死。");
                    else {
                        snprintf(buf, sizeof(buf), "今晚被殺死的是 【 %d 號玩家 】。", state->killed_last_night);
                        UI_Log(buf);
                    }
                } else {
                    UI_Log("你的解藥已用盡，無法得知今晚誰被殺死。");
                }

                const char *opts[3];
                int opt_map[3];
                int opt_cnt = 0;
                
                if (state->witch_has_antidote && state->killed_last_night != 0 && state->killed_last_night != witch_id) {
                    opts[opt_cnt] = "使用解藥";
                    opt_map[opt_cnt++] = 1;
                }
                if (state->witch_has_poison) {
                    opts[opt_cnt] = "使用毒藥";
                    opt_map[opt_cnt++] = 2;
                }
                opts[opt_cnt] = "什麼都不做";
                opt_map[opt_cnt++] = 0;

                int sel = UI_WaitChoice(opts, opt_cnt);
                int choice = opt_map[sel];

                if (choice == 1) {
                    bool ok = witch_save(players, witch_id, state->killed_last_night, state);
                    snprintf(buf, sizeof(buf), ok ? "成功救活了 %d 號玩家。" : "使用解藥失敗。", state->killed_last_night);
                    UI_Log(buf);
                } else if (choice == 2) {
                    int pt = get_valid_target("你要毒誰？(請選擇玩家編號)", players, witch_id, -1, false);
                    if (pt >= 1 && pt <= 12) {
                        witch_poison(&players[pt], state);
                        snprintf(buf, sizeof(buf), "成功對 %d 號使用毒藥。", pt);
                        UI_Log(buf);
                    }
                } else {
                    UI_Log("[系統提示] 什麼都不做。");
                }
            }
        } else {
            UI_Log("(女巫已死亡，請死者協助混淆視聽)");
        }
        UI_WaitContinue();

        // ── 獵人 ──
        UI_ClearLog();
        UI_Log("獵人請睜眼");
        int hunter_id = find_player(players, ROLE_HUNTER);

        if (hunter_id != -1 && players[hunter_id].is_alive) {
            bool can = hunter_can_shoot(&players[hunter_id]);
            UI_Log(can ? "今晚開槍狀態：（ 可以開槍 ）" : "今晚開槍狀態：（ 不能開槍 ）");
        } else {
            UI_Log("(獵人已死亡，請死者協助混淆視聽)");
        }
        UI_WaitContinue();
    }
}

// ─────────────────────────────────────────────
//  執行白天流程
// ─────────────────────────────────────────────
void run_day_phase(Player *players, GameState *state) {
    state->is_night = false;
    char buf[128];

    UI_ClearLog();
    UI_Log("================ 天亮請睜眼 ================");

    // 1. 死亡結算
    finalize_night_results(players, 13);

    int death_count = 0;
    int dead_ids[12];
    for (int i = 1; i <= 12; i++) {
        if (!players[i].is_alive &&
            (players[i].is_knifed || players[i].is_poisoned ||
             (players[i].is_saved && players[i].is_guarded))) {
            dead_ids[death_count++] = i;
        }
    }

    // 2. 熊咆哮（板子2）
    if (state->current_board == 2) {
        int bear_id = find_player(players, ROLE_BEAR);
        bool roared = false; // 預設為沒有咆哮
        
        // 🌟 修正：每天都要播報！如果熊還活著，才去判定實際咆哮狀況
        if (bear_id != -1 && players[bear_id].is_alive) {
            roared = bear_roars(players, bear_id, state);
        }
        
        // 不管熊是生是死，畫面上只會出現這兩句話，完美隱藏死訊！
        UI_Log(roared ? "=> 宣布：【 熊咆哮了！ 】" : "=> 宣布：【 熊沒有咆哮 】");
    }

    // 3. 播報死訊與詢問發動技能
    if (death_count == 0) {
        UI_Log("昨晚是平安夜，無人死亡。");
    } else {
        char line[128] = "昨晚死亡：";
        for (int i = 0; i < death_count; i++) {
            char tmp[16];
            snprintf(tmp, sizeof(tmp), "【 %d 號 】 ", dead_ids[i]);
            strcat(line, tmp);
        }
        UI_Log(line);

        // 板子2 詢問所有死亡玩家是否發動技能
        if (state->current_board == 2) {
            for (int i = 0; i < death_count; i++) {
                prompt_death_skill(players, state, dead_ids[i], false);
            }
        }
    }

    // 4. 早晨勝負判斷
    state->game_result = check_win_condition(players);
    if (state->game_result != GAME_ONGOING) {
        state->is_game_over = true;
        return;
    }

    UI_WaitContinue();

    // 5. 遺言
    if (death_count > 0) {
        UI_ClearLog();
        UI_Log(state->day_count == 1 ? "請昨晚死亡的玩家發表【遺言】。" : "昨晚死亡的玩家【沒有遺言】，請直接離場。");
        UI_WaitContinue();
    }

    // 6. 發言環節
    UI_ClearLog();
    UI_Log("================ 發言環節 ================");
    int start_id = 0;
    while (true) {
        start_id = (rand() % 12) + 1;
        if (players[start_id].is_alive)
            break;
    }
    snprintf(buf, sizeof(buf), "由抽籤決定，從【 %d 號 】開始發言。", start_id);
    UI_Log(buf);

    bool daytime_active = true;
    bool skip_voting = false;

    while (daytime_active) {

        // 動態產生發言選項
        const char *day_opts[6];
        int day_opt_map[6]; // 對應原始 choice 值
        int day_opt_cnt = 0;

        day_opts[day_opt_cnt] = "繼續下一位發言";
        day_opt_map[day_opt_cnt++] = 1;

        if (state->current_board == 1) {
            if (!state->knight_has_dueled) { 
                day_opts[day_opt_cnt] = "騎士發動決鬥";
                day_opt_map[day_opt_cnt++] = 2;
            }
            day_opts[day_opt_cnt] = "白狼王自爆";
            day_opt_map[day_opt_cnt++] = 3;
            day_opts[day_opt_cnt] = "普通狼人自爆";
            day_opt_map[day_opt_cnt++] = 4;
        } else if (state->current_board == 2) {
            day_opts[day_opt_cnt] = "狼人自爆";
            day_opt_map[day_opt_cnt++] = 2;
        }

        day_opts[day_opt_cnt] = "所有人發言完畢，進入投票";
        day_opt_map[day_opt_cnt++] = 0;

        int sel = UI_WaitChoice(day_opts, day_opt_cnt);
        int day_choice = day_opt_map[sel];

        if (day_choice == 1) {
            UI_Log("=> 繼續發言...");
        } else if (day_choice == 0) {
            UI_Log("=> 準備進入投票。");
            daytime_active = false;
        } else {
            // ── 板子1 突發技能 ──
            if (state->current_board == 1) {
                if (day_choice == 2) { // 騎士決鬥
                    int k_id = get_valid_target("發動者（騎士）編號：", players, -1, -1, false);
                    int target_id = get_valid_target("決鬥目標編號：", players, k_id, -1, false);
                    
                    if (k_id >= 1 && k_id <= 12 && target_id >= 1 && target_id <= 12) {
                        int duel_result = knight_duel(players, k_id, target_id, state);
                        if (duel_result == 1) {
                            snprintf(buf, sizeof(buf), "騎士決鬥成功！【 %d 號 】是狼人，立刻出局！", target_id);
                            UI_Log(buf);
                            daytime_active = false;
                            skip_voting = true;
                        } else if (duel_result == 0) {
                            snprintf(buf, sizeof(buf), "騎士決鬥失敗！【 %d 號 】是好人，騎士以死謝罪。", target_id);
                            UI_Log(buf);
                        } else {
                            UI_Log("[系統提示] 發動無效。（發動者並非存活騎士，或技能已用過）");
                        }
                    }
                }

                else if (day_choice == 3) { // 白狼王自爆
                    int wwk_id = get_valid_target("白狼王編號：", players, -1, -1, false);
                    int target_id = get_valid_target("要帶走的目標：", players, wwk_id, -1, false);

                    if (wwk_id >= 1 && wwk_id <= 12 && target_id >= 1 && target_id <= 12) {
                        if (players[wwk_id].role == ROLE_WHITE_WOLF_KING && players[wwk_id].is_alive) {
                            white_wolf_king_explode(players, wwk_id, target_id);
                            snprintf(buf, sizeof(buf), "白狼王自爆！強行帶走【 %d 號 】！", target_id);
                            UI_Log(buf);
                            daytime_active = false;
                            skip_voting = true;
                        } else {
                            UI_Log("[系統提示] 發動失敗。");
                        }
                    }
                } else if (day_choice == 4) { // 普通狼人自爆
                    int wolf_id = get_valid_target("自爆的狼人編號：", players, -1, -1, false);
                    if (wolf_id >= 1 && wolf_id <= 12) {
                        if (players[wolf_id].faction == FACTION_WOLF && players[wolf_id].is_alive) {
                            players[wolf_id].is_alive = false;
                            players[wolf_id].can_vote = false;
                            players[wolf_id].can_speak = false;
                            snprintf(buf, sizeof(buf), "【 %d 號 】狼人自爆！請發表遺言後進入黑夜。", wolf_id);
                            UI_Log(buf);
                            daytime_active = false;
                            skip_voting = true;
                        } else {
                            UI_Log("[系統提示] 該玩家非狼人陣營，無法自爆！");
                        }
                    }
                }
            }
            // ── 板子2 突發技能 ──
            else if (state->current_board == 2) {
                if (day_choice == 2) { // 狼人自爆
                    int wolf_id = get_valid_target("自爆的狼人編號：", players, -1, -1, false);
                    if (wolf_id >= 1 && wolf_id <= 12) {
                        if (players[wolf_id].faction == FACTION_WOLF && 
                            players[wolf_id].role != ROLE_HIDDEN_WOLF && 
                            players[wolf_id].is_alive) {
                            
                            players[wolf_id].is_alive = false;
                            players[wolf_id].can_vote = false;
                            players[wolf_id].can_speak = false;
                            snprintf(buf, sizeof(buf), "【 %d 號 】狼人自爆！請發表遺言後進入黑夜。", wolf_id);
                            UI_Log(buf);
                            daytime_active = false;
                            skip_voting = true;
                        } else {
                            UI_Log("[系統提示] 該玩家非狼人陣營，無法自爆！");
                        }
                    }
                }
            }

            // 技能後勝負檢查
            state->game_result = check_win_condition(players);
            if (state->game_result != GAME_ONGOING) {
                state->is_game_over = true;
                return;
            }
        }
    }

    // 7. 投票
    if (!skip_voting) {
        UI_ClearLog();
        UI_Log("================ 投票環節 ================");
        
        int vote_id = get_valid_target("請選擇出局者編號 (不投票請選 0)：", players, -1, -1, true);

        if (vote_id >= 1 && vote_id <= 12) {
            
            players[vote_id].is_alive = false;
            players[vote_id].can_vote = false;
            players[vote_id].can_speak = false;
            snprintf(buf, sizeof(buf), "【 %d 號玩家 】被公投出局。", vote_id);
            UI_Log(buf);

            if (state->current_board == 2) {
                prompt_death_skill(players, state, vote_id, true);
            }

            snprintf(buf, sizeof(buf), "請被公投的【 %d 號 】發表遺言。", vote_id);
            UI_Log(buf);
            UI_WaitContinue();

            state->game_result = check_win_condition(players);
            if (state->game_result != GAME_ONGOING) {
                state->is_game_over = true;
                return;
            }
        }
    }

    // 8. 天數增加，進入黑夜
    UI_Log("準備進入黑夜...");
    UI_WaitContinue();
    state->day_count++;
}