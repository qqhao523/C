#ifndef _STACK_H_
#define _STACK_H_

#include <stdio.h>
#include <stdlib.h>
#include "global.h"
#include "GUI.h"

int InitStack(Stack *S)
{
	if (S->base != NULL) return 1;                                                  //ջ�ṹ�Ѵ���,����Ҫ�ٴ���
	S->base = S->top = (STACK_DATATYPE *)malloc(sizeof(STACK_DATATYPE)* 5);           //��ʼ������Ϊ5��ջ
	if (S->base == NULL)
	{
		print("malloc error.", RED);
		return 0;
	}
	S->stacksize = 5;

	return 1;
}

int Push(Stack *S, STACK_DATATYPE elem)
{
	if (S->top - S->base == S->stacksize - 1)
	{
		//��ҪΪջ�����µĴ洢�ռ�
		S->base = (STACK_DATATYPE *)realloc(S->base, sizeof(STACK_DATATYPE)*++S->stacksize);
		if (S->base == NULL)
		{
			print("�����µ�ջ�ռ�ʧ��.", RED);
			return 0;
		}
		S->top = S->base + (S->stacksize - 2);
	}
	*S->top = elem;           //������ջ
	S->top++;               //ջ��ָ�������
	return 1;
}

int StackEmpty(Stack *S)
{
	return S->top - S->base == 0 ? 1 : 0;
}

int Pop(Stack *S, STACK_DATATYPE *elem)
{
	if (StackEmpty(S)) return 0;      //ջ�ѿ�
	S->top--;                        //ջ��ָ����ǰ�ƶ�һ��λ
	*elem = *S->top;                   //ջ��Ԫ�س�ջ
	return 1;
}

#endif    //_STACK_H_

