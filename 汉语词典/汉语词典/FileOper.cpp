#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include "global.h"
#include "FileOper.h"
#include "AVL.h"
#include "queue.h"
#include "GUI.h"

int ASCII_SUM(char *str);
int AdjustIndex(INDEXDATA _index);
int Query_Word(char *word, WORD_INFO *WI, LARGE_INTEGER *PCurrSize);
int Query();
int Add_Word();
int Alter_Word(LARGE_INTEGER CurrSize, WORD_INFO *WI);
int Delete_Word(LARGE_INTEGER CurrSize);
int Delete_Index(int ascii);
int OptimizeIndexFile();
void ShowFile();

int ASCII_SUM(char *str)
{
	int sum = 0;
	char *p = str;

	for (sum = 0; *p != NULL; p++)
		sum += (int)*p;           //���㵥�ʵ�ASCII��
	return sum;
}

int AdjustIndex(INDEXDATA _index)
{
	DWORD tmp_dw;
	LARGE_INTEGER ReferSize;

	memset(&ReferSize, NULL, sizeof(LARGE_INTEGER));

	InsertIndexToFile(_index, 0);                 //���޸��´���������
	ReferSize = _index.Start;
	AlterIndexStartAddr(AVL_ROOT, &ReferSize, 0);

	return 1;
}

int Query_Word(char *word, WORD_INFO *WI, LARGE_INTEGER *PCurrSize)
{
	//����ֵ:1.��ѯ�ɹ���0.�˴���δ����¼��-1.��ѯ����
	int word_ascii, n;
	DWORD tmp_dw;
	INDEXDATA *pIndex = NULL;
	WORD_INFO ReadBuff;
	LARGE_INTEGER MovSize, CurrSize;

	memset(&MovSize, NULL, sizeof(LARGE_INTEGER));
	memset(&CurrSize, NULL, sizeof(LARGE_INTEGER));

	word_ascii = ASCII_SUM(word);                //�ȼ����Ҫ���Ҵ����ASCII��֮��
	if ((pIndex = SearchAVL(AVL_ROOT, word_ascii)) == NULL)
		return 0;                              //�˴���δ����¼
	MovSize = pIndex->Start;
	MovSize.LowPart = SetFilePointer(hDataFile, MovSize.LowPart, &MovSize.HighPart, FILE_BEGIN);    //�ļ��ڲ�ָ���������������ʼ��λ��
	for (n = pIndex->Count; n != 0; n--)
	{
		memset(&ReadBuff, NULL, sizeof(WORD_INFO));
		if (!ReadFile(hDataFile, &ReadBuff, sizeof(WORD_INFO), &tmp_dw, NULL))
			return -1;
		if (!strcmp(ReadBuff.word, word))
		{
			//��ѯ���˵���
			if (WI != NULL)
				memcpy(WI, &ReadBuff, sizeof(WORD_INFO));
			if (PCurrSize != NULL)
			{
				//��ȡ��ǰ�ļ��ڲ�ָ��λ��
				CurrSize.LowPart = SetFilePointer(hDataFile, CurrSize.LowPart, &CurrSize.HighPart, FILE_CURRENT);
				CurrSize.QuadPart -= sizeof(WORD_INFO);
				memcpy(PCurrSize, &CurrSize, sizeof(LARGE_INTEGER));
			}
			return 1;
		}
	}

	return 0;
}

