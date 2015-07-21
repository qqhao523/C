#include "apihook.h"

APIHook::APIHook()
{
    pModuleName=NULL;
    pAPIName=NULL;
    pAPIAddress=NULL;
    memset(oldByteData,NULL,5);
    memset(newByteData,NULL,5);

    return;
}

APIHook::~APIHook()
{
    if(pModuleName!=NULL)
        delete pModuleName;
    if(pAPIName!=NULL)
        delete pAPIName;

    return;
}

int APIHook::SetHookAPI(LPSTR _pModuleName,LPSTR _pAPIName,PROC _func)
{
    if(pModuleName!=NULL)
        return -1;
    pModuleName=new char(strlen(_pModuleName)+1);
    pAPIName=new char(strlen(_pAPIName)+1);
    if(pModuleName==NULL || pAPIName==NULL)
        return -2;
    memset(pModuleName,NULL,strlen(_pModuleName)+1);
    memset(pAPIName,NULL,strlen(_pAPIName)+1);

    strcat(pModuleName,_pModuleName);
    strcat(pAPIName,_pAPIName);

    this->func=_func;

    return 0;
}

int APIHook::Hook()
{
    DWORD dwNum=0;

    //��ȡָ��ģ���е�API��ַ
    pAPIAddress=(PROC)GetProcAddress(GetModuleHandle(pModuleName),pAPIName);
    if(pAPIAddress==NULL)
        return -1;

    //�ı��ڴ�ҳ��������
    VirtualProtect((void *)pAPIAddress,5,PAGE_EXECUTE_READWRITE,&Previous);

    //����õ�ַ��5���ֽڵ�����
    if(ReadProcessMemory(GetCurrentProcess(),(LPCVOID)pAPIAddress,oldByteData,5,&dwNum)==0)
        return -2;
    if(dwNum!=5)
        return -2;

    //����JMPָ��
    newByteData[0]=0xE9;  //jmp code
    *(DWORD *)(newByteData+1)=(DWORD)func-(DWORD)pAPIAddress-5;

    //������õ�JMP��ַд�뵽�ô�
    if(WriteProcessMemory(GetCurrentProcess(),(LPVOID)pAPIAddress,newByteData,5,&dwNum)==0)
        return -3;

    //��ԭҳ�汣������
    VirtualProtect((void *)pAPIAddress,5,Previous,&Previous);
    FlushInstructionCache(GetCurrentProcess(),0,0);

    if(dwNum!=5)
        return -3;

    return 0;
}

int APIHook::UnHook()
{
    DWORD dwNum=0;

    if(pAPIAddress==NULL)
        return 0;

    VirtualProtect((void *) pAPIAddress,5,PAGE_EXECUTE_READWRITE,&Previous);
    WriteProcessMemory(GetCurrentProcess(),(LPVOID)pAPIAddress,oldByteData,5,&dwNum);
    VirtualProtect((void *)pAPIAddress,5,Previous,&Previous);

    return 0;
}




