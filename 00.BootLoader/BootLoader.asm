[ORG 0x00] ; 코드의 시작 어드레스를 0x00으로 지정
[BITS 16]

SECTION .text

;0x7c0 : 0x7c00은 BIOS가 부트로더를 로딩하는곳
jmp 0x07c0:START ; cs 세그먼트 레지스터에 0x7c0을 복사하면서 START레이블로 이동.

TOTALSECTORCOUNT: dw 1

START:
    mov ax, 0x07c0 
    mov ds, ax ;ds 초기화
    mov ax, 0xb800
    mov es, ax

    mov ax, 0x0000
    mov ss, ax
    mov sp, 0xfffe
    mov bp, 0xfffe

    mov si, 0

.SCREENCLEARLOOP:
    mov byte [es: si], 0
    mov byte [es: si + 1], 0xF
    add si, 2
    cmp si, 80 * 25 * 2
    jl .SCREENCLEARLOOP
    
    ; 시작 메시지 출력
    push MESSAGE1
    push 0
    push 0
    call PRINTMESSAGE
    add sp, 6
    ; OS 이미지를 로딩한다는 메시지 출력
    push IMAGELOADINGMESSAGE
    push 1
    push 0
    call PRINTMESSAGE
    add sp,6

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;       이미지 로딩                ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;디스크 리셋
RESETDISK:
    ;BIOS Reset Function 호출
    mov ax,0
    mov dl,0
    int 0x13

    ;에러가 발생하면 에러 처리로 이동
    jc HANDLEDISKERROR

    ;디스크에서 섹터를 읽음

    ;디스크의 내용을 메모리로 복사랄 어드레스(ES:BX)를 0x10000으로 설정
    mov si,0x1000
    mov es,si
    mov bx,0000 ; [es:bx] = 0x1000:0000
    
    mov di, word [TOTALSECTORCOUNT]; 복사할 OS이미지의 섹터 수를 DI레지스터에 설정

READDATA:
    ;모든 섹터를 다 읽었는지 확인
    cmp di,0 ;복사할 섹터가 더 남았는지 확인
    je READEND ;없으면 READEND로 이동(복사완료)
    sub di, 0x1 ;복사할 섹터 수를 1 감소
    
    ; Call BIOS Read function
    mov ah, 0x02 ;BIOS Service number 2(Read Sector)
    mov al, 0x1 ; set Number of sectors to read : 1
    mov ch, byte [TRACKNUMBER] ; set track number to read
    mov cl, byte [SECTORNUMBER] ; set sector number to read
    mov dh, byte [HEADNUMBER] ;set head number to read
    mov dl, 0x00 ; drive number to to read(0=floppy disk)
    int 0x13
    jc HANDLEDISKERROR ; if error occured, jump to HANDLEERROR

    ;복사할 어드레스와 트랙, 헤드, 섹터 어드레스 계산.
    add si, 0x0020 ; Since we have read 512 bytes(0x200, one sector), 0x200 is added to segment register
    mov es, si

    ;섹터 번호를 증가시키고 마지막 세터까지 읽었는지 판단.
    ;아니라면 다시 섹터읽기 수행.
    mov al, byte [SECTORNUMBER]
    add al, 0x01
    mov byte [SECTORNUMBER], al
    cmp al, 19
    jl READDATA

    ;마지막 섹터까지 읽었으면(섹터 번호가 19이면) 헤드를 토글하고, 섹터 번호를 1로 설정
    xor byte [HEADNUMBER], 0x01
    mov byte [SECTORNUMBER], 0x01

    ;만약 헤드가 1->0으로 바뀌었으면 양쪽 헤드를 모두 읽은것이므로 아래로 이동하여 트랙번호를 1 증가
    cmp byte [HEADNUMBER], 0x00
    jne READDATA ;헤드 번호가 0이 아니면 READDATA로 이동
    
    add byte [TRACKNUMBER], 0x01
    jmp READDATA
READEND:
    ;OS 이미지가 완료되었다는 메시지를 출력
    push LOADINGCOMPLETEMESSAGE
    push 1
    push 20
    call PRINTMESSAGE
    add sp,6

    ;로딩한 가상 OS이미지 실행
    jmp 0x1000:0x0000

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; 함수 코드 영역            ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
HANDLEDISKERROR:
    push DISKERRORMESSAGE
    push 1
    push 20
    call PRINTMESSAGE

    jmp $

;message print function
PRINTMESSAGE:
    push bp
    mov bp,sp

    ;back up Caller's regsiter value
    push es
    push si
    push di
    push ax
    push cx
    push dx

    ;set es value to video mode address
    mov ax, 0xb800
    mov es,ax

    ;x,y로 비디오 메모리 어드레스 계산
    ;y좌표 계산
    mov ax, word [bp + 6] ;bp+2 - return address
    mov si, 160
    mul si
    mov di, ax

    mov ax, word [bp + 4]
    mov si, 2
    mul si
    add di, ax

    mov si, word [bp + 8]

.MESSAGELOOP:
    mov cl, byte [si]

    cmp cl, 0
    je .MESSAGEEND

    mov byte [es:di], cl

    add si,1
    add di,2

    jmp .MESSAGELOOP

.MESSAGEEND:
    pop dx
    pop cx
    pop ax
    pop di
    pop si
    pop es
    pop bp
    ret

MESSAGE1: db 'Chino64 OS Boot Loader!',0 ;출력할 메시지 정의

DISKERRORMESSAGE: db 'DISK Error!', 0
IMAGELOADINGMESSAGE: db 'OS Image Loading...',0
LOADINGCOMPLETEMESSAGE: db 'Complete~!1', 0

;디스크 읽기에 관련된 변수들
SECTORNUMBER: db 0x02
HEADNUMBER: db 0x00
TRACKNUMBER: db 0x00

times 510 - ($ - $$) db 0x00  ;$: 현재 라인의 어드레스
                            ;$$: 현재 섹션(.text)의 어드레스
                            ;$ - $$: 현재 섹션을 기준으로 하는 오프셋
                            ;times: 반복
                            ;즉, 현재 위치부터 주소 510까지 0x00으로 채움.

db 0x55
db 0xAA ;부트로더 식별