int Query()
{
	LARGE_INTEGER CurrSize;
	WORD_INFO WI;
	int choose;
	char ch_temp[50], word[50], result[100];
	TCHAR TC_temp[50];

	memset(&WI, NULL, sizeof(WORD_INFO));
	memset(word, NULL, sizeof(word));
	memset(result, NULL, sizeof(result));
	memset(ch_temp, NULL, sizeof(ch_temp));

	do
	{
		InputBox(TC_temp, 50, _T("���������"), _T("���Ҵ���"), NULL, 300, 200);
		memcpy(ch_temp, TCHARToChar(TC_temp), sizeof(ch_temp));               //��TCHAR�����ַ���ת��Ϊchar����
	} while (strlen(ch_temp)<1);
	memcpy(word, ch_temp, 50);

	switch (Query_Word(word, &WI, &CurrSize))
	{
	case 1:
		sprintf(result, "����:%s", WI.word);
		print(result, BLACK);
		memset(result, NULL, sizeof(result));
		sprintf(result, "����:%s", WI.pronunciation);
		print(result, BLACK);
		memset(result, NULL, sizeof(result));
		sprintf(result, "����:%s", WI.property);
		print(result, BLACK);
		memset(result, NULL, sizeof(result));
		sprintf(result, "����:%s", WI.describe);
		print(result, BLACK);

		printf("1.�޸�\t2.ɾ��\t0.����\n");

		cover(40, 250, 150, 50);
		cover(40, 370, 150, 50);
		cover(30, 500, 190, 50);

		settextcolor(GREEN);
		setbkmode(TRANSPARENT);
		settextstyle(15, 15, NULL);
		outtextxy(80, 270, _T("�޸�"));
		outtextxy(80, 395, _T("ɾ��"));
		outtextxy(80, 525, _T("����"));
		while (!(choose = GetMouse()));
		if (choose == 1)
		{
			if (!Alter_Word(CurrSize, &WI))
			{
				print("�����޸�ʧ��.", RED);
			}
			text2_y = 222;                  //����̨��Ϣ��������ʼ��
			cover(251, 222, 473, 378);
			sprintf(result, "����:%s", WI.word);
			print(result, BLACK);
			memset(result, NULL, sizeof(result));
			sprintf(result, "����:%s", WI.pronunciation);
			print(result, BLACK);
			memset(result, NULL, sizeof(result));
			sprintf(result, "����:%s", WI.property);
			print(result, BLACK);
			memset(result, NULL, sizeof(result));
			sprintf(result, "����:%s", WI.describe);
			print(result, BLACK);
			print("�����޸ĳɹ�.", GREEN);
		}
		else if (choose == 2)
		{
			text2_y = 222;                  //����̨��Ϣ��������ʼ��
			cover(251, 222, 473, 378);
			if (Delete_Word(CurrSize) != 1)
			{
				print("ɾ������ʧ��.", RED);
			}
			else
			{
				memset(result, NULL, sizeof(result));
				sprintf(result, "�ѳɹ�ɾ������:%s", WI.word);
				print(result, GREEN);
			}
		}
		break;
	case 0:
		print("�����ڴ˵���.", YELLOW);
		return 0;
	default:
		print("��ѯ����.", RED);
		return -1;
	}

	return 1;
}

int Add_Word()
{
	int InsertIndexResult = 0;
	INDEXDATA IndexData;            //AVL�����ڵ���������
	WORD_INFO word;                 //����ÿ��������Ϣ
	TCHAR TC_temp[100];
	char ch_temp[100];

	memset(&IndexData, NULL, sizeof(INDEXDATA));
	memset(&word, NULL, sizeof(WORD_INFO));
	memset(TC_temp, NULL, sizeof(TC_temp));
	memset(ch_temp, NULL, sizeof(ch_temp));

	do
	{
		InputBox(TC_temp, 50, _T("���������"), _T("��Ӵ���"), NULL, 300, 200);
		memcpy(ch_temp, TCHARToChar(TC_temp), sizeof(ch_temp));               //��TCHAR�����ַ���ת��Ϊchar����
	} while (strlen(ch_temp)<1);
	memcpy(word.word, ch_temp, 50);

	IndexData.ASCII = ASCII_SUM(word.word);                    //���㵥��ASCII��
	//printf("�˵��ʵ�ASCII֮��Ϊ:%d\n",IndexData.ASCII);
	if (Query_Word(word.word, NULL, NULL) == 1)
	{
		print("����¼�˴���,�����ظ����.", RED);
		return 2;
	}

	do
	{
		InputBox(TC_temp, 50, _T("������ƴ��"), _T("��Ӵ���"), NULL, 300, 200);
		memcpy(ch_temp, TCHARToChar(TC_temp), sizeof(ch_temp));               //��TCHAR�����ַ���ת��Ϊchar����
	} while (strlen(ch_temp)<1);
	memcpy(word.pronunciation, ch_temp, 50);

	do
	{
		InputBox(TC_temp, 20, _T("���������"), _T("��Ӵ���"), NULL, 300, 200);
		memcpy(ch_temp, TCHARToChar(TC_temp), sizeof(ch_temp));               //��TCHAR�����ַ���ת��Ϊchar����
	} while (strlen(ch_temp)<1);
	memcpy(word.property, ch_temp, 20);

	do
	{
		InputBox(TC_temp, 100, _T("����������"), _T("��Ӵ���"), NULL, 300, 200);
		memcpy(ch_temp, TCHARToChar(TC_temp), sizeof(ch_temp));               //��TCHAR�����ַ���ת��Ϊchar����
	} while (strlen(ch_temp)<1);
	memcpy(word.describe, ch_temp, 100);

	if (!(InsertIndexResult = InsertIndex(&AVL_ROOT, &IndexData)))    //������޸������ڵ�
	{
		//�ڶ��������������´�����Ӧ����������
		printf("�������ʧ�ܡ�", RED);
		return 0;
	}
	if (!InsertDataToFile(word, IndexData))
	{
		//���µĴ�����������ݿ��ļ���,�ڶ����������´�������Ӧ������.
		printf("д�ļ�ʧ�ܡ�", RED);
		return 0;
	}
	//��InsertIndexResult����ֵΪ1ʱ��������ӵĴ���׷�������ݿ��ļ�ĩβ������Ҫ��ȫ���������и��£�������Ҫ��
	if (InsertIndexResult == 1)
	{
		if (!InsertIndexToFile(IndexData, 1))         //�ڶ�������1��ʾ����ӵ������ڵ�ֻ��Ҫ׷���������ļ���ĩβ
		{
			printf("д�������ļ�ʧ�ܡ�", RED);
			return 0;
		}
	}
	else
	{
		//��������ֻ���޸��˽ڵ�����,û�����������.
		switch (AdjustIndex(IndexData))
		{
		case 0:
		{
				  printf("��������ʧ�ܡ�", RED);
				  return 0;
		}
		case -1:
		{
				   printf("�ļ��쳣,�����½���!", RED);
				   return 0;
		}
		default:
			break;
		}
	}
	return 1;
}

