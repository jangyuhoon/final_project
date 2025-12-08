/**
 * Modified PlayerA.c
 * Integrates puzzle-solving functions from provided docx and
 * fixes portability / parsing issues.
 */

#define _CRT_SECURE_NO_WARNINGS

#include "api.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

 // =================================================================================================
 // [학생 구현 영역 1] AI 로직 구현부
 // =================================================================================================

 // 간단한 맨하탄 거리 계산 유틸리티 함수
static int calculate_distance(const Player* p1, const Player* p2) {
    int dx = abs(get_player_x(p1) - get_player_x(p2));
    int dy = abs(get_player_y(p1) - get_player_y(p2));
    return dx + dy;
}

// MP를 사용하지 않고 공격만 시도하는 AI 로직 (예시)
int simple_killer_ai(const Player* my_info, const Player* opponent_info) {
    int distance = calculate_distance(my_info, opponent_info);

    int my_x = get_player_x(my_info);
    int opp_x = get_player_x(opponent_info);
    int my_y = get_player_y(my_info);
    int opp_y = get_player_y(opponent_info);

    // 1. 공격 판정 
    if (distance <= 1) {
        return CMD_ATTACK;
    }

    // 2. 추격 이동 (X축 우선)
    if (my_x != opp_x) {
        if (my_x < opp_x) {
            return CMD_RIGHT;
        }
        else {
            return CMD_LEFT;
        }
    }

    // 3. Y축 추격
    if (my_y != opp_y) {
        if (my_y < opp_y) {
            return CMD_DOWN;
        }
        else {
            return CMD_UP;
        }
    }

    // 4. 예외 상황 
    return CMD_ATTACK;
}

// =================================================================================================
// [학생 구현 영역 2] 퍼즐 풀이 로직 (문서 기반)
// =================================================================================================

#define MAX_LINE_LENGTH 512
#define MAX_ITEMS 200
#define MAX_NAME_LENGTH 128
#define MAX_SLOT_LENGTH 32
#define MAX_CURSE_LENGTH 64
#define CSV_TRY_1 "game_puzzle_en.csv"
#define CSV_TRY_2 "AI1-2_C_Final.csv"

// 아이템 데이터 구조체
typedef struct {
    int id;
    char name[MAX_NAME_LENGTH];
    char slot[MAX_SLOT_LENGTH];
    int atk;
    int def;
    int hp;
    char curse[MAX_CURSE_LENGTH];
    char key_frag[MAX_LINE_LENGTH];
} ItemData;

// helper: trim leading/trailing spaces & newlines
static void trim(char* s) {
    if (!s) return;
    // trim right
    int len = (int)strlen(s);
    while (len > 0 && (s[len - 1] == '\n' || s[len - 1] == '\r' || s[len - 1] == ' ' || s[len - 1] == '\t'))
    {
        s[len - 1] = '\0';
        len--;
    }
    // trim left
    char* p = s;
    while (*p == ' ' || *p == '\t') p++;
    if (p != s) memmove(s, p, strlen(p) + 1);
}

// 파일이름 반환: 존재하는 파일을 골라준다.
static const char* pick_csv_filename() {
    FILE* f = fopen(CSV_TRY_1, "r");
    if (f) { fclose(f); return CSV_TRY_1; }
    f = fopen(CSV_TRY_2, "r");
    if (f) { fclose(f); return CSV_TRY_2; }
    return NULL;
}

