[ORG 0x00] ;코드의 시작 어드레스를 0x00으로.
[BITS 16]

SECTION .text

;;;;;;;;;;;;;;;;
; 코드 영역    ;
;;;;;;;;;;;;;;;;
START:
    mov ax,0x1000 ; 보호모드 엔트리 포인트의 시작 어드레스(0x10000)를 세그먼트 레지스터값으로 변환
    mov ds,ax ;set ds
    mov es,ax ;set es
    
    cli ;disable register
    lgdt [GDTR] ; Set GDTR data structure to processor to load GDT table
    
    ;enable protected mode
    ;disable paging, disable cache, internal fpu, disable align check
    
    mov eax,0x4000003b ; PG=0, CD=1, NW=0, AM=0, WP=0, NE=1, ET=1, TS=1, EM=0,MP=1, PE=1
    mov cr0, eax

    ;커널 코드 세그먼트를 0x00 기준으로 하는것으로 교체 후 EIP의 값을 0x00을 기준으로 재설정
    jmp dword 0x08: (PROTECTEDMODE -$$ + 0x10000)

;;;;;;;;;;;;;;;;;;;;;;;
; protected mode code.;
;;;;;;;;;;;;;;;;;;;;;;;

[BITS 32]
PROTECTEDMODE:
    mov ax, 0x10;보호 모드 커널용 데이터 세그먼트 디스크립터를 AX 레지스터에 저장
    mov ds, ax ; set ds
    mov es, ax ; set es
    mov fs, ax ; set fs
    mov gs, ax ; set gs

    ;set stack 0x00000000~0x0000FFFF(64kb)
    mov ss,ax
    mov esp,0xFFFE
    mov ebp,0xFFFE

    ;print message
    push (SWITCHSUCCESSMESSAGE - $$ + 0x10000) ;set message's address
    push 2
    push 0
    call PRINTMESSAGE
    add esp,12

    ;cs 세그먼트 ㄹ셀렉터를 0x08로 변경하면서 0x10200 어드레스로(C 커널 코드로) 이동.`
    jmp dword 0x08: 0x10200 ; c언어 커널이 존재하는 0x10200 어드레스로 이동하여 C언어 커널 수행.

;;;;;;;;;;;;;;;;;;;;;;;;;;
; function code          ;
;;;;;;;;;;;;;;;;;;;;;;;;;;
;PRINTMESSAGE
PRINTMESSAGE:
    push ebp
    mov ebp,esp
    push esi
    push edi
    push eax
    push ecx
    push edx

    mov eax, dword [ebp + 12]
    mov esi, 160
    mul esi
    mov edi,eax

    mov eax, dword [ebp + 8]
    mov esi, 2
    mul esi
    add edi,eax

    mov esi, dword [ebp + 16]

.MESSAGELOOP:
    mov cl, byte [esi]

    cmp cl,0
    je .MESSAGEEND

    mov byte [edi + 0xB8000], cl
    add esi,1
    add edi,2
    jmp .MESSAGELOOP

.MESSAGEEND:
    pop edx
    pop ecx
    pop eax
    pop edi
    pop esi
    pop ebp
    ret

;;;;;;;;;;;;
; data     ;
;;;;;;;;;;;;

;아래의 데이터들을 8바이트에 맞춰 정렬하기 위해 추가
align 8, db 0

;GDTR의 끝을 8byte로 정렬하기 위해 추가
dw 0x0000
;define GDTR data structure
GDTR:
    dw GDTEND - GDT - 1
    dd (GDT - $$ + 0x10000)
;define GDT Table
GDT:
    ;NULL Descirptor
    NULLDescriptor:
        dw 0x0000
        dw 0x0000
        db 0x00
        db 0x00
        db 0x00
        db 0x00
    CODEDESCRIPTOR:
        dw 0xFFFF ; Limit [15:0]
        dw 0x0000 ; Base [15:0]
        db 0x00 ; Base [23:16]
        db 0x9A ; P = 1, DPL = 1, Code Segment, Execute/Read
        db 0xCF ; G = 1, D = 1, L = 0, Limit[19:16]
        db 0x00 ; Base [31:24]

    DATASCRIPTOR:
        dw 0xFFFF ; Limit [15:0]
        dw 0x0000 ; Base [15:0]
        db 0x00 ; Base [23:16]
        db 0x92 ; P=1, DPL=0, Data Segment, Read/Write
        db 0xCF ; G=1, D=1, L=0, Limit[19:16]
        db 0x00 ; Base[31:24]
GDTEND:

SWITCHSUCCESSMESSAGE: db 'Switch To Protected Mode Success~!!',0
times 512 - ($ - $$) db 0x00 ;512바이트 맞추기.



