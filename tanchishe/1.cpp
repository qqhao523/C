#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <conio.h>
#include <time.h>

#define LEFT 75
#define UP 72
#define RIGHT 77
#define DOWN 80
#define PAUSE 32
#define HEAD_PIC "��"
#define BODY_PIC "��"

int food_x=0,food_y=0;                //ʳ�������
int food_number=0;                    //�ܹ��Ե�ʳ�����
static time_t start_time;             //��Ϸ��ʼʱ��
static time_t play_time=0;            //��Ϸʱ��
static int play_switch=0,Exit_Thread=0;

typedef struct snake_position
{
    //�����ÿһ��λ������
    int x;
    int y;
    int direction;
    struct snake_position *next;
} snake;

void gotoxy(int x,int y)
{
    //��λ����̨��ꡣ
    COORD coord;
    coord.X=x;
    coord.Y=y;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE),coord);
}

void print(char str[])
{
    char *p=str;
    while(1)
    {
        Sleep(20);
        putchar(*p++);
        if(*p==NULL) break;
    }
}
void draw_wall()
{
    //����Ϸ���溯��
    int i, j;
    for(i = 39,j = 0; j < 39; i--, j++)
    {
        gotoxy(j*2, 0);
        printf("��");
        if(j < 24)
        {
            gotoxy(78, j);
            printf("��");
        }
        gotoxy(i*2, 24);
        printf("��");
        if(i > 15)
        {
            gotoxy(0, i-15);
            printf("��");
        }
        Sleep(50);
    }

    gotoxy(82,3);
    print("��Ϸʱ��:0  ��");
    gotoxy(82,9);
    print("�ܹ�����:0 ������");
    gotoxy(81,24);
    print("By:�ƿ�12-1�� κ���");
    gotoxy(81,16);
    print("�밴�������ʼ��Ϸ!");
}

void init_console()
{
    system("mode con cols=102 lines=25 & color A4");
    SetConsoleTitle("̰����");
    CONSOLE_CURSOR_INFO cursor_info = {1, 0};
    SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursor_info);   //  ���ؿ���̨���
    draw_wall();    //����Ϸ����
}

snake *Create_snake()
{
    int i;
    snake *head=NULL,*body=NULL;
    body=head=(snake *)malloc(sizeof(snake));

    body->x=24;
    body->y=10;
    body->direction=RIGHT;
    for(i=0; i<4; i++)
    {
        body->next=(snake *)malloc(sizeof(snake));
        body->next->x=body->x-2;
        body->next->y=body->y;
        body=body->next;
    }
    body->next=NULL;

    return head;
}

int draw_snake(snake *head)
{
    snake *body=head;

    gotoxy(body->x,body->y);
    printf(HEAD_PIC);
    while(1)
    {
        if((body=body->next)==NULL) return 1;
        gotoxy(body->x,body->y);
        printf(BODY_PIC);
    }
}


DWORD WINAPI Control_snake(LPVOID Paramter)
{
    int ch;
    snake *head=(snake *)Paramter;
    while(1)
    {
        while(!kbhit()) if(Exit_Thread) return 1;            //����Ƿ��м�������
        {
            ch=getch();                                      //���ռ�������
            switch(ch)
            {
            case RIGHT:
                if(head->direction!=LEFT) head->direction=RIGHT;
                break;
            case LEFT:
                if(head->direction!=RIGHT) head->direction=LEFT;
                break;
            case UP:
                if(head->direction!=DOWN) head->direction=UP;
                break;
            case DOWN:
                if(head->direction!=UP) head->direction=DOWN;
                break;
            case PAUSE:
                play_switch=1;
                getch();
                play_switch=0;
                break;
            default:
                break;
            }
        }
    }
    return 1;
}

int lay_food(snake *head)
{
    snake *body=NULL;

    while(1)
    {
        srand((int)time(NULL)+rand());     //�����������
        food_x=(rand()%38+2)*2;
        food_y=rand()%23+1;     //�������ʳ�������
        if(food_x>76 || food_x<2 || food_y>23 || food_y<=1) continue;   //�ж�ʳ���Ƿ���ǽ��
        for(body=head; 1; body=body->next)
        {
            //�ж�ʳ���Ƿ���������
            if(body->x==food_x && body->y==food_y) break;
            if(body->next==NULL) goto skip;
        }
    }
skip:
    gotoxy(food_x,food_y);
    printf(BODY_PIC);
    return 0;
}

