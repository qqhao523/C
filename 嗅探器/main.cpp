#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include <string.h>
#include <conio.h>
#include <iphlpapi.h>

#define MAX_SIZE 1500
#define SIO_RCVALL _WSAIOW(IOC_VENDOR,1)

#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib,"Iphlpapi.lib")

/*
����
#define MAX_ADAPTER_NAME_LENGTH 256
#define MAX_ADAPTER_DESCRIPTION_LENGTH 128
#define MAX_ADAPTER_ADDRESS_LENGTH 8
typedef struct _IP_ADAPTER_INFO
{
struct _IP_ADAPTER_INFO* Next;//ָ����������һ����������Ϣ��ָ��
DWORD ComboIndex;//Ԥ��ֵ
char AdapterName[MAX_ADAPTER_NAME_LENGTH + 4];//����������
char Description[MAX_ADAPTER_DESCRIPTION_LENGTH + 4];//����������
UINT AddressLength;//������Ӳ����ַ���ֽڼ���ĳ���
BYTE Address[MAX_ADAPTER_ADDRESS_LENGTH];//Ӳ����ַ��BYTE��������ʾ
DWORD Index;//����������
UINT Type;//����������
UINT DhcpEnabled;//ָ������������Ƿ���
DHCP    PIP_ADDR_STRING CurrentIpAddress;//Ԥ��ֵ
IP_ADDR_STRING IpAddressList;//����������IPv4��ַ����
IP_ADDR_STRING GatewayList;//��������������IPv4��ַ����
IP_ADDR_STRING DhcpServer;//����������DHCP��������IPv4 ��ַ����
BOOL HaveWins;
IP_ADDR_STRING PrimaryWinsServer;
IP_ADDR_STRING SecondaryWinsServer;
time_t LeaseObtained;
time_t LeaseExpires;
} IP_ADAPTER_INFO,*PIP_ADAPTER_INFO;

*/

typedef struct ipheader
{
    unsigned char ip_hl:4;				/*header length(��ͷ���ȣ�*/
    unsigned char ip_v:4;				/*version(�汾)*/
    unsigned char ip_tos;				/*type os service��������*/
    unsigned short int ip_len;			/*total length (�ܳ���)*/
    unsigned short int ip_id;			/*identification (��ʶ��)*/
    unsigned short int ip_off;			/*fragment offset field(����λ��)*/
    unsigned char ip_ttl;				/*time to live (����ʱ��)*/
    unsigned char ip_p;					/*protocol(Э��)*/
    unsigned short int ip_sum;			/*checksum(У���)*/
    unsigned int ip_src;				/*source address(Դ��ַ)*/
    unsigned int ip_dst;				/*destination address(Ŀ�ĵ�ַ)*/
} IP;									/* total ip header length: 20 bytes (=160 bits) */

typedef struct tcpheader
{
    unsigned short int sport;			/*source port (Դ�˿ں�)*/
    unsigned short int dport;			/*destination port(Ŀ�Ķ˿ں�)*/
    unsigned int th_seq;				/*sequence number(�������к�)*/
    unsigned int th_ack;				/*acknowledgement number(ȷ��Ӧ���)*/
    unsigned char th_x:4;				/*unused(δʹ��)*/
    unsigned char th_off:4;				/*data offset(����ƫ����)*/
    unsigned char Flags;				/*��־ȫ*/
    unsigned short int th_win;			/*windows(����)*/
    unsigned short int th_sum;			/*checksum(У���)*/
    unsigned short int th_urp;			/*urgent pointer(����ָ��)*/
} TCP;

typedef struct udphdr
{
    unsigned short sport;				/*source port(Դ�˿ں�)*/
    unsigned short dport;				/*destination port(Ŀ�Ķ˿ں�)*/
    unsigned short len;					/*udp length(udp����)*/
    unsigned short cksum;				/*udp checksum(udpУ���)*/
} UDP;

