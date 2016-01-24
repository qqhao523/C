#ifndef MEMPOOL_H_INCLUDED
#define MEMPOOL_H_INCLUDED

#include <pthread.h>

#include "async.h"

typedef IO_OPERATION_DATA MEMPOOL_DATA_TYPE;

typedef struct _MEMPOOL_LIST_
{
    void **pMempoolCell;        //ָ���ڴ��ַ��ָ������
    unsigned long int total;    //�ܹ����õ��ڴ���
    unsigned long int surplus;  //ʣ����õ��ڴ���
    unsigned long int cellSize; //ÿ���ڴ浥Ԫ�Ĵ�С
    pthread_mutex_t mutex;
    struct _MEMPOOL_LIST_ *next;
}MEMPOOL_LIST;

MEMPOOL_LIST *create_mempool(unsigned long int dataSize,unsigned long int dataCount);
void *mempool_alloc(MEMPOOL_LIST *pMempool);
int mempool_free(MEMPOOL_LIST *pMempool,void *addr);

#endif // MEMPOOL_H_INCLUDED
