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

typedef struct
{
    pcap_t *hpcap;                        //����������
    unsigned char myIP[4];                //��������IP
    unsigned char srcMAC[6];              //ԴMAC
    unsigned char desMAC[6];              //Ŀ��MAC
    unsigned char desIP[4];               //Ŀ��IP
    char *Packet;                         //���ݰ�ָ��
    struct pcap_pkthdr pkthdr;            //�������ݰ���С
} PacketInfo;

BOOL GetAdapterMAC(char *ipbuff,char *macbuff)
{
    IP_ADAPTER_INFO AdapterInfo[16];  //����洢������Ϣ�Ľṹ����
    DWORD ArrayLength=sizeof(AdapterInfo);  //����������
    if(GetAdaptersInfo(AdapterInfo,&ArrayLength)!=ERROR_SUCCESS)
        return ERROR;
    PIP_ADAPTER_INFO PAdapterInfo=AdapterInfo;
    puts(ipbuff);
    do
    {
        if(!strcmp(ipbuff,PAdapterInfo->IpAddressList.IpAddress.String)) break;
        PAdapterInfo=PAdapterInfo->Next;
    }
    while(PAdapterInfo);

    memset(macbuff,NULL,7);
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
    if(pcap_findalldevs(&alldevs,errbuff)==-1)
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
    memset(ipbuff,NULL,16);
    a=p->addresses;
    strcat(ipbuff,iptos(((struct sockaddr_in *)a->addr)->sin_addr.s_addr));
    pcap_freealldevs(alldevs);

    return TRUE;
}

void Fill_ARPPACKET(char *ARPPacket,int packetsize,char *desmac,char *desIP,char *srcmac,char *srcip,int op)
{
    unsigned long temp;
    DLCHEADER *DLCHeader=(DLCHEADER *)ARPPacket;
    ARPFRAME *ARPFrame=(ARPFRAME *)(ARPPacket+sizeof(DLCHEADER));

    memset(ARPPacket,NULL,packetsize);  //��հ�����

    //�����̫��Ŀ�ĵ�ַ
    if(op==1)
    {
        memset(DLCHeader->DesMAC,0xff,6);
        memset(ARPFrame->Targ_HW_Addr,NULL,6);
    }
    else
    {
        memcpy(DLCHeader->DesMAC,desmac,6);
        memcpy(ARPFrame->Targ_HW_Addr,DLCHeader->DesMAC,6);
    }

    //�����̫��Դ��ַ
    memcpy(DLCHeader->SrcMAC,srcmac,6);
    memcpy(ARPFrame->Send_HW_Addr,DLCHeader->SrcMAC,sizeof(DLCHeader->SrcMAC));
    //���ARP�����ԴIP
    temp=inet_addr(srcip);
    memcpy(ARPFrame->Send_Prot_Addr,(char *)&temp,4);
    //���ARP�����Ŀ��IP
    temp=inet_addr(desIP);
    memcpy(ARPFrame->Targ_Prot_Addr,(char *)&temp,4);

    DLCHeader->EtherType=htons((unsigned short)0x0806);    //0x0806��ʾARPЭ�飬0x0800��ʾIPЭ��
    ARPFrame->HW_Addr_Len=(unsigned char)6;
    ARPFrame->Prot_Addr_Len=(unsigned char)4;
    ARPFrame->HW_Type=htons((unsigned short)1);
    ARPFrame->Opcode=htons((unsigned short)op);   //01��ʾ����02��ʾӦ��
    ARPFrame->Prot_Type=htons((unsigned short)0x0800);
}