int Alter_Word(LARGE_INTEGER CurrSize, WORD_INFO *WI)
{
	DWORD tmp_dw;
	TCHAR TC_temp[100];
	char ch_temp[100];

	memset(TC_temp, NULL, sizeof(TC_temp));
	memset(ch_temp, NULL, sizeof(ch_temp));

	do
	{
		InputBox(TC_temp, 50, _T("�޸Ķ���"), _T("�޸Ĵ���"), CharToTCHAR(WI->pronunciation), 300, 200);
		memcpy(ch_temp, TCHARToChar(TC_temp), sizeof(ch_temp));               //��TCHAR�����ַ���ת��Ϊchar����
	} while (strlen(ch_temp)<1);
	memset(WI->pronunciation, NULL, 50);
	memcpy(WI->pronunciation, ch_temp, 50);

	do
	{
		InputBox(TC_temp, 20, _T("�޸Ĵ���"), _T("�޸Ĵ���"), CharToTCHAR(WI->property), 300, 200);
		memcpy(ch_temp, TCHARToChar(TC_temp), sizeof(ch_temp));               //��TCHAR�����ַ���ת��Ϊchar����
	} while (strlen(ch_temp)<1);
	memset(WI->property, NULL, 20);
	memcpy(WI->property, ch_temp, 20);

	do
	{
		InputBox(TC_temp, 100, _T("�޸�����"), _T("�޸Ĵ���"), CharToTCHAR(WI->describe), 300, 200);
		memcpy(ch_temp, TCHARToChar(TC_temp), sizeof(ch_temp));               //��TCHAR�����ַ���ת��Ϊchar����
	} while (strlen(ch_temp)<1);
	memset(WI->describe, NULL, 100);
	memcpy(WI->describe, ch_temp, 100);

	SetFilePointer(hDataFile, CurrSize.LowPart, &CurrSize.HighPart, FILE_BEGIN);
	if (!WriteFile(hDataFile, WI, sizeof(WORD_INFO), &tmp_dw, NULL))
		return 0;
	return 1;
}

