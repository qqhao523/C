#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <conio.h>
#include <winsock.h>

#define MAX_SIZE 1024
#define SERVICEPORT 7777

#pragma comment(lib,"ws2_32.lib")

typedef struct
{
	struct sockaddr_in Address;
	SOCKET Socket;
}ClientSocket;

bool sendflag=false;
char SendBuff[MAX_SIZE];

DWORD WINAPI Input(PVOID Parameter)
{
	while(true)
	{
		fflush(stdin);
		gets(SendBuff);
		sendflag=true;
	}
	return 0;
}

DWORD WINAPI New_Client(PVOID Parameter)
{
	ClientSocket CS=*(ClientSocket *)Parameter;
	struct timeval timeout;
	fd_set readfd,writefd;
	char RecvBuff[MAX_SIZE];

    timeout.tv_sec=0;
	timeout.tv_usec=500;
	printf("�ͻ���%s���롣\n",inet_ntoa(CS.Address.sin_addr));
	while (true)
	{
		FD_ZERO(&readfd);
		FD_ZERO(&writefd);
		FD_SET(CS.Socket,&readfd);
		FD_SET(CS.Socket,&writefd);

		if(select(-1,&readfd,&writefd,NULL,&timeout)>0)
		{
			if(FD_ISSET(CS.Socket,&readfd))
			{
				memset(RecvBuff,NULL,sizeof(RecvBuff));
				if(recv(CS.Socket,RecvBuff,sizeof(RecvBuff),0)<=0)
				{
					printf("�ͻ���%s�Ͽ����ӡ�\n",inet_ntoa(CS.Address.sin_addr));
					return -1;
				}
				printf("����%s����Ϣ:\n%s\n",inet_ntoa(CS.Address.sin_addr),RecvBuff);
			}
			if(FD_ISSET(CS.Socket,&writefd))
			{
				if(sendflag)
				{
					sendflag=false;
					if(send(CS.Socket,SendBuff,sizeof(SendBuff),0)<=0)
					{
						printf("�ͻ���%s�Ͽ����ӡ�\n",inet_ntoa(CS.Address.sin_addr));
				     	return -1;
					}
					memset(SendBuff,NULL,sizeof(SendBuff));
				}
			}
		}
	}

	return 0;
}

int main(int argc,char *argv[])
{
	ClientSocket CS;
	WSADATA wsa;
	struct sockaddr_in ServerAddress;
	SOCKET ServerSocket=INVALID_SOCKET;
	int AddressLength;
	HANDLE hThread;

	memset(&ServerAddress,NULL,sizeof(ServerAddress));
	ServerAddress.sin_family=AF_INET;
	ServerAddress.sin_addr.s_addr=INADDR_ANY;
	ServerAddress.sin_port=htons(SERVICEPORT);

	if(WSAStartup(MAKEWORD(2,2),&wsa)!=0)
	{
		printf("Socket��ʼ��ʧ�ܡ�\n");
		getch();
		return -1;
	}
	if((ServerSocket=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP))==INVALID_SOCKET)
	{
		printf("�����׽���ʧ�ܡ�\n");
		getch();
		return -1;
	}
	if(bind(ServerSocket,(struct sockaddr *)&ServerAddress,sizeof(ServerAddress))!=0)
	{
		printf("�󶨱��ص�ַʧ�ܡ�\n");
		getch();
		return -1;
	}
	if(listen(ServerSocket,SOMAXCONN)!=0)
	{
		printf("�׽���ת����ģʽʧ�ܡ�\n");
		getch();
		return -1;
	}
	printf("The service started!\n");
	AddressLength=sizeof(CS.Address);
	CloseHandle((hThread=CreateThread(NULL,0,Input,NULL,0,NULL)));
	while(true)
	{
		memset(&CS,NULL,sizeof(ClientSocket));
		CS.Socket=accept(ServerSocket,(struct sockaddr *)&CS.Address,&AddressLength);
		CloseHandle((hThread=CreateThread(NULL,0,New_Client,(PVOID)&CS,0,NULL)));
		Sleep(1);
	}

	return 0;
}

/*
int PASCAL FAR select( int nfds, fd_set FAR* readfds,��fd_set FAR* writefds, fd_set FAR* exceptfds,��const struct timeval FAR* timeout);
nfds����һ������ֵ����ָ�����������ļ��������ķ�Χ���������ļ������������ֵ��1�����ܴ���Windows�����������ֵ����ν���������ò���ȷ��
readfds������ѡ��ָ�룬ָ��һ��ȴ��ɶ��Լ����׽ӿڡ�
writefds������ѡ��ָ�룬ָ��һ��ȴ���д�Լ����׽ӿڡ�
exceptfds������ѡ��ָ�룬ָ��һ��ȴ���������׽ӿڡ�
timeout��select()���ȴ�ʱ�䣬������������ΪNULL��
select()���÷��ش��ھ���״̬�����Ѿ�������fd_set�ṹ�е������������������ʱ�򷵻�0������Ļ�������SOCKET_ERROR����Ӧ�ó����ͨ��WSAGetLastError��ȡ��Ӧ������롣
������Ϊ-1ʱ����������������0��
������Ϊ0ʱ����ʱ���޸��κ�����������
������Ϊ��0ʱ����3�����������������1��λ����׼���õ�����������Ҳ����Ϊʲô��ÿ����select��Ҫ��FD_ISSET��ԭ��
*/
