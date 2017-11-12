#include "Types.h"
#include "Page.h"

void kPrintString(int iX, int iY, const char* pcString);
BOOL kInitializeKernel64Area(void);
BOOL kIsMemoryEnough(void);

void Main(void)
{
    DWORD i;
    kPrintString(0, 3, "C Language Kernel Start.................[Pass]");

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

//check memory size
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

