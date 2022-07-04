#include <iostream>
#include <iomanip>
#include <stdio.h>
#include <vector>
#include <string>
#include <mysql.h>

#include "../include/const.h"

using namespace std;

//数据库增删改
int sqlChange(const char *myQuery)
{
    MYSQL *mysql;
    // MYSQL_ROW row;

    /* 初始化 mysql 变量，失败返回NULL */
    if ((mysql = mysql_init(NULL)) == NULL)
    {
        cout << "mysql_init failed" << endl;
        mysql_close(mysql);
        return -1;
    }

    /* 连接数据库，失败返回NULL
       1、mysqld没运行
       2、没有指定名称的数据库存在 */
    if (mysql_real_connect(mysql, "localhost", DB_USER, DB_PASSWD, DB_NAME, 0, NULL, 0) == NULL)
    {
        cout << "mysql_real_connect failed(" << mysql_error(mysql) << ")" << endl;
        mysql_close(mysql);
        return -1;
    }

    /* 设置字符集，否则读出的字符乱码，即使/etc/my.cnf中设置也不行 */
    mysql_set_character_set(mysql, "gbk");

    /* 进行查询，成功返回0，不成功非0
       1、查询字符串存在语法错误
       2、查询不存在的数据表 */
    if (mysql_query(mysql, myQuery))
    {
        cout << "mysql_query failed(" << mysql_error(mysql) << ")" << endl;
        mysql_close(mysql);
        return -1;
    }

    /* 关闭整个连接 */
    mysql_close(mysql);

    return 0;
}

//数据库查询函数
int sqlQuery(vector<vector<string>> &retData, const char *myQuery)
{
    MYSQL *mysql;
    MYSQL_RES *result;
    MYSQL_ROW row;

    retData.clear();

    /* 初始化 mysql 变量，失败返回NULL */
    if ((mysql = mysql_init(NULL)) == NULL)
    {
        cout << "mysql_init failed" << endl;
        mysql_close(mysql);
        return -1;
    }

    /* 连接数据库，失败返回NULL
       1、mysqld没运行
       2、没有指定名称的数据库存在 */
    if (mysql_real_connect(mysql, "localhost", DB_USER, DB_PASSWD, DB_NAME, 0, NULL, 0) == NULL)
    {
        cout << "mysql_real_connect failed(" << mysql_error(mysql) << ")" << endl;
        mysql_close(mysql);
        return -1;
    }

    /* 设置字符集，否则读出的字符乱码，即使/etc/my.cnf中设置也不行 */
    mysql_set_character_set(mysql, "gbk");

    /* 进行查询，成功返回0，不成功非0
       1、查询字符串存在语法错误
       2、查询不存在的数据表 */
    if (mysql_query(mysql, myQuery))
    {
        cout << "mysql_query failed(" << mysql_error(mysql) << ")" << endl;
        mysql_close(mysql);
        return -1;
    }

    /* 将查询结果存储起来，出现错误则返回NULL
       注意：查询结果为NULL，不会返回NULL */
    if ((result = mysql_store_result(mysql)) == NULL)
    {
        cout << "mysql_store_result failed" << endl;
        mysql_close(mysql);
        return -1;
    }

    /* 打印当前查询到的记录的数量 */
    cout << "select return " << (int)mysql_num_rows(result) << " records" << endl;

    /* 循环读取所有满足条件的记录
       1、返回的列顺序与select指定的列顺序相同，从row[0]开始
       2、不论数据库中是什么类型，C中都当作是字符串来进行处理，如果有必要，需要自己进行转换
       3、根据自己的需要组织输出格式 */
    // while ((row = mysql_fetch_row(result)) != NULL)
    // {
    //     cout << setiosflags(ios::left);          //输出左对齐
    //     cout << "ID：" << setw(12) << row[0];    //宽度12位，左对齐
    //     cout << "姓名：" << setw(8) << row[1];   //    8
    //     cout << "密码：" << setw(4) << row[2];   //    4
    //     cout << "文件ID：" << setw(4) << row[3]; //    4
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

    /* 释放result */
    mysql_free_result(result);

    /* 关闭整个连接 */
    mysql_close(mysql);

    return 0;
}
