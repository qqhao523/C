#ifndef _AVL_H_
#define _AVL_H_

#include <windows.h>
#include <string.h>

#include "global.h"
#include "FileOper.h"
#include "stack.h"
#include "GUI.h"

int Depth(AVL *root)
{
	//����������ȡ�
	if (root == NULL)
		return 0;
	int i, j;

	if (root->left)
		i = Depth(root->left);
	else
		i = 0;

	if (root->right)
		j = Depth(root->right);
	else
		j = 0;

	return i>j ? i + 1 : j + 1;
}

void updateBF(AVL *NewNode)
{
	//����ƽ������
	if (!NewNode || !NewNode->father) return;
	AVL *CurrNode = NewNode;
	AVL *PreNode = CurrNode->father;

	if (CurrNode == PreNode->left)
	{
		//ָ�����·����
		PreNode->balance++;
		if (PreNode->balance == 2) return;
		if (PreNode->left && PreNode->right)   //�������������ӵĽڵ�
		if (PreNode->balance <= 0)
			return;
	}
	else
	{
		//ָ�����·����
		PreNode->balance--;
		if (PreNode->balance == -2) return;
		if (PreNode->left && PreNode->right)
		if (PreNode->balance >= 0)
			return;
	}
	updateBF(PreNode);

	return;
}

void RR_Rotate(AVL **root, AVL *centreNode)
{
	//������ת,RR�͡�
	AVL *temp = NULL;

	if (centreNode->right)
	{
		//���Ľڵ���ҽڵ㲻Ϊ��
		centreNode->father->left = centreNode->right;     //�����Ľڵ�����������ӵ�˫�׽ڵ���������ϡ�
		centreNode->right->father = centreNode->father;
	}
	else
		//���Ľڵ���ҽڵ�Ϊ��
		centreNode->father->left = NULL;

	temp = centreNode->father->father;
	if (centreNode->father->father != NULL)
	if (centreNode->father->father->left == centreNode->father)
		centreNode->father->father->left = centreNode;
	else
		centreNode->father->father->right = centreNode;
	else
		*root = centreNode;

	centreNode->right = centreNode->father;         //��ǰ�ڵ��˫�׳�Ϊ����������
	centreNode->father->father = centreNode;
	centreNode->father->balance -= 2;
	centreNode->father = temp;
	centreNode->balance--;

	return;
}

void LL_Rotate(AVL **root, AVL *centreNode)
{
	//������ת,LL��
	AVL *temp = NULL;

	if (centreNode->left)
	{
		//���Ľڵ����ڵ㲻Ϊ��
		centreNode->father->right = centreNode->left;    //�����Ľڵ�����������ӵ�˫�׵��������ϡ�
		centreNode->left->father = centreNode->father;   //���Ľڵ����������˫�׸�Ϊ���Ľڵ��˫�ס�
	}
	else
		centreNode->father->right = NULL;

	temp = centreNode->father->father;
	if (centreNode->father->father != NULL)
	if (centreNode->father->father->left == centreNode->father)
		centreNode->father->father->left = centreNode;
	else
		centreNode->father->father->right = centreNode;
	else
		*root = centreNode;

	centreNode->left = centreNode->father;         //�����Ľڵ�����Ӹ�Ϊ���Ľڵ��˫��
	centreNode->left->father = centreNode;         //�����Ľڵ����ӵ�˫�׸���Ϊ���Ľڵ㡣
	centreNode->balance++;
	centreNode->father->balance += 2;
	centreNode->father = temp;

	return;
}

void RL_Rotate(AVL **root, AVL *centreNode)
{
	//���Һ�����ת
	AVL *temp = NULL;

	temp = centreNode->father;
	centreNode->father->right = centreNode->left;
	centreNode->left->father = centreNode->father;
	centreNode->left->balance--;
	centreNode->balance--;

	if (centreNode->left->right)
	{
		//���Ľڵ����ڵ���Һ��Ӳ�Ϊ�ա�
		temp->right->right->father = centreNode;
	}
	centreNode->left = temp->right->right;
	temp->right->right = centreNode;
	centreNode->father = temp->right;
	if (temp->right->balance == -2)
		temp->right->balance++;
	LL_Rotate(root, temp->right);

	return;
}

