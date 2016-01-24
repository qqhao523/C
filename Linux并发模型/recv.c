#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/epoll.h>

#include "recv.h"
#include "async.h"
#include "cleanup.h"
#include "mempool.h"

extern unsigned long int TotalConnections;
extern unsigned long int ListenThreads;
extern pthread_rwlock_t LockIOData;
extern IO_OPERATION_DATA_NODE *IO_Operation_Data_Header;

int recv_data_from_client(IO_OPERATION_DATA *pIOData)
{
    /*
    ���տͻ��˴��������ݣ�������IO_OPERATION_DATA�ṹ���recvBuffer��
    ���뵱ǰ�ͻ��������Ϣ�Ľṹ��ָ��
    */
    int recvSize;
    char *recvPointer=NULL;

    if(pIOData->recvBuffer==NULL)
    {
        pIOData->recvSize=0;
        pIOData->recvBuffer=(char *)malloc(MAX_RECV_SIZE);
        if(pIOData->recvBuffer==NULL) return 0;
        memset(pIOData->recvBuffer,NULL,MAX_RECV_SIZE);
        recvPointer=pIOData->recvBuffer;
    }
    else
        recvPointer=pIOData->recvBuffer+pIOData->recvSize;

    recvSize=recv(pIOData->Socket,recvPointer,MAX_RECV_SIZE-pIOData->recvSize,0);
    if(recvSize<=0)
    {
        //socket disable
        return -2;
    }
    pIOData->recvSize+=recvSize;

    printf("Frome '%s':%s\n",inet_ntoa(pIOData->Address.sin_addr),recvPointer);

    return recvSize;
}

int listen_socket_from_client(void *Parameter)
{
    int activeCount;
    int i,j;
    IO_OPERATION_DATA_NODE *IO_Operation_Node_Pointer=NULL;
    struct epoll_event eventList[ASYNC_MAX_WAIT_OBJECTS];

    printf("Create a new thread\n");

    pthread_detach(pthread_self());     //�߳�״̬����
    IO_Operation_Node_Pointer=(IO_OPERATION_DATA_NODE *)Parameter;  //��ǰ�̲߳���������ڵ��ָ��

    while(1)
    {
        activeCount=epoll_wait(IO_Operation_Node_Pointer->epollfd,eventList,ASYNC_MAX_WAIT_OBJECTS,SOCKET_TIMEOUT);
        if(activeCount<=0) continue;

        for(j=0; j<ASYNC_MAX_WAIT_OBJECTS; j++)
        {
            if(IO_Operation_Node_Pointer->IOArray[j]==NULL) continue;
            for(i=0; i<activeCount; i++)
            {
                if(IO_Operation_Node_Pointer->IOArray[j]->Socket==eventList[i].data.fd)
                {
                    if(eventList[i].events & EPOLLIN)
                    {
                        if(recv_data_from_client(IO_Operation_Node_Pointer->IOArray[j])<=0)
                        {
                            //clean sockset;
                            pthread_rwlock_wrlock(&LockIOData);
                            clean_client_connection(IO_Operation_Node_Pointer->IOArray[j]);
                            pthread_rwlock_unlock(&LockIOData);
                        }
                    }
                    else
                    {
                        //client disconnect
                        pthread_rwlock_wrlock(&LockIOData);
                        clean_client_connection(IO_Operation_Node_Pointer->IOArray[j]);
                        pthread_rwlock_unlock(&LockIOData);
                    }
                    break;
                }
            }
        }
    }

    return 0;
}










