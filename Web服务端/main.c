#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#pragma comment(lib,"ws2_32.lib")
#pragma comment(linker,"/subsystem:windows /entry:mainCRTStartup")

#define HTTP_PORT 80
#define HTTP_BUF_SIZE 1024
#define NAME_SIZE 1024
#define PATH_SIZE 1024

struct doc_type
{
    char *suffix;//�ļ���׺
    char *type;  //�ļ�����
};

struct doc_type file_type[]=
{
    {"html","text/html"},
    {"htm","text/html"},
    {"txt","text/plain"},
    {"jpg","image/jpeg"},
    {"gif","image/gif"},
    {"bmp","application/x-bmp"},
    {"ico","application/x-ico"},
    {"swf","application/x-shockwave-flash"},
    {NULL,NULL},
};

//��Ӧ���ײ�����
char *http_head_temp="Hello World\r\n\r\n";
char *http_head_temp1="HTTP/1.1 200 OK\r\nServer:Wayne's Server <Version 1.0.0>\r\n"
                     "Accept-Ranges:bytes\r\nContent-Length:%d\r\nConnection:close\r\n"
                     "Content-Type:%s\r\n\r\n";

//ͨ���ļ���׺���Ҷ�Ӧ���ļ�����Content-Type
char *http_get_type_by_suffix(char *suffix)
{
    struct doc_type *ty;
    for(ty=file_type; (*ty).suffix; ty++)
        if(strcmp((*ty).suffix,suffix)==0)
            return (*ty).type;
    return NULL;
}

//ͨ��URL��ȡ�����ļ����ͺ�׺
int http_analyse_url(char *buf,int buflen,char *file_path,char *file_name,char *suffix)
{
    int length=0,i=0,j,k=0;
    char *begin,*end,*bias,*p,*start;
    for(j=0;j<HTTP_BUF_SIZE;j++)
        if(buf[j]==' ') k++;
    if(k<2) return -1;
    begin=strchr(buf,' ');
    begin++;
    for(p=begin; *p=='/'; p++) begin=p;
    start=begin;     //start��ʼ��λ�ò�������/��
    end=strchr(begin,' ');

    for(p=begin; p!=end; p++)
        if(*p=='/') begin=p;
    begin++;   //��ָ�붨λ�����һ����/���ĺ���һλ
    for(p=begin; p!=end; p++)
        if(*p=='.') i=1;  //�ж�����׺�Ƿ��С�.�����б�ʾ���ʵ����ļ����ޱ�ʾ���ʵ���·��

    if(i==0)
    {
        length=end-start;   //��ȡ����·������
        memcpy(file_path,start,length);
    }
    else
    {
        length=begin-start;
        memcpy(file_path,start,length);
    }
    for(p=file_path; *p!=NULL; p++)
        if(*p=='/') *p='\\';    //��·���е�'/'��Ϊ'\'
    p--;
    if(*p!='\\')
    {
        //��·����������'\'
        p++;
        *p='\\';
    }
    if(i==1)
    {
        length=end-begin;     //��ȡ�ļ�������
        memset(file_name,NULL,NAME_SIZE);
        memcpy(file_name,begin,length);

        for(p=begin; p!=end; p++)
            if(*p=='.') begin=p;    //��λ�����һ��'.'
        begin++;
        length=end-begin; //��ȡ��չ������
        memset(suffix,NULL,NAME_SIZE);
        memcpy(suffix,begin,length);
        return 1;   //�����˷���Ŀ��
    }
    return 0;    //û�ж������Ŀ��
}
void error(SOCKET soc)
{
    int file_len=46,hdr_len;
    char http_header[HTTP_BUF_SIZE],*Content_type="text/html";
    char error[HTTP_BUF_SIZE]="<P><FONT color=#ff0000 size=7>ERROR</FONT></P>";   //46
    memset(http_header,NULL,HTTP_BUF_SIZE);
    hdr_len=sprintf(http_header,http_head_temp,file_len,Content_type); //��ʽ���ײ�����
    send(soc,http_header,hdr_len,0);
    send(soc,error,file_len,0);
    printf("���ʳ���\n========================================================================\n");
}