int Delete_Word(const LARGE_INTEGER CurrSize)
{
	//����ֵ:1.ɾ���ɹ�.0.ɾ��ʧ��.-1.�����쳣
	int word_ascii;
	DWORD tmp_dw;
	WORD_INFO WordReadBuff;
	INDEXDATA IndexReadBuff;
	INDEXDATA *PIndex = NULL;
	LARGE_INTEGER refer, WriteAddr, ReadAddr;
	LARGE_INTEGER MovSize = CurrSize;

	memset(&WordReadBuff, NULL, sizeof(WORD_INFO));
	memset(&IndexReadBuff, NULL, sizeof(INDEXDATA));
	memset(&refer, NULL, sizeof(LARGE_INTEGER));
	memset(&WriteAddr, NULL, sizeof(LARGE_INTEGER));
	memset(&ReadAddr, NULL, sizeof(LARGE_INTEGER));

	SetFilePointer(hDataFile, MovSize.LowPart, &MovSize.HighPart, FILE_BEGIN);
	if (!ReadFile(hDataFile, &WordReadBuff, sizeof(WORD_INFO), &tmp_dw, NULL))
	{
		print("��ȡ���ݿ��ļ�����.", RED);
		return 0;
	}
	word_ascii = ASCII_SUM(WordReadBuff.word);
	PIndex = SearchAVL(AVL_ROOT, word_ascii);
	if (PIndex == NULL)
	{
		print("δ�ҵ��˴�������.", RED);
		return -1;
	}
	refer = PIndex->Start;                 //���浱ǰ������ʼ��ַ
	if (PIndex->Count>1)
	{
		PIndex->Count--;                 //�˴������͵�����������,ֻ�Ǵ�����Ŀ��һ
		InsertIndexToFile(*PIndex, 0);    //�޸��ļ��ڴ������Ӧ����
	}
	else
	{
		//�����͵Ĵ�����ɾ��,������֮Ҫ��ɾ��.
		Delete_Index(word_ascii);
		//ɾ���ļ�����Ӧ������Ҫ������������,��ʼ��������.
		DestroyAVL(AVL_ROOT);
		InitIndex();
	}
	if (!AlterIndexStartAddr(AVL_ROOT, &refer, 1) && GetLastError() != 0)
	{
		print("��������ʧ��.", RED);
		return 0;
	}

	//ɾ�����ݿ��ļ��еĴ�����Ϣ
	memset(&DataFileSize, NULL, sizeof(LARGE_INTEGER));
	GetFileSizeEx(hDataFile, &DataFileSize);                  //��ȡ���ݿ��ļ���С
	WriteAddr = CurrSize;
	ReadAddr.QuadPart = WriteAddr.QuadPart + sizeof(WORD_INFO);
	if (ReadAddr.QuadPart == DataFileSize.QuadPart)
	{
		SetFilePointer(hDataFile, WriteAddr.LowPart, &WriteAddr.HighPart, FILE_BEGIN);
		SetEndOfFile(hDataFile);
		return 1;
	}
	while (1)
	{
		SetFilePointer(hDataFile, ReadAddr.LowPart, &ReadAddr.HighPart, FILE_BEGIN);
		if (!ReadFile(hDataFile, &WordReadBuff, sizeof(WORD_INFO), &tmp_dw, NULL))
			return 0;
		if (tmp_dw<1) break;
		SetFilePointer(hDataFile, WriteAddr.LowPart, &WriteAddr.HighPart, FILE_BEGIN);
		if (!WriteFile(hDataFile, &WordReadBuff, sizeof(WORD_INFO), &tmp_dw, NULL))
			return 0;
		WriteAddr.QuadPart += sizeof(WORD_INFO);
		ReadAddr.QuadPart += sizeof(WORD_INFO);
		if (ReadAddr.QuadPart == DataFileSize.QuadPart)
		{
			SetFilePointer(hDataFile, WriteAddr.LowPart, &WriteAddr.HighPart, FILE_BEGIN);
			SetEndOfFile(hDataFile);
			return 1;
		}
	}

	return -1;
}

