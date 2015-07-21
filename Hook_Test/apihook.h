#ifndef APIHOOK_H_INCLUDED
#define APIHOOK_H_INCLUDED

#include <windows.h>

class APIHook
{
public:
    APIHook();
    ~APIHook();

    //Set API Info
    int SetHookAPI(LPSTR _pModuleName,LPSTR _pAPIName,PROC _func);
    //HOOK API
    int Hook();
    //Clear HOOK
    int UnHook();

private:
    LPSTR pModuleName;      //API�������ӿ�����
    LPSTR pAPIName;         //API����
    PROC pAPIAddress;       //API��ַ
    PROC func;              //�ص��ĺ���
    BYTE oldByteData[5];    //ԭ������ڴ���
    BYTE newByteData[5];    //���ĺ�ĺ�����ڴ���
};

#endif // APIHOOK_H_INCLUDED