// 파일 읽기 (CSV)
static int read_csv_file(const char* filename, ItemData items[], int max_items) {
    if (filename == NULL) return 0;

    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        return 0;
    }

    char line[MAX_LINE_LENGTH];
    int item_count = 0;

    // 헤더 라인 건너뛰기
    if (fgets(line, sizeof(line), file) == NULL) {
        fclose(file);
        return 0;
    }

    while (fgets(line, sizeof(line), file) != NULL && item_count < max_items) {
        char buf[MAX_LINE_LENGTH];
        strncpy(buf, line, sizeof(buf) - 1);
        buf[sizeof(buf) - 1] = '\0';

        // 토큰화 준비 (ID,NAME,SLOT,ATK,DEF,HP,CURSE,KEY_FRAG)
        char* saveptr = NULL;
        char* token = NULL;

        // ID
        token = strtok(buf, ",");
        if (!token) continue;
        trim(token);
        items[item_count].id = atoi(token);

        // NAME
        token = strtok(NULL, ",");
        if (!token) continue;
        trim(token);
        strncpy(items[item_count].name, token, MAX_NAME_LENGTH - 1);
        items[item_count].name[MAX_NAME_LENGTH - 1] = '\0';

        // SLOT
        token = strtok(NULL, ",");
        if (!token) continue;
        trim(token);
        strncpy(items[item_count].slot, token, MAX_SLOT_LENGTH - 1);
        items[item_count].slot[MAX_SLOT_LENGTH - 1] = '\0';

        // ATK
        token = strtok(NULL, ",");
        if (!token) continue;
        trim(token);
        items[item_count].atk = atoi(token);

        // DEF
        token = strtok(NULL, ",");
        if (!token) continue;
        trim(token);
        items[item_count].def = atoi(token);

        // HP
        token = strtok(NULL, ",");
        if (!token) continue;
        trim(token);
        items[item_count].hp = atoi(token);

        // CURSE (may be empty)
        token = strtok(NULL, ",");
        if (token) {
            trim(token);
            strncpy(items[item_count].curse, token, MAX_CURSE_LENGTH - 1);
            items[item_count].curse[MAX_CURSE_LENGTH - 1] = '\0';
        }
        else {
            items[item_count].curse[0] = '\0';
        }

        // KEY_FRAG (rest of line; may contain commas/newlines trimmed by strtok)
        token = strtok(NULL, ",\n\r");
        if (token) {
            trim(token);
            strncpy(items[item_count].key_frag, token, MAX_LINE_LENGTH - 1);
            items[item_count].key_frag[MAX_LINE_LENGTH - 1] = '\0';
        }
        else {
            items[item_count].key_frag[0] = '\0';
        }

        item_count++;
    }

    fclose(file);
    return item_count;
}

// 퍼즐 1: 독 스킬 해금
static const char* skill_1() {
    static char final_answer[MAX_LINE_LENGTH];
    final_answer[0] = '\0';

    const char* filename = pick_csv_filename();
    ItemData items[MAX_ITEMS];
    int item_count = read_csv_file(filename, items, MAX_ITEMS);
    if (item_count == 0) return final_answer;

    // collect matching names (preserve file order)
    char names[MAX_ITEMS][MAX_NAME_LENGTH];
    int match_count = 0;
    for (int i = 0; i < item_count; ++i) {
        if (items[i].atk >= 4 && items[i].def <= 5 && items[i].hp <= 100) {
            strncpy(names[match_count], items[i].name, MAX_NAME_LENGTH - 1);
            names[match_count][MAX_NAME_LENGTH - 1] = '\0';
            match_count++;
        }
    }

    // build reversed | separated string
    final_answer[0] = '\0';
    for (int i = match_count - 1; i >= 0; --i) {
        strcat(final_answer, names[i]);
        if (i > 0) strcat(final_answer, "|");
    }

    return final_answer;
}

// 퍼즐 2: 강타 스킬 해금
static const char* skill_2() {
    static char final_answer[MAX_LINE_LENGTH];
    final_answer[0] = '\0';

    const char* filename = pick_csv_filename();
    ItemData items[MAX_ITEMS];
    int item_count = read_csv_file(filename, items, MAX_ITEMS);
    if (item_count == 0) return final_answer;

    int total_index_sum = 0;
    for (int i = 0; i < item_count; ++i) {
        // compare SLOT exactly to "W"
        if (strcmp(items[i].slot, "W") == 0) {
            char* p = strchr(items[i].key_frag, 'T');
            if (p) total_index_sum += (int)(p - items[i].key_frag);
            else total_index_sum += 0; // problem spec: absent -> 0
        }
    }

    sprintf(final_answer, "%dkey", total_index_sum);
    return final_answer;
}

