#include <iostream>
#include <iomanip>
#include <stdio.h>
#include <vector>
#include <string>
#include <mysql.h>

#include "../include/const.h"

using namespace std;

//���ݿ���ɾ��
int sqlChange(const char *myQuery)
{
    MYSQL *mysql;
    // MYSQL_ROW row;

    /* ��ʼ�� mysql ������ʧ�ܷ���NULL */
    if ((mysql = mysql_init(NULL)) == NULL)
    {
        cout << "mysql_init failed" << endl;
        mysql_close(mysql);
        return -1;
    }

    /* �������ݿ⣬ʧ�ܷ���NULL
       1��mysqldû����
       2��û��ָ�����Ƶ����ݿ���� */
    if (mysql_real_connect(mysql, "localhost", DB_USER, DB_PASSWD, DB_NAME, 0, NULL, 0) == NULL)
    {
        cout << "mysql_real_connect failed(" << mysql_error(mysql) << ")" << endl;
        mysql_close(mysql);
        return -1;
    }

    /* �����ַ���������������ַ����룬��ʹ/etc/my.cnf������Ҳ���� */
    mysql_set_character_set(mysql, "gbk");

    /* ���в�ѯ���ɹ�����0�����ɹ���0
       1����ѯ�ַ��������﷨����
       2����ѯ�����ڵ����ݱ� */
    if (mysql_query(mysql, myQuery))
    {
        cout << "mysql_query failed(" << mysql_error(mysql) << ")" << endl;
        mysql_close(mysql);
        return -1;
    }

    /* �ر��������� */
    mysql_close(mysql);

    return 0;
}

//���ݿ��ѯ����
int sqlQuery(vector<vector<string>> &retData, const char *myQuery)
{
    MYSQL *mysql;
    MYSQL_RES *result;
    MYSQL_ROW row;

    retData.clear();

    /* ��ʼ�� mysql ������ʧ�ܷ���NULL */
    if ((mysql = mysql_init(NULL)) == NULL)
    {
        cout << "mysql_init failed" << endl;
        mysql_close(mysql);
        return -1;
    }

    /* �������ݿ⣬ʧ�ܷ���NULL
       1��mysqldû����
       2��û��ָ�����Ƶ����ݿ���� */
    if (mysql_real_connect(mysql, "localhost", DB_USER, DB_PASSWD, DB_NAME, 0, NULL, 0) == NULL)
    {
        cout << "mysql_real_connect failed(" << mysql_error(mysql) << ")" << endl;
        mysql_close(mysql);
        return -1;
    }

    /* �����ַ���������������ַ����룬��ʹ/etc/my.cnf������Ҳ���� */
    mysql_set_character_set(mysql, "gbk");

    /* ���в�ѯ���ɹ�����0�����ɹ���0
       1����ѯ�ַ��������﷨����
       2����ѯ�����ڵ����ݱ� */
    if (mysql_query(mysql, myQuery))
    {
        cout << "mysql_query failed(" << mysql_error(mysql) << ")" << endl;
        mysql_close(mysql);
        return -1;
    }

    /* ����ѯ����洢���������ִ����򷵻�NULL
       ע�⣺��ѯ���ΪNULL�����᷵��NULL */
    if ((result = mysql_store_result(mysql)) == NULL)
    {
        cout << "mysql_store_result failed" << endl;
        mysql_close(mysql);
        return -1;
    }

    /* ��ӡ��ǰ��ѯ���ļ�¼������ */
    cout << "select return " << (int)mysql_num_rows(result) << " records" << endl;

    /* ѭ����ȡ�������������ļ�¼
       1�����ص���˳����selectָ������˳����ͬ����row[0]��ʼ
       2���������ݿ�����ʲô���ͣ�C�ж��������ַ��������д�������б�Ҫ����Ҫ�Լ�����ת��
       3�������Լ�����Ҫ��֯�����ʽ */
    // while ((row = mysql_fetch_row(result)) != NULL)
    // {
    //     cout << setiosflags(ios::left);          //��������
    //     cout << "ID��" << setw(12) << row[0];    //���12λ�������
    //     cout << "������" << setw(8) << row[1];   //    8
    //     cout << "���룺" << setw(4) << row[2];   //    4
    //     cout << "�ļ�ID��" << setw(4) << row[3]; //    4
    //     cout << endl;
    // }
    while ((row = mysql_fetch_row(result)) != NULL)
    {
        vector<string> nowRow = vector<string>();
        for (unsigned int i = 0; i < mysql_num_fields(result); i++)
        {
            if(row[i])
            {
                nowRow.push_back(row[i]);
                // printf("%s\n", row[i]);
            }
            else
                nowRow.push_back("");
        }
        retData.push_back(nowRow);
    }

    /* �ͷ�result */
    mysql_free_result(result);

    /* �ر��������� */
    mysql_close(mysql);

    return 0;
}
