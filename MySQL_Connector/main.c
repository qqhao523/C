// MySQL Connector.cpp : Defines the entry point for the console application.
//
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <winsock2.h>
#include "mysql.h"/*ע��Ҫ�������ͷ�ļ�*/
#pragma comment(lib,"libmysql")
/*������һЩ���ݿ�������Ҫ�ĺ�*/
#define HOST "localhost"
#define USERNAME "root"
#define PASSWORD "305859616"
#define DATABASE "test"
/*�����������ִ�д����sql�Z��*/
void exe_sql(char* sql)
{
    MYSQL my_connection; /*����һ�����ݿ�����*/
    int res; /*ִ��sql�Z���ķ��ر�־*/
    /*��ʼ��mysql����my_connection*/
    mysql_init(&my_connection);    //��ʼ����MYSQL���
    /*�����������mysql.h���һ��������������֮ǰ�������Щ�꽨��mysql���ӣ���
    ����һ��ֵ�����ز�Ϊ��֤�������ǳɹ���*/
    if (mysql_real_connect(&my_connection, HOST, USERNAME, PASSWORD,DATABASE, 0, NULL, CLIENT_FOUND_ROWS))  /*���ӳɹ�*/
    {
        printf("���ݿ�ִ��exe_sql���ӳɹ���\n");
        /*��仰�����ò�ѯ����Ϊutf8������֧������*/
        mysql_query(&my_connection, "set names utf8");
        /*������仰������mysql_query������ִ�����Ǹոմ����sql�Z�䣬
        ��᷵��һ��intֵ�����Ϊ0��֤���Z��ִ�гɹ�*/
        res = mysql_query(&my_connection, sql);   //��ѯ�ɹ�����0
        if (res)  /*���ھʹ���ִ��ʧ����*/
        {
            printf("Error�� mysql_query !\n");
            /*��Ҫ���˹ر�����*/
            mysql_close(&my_connection);
        }
        else    /*���ھʹ���ִ�гɹ���*/
        {
            /*mysql_affected_rows�᷵��ִ��sql��Ӱ�������*/
            printf("%d ���ܵ�Ӱ�죡\n",
                   mysql_affected_rows(&my_connection));   //mysql_affected_rows() ��������ǰһ�� MySQL ������Ӱ��ļ�¼������
            /*��Ҫ���˹ر�����*/
            mysql_close(&my_connection);
        }
    }
    else
    {
        /*���ݿ�����ʧ��*/
        printf("���ݿ�ִ��exe_sql����ʧ�ܣ�\n");
    }
}
/*�����������ִ�д����sql�Z�䣬����ӡ����ѯ�Y��*/
void query_sql(char* sql)
{
    MYSQL my_connection; /*����һ�����ݿ����ӱ�ʶ*/
    int res; /*ִ��sql�Z���ķ��ر�־*/
    MYSQL_RES *res_ptr; /*ָ���ѯ�����ָ��*/
    MYSQL_FIELD *field; /*�ֶνṹָ��*/
    MYSQL_ROW result_row; /*���з��صĲ�ѯ��Ϣ*/
    int row, column; /*��ѯ���ص�����������*/
    int i, j; /*ֻ�ǿ���ѭ������������*/
    /*��ʼ��mysql����my_connection*/
    mysql_init(&my_connection);
    /*�����������mysql.h���һ��������������֮ǰ�������Щ�꽨��mysql���ӣ���
    ����һ��ֵ�����ز�Ϊ��֤�������ǳɹ���*/
    if (mysql_real_connect(&my_connection, HOST, USERNAME, PASSWORD,DATABASE, 0, NULL, CLIENT_FOUND_ROWS))  /*���ӳɹ�*/
    {
        printf("���ݿ��ѯquery_sql���ӳɹ���\n");
        /*��仰�����ò�ѯ����Ϊutf8������֧������*/
        mysql_query(&my_connection, "set names utf8");
        /*������仰������mysql_query������ִ�����Ǹոմ����sql�Z�䣬
        ��᷵��һ��intֵ�����Ϊ0��֤���Z��ִ�гɹ�*/
        res = mysql_query(&my_connection, sql);
        if (res)   /*���ھʹ���ִ��ʧ����*/
        {
            printf("Error�� mysql_query !\n");
            /*��Ҫ���˹ر�����*/
            mysql_close(&my_connection);
        }
        else     /*���ھʹ���ִ�гɹ���*/
        {
            /*����ѯ�ĽY����res_ptr*/
            res_ptr = mysql_store_result(&my_connection);
            /*��������Ϊ�գ��Ͱѽ��print*/
            if (res_ptr)
            {
                /*ȡ�ýY����������*/
                column = mysql_num_fields(res_ptr);
                row = mysql_num_rows(res_ptr) + 1;
                printf("��ѯ�� %lu �� \n", row);
                /*����Y�����ֶ���*/
                for (i = 0; field = mysql_fetch_field(res_ptr); i++)
                    printf("%s\t", field->name);
                printf("\n");
                /*��������Y��*/
                for (i = 1; i < row; i++)
                {
                    result_row = mysql_fetch_row(res_ptr);
                    for (j = 0; j < column; j++)
                        printf("%s\t", result_row[j]);
                    printf("\n");
                }
            }
            /*��Ҫ���˹ر�����*/
            mysql_close(&my_connection);
        }
    }
}
int main(int argc, char *argv[])
{
    /*�������������������*/
    char *query;
    char *exe = "create database wayne;";
    //exe_sql(exe);
    /*�����²�ѯ*/
    query="select * from test_1;";
    query_sql(query);
    return 0;
}
