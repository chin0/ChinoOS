#include "Types.h"
#include "AssemblyUtility.h"
#include "Keyboard.h"

////////////////////////////////////////////////
//키보드 컨트롤러와 키보드 제어에 관련된 함수
/////////////////////////////////////////////
//출력 버퍼(포트 0x60)에 수신된 데이터가 있는지 여부를 반환
BOOL kIsOutputBufferFull(void)
{
    //상태 레지스터(port 0x64)에서 읽은 값에 출력 버퍼 상태 비트(비트 0)가
    //1로 설정되어 있으면 출력 버퍼에 키보드가 전송한 데이터가 존재함.
    if(kInPortByte(0x64) & 0x01) {
        return TRUE;
    }
    return FALSE;
}

//입력 버퍼(포트 0x60)에 쓴 데이터가 남아있는지 여부를 반환
BOOL kIsInputBufferFull(void)
{
    //상태 레지스터에서 읽은 값에 입력 버퍼 상태 비트(비트 1)가
    //1로 설정되어있으면 아직 키보드가 데이터를 가져가지 않았음
    if(kInPortByte(0x64) & 0x02) {
        return TRUE;
    }
    return FALSE;
}

//키보드를 활성화
BOOL kActivateKeyboard(void)
{
    int i;
    int j;

    //컨트롤 레지스터(포트 0x64)에 키보드 활성화 커맨드(0xAE)를 전달하여 키보드 디바이스 활성화
    kOutPortByte(0x64, 0xAE);

    //입력 버퍼가 빌 때까지 충분히 기다렸다가 키보드 활성화 커맨드를 전송
    for(i = 0; i < 0xFFFF; i++) {
        if(kIsInputBufferFull() == FALSE) {
            break;
        }
    }

    // 입력 버퍼로 키보드 활성화(0xF4) 커맨드를 전달하여 키보드로 전송
    kOutPortByte(0x60, 0xF4);

    //ACK가 올때까지 대기.
    //ACK가 오기 전에 키보드 출력 버퍼에 키 데이터가 저장되어있을수 있으므로
    //키보드에서 전달된 데이터를 최대 0x100개까지 수신하여 ACK확인
    for(j = 0; j < 100; j++) {
        for(i = 0; i < 0xFFFF; i++) {
            //출력 버퍼가 차있으면 데이터를 읽을수있음.
            if(kIsOutputBufferFull() == TRUE) {
                break;
            }
        }
        //출력 버퍼에서 읽은 데이터가ACK이면 성공.
        if(kInPortByte(0x60) == 0xFA) {
            return TRUE;
        }
    }
    return FALSE;
}

//출력 버퍼에서 키를읽음
BYTE kGetKeyboardScanCode(void)
{
    //출력 버퍼에 데이터가 있을때까지 대기.
    while(kIsOutputBufferFull() == FALSE) {
        ;
    }
    return kInPortByte(0x60);
}

// 키보드 LED의 ON/OFF를 변경
BOOL kChangeKeyboardLED(BOOL bCapsLockOn, BOOL bNumLockOn, BOOL bScrollLockOn)
{
    int i,j;

    // 키보드에 LED 변경 커맨드 전송하고 커맨드가 처리될 때까지 대기
    for(i = 0; i < 0xFFFF; i++) {
        //입력 버퍼가 비었으면 커맨드 전송 가능
        if(kIsInputBufferFull() == FALSE) {
            break;
        }
    }

    // 출력 버퍼로 LED상태 변경 커맨드 전송(0xED)
    kOutPortByte(0x60, 0xED);
    for(i = 0; i < 0xFFFF; i++) {
        //입력 버퍼가 비어 있으면 키보드가 커맨드를 가져간 것임
        if(kIsInputBufferFull() == FALSE) {
            break;
        }
    }

    //키보드가 LED 상에 변경 커맨드를 가져갔으므로 ACK가 올때까지 대기
    for(i = 0; i < 100; i++) {
        for(j = 0; j < 0xFFFF; j++) {
            //출력 버퍼가 차 있으면 데이터를 읽을 수 있음.
            if(kIsOutputBufferFull() == FALSE) {
                break;
            }
        }

        //출력 버퍼에서 읽은 데이터가 ACK이면 성공
        if(kInPortByte(0x60) == 0xFA) {
            break;
        }
    }
    if(i >= 100) {
        return FALSE;
    }

    // LED 변경 값을 키보드로 전송하고 데이터가 처리가 완료될 때까지 대기
    kOutPortByte(0x60, (bCapsLockOn<<2) | (bNumLockOn << 1) | bScrollLockOn);
    for(i = 0; i < 0xFFFF; i++) {
        ///입력 포트가 비어있으면 키보드가 LED 데이터를 가져간것임.
        if(kIsInputBufferFull() == FALSE) {
            break;
        }
    }

    // 키보드가 LED 데이터를 가져갔으므로 ACK가 올 때까지 대기함.
    for(i = 0; i < 100; i++) {
        for(j = 0; i < 0xFFFF; i++) {
            //풀력 버퍼가 차 있으면 데이터를 얻을 수 있음
            if(kIsOutputBufferFull() == TRUE) {
                break;
            }
        }
        //출력 버퍼에서 읽은 데이터가 ACK 이면 성공
        if(kInPortByte(0x60) == 0xFA) {
            break;
        }
    }
    if(i >= 100) {
        return FALSE;
    }

    return TRUE;
}