char *Choose_Adapter()
{
    static char IP_Address[20];
    int Adapters_Amount=0;
    int Adapter;
    IP_ADAPTER_INFO AdapterInfo[16];     //����洢������Ϣ�Ľṹ����
    DWORD ArrayLength=sizeof(AdapterInfo);                   //����������

    if(GetAdaptersInfo(AdapterInfo,&ArrayLength)!=ERROR_SUCCESS)
        return NULL;
    PIP_ADAPTER_INFO PAdapterInfo = AdapterInfo;    //IP_ADAPTER_INFO�ṹ��ָ��
    do
    {
        Adapters_Amount++;
        printf("-------------------------------------------------------\n%d.\n\n",Adapters_Amount);
        printf("������:%s\n",PAdapterInfo->AdapterName);
        printf("��������:%s\n",PAdapterInfo->Description);
        printf("IP��ַ:%s\n",PAdapterInfo->IpAddressList.IpAddress.String);
        printf("��������:%s\n",PAdapterInfo->IpAddressList.IpMask.String);
        printf("����:%s\n",PAdapterInfo->GatewayList.IpAddress.String);
        PAdapterInfo=PAdapterInfo->Next;
    }
    while(PAdapterInfo);
    printf("-------------------------------------------------------\n\n");

    printf("��ѡ��һ������:");
    while(true)
    {
        fflush(stdin);
        if(scanf("%d",&Adapter)==1 && Adapter<=Adapters_Amount && Adapter>0) break;
        printf("��������,������ѡ��:");
    }
    memset(IP_Address,NULL,sizeof(IP_Address));
    strcat(IP_Address,AdapterInfo[Adapter-1].IpAddressList.IpAddress.String);

    return IP_Address;
}

bool filter(IP *PIP,TCP *PTCP,UDP *PUDP,char *Sniff_IP,int Sniff_Port)
{
    if(strcmpi(Sniff_IP,"0"))
    {
        //��ָ̽����IP
        if(!strcmpi(Sniff_IP,inet_ntoa(*(in_addr *)&PIP->ip_src)) || !strcmpi(Sniff_IP,inet_ntoa(*(in_addr *)&PIP->ip_dst)))
        {
            if(Sniff_Port)
            {
                //��ָ̽���Ķ˿�
                if(PTCP)
                {
                    if((int)ntohs(PTCP->sport)==Sniff_Port || (int)ntohs(PTCP->dport)==Sniff_Port)
                    {
                        return true;
                    }
                }
                else if(PUDP)
                {
                    if(ntohs(PUDP->sport)==Sniff_Port || ntohs(PUDP->dport)==Sniff_Port)
                    {
                        return true;
                    }
                }
            }
            else
                return true;
        }
    }
    else
    {
        if(Sniff_Port!=0)
        {
            //��ָ̽���Ķ˿�
            if(PTCP)
            {
                if(ntohs(PTCP->sport)==Sniff_Port || ntohs(PTCP->dport)==Sniff_Port)
                {
                    return true;
                }
            }
            else if(PUDP)
            {
                if(ntohs(PUDP->sport)==Sniff_Port || ntohs(PUDP->dport)==Sniff_Port)
                {
                    return true;
                }
            }
        }
        else
            return true;
    }
    return false;
}

