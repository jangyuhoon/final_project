/**
 * =================================================================================================
 * [C Command Battle AI Challenge]
 * =================================================================================================
 * * 과제 설명:
 * 본 파일(PlayerX.c)은 학생이 구현할 AI 로직을 담고 있습니다.
 * 학생은 simple_killer_ai와 같은 CommandFn 형태의 AI 함수를 작성하여,
 * 매 턴 게임 상태(my_info, opponent_info)를 분석하고 다음 행동(CMD_ID)을 반환해야 합니다.
 * * -------------------------------------------------------------------------------------------------
 * * 🚨 핵심 제약 사항 (절대 준수해야 함) 🚨
 * * 1. 헤더 파일 제한:
 * - 프로젝트 내에서 "api.h" 외의 다른 헤더 파일을 include 하는 것은 엄격히 금지됩니다.
 * - 표준 라이브러리(Standard Library) 함수는 <stdlib.h>, <stdio.h> 등에 포함된
 * 기본적인 함수(abs, rand, printf, scanf 등)만 사용해야 합니다.
 * * 2. 실행 흐름:
 * - 본 파일은 main.c의 main 함수 실행 전에 student1_ai_entry() 함수를 통해 시스템에 연결됩니다.
 * - **AI 로직은 반드시 CommandFn 형태의 함수로 구현**되어야 합니다.
 * * -------------------------------------------------------------------------------------------------
 * * 🎯 학생의 주요 임무 (매 턴 수행):
 * * - AI 함수는 오직 하나의 커맨드 ID (1 ~ 19)를 반환하는 것에 집중합니다.
 * - 함수는 get_player_x() 등의 API Getter 함수만을 사용하여 정보를 조회하고,
 * 게임 상태를 직접 변경해서는 안 됩니다.
 * * -------------------------------------------------------------------------------------------------
 * * 💡 시스템 작동 보장 및 폴백 (Fallback)
 * * - AI 함수 미구현 시: 만약 학생이 AI 함수를 구현하지 않았거나 등록에 실패한 경우,
 * 프로그램은 자동으로 내장된 수동 입력 모드로 전환되어, 사용자가 직접 콘솔에 커맨드를
 * 입력하여 캐릭터를 제어할 수 있도록 합니다. (과제 제출 시에는 반드시 AI 함수를 등록해야 함.)
 * * =================================================================================================
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
// [학생 구현 영역 2] 퍼즐 풀이 로직
// =================================================================================================

#define MAX_LINE_LENGTH 256
#define MAX_ITEMS 30
#define MAX_NAME_LENGTH 50

// CMD_POISON 스킬 해금을 위한 퍼즐 풀이 함수
const char* skill_1() {
    // 정답 문자열을 저장할 static 버퍼
    static char final_answer[MAX_LINE_LENGTH] = "";
    
    FILE* file = fopen("E:/C_FinalProject/C_FinalProject/AI1-2_C_Final.csv", "r");
    if (file == NULL) {
        perror("Error opening file C_FinalProject/AI1-2_C_Final.csv");
        return ""; // 파일 열기 실패 시 빈 문자열 반환
    }

    char line[MAX_LINE_LENGTH];
    // 아이템 이름을 동적으로 저장할 포인터 배열
    char* matching_names[MAX_ITEMS];
    int matching_count = 0;

    // 헤더 라인 건너뛰기
    if (fgets(line, sizeof(line), file) == NULL) {
        fclose(file);
        return "";
    }

    // 파일 끝까지 한 줄씩 읽기
    while (fgets(line, sizeof(line), file) != NULL && matching_count < MAX_ITEMS) {
        char* context = NULL; // strtok_s를 위한 context
        char* token;
        int atk, def, hp;
        char name[MAX_NAME_LENGTH];
        
        // 원본 line은 보존하고, 복사본을 strtok_s에 사용
        char line_copy[MAX_LINE_LENGTH];
        strncpy(line_copy, line, MAX_LINE_LENGTH - 1);
        line_copy[MAX_LINE_LENGTH - 1] = '\0';

        // ID
        token = strtok_s(line_copy, ",", &context);
        if (token == NULL) continue;

        // NAME
        token = strtok_s(NULL, ",", &context);
        if (token == NULL) continue;
        strncpy(name, token, MAX_NAME_LENGTH - 1);
        name[MAX_NAME_LENGTH - 1] = '\0';
        
        // SLOT
        token = strtok_s(NULL, ",", &context);
        if (token == NULL) continue;
        
        // ATK
        token = strtok_s(NULL, ",", &context);
        if (token == NULL) continue;
        atk = atoi(token);

        // DEF
        token = strtok_s(NULL, ",", &context);
        if (token == NULL) continue;
        def = atoi(token);

        // HP
        token = strtok_s(NULL, ",", &context);
        if (token == NULL) continue;
        hp = atoi(token);

        // 조건 확인: ATK >= 4, DEF <= 5, HP <= 100
        if (atk >= 4 && def <= 5 && hp <= 100) {
            // 조건에 맞는 아이템 이름을 동적 할당하여 저장
            matching_names[matching_count] = _strdup(name);
            if (matching_names[matching_count] != NULL) {
                matching_count++;
            }
        }
    }
    fclose(file);

    // 찾은 아이템들을 역순으로 | 기호와 함께 조합
    final_answer[0] = '\0'; // 버퍼 초기화
    for (int i = matching_count - 1; i >= 0; i--) {
        strcat(final_answer, matching_names[i]);
        if (i > 0) {
            strcat(final_answer, "|");
        }
        free(matching_names[i]); // 동적 할당된 메모리 해제
    }

    return final_answer;
}


// =================================================================================================
// [학생 구현 영역 3] 시스템 진입 및 해금 영역
// =================================================================================================

// 이 함수는 main.c에서 extern으로 호출되는 학생 코드의 진입점입니다.
void student1_ai_entry() {

    // 이 섹션의 모든 코드는 반드시 이 예제와 같이 구현되어야 합니다. (노가다 섹션)

    int my_secret_key = register_player_ai("TEAM-ALPHA", simple_killer_ai);

    // ------------------------------------------------------------------

    // 1. 퍼즐 풀이 함수를 호출하여 동적으로 정답을 얻음
    const char* poison_answer = skill_1();
    attempt_skill_unlock(my_secret_key, CMD_POISON, poison_answer);
    if (is_skill_unlocked(my_secret_key, CMD_POISON))
        printf("TEAM-ALPHA : CMD_POISON 해금 완료\n");

    attempt_skill_unlock(my_secret_key, CMD_STRIKE, "2key");
    if (is_skill_unlocked(my_secret_key, CMD_STRIKE))
        printf("TEAM-ALPHA : CMD_STRIKE 해금 완료\n");
    else
        printf("TEAM-ALPHA : CMD_STRIKE 해금 실패 ㅜㅜ\n");


    attempt_skill_unlock(my_secret_key, CMD_BLINK_DOWN, "*A**C**F**T*");
    if (is_skill_unlocked(my_secret_key, CMD_BLINK_DOWN))
        printf("TEAM-ALPHA : CMD_BLINK 4종 해금 완료\n");
    else
        printf("TEAM-ALPHA : CMD_BLINK 4종 해금 실패 ㅜㅜ\n");


    attempt_skill_unlock(my_secret_key, CMD_HEAL_ALL, "*H*");
    if (is_skill_unlocked(my_secret_key, CMD_HEAL_ALL))
        printf("TEAM-ALPHA : CMD_HEAL_ALL 해금 완료\n");
    else
        printf("TEAM-ALPHA : CMD_HEAL_ALL 해금 실패 ㅜㅜ\n");

    attempt_skill_unlock(my_secret_key, CMD_RANGE_ATTACK, "\"LOT,A\"");
    if (is_skill_unlocked(my_secret_key, CMD_RANGE_ATTACK))
        printf("TEAM-ALPHA : CMD_RANGE_ATTACK 해금 완료\n");
    else
        printf("TEAM-ALPHA : CMD_RANGE_ATTACK 해금 실패 ㅜㅜ\n");

    attempt_skill_unlock(my_secret_key, CMD_SELF_DESTRUCT, "T");
    if (is_skill_unlocked(my_secret_key, CMD_SELF_DESTRUCT))
        printf("TEAM-ALPHA : CMD_SELF_DESTRUCT 해금 완료\n");
    else
        printf("TEAM-ALPHA : CMD_SELF_DESTRUCT 해금 실패 ㅜㅜ\n");

    attempt_skill_unlock(my_secret_key, CMD_H_ATTACK, "Inf_03");
    if (is_skill_unlocked(my_secret_key, CMD_H_ATTACK))
        printf("TEAM-ALPHA : CMD_H_ATTACK,CMD_V_ATTACK  해금 완료\n");
    else
        printf("TEAM-ALPHA : CMD_H_ATTACK,CMD_V_ATTACK 해금 실패 ㅜㅜ\n");

    // CMD_SECRETE (비밀 메시지) 해금 및 설정 예시
    attempt_skill_unlock(my_secret_key, CMD_SECRETE, "wn_St");
    if (is_skill_unlocked(my_secret_key, CMD_SECRETE))
    {
        printf("TEAM-ALPHA : CMD_SECRETE 해금 완료\n");
        // set_custom_secrete_message 함수를 사용하여 도발 메시지를 등록합니다.
        set_custom_secrete_message(my_secret_key, "후후후 좁밥들...");
    }
    else
        printf("TEAM-ALPHA : CMD_SECRETE 해금 실패 ㅜㅜ\n");

    // ------------------------------------------------------------------

    printf("TEAM-ALPHA : 플레이어 초기화 완료. 아무키나 누르시오.\n");

    // getchar()는 main.c의 수동 입력 폴백 로직과 충돌할 수 있으므로, 
    // 실제 실행에서는 주석 처리되거나 제거되어야 합니다.
    getchar();
}