// A 20 게이트를 활성화
void kEnableA20Gate( void ) 
{
    BYTE bOutputPortData;
    int i;

    //컨트롤 레지스터에 키보드 컨트롤러의 출력 포트 값을 읽는 커맨드 전송.
    kOutPortByte(0x64, 0xD0);
    
    //출력 포트의 데이터를 기다렸다가 읽음
    for(i = 0; i < 0xFFFF; i++) {
        //출력 버퍼가 차 있으면 데이터를 읽을 수 있음.
        if(kIsOutputBufferFull() == TRUE) {
            break;
        }
    }
    //출력 포트에 수신된 키보드 컨트롤러의 출력 포트 값을 읽음.
    bOutputPortData = kInPortByte(0x60);

    //A20 게이트 비트 설정
    bOutputPortData |= 0x01;

    //입력 버퍼에 데이터가 비어 있으면 출력 포트에 값을 쓰는 커맨드와 출력 포트 데이터 전송
    for(i = 0; i < 0xFFFF; i++) {
        //입력 버퍼가 비었으면 커맨드 전송 가능
        if(kIsInputBufferFull() == FALSE) {
            break;
        }
    }

    // 커맨드 레지스터에 출력 포트 설정 커맨드(0xD1)를 전달
    kOutPortByte(0x64, 0xD1);

    //입력 버퍼에 A20 게이트가 1로 설정된 값을 전달
    kOutPortByte(0x60, bOutputPortData);
}

// 프로세서를 리셋
void kReboot(void)
{
    int i;
    
    //입력 버퍼에 데이터가 비어있으면 출력 포트에 값을 쓰는 커맨드와 출력 포트 데이터 전송
    for(i = 0; i < 0xFFFF; i++) {
        //입력 버퍼가 비었으면 커맨드 전송 가능
        if( kIsInputBufferFull() == FALSE) {
            break;
        }
    }

    // 커맨드 레지스터에 출력 포트 설정 커맨드(0xD1)를 전달
    kOutPortByte(0x64, 0xD1);

    //입력 버퍼에 0을 전달하여 프로세스를 리셋함
    kOutPortByte(0x60, 0x00);

    while(1) {
        ;
    }
}

/////////////////////////////////////////////////////////
// 스캔 코드를 아스 코드로 변환하는 기능에 관련된 함수들
// //////////////////////////////////////////////////////
// 키보드 상태를 관리하는 키보드 매니저
static KEYBOARDMANAGER gs_stKeyboardManager = {0,};

