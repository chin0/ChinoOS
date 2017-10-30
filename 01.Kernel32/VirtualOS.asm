[ORG 0x00]
[BITS 16]

SECTION .text

jmp 0x1000:START

SECTORCOUNT: dw 0x0000 ; 현재실행중인 섹터 번호를 저장
TOTALSECTORCOUNT equ 1024

START:
    mov ax, cs
    mov ds, ax
    mov ax, 0xB800
    mov es, ax

    %assign i 0 ; i라는 변수 지정후 0으로 초기화
    %rep TOTALSECTORCOUNT ;TOTALSECTORCOUNT에 지정된 값만큼 아래 코드를 반복
        %assign i i+1

        mov ax,2

        mul word [SECTORCOUNT]
        mov si, ax

        mov byte [ es: si + (160 * 2) ], '0' + (i % 10)
        add word [SECTORCOUNT],1

        %if i == TOTALSECTORCOUNT
            jmp $
        %else
            jmp (0x1000 + i * 0x20): 0x0000 ; 다음 섹터 오프셋으로 이동
        %endif

    times (512 - ($ - $$) % 512) db 0x00
%endrep
