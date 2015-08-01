#ifndef _GLOBAL_H_
#define _GLOBAL_H_

#include <windows.h>

//�����ṹ��
typedef struct
{
	char word[50];               //������
	char property[20];           //����
	char pronunciation[50];      //����
	char describe[100];          //����
}WORD_INFO;
/*------------------------AVL----------------------------------------*/
//�����ṹ��
typedef struct
{
	int ASCII;                //����ASCII֮��
	LARGE_INTEGER Start;      //�ļ��ڵ���ʼλ��
	unsigned int Count;       //��ͬASCII�͵Ĵ�����Ŀ
}INDEXDATA;

//AVL���ڵ�Ԫ��
typedef struct BiTree
{
	int balance;              //ƽ������
	INDEXDATA data;           //�ڵ�����
	struct BiTree *left;
	struct BiTree *right;
	struct BiTree *father;    //ָ��˫�׽ڵ�
} AVL;

AVL *AVL_ROOT = NULL;                  //AVL����ָ��
/*------------------------Stack----------------------------------------*/
#define STACK_DATATYPE AVL*          //ջԪ������ΪAVL�ڵ�ָ��
typedef struct
{
	STACK_DATATYPE *top;             //ջ��ָ��
	STACK_DATATYPE *base;            //ջ��ָ��
	int stacksize;             //ջ����
}Stack;

Stack S;                      //ջ�ṹ����
/*------------------------Queue----------------------------------------*/
#define QUEUE_DATATYPE AVL*
typedef struct QueueNode
{
	QUEUE_DATATYPE data;
	struct QueueNode *next;
}QNode;

typedef struct
{
	QNode *start;               //������ʼ��ַ
	QNode *end;                 //����ĩβ��ַ
	int queuesize;              //���нڵ����
}Queue;

/*----------------------------------------------------------------------*/

HANDLE hDataFile;                       //���ݿ��ļ����
HANDLE hIndexFile;                      //�����ļ����
LARGE_INTEGER DataFileSize;             //���ݿ��ļ���С

const char *IndexFilePath = "IndexFile";    //�����ļ�·��
const char *DataFilePath = "DataFile";      //���ݿ��ļ�·��

#endif        //_GLOBAL_H_