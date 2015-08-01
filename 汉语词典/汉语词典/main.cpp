#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <conio.h>

#include "GUI.h"
#include "global.h"
#include "AVL.h"
#include "FileOper.h"
#include "init.h"
#include "menu.h"

int main(int argc, char *argv[])
{
	InitGUI();
	int choose, result;

	if (!InitFile())
	{
		print("��ʼ���ļ�ʧ�ܡ�", RED);
		_getch();
		return -1;
	}
	if (!InitIndex())
	{
		print("��ʼ������ʧ�ܡ�", RED);
		_getch();
		return -1;
	}

again:
	ShowAVLDepth();
	ShowCount();
	ShowFileSize();
	while (!(choose = GetMouse()));
	text2_y = 222;                  //����̨��Ϣ��������ʼ��
	cover(251, 222, 473, 378);
	switch (choose)
	{
	case 1:
		if (!(result = Add_Word()))
		{
			print("��Ӵ���ʧ�ܡ�", RED);
		}
		else if (result == 1)
		{
			print("��Ӵ����ɹ���", GREEN);
		}
		else
		{
			print("�ʿ�û�б䶯��", YELLOW);
		}
		break;
	case 2:
		Query();
		InitMenu();
		break;
	case 3:
		if (!OptimizeIndexFile())
		{
			print("�Ż������ļ�ʧ��,�����ļ������Ѿ���!", RED);
		}
		else
		{
			print("�Ż������ļ��ɹ�.", GREEN);
		}
		print("�Ƿ�鿴�����ļ��ṹ?[Y/N]", GREEN);
		fflush(stdin);
		choose = _getch();
		if (choose == 'y' || choose == 'Y')
			ShowFile();
		text2_y = 222;                  //����̨��Ϣ��������ʼ��
		cover(251, 222, 473, 378);
		break;
	}
	Sleep(500);           //��ֹ�������
	goto again;
	return 0;
}