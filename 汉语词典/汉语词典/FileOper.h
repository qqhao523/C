#ifndef _FILEOPER_H_
#define _FILEOPER_H_

#include <stdio.h>
#include <stdlib.h>
#include "unicode.h"
#include "global.h"
#include "GUI.h"

int CreateDataFile()
{
	hDataFile = CreateFile(CharToTCHAR(DataFilePath), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, \
		FILE_ATTRIBUTE_NORMAL, NULL);
	if (hDataFile == INVALID_HANDLE_VALUE)
	{
		print("�������ݿ��ļ�ʧ�ܡ�", RED);
		return 0;
	}
	return 1;
}

int CreateIndexFile()
{
	hIndexFile = CreateFile(CharToTCHAR(IndexFilePath), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, \
		FILE_ATTRIBUTE_NORMAL, NULL);
	if (hIndexFile == INVALID_HANDLE_VALUE)
	{
		print("���������ļ�ʧ�ܡ�", RED);
		return 0;
	}
	return 1;
}

int OpenDataFile()
{
	hDataFile = CreateFile(CharToTCHAR(DataFilePath), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, \
		FILE_ATTRIBUTE_NORMAL, NULL);
	if (hIndexFile == INVALID_HANDLE_VALUE)
	{
		printf("���ݿ��ļ���ʧ�ܡ�", RED);
		return 0;
	}
	return 1;
}

int OpenIndexFile()
{
	hIndexFile = CreateFile(CharToTCHAR(IndexFilePath), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, \
		FILE_ATTRIBUTE_NORMAL, NULL);
	if (hIndexFile == INVALID_HANDLE_VALUE)
	{
		print("�����ļ���ʧ�ܡ�", RED);
		return 0;
	}
	return 1;
}

int InsertDataToFile(WORD_INFO word, INDEXDATA index)
{
	DWORD tmp_dw;
	WORD_INFO temp;
	LARGE_INTEGER MovSize;

	memset(&MovSize, NULL, sizeof(LARGE_INTEGER));

	MovSize.QuadPart = index.Start.QuadPart + sizeof(WORD_INFO)*(index.Count - 1);     //����д���´����ĵ�ַ

	GetFileSizeEx(hDataFile, &DataFileSize);                                      //��ȡ��ǰ���ݿ��ļ��Ĵ�С
	if (DataFileSize.QuadPart != MovSize.QuadPart)
	{
		//Ҫ����Ĵ����������ݿ��ļ���β��
		do
		{
			DataFileSize.QuadPart -= sizeof(WORD_INFO);                         //���ļ��ڲ�λ��ָ���������һ�������Ŀ�ʼ��
			DataFileSize.LowPart = SetFilePointer(hDataFile, DataFileSize.LowPart, &DataFileSize.HighPart, FILE_BEGIN);
			ReadFile(hDataFile, &temp, sizeof(WORD_INFO), &tmp_dw, NULL);         //�����˴���
			WriteFile(hDataFile, &temp, sizeof(WORD_INFO), &tmp_dw, NULL);
		} while (DataFileSize.QuadPart != MovSize.QuadPart);
	}

	MovSize.LowPart = SetFilePointer(hDataFile, MovSize.LowPart, &MovSize.HighPart, FILE_BEGIN);      //�ļ��ڲ�λ��ָ��������Ҫ׷�ӵ�λ��
	WriteFile(hDataFile, &word, sizeof(WORD_INFO), &tmp_dw, NULL);

	return 1;
}

int InsertIndexToFile(INDEXDATA IndexNode, int choose)
{
	//�ڶ�������1��ʾ���ļ�ĩβ���������,0��ʾ�޸�ĳһ����ֵ.
	int n = 0;
	DWORD tmp_dw;
	LARGE_INTEGER MovSize;
	INDEXDATA ReadBuff;

	memset(&MovSize, NULL, sizeof(LARGE_INTEGER));
	memset(&ReadBuff, NULL, sizeof(INDEXDATA));

	if (choose == 0)
	{
		//������ʱ��Ҫ�޸������ļ���ĳ����
		SetFilePointer(hIndexFile, MovSize.LowPart, &MovSize.HighPart, FILE_BEGIN);           //�ļ��ڲ�λ��ָ��ָ����ʼλ��
		while (1)
		{
			if (!ReadFile(hIndexFile, &ReadBuff, sizeof(INDEXDATA), &tmp_dw, NULL))
				return 0;
			if (tmp_dw<1) break;
			if (ReadBuff.ASCII == IndexNode.ASCII)
			{
				memset(&MovSize, NULL, sizeof(LARGE_INTEGER));                  //��ʼ��λ�Ʊ���
				MovSize.QuadPart = n*sizeof(INDEXDATA);                         //����ƶ�һ�������ڵ�ĳ���
				SetFilePointerEx(hIndexFile, MovSize, NULL, FILE_BEGIN);         //���������������ƶ����λ��
				break;
			}
			n++;
			memset(&ReadBuff, NULL, sizeof(INDEXDATA));
		}
	}
	else
	{
		//���µ���������׷�ӵ������ļ���β��
		MovSize.LowPart = SetFilePointer(hIndexFile, MovSize.LowPart, &MovSize.HighPart, FILE_END);             //���ļ��ڲ�ָ�������ļ�ĩβ
	}
	if (!WriteFile(hIndexFile, (LPCVOID)&IndexNode, sizeof(INDEXDATA), &tmp_dw, NULL))         //���µ������ڵ�д���ļ�
		return 0;

	return 1;
}

#endif     //_FILEOPER_H_
