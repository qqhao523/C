#include <stdio.h>
#include <windows.h>
#include <conio.h>
#include <io.h>
#include "FileOper.h"
#include "global.h"
#include "AVL.h"
#include "GUI.h"

int InitFile()
{
	//����ļ���������,��������ļ�.
	char choose;

	if (access(DataFilePath, 0) || access(IndexFilePath, 0))
	{
		//������ݿ��ļ��������ļ��Ƿ����
		print("�ļ���ʧ,�Ƿ��ؽ�?(Y/N)", RED);
		while (1)
		{
			fflush(stdin);
			choose = _getch();
			if (choose == 'Y' || choose == 'y')
				break;
			else if (choose == 'N' || choose == 'n')
				exit(0);
		}
		if (!CreateDataFile()) return 0;
		if (!CreateIndexFile()) return 0;
		print("�ļ������ɹ�.", GREEN);
		return 1;
	}
	if (!OpenDataFile()) return 0;
	if (!OpenIndexFile()) return 0;
	return 1;
}

int InitIndex()
{
	DWORD tmp_dw;
	INDEXDATA ReadBuff;
	LARGE_INTEGER MovSize, CurrSize;

	AVL_ROOT = NULL;                     //AVL�������ڵ�ָ���ÿ�
	memset(&ReadBuff, NULL, sizeof(INDEXDATA));
	memset(&MovSize, NULL, sizeof(LARGE_INTEGER));
	memset(&CurrSize, NULL, sizeof(LARGE_INTEGER));

	SetFilePointerEx(hIndexFile, MovSize, &CurrSize, FILE_BEGIN);        //���ļ�ָ�������ļ���ʼλ��
	while (1)
	{
		if (!ReadFile(hIndexFile, (LPVOID)&ReadBuff, (DWORD)sizeof(INDEXDATA), &tmp_dw, NULL))
			return 0;
		if (tmp_dw<1) break;
		InsertIndexFromFile(&AVL_ROOT, &ReadBuff);          //�����ļ��ڶ������������뵽�ڴ���
		memset(&ReadBuff, NULL, sizeof(INDEXDATA));
	}

	return 1;
}