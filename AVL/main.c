#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>

#define DATATYPE int
#define FORM "%d"

typedef struct BiTree
{
    int balance;              //ƽ������
    DATATYPE data;            //�ڵ�����
    struct BiTree *left;
    struct BiTree *right;
    struct BiTree *father;    //ָ��˫�׽ڵ�
} AVL;

int Depth(AVL *root)
{
    //����������ȡ�
    int i,j;

    if(root->left)
        i=Depth(root->left);
    else
        i=0;

    if(root->right)
        j=Depth(root->right);
    else
        j=0;

    return i>j?i+1:j+1;
}

void updateBF(AVL *NewNode)
{
    //����ƽ������
    if(!NewNode || !NewNode->father) return;
    AVL *CurrNode=NewNode;
    AVL *PreNode=CurrNode->father;

    if(CurrNode==PreNode->left)
    {
        //ָ�����·����
        PreNode->balance++;
        if(PreNode->balance==2) return;
        if(PreNode->left && PreNode->right)   //�������������ӵĽڵ�
            if(PreNode->balance<=0)
                return;
    }
    else
    {
        //ָ�����·����
        PreNode->balance--;
        if(PreNode->balance==-2) return;
        if(PreNode->left && PreNode->right)
            if(PreNode->balance>=0)
                return;
    }
    updateBF(PreNode);

    return;
}

void RR_Rotate(AVL **root,AVL *centreNode)
{
    //������ת,RR�͡�
    AVL *temp=NULL;

    if(centreNode->right)
    {
        //���Ľڵ���ҽڵ㲻Ϊ��
        centreNode->father->left=centreNode->right;     //�����Ľڵ�����������ӵ�˫�׽ڵ���������ϡ�
        centreNode->right->father=centreNode->father;
    }
    else
        //���Ľڵ���ҽڵ�Ϊ��
        centreNode->father->left=NULL;

    temp=centreNode->father->father;
    if(centreNode->father->father!=NULL)
        if(centreNode->father->father->left==centreNode->father)
            centreNode->father->father->left=centreNode;
        else
            centreNode->father->father->right=centreNode;
    else
        *root=centreNode;

    centreNode->right=centreNode->father;         //��ǰ�ڵ��˫�׳�Ϊ����������
    centreNode->father->father=centreNode;
    centreNode->father->balance-=2;
    centreNode->father=temp;
    centreNode->balance--;

    return;
}

void LL_Rotate(AVL **root,AVL *centreNode)
{
    //������ת,LL��
    AVL *temp=NULL;

    if(centreNode->left)
    {
        //���Ľڵ����ڵ㲻Ϊ��
        centreNode->father->right=centreNode->left;    //�����Ľڵ�����������ӵ�˫�׵��������ϡ�
        centreNode->left->father=centreNode->father;   //���Ľڵ����������˫�׸�Ϊ���Ľڵ��˫�ס�
    }
    else
        centreNode->father->right=NULL;

    temp=centreNode->father->father;
    if(centreNode->father->father!=NULL)
        if(centreNode->father->father->left==centreNode->father)
            centreNode->father->father->left=centreNode;
        else
            centreNode->father->father->right=centreNode;
    else
        *root=centreNode;

    centreNode->left=centreNode->father;         //�����Ľڵ�����Ӹ�Ϊ���Ľڵ��˫��
    centreNode->left->father=centreNode;         //�����Ľڵ����ӵ�˫�׸���Ϊ���Ľڵ㡣
    centreNode->balance++;
    centreNode->father->balance+=2;
    centreNode->father=temp;

    return;
}

void RL_Rotate(AVL **root,AVL *centreNode)
{
    //���Һ�����ת
    AVL *temp=NULL;

    temp=centreNode->father;
    centreNode->father->right=centreNode->left;
    centreNode->left->father=centreNode->father;
    centreNode->left->balance--;
    centreNode->balance--;

    if(centreNode->left->right)
    {
        //���Ľڵ����ڵ���Һ��Ӳ�Ϊ�ա�
        temp->right->right->father=centreNode;
    }
    centreNode->left=temp->right->right;
    temp->right->right=centreNode;
    centreNode->father=temp->right;
    if(temp->right->balance==-2)
        temp->right->balance++;
    LL_Rotate(root,temp->right);

    return;
}

