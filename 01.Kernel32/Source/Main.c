#include "Types.h"
#include "Page.h"
#include "ModeSwitch.h"

void kPrintString(int iX, int iY, const char* pcString);
BOOL kInitializeKernel64Area(void);
BOOL kIsMemoryEnough(void);
void kCopyKernel64ImageTo2Mbyte(void);

void Main(void)
{
    DWORD i;
    DWORD dwEAX, dwEBX, dwECX, dwEDX;
    char vcVendorString[13] = { 0, };

    kPrintString(0, 3, "Protected Mode C Language Kernel Start.................[Pass]");

    //최소 메모리 크기를 만족하는지 검사
    kPrintString(0,4, "Minimum Memory Size Check................[    ]");
    if(kIsMemoryEnough() == FALSE) 
    {
        kPrintString(42,4,"Fail");
        kPrintString(0,5,"Not enough Memory~!! Mint64 OS Requires Over 64Mbyte Memory~!!");
        while(1);
    }
    else
    {
        kPrintString(42,4,"Pass");
    }

    kPrintString(0,5, "IA-32e Kernel Area Initialize..........[    ]");
    if(kInitializeKernel64Area() == FALSE)
    {
        kPrintString(40,5,"Fail");
        kPrintString(0,6,"Kernel Area Initialization Fail.");
        while(1);
    }
    kPrintString(40,5, "Pass");

    kPrintString(0,6, "IA-32e Page Tables Initialize..........[    ]");
    kInitializePageTables();
    kPrintString(40,6,"Pass");

    //프로세서 제조사 정보 읽기
    //제조사 문자열을 받을때 하위바이트에서 상위바이트의 순서대로 저장되므로 이를 문자열 버퍼로 그대로 복사하면 하나의 문자열을 얻을수있음.
    kReadCPUID(0x00, &dwEAX, &dwEBX, &dwECX, &dwEDX);
    *(DWORD*)vcVendorString = dwEBX;
    *((DWORD*)vcVendorString + 1) = dwEDX;
    *((DWORD*)vcVendorString + 2) = dwECX;
    kPrintString(0,7,"Processor Vendor String.............[              ]");
    kPrintString(38,7,vcVendorString);
    
    //64비트 지원 유무 확인
    kReadCPUID(0x80000001, &dwEAX, &dwEBX, &dwECX, &dwEDX);
    kPrintString(0, 8, "64bit mode support check.............[    ]");
    if(dwEDX & (1 << 29))
    {
        kPrintString(38,8, "Pass");
    }
    else
    {
        kPrintString(45,8,"Fail");
        kPrintString(0,9,"This processor does not support 64bit mode.");
        while(1);
    }

    //IA-32e모드 커널을 2MB 어드레스로 이동
    kPrintString(0,9, "Copy IA-32e Kernel To 2M Address.........[    ]");
    kCopyKernel64ImageTo2Mbyte();
    kPrintString(43,9,"PASS");
    //switch to IA-32e mode.
    kPrintString(0,10,"Switch To IA-32e mode");
    kSwitchAndExecute64bitKernel();
    
    while(1);
}

void kPrintString(int iX, int iY, const char* pcString)
{
    CHARACTER* pstScreen = (CHARACTER*) 0xb8000;
    int i;

    pstScreen += (iY * 80) + iX;
    for(i = 0; pcString[i] != 0; i++)
        pstScreen[i].bCharactor = pcString[i];
}

// IA-32e 모드용 커널영역을 0으로 초기화
// 1Mbyte ~ 6Mbyte까지 영역을 초기화
BOOL kInitializeKernel64Area(void)
{
    DWORD* pdwCurrentAddress;

    //초기화를 시작할 어드레스(0x100000=1MB)설정.
    pdwCurrentAddress = (DWORD*) 0x100000;

    while((DWORD) pdwCurrentAddress < 0x600000) //6MB까지 루프를 돌면서 4바이트씩 0으로 채움.
    {
        *pdwCurrentAddress = 0x00;

        if(*pdwCurrentAddress != 0)
        {
            return FALSE;
        }

        pdwCurrentAddress++;
    }
    return TRUE;
}

//check memory size (64MB이상의 메모리를 가지고 있는지 검사)
BOOL kIsMemoryEnough(void)
{
    DWORD* pdwCurrentAddress;

    //1MB부터 검사 시작
    pdwCurrentAddress = (DWORD*) 0x100000;

    //64MB(0x4000000)까지 검사
    while((DWORD) pdwCurrentAddress < 0x4000000)
    {
        *pdwCurrentAddress = 0x12345678;

        //0x12345678로 저장한 후 다시 읽었을 때 0x12345678이 나오지 않으면 해당 어드레스를 사용하는데 문제가 있는것이므로 종료
        if(*pdwCurrentAddress != 0x12345678)
        {
            return FALSE;
        }
        //1MB씩 이동하면서 확인
        pdwCurrentAddress += (0x1000000 / 4);
    }
    return TRUE;
}

void kCopyKernel64ImageTo2Mbyte(void)
{
    WORD wKernel32SectorCount, wTotalKernelSectorCount;
    DWORD* pdwSourceAddress, * pdwDestinationAddress;
    int i;

    //0x7c05에 총 커널 섹터 수, 0x7c07에 보호 모드 커널 섹터 수가 들어있음.
    wTotalKernelSectorCount = *((WORD*) 0x7c05);
    wKernel32SectorCount = *((WORD*) 0x7c07);

    pdwSourceAddress = (DWORD*) (0x10000 + (wKernel32SectorCount * 512));
    pdwDestinationAddress = (DWORD*) 0x200000;

    //IA-32e 모드 커널 섹터 크기만큼 복사
    for(i = 0; i < 512 * (wTotalKernelSectorCount - wKernel32SectorCount) / 4; i++)
    {
        *pdwDestinationAddress = *pdwSourceAddress;
        pdwDestinationAddress++;
        pdwSourceAddress++;
    }
}


