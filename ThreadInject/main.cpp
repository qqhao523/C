#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include <windows.h>
#include <tlhelp32.h>

bool Privilege()
{
    //������ǰ���̵ķ�������
    HANDLE hToken=NULL;

    if(OpenProcessToken(GetCurrentProcess(),TOKEN_ALL_ACCESS,&hToken)!=TRUE) return false;
    TOKEN_PRIVILEGES tp;
    tp.PrivilegeCount=1;
    LookupPrivilegeValue(NULL,SE_DEBUG_NAME,&tp.Privileges[0].Luid);
    tp.Privileges[0].Attributes=SE_PRIVILEGE_ENABLED;
    AdjustTokenPrivileges(hToken,FALSE,&tp,sizeof(tp),NULL,NULL);

    CloseHandle(hToken);
    return true;
}

bool ListProcess()
{
    HANDLE hProcessSnapshot=CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,NULL);   //��ȡ���̿���
    if(hProcessSnapshot==INVALID_HANDLE_VALUE)
    {
        printf("�������̿���ʧ��!\n");
        return false;
    }
    Privilege();    //��������Ȩ��
    PROCESSENTRY32 pe32;    //������ſ��ս�����Ϣ�Ľṹ��
    /*
        typedef struct tagPROCESSENTRY32
        {
            DWORD dwSize;
            DWORD cntUsage;
            DWORD th32ProcessID;
            ULONG_PTR th32DefaultHeapID;
            DWORD th32ModuleID;
            DWORD cntThreads;
            DWORD th32ParentProcessID;
            LONG pcPriClassBase;
            DWORD dwFlags;
            TCHAR szExeFile[MAX_PATH];
        } PROCESSENTRY32, *PPROCESSENTRY32;
    */
    pe32.dwSize=sizeof(PROCESSENTRY32);   //��Žṹ���С
    if(!Process32First(hProcessSnapshot,&pe32))
    {
        printf("�оٵ�һ������ʧ�ܡ�\n");
        return false;
    }
    do
    {
        printf("FileName:%s\t\t\tPID:%d\n",pe32.szExeFile,pe32.th32ProcessID);
    }
    while(Process32Next(hProcessSnapshot,&pe32));

    return true;
}

int main(int argc,char *argv[])
{
    HANDLE hRemoteProcess;
    HANDLE hRemoteThread;
    DWORD dwRemoteProcess;
    char DllPath[260];
    DWORD size;

    ListProcess();
    printf("������Ҫע����̵�ID:");
    if(scanf("%d",&dwRemoteProcess)!=1) return -1;

    hRemoteProcess=OpenProcess(PROCESS_ALL_ACCESS,false,dwRemoteProcess);   //��Զ�̽���
    if(hRemoteProcess==0)
    {
        printf("�򿪽���ʧ�ܡ�\n");
        getch();
        return -1;
    }

    memset(DllPath,NULL,sizeof(DllPath));
    GetCurrentDirectoryA(sizeof(DllPath)-1,DllPath);
    strcat(DllPath,"\\DLL_Test.dll");
    puts(DllPath);
    LPVOID pRemoteDllPath=VirtualAllocEx(hRemoteProcess,NULL,strlen(DllPath)+1,MEM_COMMIT,PAGE_READWRITE);  //�ڽ����п��ٿռ�
    if(pRemoteDllPath==NULL)
    {
        printf("VirtualAlloc Error!\n");
        getch();
        return -1;
    }

    if(WriteProcessMemory(hRemoteProcess,pRemoteDllPath,DllPath,strlen(DllPath)+1,&size)==0)   //����̿ռ���д������
    {
        printf("WriteProcessMemory Error!\n");
        getch();
        return -1;
    }

    //���Զ�̽�����LoadLibrary()�ĵ�ַ
    LPTHREAD_START_ROUTINE pLoadLibrary = (LPTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandle(TEXT("kernel32.dll")), \
        "LoadLibraryA");
    if (pLoadLibrary == NULL)
    {
        printf("GetProcAddress error\n");
        getch();
        return -1;
    }
    if((hRemoteThread=CreateRemoteThread(hRemoteProcess,NULL,0,pLoadLibrary,pRemoteDllPath,0,NULL))==NULL)
    {
        printf("�����߳�ʧ�ܡ�\n");
        getch();
        return -1;
    }
    WaitForSingleObject(hRemoteThread,INFINITE);
    //�ͷ�ռ�õ��ڴ�
    if(VirtualFreeEx(hRemoteProcess,pRemoteDllPath,0,MEM_RELEASE)==NULL)
    {
        printf("VirtualFreeEx Error!\n");
        getch();
        return -1;
    }
    CloseHandle(hRemoteProcess);
    CloseHandle(hRemoteThread);
    printf("���������\n");
    getch();

    return 0;
}