DWORD WINAPI filter(PVOID Parameter)
{
    PacketInfo PI=*(PacketInfo *)Parameter;
    char *Packet=NULL;

    Packet=(char *)malloc(PI.pkthdr.caplen);     //���봢�����ݰ��ռ�
    memcpy(Packet,PI.Packet,PI.pkthdr.caplen);

    ARPFRAME *ARPFrame;
    DLCHEADER *DLCHeader=(DLCHEADER *)Packet;

    char tempmac[6];
    memset(tempmac,0xff,6);

    if(ntohs(DLCHeader->EtherType)==0x0806)
    {
        ARPFrame=(ARPFRAME *)(Packet+sizeof(DLCHEADER));
        if(!strncmp(DLCHeader->DesMAC,tempmac,6) && !strncmp(ARPFrame->Send_Prot_Addr,PI.myIP,4))
        {
            system("color b");
            int i;
            puts("");
            for(i=0; i<6; i++)
                printf("%02x ",ARPFrame->Send_HW_Addr[i]);
            puts("");
            printf("���ݰ���С:%d\n",PI.pkthdr.caplen);
            printf("���ݰ�����:\n");
            for(i=0; i<PI.pkthdr.caplen; i++)
                printf("%02x ",Packet[i]);
        }
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
    int i;

    while(TRUE)
    {
        if(pcap_next_ex(hpcap,&pkthdr,&recvBuff)>0)
        {
            /*
                        printf("���ݰ���С:%d\n\n ",pkthdr->caplen);
                        printf("���շ�MAC:");
                        for(i=0; i<6; i++)
                            printf("%02x ",recvBuff[i]);
                        puts("");
                        printf("���ͷ�MAC:");
                        for(; i<12; i++)
                            printf("%02x ",recvBuff[i]);
                        puts("");
                        printf("֡����:0x%02x%02x\n",recvBuff[i],recvBuff[i+1]);
                        printf("\n----------------------------------------------\n");
            */
           // printf("ץȡ��%d�����ݰ���\n\n",++i);
            PI.Packet=recvBuff;
            PI.pkthdr=*pkthdr;
            CloseHandle((hFilterThread=CreateThread(NULL,0,filter,(PVOID)&PI,0,NULL)));
        }
    }

    return 0;
}

BOOL SendARPPacket(pcap_t *hpcap,char *devName,char *ARPPacket,int packetsize)
{
    int n=0;
    while(1)
        if(pcap_sendpacket(hpcap,ARPPacket,packetsize)==0)
        {
            printf("�ѷ���%d�����ݰ���\r",++n);
            getch();
        }
        else
        {
            printf("���ݰ�����ʧ�ܡ�\n");
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

int main()
{
    char devName[100];
    char myIPAddress[16];
    char ARPPacket[42];
    char srcMAC[7]= {0},desMAC[7]= {0};
    char query[20];
    pcap_t *hpcap=NULL;
    HANDLE hSnifferThread;
    PacketInfo PI;
    unsigned long temp;

    if(ChooseDev(devName,sizeof(devName),myIPAddress)!=TRUE)
    {
        printf("��ȡ����ʧ�ܡ�\n");
        getch();
        return -1;
    }
    if(GetAdapterMAC(myIPAddress,srcMAC)!=TRUE)
    {
        printf("��ȡ����MACʧ�ܡ�\n");
        getch();
        return -1;
    }
    if((hpcap=OpenAdapter(devName))==NULL)
    {
        printf("�����򿪳���\n");
        getch();
        return -1;
    }

    memset(desMAC,0xff,6);      //�㲥
    memset(query,NULL,sizeof(query));
    printf("������Ҫ��ѯ��IP:");
    fflush(stdin);
    gets(query);

    Fill_ARPPACKET(ARPPacket,sizeof(ARPPacket),desMAC,query,srcMAC,myIPAddress,2);
    int i;
    for(i=0; i<sizeof(ARPPacket); i++)
        printf("%02x ",ARPPacket[i]);
    puts("\n");
    //��䴫�ݲ���
    PI.hpcap=hpcap;
    memcpy(PI.srcMAC,srcMAC,6);
    temp=inet_addr(query);
    memcpy(PI.desIP,(char *)&temp,4);
    memcpy(PI.desMAC,desMAC,6);
    temp=inet_addr(myIPAddress);
    memcpy(PI.myIP,(char *)&temp,4);

    CloseHandle((hSnifferThread=CreateThread(NULL,0,Ether_Sniffer,(PVOID)&PI,0,NULL)));
    SendARPPacket(hpcap,devName,ARPPacket,sizeof(ARPPacket));
    while(1) Sleep(111);

    return 0;
}