void LR_Rotate(AVL **root, AVL *centreNode)
{
	//���������ת��LR�͡�
	AVL *temp = NULL;

	temp = centreNode->father;
	centreNode->father->left = centreNode->right;
	centreNode->right->father = centreNode->father;
	centreNode->right->balance++;
	centreNode->balance++;

	if (centreNode->right->left)
	{
		//���Ľڵ����ڵ���Һ��Ӳ�Ϊ�ա�
		temp->left->left->father = centreNode;
	}
	centreNode->right = temp->left->left;
	temp->left->left = centreNode;
	centreNode->father = temp->left;
	if (temp->left->balance == 2)
		temp->left->balance--;
	RR_Rotate(root, temp->left);

	return;
}

void adjustAVL(AVL **root, AVL *NewNode)
{
	if (!NewNode) return;
	AVL *CurrNode = NewNode;
	AVL *PreNode = NULL;

	while (CurrNode->father != NULL && CurrNode->father->balance != 2 && CurrNode->father->balance != -2)
	{
		PreNode = CurrNode;
		CurrNode = CurrNode->father;
	}
	if (!CurrNode->father) return;

	if (CurrNode->father->left == CurrNode)
	{
		//�Լ���˫�׵���ڵ�
		if (CurrNode->left == PreNode)
			//ָ�����·���ݡ�
			RR_Rotate(root, CurrNode);
		else
			//ָ�����·����
			LR_Rotate(root, CurrNode);
	}
	else
	{
		//�Լ���˫�׵��ҽڵ�
		if (CurrNode->right == PreNode)
			//ָ�����·����
			LL_Rotate(root, CurrNode);
		else
			//ָ�����·����
			RL_Rotate(root, CurrNode);
	}

	return;
}

int InsertIndex(AVL **root, INDEXDATA *elem)
{
	AVL *CurrNode = NULL;
	AVL *PreNode = NULL;
	AVL *NewNode = NULL;

	if (*root == NULL)                  //������ڵ㲻�����򴴽�
	{
		if ((*root = (AVL *)malloc(sizeof(AVL))) == NULL)
			return 0;
		memset(*root, NULL, sizeof(AVL));
		elem->Count = 1;               //��ASCII�͵Ĵ�������
		(*root)->data = *elem;
		return 1;
	}

	CurrNode = *root;
	while (CurrNode)
	{
		PreNode = CurrNode;
		if (elem->ASCII<CurrNode->data.ASCII)
			CurrNode = CurrNode->left;
		else if (elem->ASCII>CurrNode->data.ASCII)
			CurrNode = CurrNode->right;
		else
		{
			//�Ѵ��ڸ�����
			CurrNode->data.Count++;                  //�����͵Ĵ���������һ
			elem->Start = CurrNode->data.Start;
			elem->Count = CurrNode->data.Count;
			return 2;    //��ʾ�Ѵ��ڸ�Ԫ��
		}
	}
	//�������޴˽ڵ㣬����µĽڵ㡣
	if ((NewNode = (AVL *)malloc(sizeof(AVL))) == NULL)        //Ϊ�½ڵ㿪�ٿռ�
		return 0;
	memset(NewNode, NULL, sizeof(AVL));                     //��ʼ���½ڵ��е�����
	if (GetFileSizeEx(hDataFile, &DataFileSize) != TRUE)      //��ȡ���ݿ��ļ��ܴ�С
		return 0;
	elem->Start = DataFileSize;                             //������ӵĴ���׷�ӵ����ݿ��ļ�β��
	elem->Count = 1;                                        //�����͵Ĵ�����ĿΪһ
	NewNode->data = *elem;                                  //Ϊ�½ڵ㸳ֵ
	NewNode->father = PreNode;
	//�����½ڵ�
	if (elem->ASCII<PreNode->data.ASCII)
		PreNode->left = NewNode;
	else
		PreNode->right = NewNode;
	updateBF(NewNode);          //����AVL��ƽ������
	adjustAVL(root, NewNode);    //AVL�Ե���

	return 1;
}