int snake_move(snake *head)
{
    //�������˶�,һ���ƶ�һ��
    snake *body=head,tmp1,tmp2,tail;

	/*����̬�����ÿ���ڵ������ͷ��������һ���ڵ������ͷ���*/
    tmp1=*body->next;
    body->next->x=body->x;
    body->next->y=body->y;
    body->next->direction=body->direction;
    body=body->next;

    while(body->next)
    {
        if(!body->next->next) tail=*body->next;
        tmp2=*body->next;
        body->next->x=tmp1.x;
        body->next->y=tmp1.y;
        body->next->direction=tmp1.direction;
        body=body->next;

        if(!body->next) break;

        if(!body->next->next) tail=*body->next;
        tmp1=*body->next;
        body->next->x=tmp2.x;
        body->next->y=tmp2.y;
        body->next->direction=tmp2.direction;
        body=body->next;
    }
    gotoxy(tail.x,tail.y);
    printf("  ");

    body=head;
    gotoxy(body->x,body->y);
    printf(BODY_PIC);
    switch(body->direction)
    {
        //�ƶ�ͷ��
    case RIGHT:
        body->x+=2;
        break;
    case LEFT:
        body->x-=2;
        break;
    case UP:
        body->y-=1;
        break;
    case DOWN:
        body->y+=1;
        break;
    }
    gotoxy(body->x,body->y);
    printf(HEAD_PIC);

    return 1;
}

int eat_food(snake *head)
{
    snake *body=head;

    if(body->x==food_x && body->y==food_y)
    {
        food_number++;
        for(body=body->next; 1; body=body->next)
        {
            if(body->next==NULL)
            {
                body->next=(snake *)malloc(sizeof(snake));
                body->next->next=NULL;
                body->next->direction=body->direction;
                switch(body->direction)     //������β������
                {
                case RIGHT:
                    body->next->x=body->x-2;
                    body->next->y=body->y;
                    break;
                case LEFT:
                    body->next->x=body->x+2;
                    body->next->y=body->y;
                    break;
                case UP:
                    body->next->y=body->y+1;
                    body->next->x=body->x;
                    break;
                case DOWN:
                    body->next->y=body->y-1;
                    body->next->x=body->x;
                    break;
                }
                body=body->next;
                gotoxy(body->x,body->y);
                printf(BODY_PIC);
                return 1;
            }
        }
    }
    return 0;
}

int Check_wall(snake *head)
{
    if((head->x<2 || head->x>=78) || (head->y<=0 || head->y>=24))   //���ͷ�������Ƿ���ײ��ǽ��
        return 1;
    else
        return 0;

}

int eat_myself(snake *head)
{
    snake *body=NULL;

    for(body=head->next; 1; body=body->next)
    {
        if(head->x==body->x && head->y==body->y) return 1;
        if(!body->next) break;
    }
    return 0;
}

int show_food_number()
{
    gotoxy(91,9);
    printf("%d",food_number);
    return 0;
}

int show_time()
{
    if(play_time!=time(NULL)-start_time)
    {
        play_time=time(NULL)-start_time;
        gotoxy(91,3);
        printf("   ");
        gotoxy(91,3);
        printf("%d",play_time);
    }
    return 1;
}

int Game_Over()
{
    char ch;
    Exit_Thread=1;
    gotoxy(82,16);
    printf("                   ");
    gotoxy(85,16);
    printf("Game Over!\n");
    gotoxy(84,18);
    printf("����һ��? Y/N");

    while(1)
    {
        ch=getch();
        if(ch=='y' || ch =='Y') return 1;
        else if(ch=='N' || ch=='n') return 0;
    }
}

int main(int argc,char *argv[])
{
    HANDLE hThread=NULL;
    snake *head=NULL,*body=NULL,*tmp=NULL;
    int food_switch=1;
again:
    init_console();
    head=Create_snake();
    draw_snake(head);
    CloseHandle(hThread=CreateThread(NULL,0,Control_snake,(LPVOID)head,0,NULL)); //������ȡ������Ϣ
    getch();
    gotoxy(82,16);
    printf("                   ");
    gotoxy(82,16);
    printf("���ո����ͣ��Ϸ");
    start_time=time(NULL);

    while(1)
    {
        if(!play_switch)
        {
            show_time();                          //��ʾ��Ϸʱ��
            show_food_number();                   //��ʾ�Ե�ʳ�����
            if(food_switch)                       //����ʳ��
            {
                lay_food(head);
                food_switch=0;
            }
            snake_move(head);                     //�������ƶ�
            if(eat_food(head)) food_switch=1;     //�ж��Ƿ�Ե�ʳ��
            if(Check_wall(head)) break;           //�ж��Ƿ�ײǽ
            if(eat_myself(head)) break;           //�ж����Ƿ�ҧ���Լ�
            gotoxy(83,1);
            printf("X:%d   Y:%d",head->x,head->y);
            Sleep(100);
        }
        else
        {
            gotoxy(82,16);
            printf("                ");
            gotoxy(82,16);
            printf("�������������Ϸ");
            while(play_switch) show_time();;
            gotoxy(82,16);
            printf("                   ");
            gotoxy(82,16);
            printf("���ո����ͣ��Ϸ");
        }
    }
    if(Game_Over())
    {
        Exit_Thread=0;
        gotoxy(85,16);
        printf("           ");
        gotoxy(84,18);
        printf("             ");
        play_time=0;
        food_switch=1;
        food_x=0;
        food_y=0;
        food_number=0;
        for(body=head; !body; body=tmp)
        {
            //�ͷ�����ռ�õ��ڴ�
            tmp=body->next;
            free(body);
        }
        head=NULL;
        goto again;
    }

    return 0;
}