void LR_Rotate(AVL **root,AVL *centreNode)
{
    //���������ת��LR�͡�
    AVL *temp=NULL;

    temp=centreNode->father;
    centreNode->father->left=centreNode->right;
    centreNode->right->father=centreNode->father;
    centreNode->right->balance++;
    centreNode->balance++;

    if(centreNode->right->left)
    {
        //���Ľڵ����ڵ���Һ��Ӳ�Ϊ�ա�
        temp->left->left->father=centreNode;
    }
    centreNode->right=temp->left->left;
    temp->left->left=centreNode;
    centreNode->father=temp->left;
    if(temp->left->balance==2)
        temp->left->balance--;
    RR_Rotate(root,temp->left);

    return;
}

void adjustAVL(AVL **root,AVL *NewNode)
{
    if(!NewNode) return;
    AVL *CurrNode=NewNode;
    AVL *PreNode=NULL;

    while(CurrNode->father!=NULL && CurrNode->father->balance!=2 && CurrNode->father->balance!=-2)
    {
        PreNode=CurrNode;
        CurrNode=CurrNode->father;
    }
    if(!CurrNode->father) return;
    //printf(":%d\n",CurrNode->data);

    if(CurrNode->father->left==CurrNode)
    {
        //�Լ���˫�׵���ڵ�
        if(CurrNode->left==PreNode)
            //ָ�����·���ݡ�
            RR_Rotate(root,CurrNode);
        else
            //ָ�����·����
            LR_Rotate(root,CurrNode);
    }
    else
    {
        //�Լ���˫�׵��ҽڵ�
        if(CurrNode->right==PreNode)
            //ָ�����·����
            LL_Rotate(root,CurrNode);
        else
            //ָ�����·����
            RL_Rotate(root,CurrNode);
    }

    return;
}

int InsertNode(AVL **root,DATATYPE elem)
{
    AVL *CurrNode=NULL;
    AVL *PreNode=NULL;
    AVL *NewNode=NULL;

    if(*root==NULL)  //������ڵ㲻�����򴴽�
    {
        if((*root=(AVL *)malloc(sizeof(AVL)))==NULL)
            return 0;
        memset(*root,NULL,sizeof(AVL));
        (*root)->data=elem;
        return 1;
    }

    CurrNode=*root;
    while(CurrNode)
    {
        PreNode=CurrNode;
        if(elem<CurrNode->data)
            CurrNode=CurrNode->left;
        else if(elem>CurrNode->data)
            CurrNode=CurrNode->right;
        else
            return 2;    //��ʾ�Ѵ��ڸ�Ԫ��
    }
    if((NewNode=(AVL *)malloc(sizeof(AVL)))==NULL)
        return 0;
    memset(NewNode,NULL,sizeof(AVL));
    NewNode->data=elem;        //Ϊ�½ڵ㸳ֵ
    NewNode->father=PreNode;
    //�����½ڵ�
    if(elem<PreNode->data)
        PreNode->left=NewNode;
    else
        PreNode->right=NewNode;
    updateBF(NewNode);     //����AVL��ƽ������
    adjustAVL(root,NewNode);    //AVL�Ե���

    return 1;
}

int InOrderTraversal(AVL *root)
{
    int n=0;
    if(root==NULL)
        return 0;
    n=InOrderTraversal(root->left);
    n++;
    n+=InOrderTraversal(root->right);
    return n;
}

int search(AVL *root,DATATYPE elem)
{
    if(!root) return 0;
    if(elem==root->data) return 1;
    if(elem<root->data)
        if(search(root->left,elem)) return 1;
    if(elem>root->data)
        if(search(root->right,elem)) return 1;
    return 0;
}

int main(int argc,char *argv[])
{
    int j=0,i=0;
    DATATYPE elem;
    AVL *root=NULL;

    while(i<30000)
    {
        srand(j++);
        elem=rand();
        if(InsertNode(&root,elem)==2) continue;    //����ڵ㡣
        printf("�Ѳ���%d���ڵ㡣\r",++i);
    }
    //InOrderTraversal(root);
    puts("");
    printf("����:%d\n\n\n",Depth(root));

    while(1)
    {
        printf("��%d���ڵ㡣\n",InOrderTraversal(root));
        printf("���������:%d,���������:%d\n",Depth(root->left),Depth(root->right));
        printf("������ҵ�Ԫ��:");
        fflush(stdin);
        if(scanf(FORM,&elem)!=1) continue;
        if(search(root,elem))
            printf("���ҵ���\n");
        else
        {
            printf("û�ҵ�������ӡ�\n");
            InsertNode(&root,elem);
        }
    }


    return 0;
}