int InsertIndexFromFile(AVL **root, INDEXDATA *elem)
{
	AVL *CurrNode = NULL;
	AVL *PreNode = NULL;
	AVL *NewNode = NULL;

	if (*root == NULL)  //������ڵ㲻�����򴴽�
	{
		if ((*root = (AVL *)malloc(sizeof(AVL))) == NULL)
			return 0;
		memset(*root, NULL, sizeof(AVL));
		(*root)->data = *elem;
		return 1;
	}

	CurrNode = *root;
	while (CurrNode)
	{
		PreNode = CurrNode;
		if (elem->ASCII<CurrNode->data.ASCII)
			CurrNode = CurrNode->left;
		else if (elem->ASCII>CurrNode->data.ASCII)
			CurrNode = CurrNode->right;
		else
		{
			//�Ѵ��ڸ�����
			return 2;    //��ʾ�Ѵ��ڸ�Ԫ��
		}
	}
	//�������޴˽ڵ㣬����µĽڵ㡣
	if ((NewNode = (AVL *)malloc(sizeof(AVL))) == NULL)       //Ϊ�½ڵ㿪�ٿռ�
		return 0;
	memset(NewNode, NULL, sizeof(AVL));                    //��ʼ���½ڵ��е�����
	NewNode->data = *elem;                                 //Ϊ�½ڵ㸳ֵ
	NewNode->father = PreNode;
	//�����½ڵ�
	if (elem->ASCII<PreNode->data.ASCII)
		PreNode->left = NewNode;
	else
		PreNode->right = NewNode;
	updateBF(NewNode);          //����AVL��ƽ������
	adjustAVL(root, NewNode);    //AVL�Ե���

	return 1;
}

int AlterIndexStartAddr(AVL *root, const LARGE_INTEGER *refer, int pattern)
{
	//�������,����������:0.�д��������.1.�д�����ɾ��.
	if (root == NULL)
		return 0;
	AlterIndexStartAddr(root->left, refer, pattern);

	if (root->data.Start.QuadPart>refer->QuadPart)
	{
		if (pattern == 0)
		{
			//�д��������,����������ʼ��ַ����.
			root->data.Start.QuadPart += sizeof(WORD_INFO);    //�޸�����������������ʼ��ַ
		}
		else
		{
			//�д�����ɾ��,����������ʼ��ַǰ��
			root->data.Start.QuadPart -= sizeof(WORD_INFO);
		}
		InsertIndexToFile(root->data, 0);                 //���޸Ľ�������ļ���
	}

	AlterIndexStartAddr(root->right, refer, pattern);
	return 1;
}

INDEXDATA *SearchAVL(AVL *root, int ASCII)
{
	INDEXDATA *result = NULL;

	if (!root) return NULL;
	if (ASCII == root->data.ASCII) return &root->data;
	if (ASCII<root->data.ASCII)
	if ((result = SearchAVL(root->left, ASCII))) return result;
	if (ASCII>root->data.ASCII)
	if ((result = SearchAVL(root->right, ASCII))) return result;

	return NULL;
}

int PushAVLToStack(Stack *S, AVL *root)
{
	//���������ջ
	if (!root) return 0;

	Push(S, root);
	if (root->left)
		PushAVLToStack(S, root->left);
	if (root->right)
		PushAVLToStack(S, root->right);

	return 1;
}

int DestroyAVL(AVL *root)
{
	AVL *tmp = NULL;

	if (!InitStack(&S))             //��ʼ��ջ
	{
		print("��ʼ��ջʧ��.", RED);
		return 0;
	}
	PushAVLToStack(&S, root);      //AVL���нڵ�ָ����ջ
	AVL_ROOT = NULL;                //AVL��ָ���ÿ�
	while (!StackEmpty(&S))
	{
		Pop(&S, &tmp);             //ָ�������ջ
		free(tmp);                //�ͷ��ڴ�
	}
	return 1;
}

int NodeCount(AVL *root)
{
	//ͳ�������ж��ٸ��ڵ�
	int n = 0;
	if (root == NULL)
		return 0;
	n = NodeCount(root->left);
	n++;
	n += NodeCount(root->right);
	return n;
}

int WordCount(AVL *root)
{
	//ͳ�����������˶��ٸ�����
	int n = 0;
	if (root == NULL)
		return 0;
	n = WordCount(root->left);
	n += root->data.Count;
	n += WordCount(root->right);
	return n;
}

#endif          //_AVL_H_