// 스캔 코드를 ASCII 코드로 변환하는 테이블
static KEYMAPPINGENTRY gs_vstKeyMappingTable[KEY_MAPPINGTABLEMAXCOUNT] = {
    {KEY_NONE, KEY_NONE},
    {KEY_ESC, KEY_ESC},
    {'1', '!'},
    {'2', '@'},
    {'3', '#'},
    {'4', '$'},
    {'5', '%'},
    {'6', '^'},
    {'7', '&'},
    {'8', '*'},
    {'9', '(' },
    {'0', ')'},
    {'-', '_'},
    {'=', '+'},
    {KEY_BACKSPACE, KEY_BACKSPACE},
    {KEY_TAB, KEY_TAB},
    {'q', 'Q'},
    {'w', 'W'},
    {'e', 'E'},
    {'r', 'R'},
    {'t', 'T'},
    {'y', 'Y'},
    {'u', 'U'},
    {'i', 'I'},
    {'o', 'O'},
    {'p', 'P'},
    {'[', '{'},
    {']', '}'},
    {'\n', '\n'},
    {KEY_CTRL, KEY_CTRL},
    {'a', 'A'},
    {'s', 'S'},
    {'d', 'D'},
    {'f', 'F'},
    {'g', 'G'},
    {'h', 'H'},
    {'j', 'J'},
    {'k', 'K'},
    {'l', 'L'},
    {';', ':'},
    {'\'', '\"'},
    {'`', '~'},
    {KEY_LSHIFT, KEY_LSHIFT},
    {'\\', '|'},
    {'z', 'Z' },
    {'x', 'X'},
    {'c', 'C'},
    {'v', 'V'},
    {'b', 'B'},
    {'n', 'N'},
    {'M', 'M'},
    {',', '<'},
    {'.', '>'},
    {'/', '?'},
    {KEY_RSHIFT, KEY_RSHIFT},
    {'*', '*'},
    {KEY_LALT, KEY_LALT},
    {' ', ' '},
    {KEY_CAPSLOCK, KEY_CAPSLOCK},
    {KEY_F1, KEY_F1},
    {KEY_F2, KEY_F2},
    {KEY_F3, KEY_F3},
    {KEY_F4, KEY_F4},
    {KEY_F5, KEY_F5},
    {KEY_F6, KEY_F6},
    {KEY_F7, KEY_F7},
    {KEY_F8, KEY_F8},
    {KEY_F9, KEY_F9},
    {KEY_F10, KEY_F10},
    {KEY_NUMLOCK, KEY_NUMLOCK},
    {KEY_SCROLLLOCK, KEY_SCROLLLOCK},

    {KEY_HOME, '7'},
    {KEY_UP, '8'},
    {KEY_PAGEUP, '9'},
    {'-', '-'},
    {KEY_LEFT, '4'},
    {KEY_CENTER, '5'},
    {KEY_PAGEUP, '6'},
    {'+', '+'},
    {KEY_END, '1'},
    {KEY_DOWN, '2'},
    {KEY_PAGEDOWN, '3'},
    {KEY_INS, '0'},
    {KEY_DEL, '.'},
    {KEY_NONE, KEY_NONE},
    {KEY_NONE, KEY_NONE},
    {KEY_NONE, KEY_NONE},
    {KEY_F11, KEY_F11},
    {KEY_F12, KEY_F12},
};

//스캔 코드가 알파벳 범위인지 여부를 반환
BOOL kIsAlphabetScanCode(BYTE bScanCode) 
{
    if(('a' <= gs_vstKeyMappingTable[bScanCode].bNormalCode) &&
        (gs_vstKeyMappingTable[bScanCode].bNormalCode <= 'z')) 
    {
        return TRUE;
    }
    return FALSE;
}

//숫자 또는 기호 범위인지 여부를 반환
BOOL kIsNumberOrSymbolScanCode(BYTE bScanCode)
{
    //숫자 패드나 확장 키 범위를 제외한 범위(스캔코드 2~53)에서 영문자가 아니면
    //숫자나 기호임
    if((2<=bScanCode) && (bScanCode <= 53) &&
            (kIsAlphabetScanCode(bScanCode) == FALSE)) 
    {
        return TRUE;
    }
    return FALSE;
}
//숫자 패드 범위인지 여부를 반환
BOOL kIsNumberPadScanCode(BYTE bScanCode) 
{
    //숫자 패드는 스캔 코드의 71~83에 있음
    if((71 <= bScanCode) && (bScanCode <= 83)) {
        return TRUE;
    }

    return FALSE;
}

//조합된 키 값을 사용해야 하는지 여부를 반환
BOOL kIsUseCombinedCode(BYTE bScanCode) 
{
    BYTE bDownScanCode;
    BOOL bUseCombinedKey = FALSE;

    bDownScanCode = bScanCode & 0x7f;

    //알파벳 키라면 Shift키와 Caps Lock의 영향을 받음.
    if(kIsAlphabetScanCode(bDownScanCode) == TRUE) {
        if(gs_stKeyboardManager.bShiftDown ^ gs_stKeyboardManager.bCapsLockOn) {
            bUseCombinedKey = TRUE;
        } else {
            bUseCombinedKey = FALSE;
        }
    }
    //숫자와 기호라면 Shift키의 영향을 받음.
    else if(kIsNumberOrSymbolScanCode(bScanCode) == TRUE) {
        if(gs_stKeyboardManager.bShiftDown ^ gs_stKeyboardManager.bCapsLockOn) {
            bUseCombinedKey = TRUE;
        } else {
            bUseCombinedKey = FALSE;
        }
    }
    //숫자 패드 키라면 Num Lock키의 영향을 받음
    //0xE0만 제외하면 확장 키코드와 숫자 패드의 코드가 겹치므로,
    //확장 키 코드가 수신되지 않았을 때만 처리 조합된 코드 사용
    else if ((kIsNumberPadScanCode(bDownScanCode) == TRUE) &&
            (gs_stKeyboardManager.bExtendedCodeIn == FALSE )) {
        if(gs_stKeyboardManager.bNumLockOn == TRUE) {
            bUseCombinedKey = TRUE;
        } else {
            bUseCombinedKey = FALSE;
        }
    }

    return bUseCombinedKey;
}