int http_send_response(SOCKET soc,char *buf,int buf_len)
{
    FILE *file=NULL;
    char http_header[HTTP_BUF_SIZE],file_name[NAME_SIZE]="index.html",file_name2[NAME_SIZE]="index.htm",file_path[PATH_SIZE],file_path_temp[PATH_SIZE],suffix[NAME_SIZE]="html";
    char HTTP_Send_Buff[HTTP_BUF_SIZE],*Content_type=NULL,read_buf[HTTP_BUF_SIZE],*p,*q,target_file[NAME_SIZE];
    int file_len=0,hdr_len=0,send_len=0,read_len=0;
    int results,i=0;
    memset(file_path,NULL,PATH_SIZE);
    if((results=http_analyse_url(buf,HTTP_BUF_SIZE,file_path,file_name,suffix))==-1)
    {
        error(soc);
        return -1;
    }
    memset(target_file,NULL,NAME_SIZE);
    for(p=file_path; *p!=NULL; p++)
    {
        if(*p=='\\')
        {
            p++;
            memset(file_path_temp,NULL,PATH_SIZE);
            for(q=p; *q=='\\'; q++);
            memcpy(file_path_temp,q,strlen(q));
            memcpy(p,file_path_temp,strlen(file_path_temp));
            if(strlen(file_path_temp)==0)
            {
                *p=NULL;
                break;
            }
        }
    }
    strcat(target_file,&file_path[1]);
    strcat(target_file,file_name);
    if(results==1)
    {
        if((file=fopen(target_file,"rb"))==NULL)
        {
            printf("����\"%s\"ʧ��,�����ڴ��ļ�\n",target_file);
            error(soc);
            return -1;
        }
    }
    //��ȡ�ļ���С
    else
    {
        if((file=fopen(target_file,"rb"))==NULL)
        {
            memset(target_file,NULL,NAME_SIZE);
            strcat(target_file,&file_path[1]);
            strcat(target_file,file_name2);
            if((file=fopen(target_file,"rb"))==NULL)
            {
                printf("����ҳ�ļ�\n");
                error(soc);
                return -1;
            }
        }
    }
    printf("�����ļ�:%s\n",target_file);
    fseek(file,0,SEEK_END);
    file_len=ftell(file);
    fseek(file,0,SEEK_SET);
    if((Content_type=http_get_type_by_suffix(suffix))==NULL)
    {
        puts("�޴��ļ�����");
        error(soc);
        return -1;
    }
    hdr_len=sprintf(http_header,http_head_temp,file_len,Content_type); //��ʽ���ײ�����
    if((send_len=send(soc,http_header,hdr_len,0))==SOCKET_ERROR)
    {
        return -1;
    }
    do
    {
        read_len=fread(read_buf,sizeof(char),HTTP_BUF_SIZE,file);
        if(read_len>0)
        {
            send_len=send(soc,read_buf,read_len,0);
            file_len-=read_len;
        }
    }
    while((read_len>0)&&(file_len>0));
    fclose(file);
    printf("========================================================================\n");
    return 1;
}

int main(int argc, _TCHAR* argv[])
{
    WSADATA wsa;
    SOCKET srv_soc,acpt_soc;
    struct sockaddr_in serv_addr;
    struct sockaddr_in from_addr;
    char recv_buf[HTTP_BUF_SIZE];
    memset(recv_buf,NULL,sizeof(recv_buf));
    int port=HTTP_PORT,from_len=sizeof(from_addr),recv_len;
    /*if(argc==2)
    {
        port=atoi(argv[1]);
    }*/
    WSAStartup(MAKEWORD(2,0),&wsa);
    if((srv_soc=socket(AF_INET,SOCK_STREAM,0))==INVALID_SOCKET) exit(0);
    serv_addr.sin_family=AF_INET;
    serv_addr.sin_port=htons(port);
    serv_addr.sin_addr.s_addr=htonl(INADDR_ANY);
    if(bind(srv_soc,(struct sockaddr *)&serv_addr,sizeof(serv_addr))==SOCKET_ERROR)
    {
        puts("�󶨵�ַʧ��");
        system("pause");
        exit(0);
    }
    if(listen(srv_soc,SOMAXCONN)!=0)
    {
        puts("�׽ӿ�ת����ģʽʧ��");
        system("pause");
        exit(0);
    }
    printf("The Service Started...\n");
    while(1)
    {
        if((acpt_soc=accept(srv_soc,(struct sockaddr *)&from_addr,&from_len))==INVALID_SOCKET)
        {
            puts("���ܿͻ������ӷ�������ʧ��");
            closesocket(acpt_soc);
            continue;
        }
        printf("�ͻ���%s����\n",inet_ntoa(from_addr.sin_addr));
        if(recv(acpt_soc,recv_buf,HTTP_BUF_SIZE,0)==SOCKET_ERROR)
        {
            puts("��������ʧ��");
            closesocket(acpt_soc);
            continue;
        }
        recv_buf[HTTP_BUF_SIZE-1]=NULL;
        http_send_response(acpt_soc,recv_buf,recv_len);
        closesocket(acpt_soc);
    }
}












