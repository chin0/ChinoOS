[ORG 0x00] ; 코드의 시작 어드레스를 0x00으로 지정
[BITS 16]

SECTION .text

;0x7c0 : 0x7c00은 BIOS가 부트로더를 로딩하는곳
jmp 0x07c0:START ; cs 세그먼트 레지스터에 0x7c0을 복사하면서 START레이블로 이동.

START:
    mov ax, 0x07c0 
    mov ds, ax ;ds 초기화
    mov ax, 0xb800
    mov es, ax
    mov si, 0

    .SCREENCLEARLOOP:
        mov byte [es: si], 0
        mov byte [es: si + 1], 0xf
        add si, 2
        cmp si, 80 * 25 * 2
        jl .SCREENCLEARLOOP

    mov si,0
    mov di,0

    .MESSAGELOOP:
        mov cl, byte [si + MESSAGE1] ;CL은 CX레지스터의 하위 1바이트임. 문자는 1바이트이므로 1바이트만 쓰면 충분하기 때문.

        cmp cl,0
        je .MESSAGEEND

        mov byte[es: di], cl
        add si, 1
        add di, 2

        jmp .MESSAGELOOP

    .MESSAGEEND:
        jmp $ ;현재 위치에서 무한 루프

MESSAGE1: db 'Chino64 OS Boot Loader!',0 ;출력할 메시지 정의

times 510 - ($ - $$) db 0x00  ;$: 현재 라인의 어드레스
                            ;$$: 현재 섹션(.text)의 어드레스
                            ;$ - $$: 현재 섹션을 기준으로 하는 오프셋
                            ;times: 반복
                            ;즉, 현재 위치부터 주소 510까지 0x00으로 채움.

db 0x55
db 0xAA ;부트로더 식별

