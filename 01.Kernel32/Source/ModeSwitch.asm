[BITS 32]

;C 언어에서 호출할 수 있더록 이름 노출
global kReadCPUID, kSwitchAndExecute64bitKernel

SECTION .text

;return CPUID
; PARAM: DWORD dwEAX, DWORD* pdwEAX, *pdwEBX, *pdwECX, *pdwEDX
kReadCPUID:
    ;create stack frame
    push ebp        
    mov ebp, esp 
    ;backup register
    push eax 
    push ebx
    push ecx
    push edx
    push esi

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; Execute the CPUID instruction with the value of the EAX register. ;
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    mov eax, dword [ebp + 8] ; Store parameter 1 in the EAX register.
    cpuid

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; store returned value in the params ;
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; *pdwEAX
    mov esi, dword [ebp+12]
    mov dword [esi], eax

    ; *pdwEBX
    mov esi, dword [ebp+16]
    mov dword [esi], ebx

    ; *pdwECX
    mov esi, dword [ebp+20]
    mov dword [esi], ecx

    ; *pdwEDX
    mov esi, dword [ebp + 24];
    mov dword [esi], edx

    pop esi
    pop edx
    pop ecx
    pop ebx
    pop eax
    pop ebp
    ret

;switch to IA-32e mode and run 64bit kernel.
; PARAM: none.
kSwitchAndExecute64bitKernel:
    ;CR4 컨트롤 레지스터의 PAE 비트를 1로 설정
    mov eax, cr4 
    or eax, 0x20 ; PAE 비트(비트 5)를 1로 설정
    mov cr4, eax

    ;CR3 컨트롤 레지스터에 PML4 테이블의 어드레스와 캐시 활성화
    mov eax, 0x100000 ;EAX레지스터에 PML4 테이블이 존재하는 0x100000(1MB)를 저장
    mov cr3, eax 
    
    ; IA32_EFER.LME를 1로 설정하여 IA-32e 모드를 활성화
    mov ecx, 0xC0000080 ;IA32_EFER MSR 레지스터의 어드레스 저장
    rdmsr ;MSR 레지스터 읽기

    or eax, 0x0100 ;EAX 레지스터에 저장된 IA_EFER MSR의 하위 32비트에서
                    ; LME 비트(비트 8)을 1로 설정

    wrmsr       ;MSR 레지스터에 쓰기.

    ;CR0 컨트롤 레지스터를 NW 비트(비트 29) = 0, CD 비트(비트 30) = 0, PG 비트(비트 31) = 1로
    ;설정하여 캐시 기능과 페이징 기능 활성화
    mov eax, cr0 ;EAX 레지스터에 CR0 턴트롤 레지스터를 저장
    or eax, 0xE0000000 ; NW,CD,PG를 모두 1로 설정.
    xor eax,0x60000000 ;NW,CD를 XOR하여 0으로 설정
    mov cr0, eax

    ;CS는 mov로 설정을 할수없으므로 jmp로 조작.
    jmp 0x08:0x200000 ;CS세그먼트 셀렉터를 IA-32e모드용 코드 세그먼트로 교체하고 0x200000(2MB) 어드레스로 이동.

    ;여기는 실행되지 않음.
    jmp $
    