//조합 키의 상태를 갱신하고 LED 상태도 동기화함.
void UpdateCombinationKeyStatusAndLED(BYTE bScanCode)
{
    BOOL bDown;
    BYTE bDownScanCode;
    BOOL bLEDStatusChanged = FALSE;

    //눌림 또는 떨어짐 상태처리, 최상위 비트(비트 7)이 1이면 키가 떨어졌음을 의미하고
    //0이면 눌림을 의미함
    if(bScanCode & 0x80) {
        bDown = FALSE;
        bDownScanCode = bScanCode & 0x7f;
    } else {
        bDown = TRUE;
        bDownScanCode = bScanCode;
    }

    // 조합 키 선택
    // Shift키의 스캔 코드(42 or 54)이면 Shift키의 상태 갱신
    if((bDownScanCode == 42) || (bDownScanCode == 54)) {
        gs_stKeyboardManager.bShiftDown = bDown;
    }
    // Caps Lock 키의 스캔 코드(58)이면 Caps Lock의 상태 갱신하고 LED 상태 변겅
    else if((bDownScanCode == 58) && (bDown == TRUE)) {
        gs_stKeyboardManager.bCapsLockOn ^= TRUE;
        bLEDStatusChanged = TRUE;
    }
    //Num Lock 키의 스캔 코드(69)이면 Num Lock의 상태를 갱신하고 LED 상태 변겅
    else if((bDownScanCode == 69) && (bDown == TRUE)) {
        gs_stKeyboardManager.bNumLockOn ^= TRUE;
        bLEDStatusChanged = TRUE;
    }
    //Scroll Lock 키의 스캔 코드(70)이면 Scroll Lock의 상태를 갱신하고 LED상태 변경
    else if((bDownScanCode == 70) && (bDown == TRUE)) {
        gs_stKeyboardManager.bScrollLockOn ^= TRUE;
        bLEDStatusChanged = TRUE;
    }
    //LED 상태가 변했으면 키보드로 커맨드를 전송하여 LED변겅
    if(bLEDStatusChanged == TRUE) {
        kChangeKeyboardLED(gs_stKeyboardManager.bCapsLockOn,
                gs_stKeyboardManager.bNumLockOn, gs_stKeyboardManager.bScrollLockOn);
    }
}

// 스캔 코드를 ASCII코드로 변환
BOOL kConvertScanCodeToASCIICode(BYTE bScanCode, BYTE* pbASCIICode, BOOL* pbFlags)
{
    BOOL bUseCombinedKey;

    //이전에 Pause키가 수신되었다면, Pause의 남은 스캔 코드를 무시
    if(gs_stKeyboardManager.iSkipCountForPause > 0) {
        gs_stKeyboardManager.iSkipCountForPause--;
        return FALSE;
    }

    //Pause 키는 특별히 처리
    if(bScanCode == 0xE1) {
        *pbASCIICode = KEY_PAUSE;
        *pbFlags = KEY_FLAGS_DOWN;
        gs_stKeyboardManager.iSkipCountForPause = KEY_SKIPCOUNTFORPAUSE;
        return TRUE;
    }
    //확장 키 코드가 들어왔을 때, 실제 키 값은 다음에 들어오므로 플래그 설정만 하고 종료
    else if(bScanCode == 0xE0) {
        gs_stKeyboardManager.bExtendedCodeIn = TRUE;
        return FALSE;
    }
    
    //조합된 키를 반환해야 하는가?
    bUseCombinedKey = kIsUseCombinedCode(bScanCode);

    //키값 설정
    if(bUseCombinedKey == TRUE) {
        *pbASCIICode = gs_vstKeyMappingTable[bScanCode & 0x7f].bCombinedCode;
    } else {
        *pbASCIICode = gs_vstKeyMappingTable[bScanCode & 0x7f].bNormalCode;
    }
    //확장 키 유무 설정
    if(gs_stKeyboardManager.bExtendedCodeIn == TRUE) {
        *pbFlags = KEY_FLAGS_EXTENDEDKEY;
        gs_stKeyboardManager.bExtendedCodeIn = FALSE;
    } else {
        *pbFlags = 0;
    }
    //눌러짐이나 떨어짐 유무 설정
    if((bScanCode & 0x80) == 0) {
        *pbFlags |= KEY_FLAGS_DOWN;
    }

    //조합 키 눌림이나 떨어짐 상태를 갱신
    UpdateCombinationKeyStatusAndLED(bScanCode);
    return TRUE;
}