int Delete_Index(int ascii)
{
	DWORD tmp_dw;
	INDEXDATA ReadBuff;
	LARGE_INTEGER ReadAddr, CurrSize, WriteAddr;

	memset(&ReadBuff, NULL, sizeof(INDEXDATA));
	memset(&ReadAddr, NULL, sizeof(LARGE_INTEGER));
	memset(&CurrSize, NULL, sizeof(LARGE_INTEGER));
	memset(&WriteAddr, NULL, sizeof(LARGE_INTEGER));
	memset(&DataFileSize, NULL, sizeof(LARGE_INTEGER));

	GetFileSizeEx(hIndexFile, &DataFileSize);                                      //��ȡ�����ļ���С
	SetFilePointer(hIndexFile, CurrSize.LowPart, &CurrSize.HighPart, FILE_BEGIN);    //�ļ��ڲ�ָ��������ʼλ��
	while (1)
	{
		if (!ReadFile(hIndexFile, &ReadBuff, sizeof(INDEXDATA), &tmp_dw, NULL))
		{
			print("��ȡ�����ļ�����.", RED);
			return 0;
		}
		if (tmp_dw<1) break;
		if (ReadBuff.ASCII == ascii)
		{
			memset(&CurrSize, NULL, sizeof(LARGE_INTEGER));
			CurrSize.LowPart = SetFilePointer(hIndexFile, CurrSize.LowPart, &CurrSize.HighPart, FILE_CURRENT);   //��ȡ��ǰ�ļ�ָ��
			ReadAddr = CurrSize;
			memset(&WriteAddr, NULL, sizeof(LARGE_INTEGER));
			WriteAddr.QuadPart = ReadAddr.QuadPart - sizeof(INDEXDATA);            //�����һ�������ĳ���
			if (ReadAddr.QuadPart == DataFileSize.QuadPart)
			{
				SetFilePointer(hIndexFile, WriteAddr.LowPart, &WriteAddr.HighPart, FILE_BEGIN);
				SetEndOfFile(hIndexFile);
				return 1;
			}
			while (1)
			{
				memset(&ReadBuff, NULL, sizeof(INDEXDATA));
				if (!ReadFile(hIndexFile, &ReadBuff, sizeof(INDEXDATA), &tmp_dw, NULL))
					return 0;
				if (tmp_dw<1) break;
				SetFilePointer(hIndexFile, WriteAddr.LowPart, &WriteAddr.HighPart, FILE_BEGIN);     //��λ����ʼд���λ��
				if (!WriteFile(hIndexFile, &ReadBuff, sizeof(INDEXDATA), &tmp_dw, NULL))
					return 0;
				ReadAddr.QuadPart += sizeof(INDEXDATA);
				WriteAddr.QuadPart += sizeof(INDEXDATA);
				if (ReadAddr.QuadPart == DataFileSize.QuadPart)
				{
					SetFilePointer(hIndexFile, WriteAddr.LowPart, &WriteAddr.HighPart, FILE_BEGIN);
					SetEndOfFile(hIndexFile);                     //�����ļ�ĩ��ַ
					return 1;
				}
				SetFilePointer(hIndexFile, ReadAddr.LowPart, &ReadAddr.HighPart, FILE_BEGIN);         //��λ����ʼ��ȡ��λ��
			}
		}
	}

	return -1;
}

int OptimizeIndexFile()
{
	Queue Q;
	DWORD tmp_dw;
	QUEUE_DATATYPE elem;
	LARGE_INTEGER MovSize;

	memset(&Q, NULL, sizeof(Queue));
	memset(&MovSize, NULL, sizeof(LARGE_INTEGER));

	SetFilePointer(hIndexFile, MovSize.LowPart, &MovSize.HighPart, FILE_BEGIN);                 //��λ����ʼλ��
	SetEndOfFile(hIndexFile);            //��������ļ�����������

	//������ȱ���
	if (!EnQueue(&Q, AVL_ROOT))           //AVL�������
		return 0;
	while (Q.queuesize != 0)
	{
		DeQueue(&Q, &elem);
		if (elem->left)
		if (!EnQueue(&Q, elem->left))
			return 0;
		if (elem->right)
		if (!EnQueue(&Q, elem->right))
			return 0;
		if (!WriteFile(hIndexFile, &elem->data, sizeof(INDEXDATA), &tmp_dw, NULL))
			return 0;
	}

	return 1;
}

void ShowFile()
{
	FILE *file = NULL;
	LARGE_INTEGER MovSize;
	INDEXDATA IndexReadBuff;
	DWORD tmp_dw;

	memset(&MovSize, NULL, sizeof(LARGE_INTEGER));
	memset(&IndexReadBuff, NULL, sizeof(INDEXDATA));

	if ((file = fopen("IndexForm.txt", "wt+")) == NULL)
	{
		print("������ʱ�ļ�ʧ��.", RED);
		return;
	}

	fprintf(file, "�����ļ��ṹ:\n\n");
	memset(&MovSize, NULL, sizeof(LARGE_INTEGER));
	SetFilePointer(hIndexFile, MovSize.LowPart, &MovSize.HighPart, FILE_BEGIN);
	while (1)
	{
		if (!ReadFile(hIndexFile, &IndexReadBuff, sizeof(INDEXDATA), &tmp_dw, NULL))
			break;
		if (tmp_dw<1) break;
		fprintf(file, "ASCII��%d\t\t", IndexReadBuff.ASCII);
		fprintf(file, "StartAddr:%d\t\t", IndexReadBuff.Start.QuadPart);
		fprintf(file, "WordCount:%d\n", IndexReadBuff.Count);
	}
	fclose(file);
	ShellExecute(NULL, _T("open"), _T("IndexForm.txt"), NULL, NULL, SW_SHOWNORMAL);
	return;
}