bool Sniffer(char *IP_Address,int Proto,char *Sniff_IP,int Sniff_Port)
{
    FILE *file=NULL;
    SOCKET soc=INVALID_SOCKET;
    struct sockaddr_in saddr;
    WSADATA wsa;
    char RecvBuff[MAX_SIZE];
    unsigned long SIO_Parameter=1;

    IP *PIP=(IP *)RecvBuff;         //ָ��IP�ṹ���ָ��
    TCP *PTCP=(TCP *)(RecvBuff+sizeof(IP));
    UDP *PUDP=(UDP *)(RecvBuff+sizeof(IP));

    memset(&saddr,NULL,sizeof(saddr));
    saddr.sin_family=AF_INET;
    saddr.sin_addr.S_un.S_addr=inet_addr(IP_Address);
    saddr.sin_port=htons(0);

    if((file=fopen("sniffer.txt","wt"))==NULL)
    {
        printf("�����ļ�ʧ�ܡ�\n");
        return false;
    }

    if(WSAStartup(MAKEWORD(2,2),&wsa)!=0)
    {
        printf("��ʼ��ʧ�ܡ�\n");
        return false;
    }

    if((soc=socket(AF_INET,SOCK_RAW,IPPROTO_IP))==INVALID_SOCKET) //����ԭʼ�׽���
    {
        printf("�����׽���ʧ�ܡ�\n");
        WSACleanup();
        return false;
    }

    if(bind(soc,(struct sockaddr *)&saddr,sizeof(saddr))!=0)
    {
        printf("�󶨵�ַʧ�ܡ�\n");
        closesocket(soc);
        WSACleanup();
        return false;
    }

    ioctlsocket(soc,SIO_RCVALL,&SIO_Parameter);  //�����׽ӿ�ģʽ�������������ݰ���

    printf("��ʼ��̽...\n+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n\n");
    while(true)
    {
        memset(RecvBuff,NULL,sizeof(RecvBuff));
        recv(soc,RecvBuff,sizeof(RecvBuff),0);
        if(PIP->ip_p==IPPROTO_TCP && (Proto==0 || Proto==1))
        {
            if(filter(PIP,PTCP,NULL,Sniff_IP,Sniff_Port))
            {
                printf("Э��:TCP\n");
                printf("��Դ:%s:%d\n",inet_ntoa(*(in_addr *)&PIP->ip_src),ntohs(PTCP->sport));
                printf("Ŀ��:%s:%d\n",inet_ntoa(*(in_addr *)&PIP->ip_dst),ntohs(PTCP->dport));
                puts("���ݰ�����:\n\n\n");
                printf("%s\n",RecvBuff+sizeof(IP)+sizeof(TCP));
                puts("\n\n--------------------------------------------------------------------\n");

                fprintf(file,"Э��:TCP\n");
                fprintf(file,"��Դ:%s:%d\n",inet_ntoa(*(in_addr *)&PIP->ip_src),ntohs(PTCP->sport));
                fprintf(file,"Ŀ��:%s:%d\n",inet_ntoa(*(in_addr *)&PIP->ip_dst),ntohs(PTCP->dport));
                fputs("���ݰ�����:\n\n\n",file);
                fprintf(file,"%s\n",RecvBuff+sizeof(IP)+sizeof(TCP));
                fputs("\n\n--------------------------------------------------------------------\n",file);
                fflush(file);
            }
        }
        else if(PIP->ip_p==IPPROTO_UDP && (Proto==0 || Proto==2))
        {
            if(filter(PIP,NULL,PUDP,Sniff_IP,Sniff_Port))
            {
                printf("Э��:UDP\n");
                printf("��Դ:%s:%d\n",inet_ntoa(*(in_addr *)&PIP->ip_src),ntohs(PUDP->sport));
                printf("Ŀ��:%s:%d\n",inet_ntoa(*(in_addr *)&PIP->ip_dst),ntohs(PUDP->dport));
                puts("���ݰ�����:\n\n\n");
                printf("%s\n",RecvBuff+sizeof(IP)+sizeof(UDP));
                puts("\n\n--------------------------------------------------------------------\n");

                fprintf(file,"Э��:UDP\n");
                fprintf(file,"��Դ:%s:%d\n",inet_ntoa(*(in_addr *)&PIP->ip_src),ntohs(PUDP->sport));
                fprintf(file,"Ŀ��:%s:%d\n",inet_ntoa(*(in_addr *)&PIP->ip_dst),ntohs(PUDP->dport));
                fputs("���ݰ�����:\n\n\n",file);
                fprintf(file,"%s\n",RecvBuff+sizeof(IP)+sizeof(UDP));
                fputs("\n\n--------------------------------------------------------------------\n",file);
                fflush(file);
            }
        }
        else
            continue;
    }
}

int main(int argc,char *argv[])
{
    system("color b");
    char *IP_Address=NULL;
    char Sniff_IP[20];
    int Sniff_Port;
    int Proto;

    if((IP_Address=Choose_Adapter())==NULL)
    {
        printf("��ȡ������Ϣʧ�ܡ�\n");
        getch();
        return -1;
    }

    printf("��ѡ����̽��Э��(0.TCP & UDP; 1.TCP; 2.UDP):\n");
again1:
    fflush(stdin);
    if(scanf("%d",&Proto)!=1 || Proto<0 || Proto>2)
    {
        printf("��������������ѡ��:");
        goto again1;
    }
    memset(Sniff_IP,NULL,sizeof(Sniff_IP));
    printf("������������̽��IP(\"0\"Ϊ��̽����IP):\n");
    fflush(stdin);
    gets(Sniff_IP);
    printf("������������̽�Ķ˿�(\"0\"Ϊ��̽���ж˿�):\n");
again2:
    fflush(stdin);
    if(scanf("%d",&Sniff_Port)!=1)
    {
        printf("��������,����������:");
        goto again2;
    }

    if(!Sniffer(IP_Address,Proto,Sniff_IP,Sniff_Port))
    {
        printf("������̽ʧ�ܡ�\n");
        getch();
        return -1;
    }
    return 0;
}