// NIL 문자열 패스
static int is_nil(const char* s) {
    if (!s) return 1;
    char buf[64];
    strncpy(buf, s, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';
    trim(buf);
    return (strcmp(buf, "NIL") == 0);
}

// 퍼즐 3: 점멸(네 조합) - build concatenated key from conditions in docx
static const char* skill_3() {
    static char final_answer[MAX_LINE_LENGTH];
    final_answer[0] = '\0';

    const char* filename = pick_csv_filename();
    ItemData items[MAX_ITEMS];
    int item_count = read_csv_file(filename, items, MAX_ITEMS);
    if (item_count == 0) return final_answer;

    // 1) ID 202 DEF + ID 208 DEF = 어떤 아이템의 HP → 그 아이템의 KEY_FRAG
    // 2) ID 205 ATK * ID 212 ATK = 어떤 아이템의 ATK → 그 아이템의 KEY_FRAG
    // 3) CURSE에 "C_01" 포함된 아이템 중 가장 마지막 → KEY_FRAG
    // 4) NAME이 'I'로 시작하는 아이템 중 가장 처음 → KEY_FRAG
    char key1[MAX_LINE_LENGTH] = "";
    char key2[MAX_LINE_LENGTH] = "";
    char key3[MAX_LINE_LENGTH] = "";
    char key4[MAX_LINE_LENGTH] = "";

    int def202 = -1, def208 = -1, atk205 = -1, atk212 = -1;

    // 먼저 필요한 값 추출
    for (int i = 0; i < item_count; ++i) {
        if (items[i].id == 202) def202 = items[i].def;
        if (items[i].id == 208) def208 = items[i].def;
        if (items[i].id == 205) atk205 = items[i].atk;
        if (items[i].id == 212) atk212 = items[i].atk;
    }

    // 조건 1: DEF 202 + DEF 208 = HP 값
    if (def202 != -1 && def208 != -1) {
        int target_hp = def202 + def208;
        for (int i = 0; i < item_count; ++i) {
            if (items[i].hp == target_hp && !is_nil(items[i].key_frag)) {
                strncpy(key1, items[i].key_frag, sizeof(key1) - 1);
                key1[sizeof(key1) - 1] = '\0';
                break;
            }
        }
    }

    // 조건 2: ATK 205 * ATK 212 = ATK 값
    if (atk205 != -1 && atk212 != -1) {
        int target_atk = atk205 * atk212;
        for (int i = 0; i < item_count; ++i) {
            if (items[i].atk == target_atk && !is_nil(items[i].key_frag)) {
                strncpy(key2, items[i].key_frag, sizeof(key2) - 1);
                key2[sizeof(key2) - 1] = '\0';
                break;
            }
        }
    }

    // 조건 3: CURSE에 C_01 포함 + 가장 마지막
    for (int i = item_count - 1; i >= 0; --i) {
        if (strstr(items[i].curse, "C_01") != NULL && !is_nil(items[i].key_frag)) {
            strncpy(key3, items[i].key_frag, sizeof(key3) - 1);
            key3[sizeof(key3) - 1] = '\0';
            break;
        }
    }

    // 조건 4: NAME이 I로 시작하는 아이템 첫 번째
    for (int i = 0; i < item_count; ++i) {
        if (items[i].name[0] == 'I' && !is_nil(items[i].key_frag)) {
            strncpy(key4, items[i].key_frag, sizeof(key4) - 1);
            key4[sizeof(key4) - 1] = '\0';
            break;
        }
    }

    // 최종 조합
    final_answer[0] = '\0';
    strcat(final_answer, key1);
    strcat(final_answer, key2);
    strcat(final_answer, key3);
    strcat(final_answer, key4);

    return final_answer;
}

// 퍼즐 4: 회복2 (strcmp based)
static const char* skill_4() {
    static char final_answer[MAX_LINE_LENGTH];
    final_answer[0] = '\0';

    const char* filename = pick_csv_filename();
    ItemData items[MAX_ITEMS];
    int item_count = read_csv_file(filename, items, MAX_ITEMS);
    if (item_count == 0) return final_answer;

    for (int i = 0; i < item_count; ++i) {
        // PART1 = NAME, PART2 = SLOT
        int cmp = strcmp(items[i].name, items[i].slot);
        if (cmp >= 0) {
            strncpy(final_answer, items[i].key_frag, MAX_LINE_LENGTH - 1);
            final_answer[MAX_LINE_LENGTH - 1] = '\0';
            return final_answer;
        }
    }
    return final_answer;
}

// 퍼즐 5: 원거리 공격
static const char* skill_5() {
    static char final_answer[MAX_LINE_LENGTH];
    final_answer[0] = '\0';

    const char* filename = pick_csv_filename();
    if (!filename) {
        return final_answer;
    }

    // 1) CSV 읽기
    ItemData items[MAX_ITEMS];
    int item_count = read_csv_file(filename, items, MAX_ITEMS);
    if (item_count == 0) {
        return final_answer;
    }

    // 2) KEY_FRAG == "*K*"인 아이템의 HP 찾기
    int target_hp = -1;
    for (int i = 0; i < item_count; ++i) {
        if (strcmp(items[i].key_frag, "*K*") == 0) {
            target_hp = items[i].hp;
            break;
        }
    }
    if (target_hp < 0) {
        return final_answer;
    }

    // 3) 파일 열기
    FILE* f = fopen(filename, "rb");
    if (!f) {
        return final_answer;
    }

    // 3-1) fseek 위치 조정 (HP - 1)
    int offset = target_hp - 1;
    if (offset < 0) offset = 0;

    if (fseek(f, offset, SEEK_SET) != 0) {
        fclose(f);
        return final_answer;
    }

    // 4) 현재 위치에서 5글자 읽기
    char raw[16] = { 0 };
    size_t r = fread(raw, 1, 5, f);
    fclose(f);

    raw[r] = '\0';

    // 5) 최종 문자열 "xx" → \"xx\" 로 조합
    snprintf(final_answer, sizeof(final_answer), "\"%s\"", raw);

    return final_answer;
}

// 퍼즐 6: 자폭 (Sword in NAME) -> build S of key_frags in order -> strtok by '*' and pick longest token
static const char* skill_6() {
    static char final_answer[MAX_LINE_LENGTH];
    final_answer[0] = '\0';

    const char* filename = pick_csv_filename();
    ItemData items[MAX_ITEMS];
    int item_count = read_csv_file(filename, items, MAX_ITEMS);
    if (item_count == 0) return final_answer;

    // Build S by concatenating key_frags of items whose NAME contains "Sword" in file order
    char S[MAX_LINE_LENGTH] = "";
    for (int i = 0; i < item_count; ++i) {
        if (strstr(items[i].name, "Sword") != NULL) {
            strcat(S, items[i].key_frag);
        }
    }

    if (S[0] == '\0') return final_answer;

    // Tokenize by '*' and find longest token (first in tie)
    char copyS[MAX_LINE_LENGTH];
    strncpy(copyS, S, sizeof(copyS) - 1);
    copyS[sizeof(copyS) - 1] = '\0';

    char* tok = strtok(copyS, "*");
    char longest[MAX_LINE_LENGTH] = "";
    while (tok) {
        if ((int)strlen(tok) > (int)strlen(longest)) {
            strncpy(longest, tok, sizeof(longest) - 1);
            longest[sizeof(longest) - 1] = '\0';
        }
        tok = strtok(NULL, "*");
    }

    strncpy(final_answer, longest, sizeof(final_answer) - 1);
    final_answer[sizeof(final_answer) - 1] = '\0';
    return final_answer;
}

// 퍼즐 7: 가로/세로 마법 (longest NAME front3 + shortest CURSE back3)
static const char* skill_7() {
    static char final_answer[MAX_LINE_LENGTH];
    final_answer[0] = '\0';

    const char* filename = pick_csv_filename();
    ItemData items[MAX_ITEMS];
    int item_count = read_csv_file(filename, items, MAX_ITEMS);
    if (item_count == 0) return final_answer;

    int max_len = -1, idx_max = -1;
    int min_curse_len = 100000, idx_min_curse = -1;

    for (int i = 0; i < item_count; ++i) {
        int nlen = (int)strlen(items[i].name);
        if (nlen > max_len) { max_len = nlen; idx_max = i; }
        int clen = (int)strlen(items[i].curse);
        if (clen > 0 && clen < min_curse_len) { min_curse_len = clen; idx_min_curse = i; }
    }

    if (idx_max >= 0) {
        char part1[8] = "";
        strncpy(part1, items[idx_max].name, 3);
        part1[3] = '\0';
        strcat(final_answer, part1);
    }
    if (idx_min_curse >= 0) {
        char* curse = items[idx_min_curse].curse;
        int clen = (int)strlen(curse);
        char part2[8] = "";
        if (clen >= 3) {
            strncpy(part2, curse + (clen - 3), 3);
            part2[3] = '\0';
        }
        else {
            // if fewer than 3, take whole curse
            strncpy(part2, curse, 3);
            part2[3] = '\0';
        }
        strcat(final_answer, part2);
    }

    return final_answer;
}

// 퍼즐 8: 숨겨진 저주 코드 (NAME contains "Stone", tokenize by vowels A,E,I,O,U)
static const char* skill_8() {
    static char final_answer[MAX_LINE_LENGTH];
    final_answer[0] = '\0';

    const char* filename = pick_csv_filename();
    ItemData items[MAX_ITEMS];
    int item_count = read_csv_file(filename, items, MAX_ITEMS);
    if (item_count == 0) return final_answer;

    // find first item whose name contains "Stone"
    for (int i = 0; i < item_count; ++i) {
        if (strstr(items[i].name, "Stone") != NULL) {
            // tokenize by vowels A,E,I,O,U (both uppercase and lowercase)
            char copy[MAX_NAME_LENGTH];
            strncpy(copy, items[i].name, sizeof(copy) - 1);
            copy[sizeof(copy) - 1] = '\0';

            char token_buf[MAX_LINE_LENGTH];
            int best_len = -1;
            char best_token[MAX_LINE_LENGTH] = "";

            char cur[MAX_LINE_LENGTH] = "";
            int ci = 0;
            for (int j = 0; copy[j] != '\0'; ++j) {
                char c = copy[j];
                char up = (c >= 'a' && c <= 'z') ? (c - 32) : c;
                if (up == 'A' || up == 'E' || up == 'I' || up == 'O' || up == 'U') {
                    // vowel: terminate current token
                    if (ci > 0) {
                        cur[ci] = '\0';
                        if ((int)strlen(cur) > best_len) {
                            strncpy(best_token, cur, sizeof(best_token) - 1);
                            best_token[sizeof(best_token) - 1] = '\0';
                            best_len = (int)strlen(cur);
                        }
                        ci = 0;
                    }
                    // skip vowel
                }
                else {
                    if (ci < (int)sizeof(cur) - 2) cur[ci++] = c;
                }
            }
            // last token
            if (ci > 0) {
                cur[ci] = '\0';
                if ((int)strlen(cur) > best_len) {
                    strncpy(best_token, cur, sizeof(best_token) - 1);
                    best_token[sizeof(best_token) - 1] = '\0';
                    best_len = (int)strlen(cur);
                }
            }

            strncpy(final_answer, best_token, sizeof(final_answer) - 1);
            final_answer[sizeof(final_answer) - 1] = '\0';
            return final_answer;
        }
    }

    return final_answer;
}

// =================================================================================================
// [학생 구현 영역 3] 시스템 진입 및 해금 영역
// =================================================================================================

// 이 함수는 main.c에서 extern으로 호출되는 학생 코드의 진입점입니다.
void student1_ai_entry() {
    int my_secret_key = register_player_ai("TEAM-ALPHA", simple_killer_ai);

    // call dynamic puzzle solvers
    const char* poison_answer = skill_1();
    attempt_skill_unlock(my_secret_key, CMD_POISON, poison_answer);
    if (is_skill_unlocked(my_secret_key, CMD_POISON))
        printf("TEAM-ALPHA : CMD_POISON 해금 완료\n");
    else
        printf("TEAM-ALPHA : CMD_POISON 해금 실패 ㅜㅜ\n");

    const char* strike_answer = skill_2();
    attempt_skill_unlock(my_secret_key, CMD_STRIKE, strike_answer);
    if (is_skill_unlocked(my_secret_key, CMD_STRIKE))
        printf("TEAM-ALPHA : CMD_STRIKE 해금 완료\n");
    else
        printf("TEAM-ALPHA : CMD_STRIKE 해금 실패 ㅜㅜ\n");

    const char* blink_answer = skill_3();
    attempt_skill_unlock(my_secret_key, CMD_BLINK_DOWN, blink_answer);
    if (is_skill_unlocked(my_secret_key, CMD_BLINK_DOWN))
        printf("TEAM-ALPHA : CMD_BLINK 4종 해금 완료\n");
    else
        printf("TEAM-ALPHA : CMD_BLINK 4종 해금 실패 ㅜㅜ\n");

    const char* heal_answer = skill_4();
    attempt_skill_unlock(my_secret_key, CMD_HEAL_ALL, heal_answer);
    if (is_skill_unlocked(my_secret_key, CMD_HEAL_ALL))
        printf("TEAM-ALPHA : CMD_HEAL_ALL 해금 완료\n");
    else
        printf("TEAM-ALPHA : CMD_HEAL_ALL 해금 실패 ㅜㅜ\n");

    const char* range_answer = skill_5();
    attempt_skill_unlock(my_secret_key, CMD_RANGE_ATTACK, range_answer);
    if (is_skill_unlocked(my_secret_key, CMD_RANGE_ATTACK))
        printf("TEAM-ALPHA : CMD_RANGE_ATTACK 해금 완료\n");
    else
        printf("TEAM-ALPHA : CMD_RANGE_ATTACK 해금 실패 ㅜㅜ\n");

    const char* selfdestruct_answer = skill_6();
    attempt_skill_unlock(my_secret_key, CMD_SELF_DESTRUCT, selfdestruct_answer);
    if (is_skill_unlocked(my_secret_key, CMD_SELF_DESTRUCT))
        printf("TEAM-ALPHA : CMD_SELF_DESTRUCT 해금 완료\n");
    else
        printf("TEAM-ALPHA : CMD_SELF_DESTRUCT 해금 실패 ㅜㅜ\n");

    const char* hv_answer = skill_7();
    attempt_skill_unlock(my_secret_key, CMD_H_ATTACK, hv_answer);
    if (is_skill_unlocked(my_secret_key, CMD_H_ATTACK))
        printf("TEAM-ALPHA : CMD_H_ATTACK,CMD_V_ATTACK  해금 완료\n");
    else
        printf("TEAM-ALPHA : CMD_H_ATTACK,CMD_V_ATTACK 해금 실패 ㅜㅜ\n");

    const char* secrete_answer = skill_8();
    attempt_skill_unlock(my_secret_key, CMD_SECRETE, secrete_answer);
    if (is_skill_unlocked(my_secret_key, CMD_SECRETE)) {
        printf("TEAM-ALPHA : CMD_SECRETE 해금 완료\n");
        set_custom_secrete_message(my_secret_key, "후후후 좁밥들...");
    }
    else {
        printf("TEAM-ALPHA : CMD_SECRETE 해금 실패 ㅜㅜ\n");
    }

    printf("TEAM-ALPHA : 플레이어 초기화 완료. 아무키나 누르시오.\n");
    getchar();
}
