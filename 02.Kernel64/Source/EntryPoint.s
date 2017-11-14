[BITS 64]

SECTION .text

;외부에서 정의한 함수를 쓸 수 있도록 선언함(Import)
extern Main

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;   코드영역
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
START:
    mov ax, 0x10 ;IA-32e 모드 커널용 데이터 세그먼트 디스크립터를 AX 레지스터에 저장
    ;세그먼트 셀렉터에 설정
    mov ds,ax
    mov es,ax
    mov fs,ax
    mov gs,ax

    ;스택을 0x600000 ~ 0x6FFFFF 영역에 1MB로 설정
    mov ss, ax ; SS 세그먼트 셀렉터에 설정
    mov rsp, 0x6FFFF8 ;주소정렬때문에 0x6FFFFF로
    mov rbp, 0x6FFFF8 

    call Main

    jmp $


