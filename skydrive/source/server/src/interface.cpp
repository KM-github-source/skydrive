#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <ifaddrs.h>
#include <sys/prctl.h>
#include <signal.h>
#include <vector>
#include <string>
#include <map>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <fcntl.h>
#include <mysql.h>

#include "../include/const.h"
#include "../include/tools.h"
#include "../include/sql.h"

using namespace std;

string genJsonResponse(string res_data)
{
    string ret = "HTTP/1.1 200 OK\r\n";
    ret += "connection: close\r\n"; // 重要，如果不写则会多个请求发往同一个socket连接，服务器无法处理
    ret += "access-control-allow-origin: *\r\n";
    ret += "content-length: ";
    ret += to_string(res_data.length()) + "\r\n";
    ret += "content-type: application/json\r\n\r\n";
    ret += res_data;
    return ret;
}

int getBinaryResponseHead(char *res_data, int len)
{
    sprintf(res_data, "HTTP/1.1 200 OK\r\n"
                      "content-length: %d\r\n"
                      "connection: close\r\n" // 重要，如果不写则会多个请求发往同一个socket连接，服务器无法处理
                      "access-control-allow-origin: *\r\n"
                      "cache-control: no-cache, no-store, must-revalidate\r\n"
                      "content-type: application/octet-stream\r\n\r\n",
            len);

    return 0;
}

int getBinaryResponseHeadForWeb(char *res_data, int len)
{
    sprintf(res_data, "HTTP/1.1 200 OK\r\n"
                      "content-length: %d\r\n"
                      "connection: close\r\n" // 重要，如果不写则会多个请求发往同一个socket连接，服务器无法处理
                      "access-control-allow-origin: *\r\n"
                      "Content-Transfer-Encoding: binary\r\n"
                    //   "cache-control: no-cache, no-store, must-revalidate\r\n"
                      "content-disposition: attachment;filename=123.txt\r\n"
                      "content-type: application/octet-stream\r\n\r\n",
            len);

    return 0;
}

string getAllContent(int sock, char *buf, int body_start, int total_len, int content_len)
{
    string content = buf + body_start;
    int ret;
    while (total_len < content_len)
    {
        ret = read(sock, buf, BUF_SIZE);
        if (ret <= 0)
            break;
        buf[ret] = '\0';
        content += buf;
        total_len += ret;
    }
    return content;
}

// 解析formdata
int getAllFormData(int sock, char *buf, int body_start, int total_len, int content_len, char *form_content)
{
    memmove(form_content, (buf + body_start), total_len);
    int ret;
    while (total_len < content_len)
    {
        ret = read(sock, form_content + total_len, BUF_SIZE);
        if (ret <= 0)
            return -1;
        total_len += ret;
    }
    return 0;
}

// 新建文件夹，返回logic_file表的id
string mkDir(string gbk_dir_name, string parent_id)
{
    int ret;
    // 新建文件夹
    string sql = "insert into logic_file(file_name,parent_id,is_dir) values(\'";
    sql += gbk_dir_name + "\'," + parent_id + ",1);";
    printf("sql = %s\n", sql.c_str());
    ret = sqlChange(sql.c_str());
    if (ret)
    {
        printf("mkDir sqlChange err\n");
        return "";
    }

    // 取得新建的id
    vector<vector<string>> sql_query_data;
    sql = "select id from logic_file where file_name = \'";
    sql += gbk_dir_name + "\' and parent_id = ";
    sql += parent_id + " and is_dir = 1;";
    ret = sqlQuery(sql_query_data, sql.c_str());
    if (ret)
    {
        printf("mkDir sqlQuery err\n");
        return "";
    }

    return sql_query_data.at(0).at(0);
}

// 向logic文件表插入一条记录
int createLogicFile(string gbk_file_name, string md5, string parent_id, string is_dir)
{
    string sql = "insert into logic_file(file_name, md5, parent_id, is_dir) values (\'";
    sql += gbk_file_name + "\', \'";
    sql += md5 + "\', ";
    sql += parent_id + ", ";
    sql += is_dir + ");";
    return sqlChange(sql.c_str());
}

int respErrMsg(int sock, string msg)
{
    map<string, string> res_data;
    res_data["msg"] = msg;
    string res = genJsonResponse(toJson(res_data));
    if (write(sock, res.c_str(), res.length()) <= 0)
        return -1;
    return 0;
}

int userLogin(int sock, char *buf, int body_start, int total_len, int content_len, string clientinfo)
{
    int ret;
    string content = getAllContent(sock, buf, body_start, total_len, content_len);
    map<string, string> res_data;

    // 解析json参数
    // printf("login content = %s", content.c_str());
    auto req = getJson(content.c_str());
    // cout << "解析后：user_name = " << req["user_name"] << ", passwd = " << req["passwd"] << endl;
    vector<vector<string>> sql_query_data;
    string sql = "select * from user where name = \'";
    sql += utf8_to_gbk(req["user_name"]) + "\'";
    printf("sql = %s\n", sql.c_str());
    ret = sqlQuery(sql_query_data, sql.c_str());
    if (ret == -1)
    {
        res_data["msg"] = "query error";
        res_data["root_id"] = "-1";
        res_data["user_id"] = "-1";
    }
    else
    {
        if (sql_query_data.size() == 0)
        {
            res_data["msg"] = "user not found";
            res_data["root_id"] = "-1";
            res_data["user_id"] = "-1";
        }
        else
        {
            if (sql_query_data.at(0).at(2) == req["passwd"]) //成功
            {
                res_data["msg"] = "login success";
                res_data["root_id"] = sql_query_data.at(0).at(3);
                res_data["user_id"] = sql_query_data.at(0).at(0);
                //插入用户登录log
                string sql = "insert into user_login_info(user_id,ipaddr,login_time) values (";
                sql += sql_query_data.at(0).at(0) + ",\'" + clientinfo + "\'," + req["login_time"] + ") ";
                sql += "on duplicate key update ipaddr = \'" + clientinfo + "\',login_time=" + req["login_time"];
                cout << sql << endl;
                ret = sqlChange(sql.c_str());
                if (ret == -1) //出错
                {
                    res_data["msg"] = "success with login error";
                }
            }
            else
            {
                res_data["msg"] = "passwd error";
                res_data["root_id"] = "-1";
                res_data["user_id"] = "-1";
            }
        }
    }

    string res = genJsonResponse(toJson(res_data));
    write(sock, res.c_str(), res.length());

    return 0;
}

int userRegister(int sock, char *buf, int body_start, int total_len, int content_len)
{
    int ret;
    string content = getAllContent(sock, buf, body_start, total_len, content_len);
    map<string, string> res_data;
    auto req = getJson(content.c_str());
    req["user_name"] = utf8_to_gbk(req["user_name"]);

    // 首先，查询数据库是否已经有用户名
    vector<vector<string>> retData;
    string sql = "select * from user where name = \'" + req["user_name"] + "\';";
    printf("sql = %s\n", sql.c_str());
    ret = sqlQuery(retData, sql.c_str());
    if (ret)
    {
        printf("sql query err\n");
        res_data["msg"] = "sql query err";
    }
    else if (retData.size() != 0)
        res_data["msg"] = "user_name already exist";
    else
    {
        string root_id = mkDir(req["user_name"], "-1");
        if (root_id == "")
            res_data["msg"] = "user_name already exist";
        else
        {
            sql = "insert into user(name,passwd,root_id) values(\'";
            sql += req["user_name"] + "\',\'" + req["passwd"] + "\',";
            sql += root_id + ");";
            printf("%s\n", sql.c_str());
            ret = sqlChange(sql.c_str());
            if (ret)
            {
                printf("sql insert err\n");
                res_data["msg"] = "sql insert err";
            }
            else
                res_data["msg"] = "success";
        }
    }

    string retJson = toJson(res_data);
    string res = genJsonResponse(retJson);
    write(sock, res.c_str(), res.length());

    return 0;
}

// int uploadFileNormal(int sock, char *buf, int body_start, int total_len, int content_len)
// {
//     int ret;
//     char *form_content = new char[SLICE_LEN + BUF_SIZE];
//     ret = getAllFormData(sock, buf, body_start, total_len, content_len, form_content);
//     // 处理post请求出错的情况
//     if (ret)
//     {
//         printf("uploadFileNormal post err\n");
//         respErrMsg(sock, "uploadFileNormal post err");
//         //释放
//         delete[] form_content;
//         return -1;
//     }
//     // string content = getAllContent(sock, buf, body_start, total_len, content_len);
//     map<string, string> res_data;
//     map<string, int> req;
//     vector<vector<string>> sql_query_res;

//     //文件内容
//     const char *file_content;
//     // 取得请求内容
//     ret = analyseFormDataBinary(form_content, req, file_content);
//     // printf("upload clen = %lu, ul = %d\n", content.size(), req["uploading_len"]);

//     // cout << "uploading_id: " << form_data["uploading_id"] << endl;
//     // cout << "uploading_len: " << form_data["uploading_len"] << endl;
//     // cout << "file_content: " << file_content << endl;
//     // cout << "get:" << endl
//     //      << content << endl;

//     // 查询待上传文件表信息
//     string sql = "select * from uploading_file where id = ";
//     sql += to_string(req["uploading_id"]) + ";";
//     ret = sqlQuery(sql_query_res, sql.c_str());
//     if (ret)
//     {
//         printf("uploadFileNormal sqlQuery err\n");
//         respErrMsg(sock, "uploadFileNormal sqlQuery err");
//         //释放
//         delete[] form_content;
//         return -1;
//     }

//     string file_path = SKY_REAL_PATH + sql_query_res.at(0).at(3);
//     // int file_pointer = str2int(sql_query_res.at(0).at(5));
//     int file_pointer;
//     int file_len = str2int(sql_query_res.at(0).at(4));

//     FILE *out_file;
//     out_file = fopen(file_path.c_str(), "rb+");
//     if (out_file == NULL)
//     {
//         printf("open %s err\n", file_path.c_str());
//         respErrMsg(sock, "uploadFileNormal openfile err");
//         //释放
//         delete[] form_content;
//         return -1;
//     }
//     // fseek(out_file, file_pointer, SEEK_SET);
//     fseek(out_file, 0, SEEK_END);
//     file_pointer = ftell(out_file);
//     printf("file_pointer = %d\n", file_pointer);
//     ret = fwrite(file_content, sizeof(char), req["uploading_len"], out_file);
//     fclose(out_file);

//     //释放
//     delete[] form_content;

//     printf("file_pointer = %d, ret = %d\n", file_pointer, ret);
//     file_pointer += ret;
//     // 如果文件上传完毕，则维护数据库
//     if (file_pointer >= file_len)
//     {
//         // 删除待上传信息
//         sql = "delete from uploading_file where id = ";
//         sql += to_string(req["uploading_id"]) + ";";
//         ret = sqlChange(sql.c_str());
//         if (ret)
//         {
//             printf("uploadFileNormal sqlChange err\n");
//             respErrMsg(sock, "uploadFileNormal sqlChange err");
//             return -1;
//         }
//         // 插入物理文件表
//         sql = "insert into physic_file(md5,file_size,file_link_cnt) values (\'";
//         sql += sql_query_res.at(0).at(3) + "\', ";
//         sql += sql_query_res.at(0).at(4) + ",1);";
//         ret = sqlChange(sql.c_str());
//         if (ret)
//         {
//             printf("uploadFileNormal sqlChange err\n");
//             respErrMsg(sock, "uploadFileNormal sqlChange err");
//             return -1;
//         }
//         // 插入逻辑文件表
//         ret = createLogicFile(sql_query_res.at(0).at(7),
//                               sql_query_res.at(0).at(3),
//                               sql_query_res.at(0).at(6),
//                               "0");
//         if (ret)
//         {
//             printf("uploadFileNormal sqlChange err\n");
//             respErrMsg(sock, "uploadFileNormal sqlChange err");
//             return -1;
//         }
//         // 如果是随着文件夹上传，还需要维护文件夹信息
//         if (sql_query_res.at(0).at(8) != "-1")
//         {
//             sql = "update uploading_dir set file_now=file_now+1 where id = ";
//             sql += sql_query_res.at(0).at(8);
//             ret = sqlChange(sql.c_str());
//             if (ret)
//             {
//                 printf("uploadFileNormal update uploading_dir err\n");
//                 respErrMsg(sock, "uploadFileNormal update uploading_dir err");
//                 return -1;
//             }
//         }
//     }
//     else
//     {
//         // 否则，更新file_pointer
//         sql = "update uploading_file set file_pointer = ";
//         sql += to_string(file_pointer) + " where id = ";
//         sql += to_string(req["uploading_id"]) + ";";
//         ret = sqlChange(sql.c_str());
//         if (ret)
//         {
//             printf("uploadFileNormal sqlChange err\n");
//             respErrMsg(sock, "uploadFileNormal sqlChange err");
//             return -1;
//         }
//     }

//     res_data["msg"] = "success";
//     res_data["file_pointer"] = to_string(file_pointer);
//     string res = genJsonResponse(toJson(res_data));
//     write(sock, res.c_str(), res.length());

//     return 0;
// }

int uploadFileNormal(int sock, char *buf, int body_start, int total_len, int content_len)
{
    int ret;
    char *form_content = new char[SLICE_LEN + BUF_SIZE];
    ret = getAllFormData(sock, buf, body_start, total_len, content_len, form_content);
    // 处理post请求出错的情况
    if (ret)
    {
        printf("uploadFileNormal post err\n");
        respErrMsg(sock, "uploadFileNormal post err");
        //释放
        delete[] form_content;
        return -1;
    }
    // string content = getAllContent(sock, buf, body_start, total_len, content_len);
    map<string, string> res_data;
    map<string, int> req;
    vector<vector<string>> sql_query_res;

    //文件内容
    const char *file_content;
    // 取得请求内容
    ret = analyseFormDataBinary(form_content, req, file_content);
    // printf("upload clen = %lu, ul = %d\n", content.size(), req["uploading_len"]);

    // cout << "uploading_id: " << form_data["uploading_id"] << endl;
    // cout << "uploading_len: " << form_data["uploading_len"] << endl;
    // cout << "file_content: " << file_content << endl;
    // cout << "get:" << endl
    //      << content << endl;

    // 查询待上传文件表信息
    string sql = "select * from uploading_file where id = ";
    sql += to_string(req["uploading_id"]) + ";";
    ret = sqlQuery(sql_query_res, sql.c_str());
    if (ret)
    {
        printf("uploadFileNormal sqlQuery err\n");
        respErrMsg(sock, "uploadFileNormal sqlQuery err");
        //释放
        delete[] form_content;
        return -1;
    }

    string file_path = SKY_REAL_PATH + sql_query_res.at(0).at(3);
    // int file_pointer = str2int(sql_query_res.at(0).at(5));
    int file_pointer;
    int file_len = str2int(sql_query_res.at(0).at(4));

    FILE *out_file;
    out_file = fopen(file_path.c_str(), "rb+");
    if (out_file == NULL)
    {
        printf("open %s err\n", file_path.c_str());
        respErrMsg(sock, "uploadFileNormal openfile err");
        //释放
        delete[] form_content;
        return -1;
    }
    // fseek(out_file, file_pointer, SEEK_SET);
    fseek(out_file, 0, SEEK_END);
    file_pointer = ftell(out_file);
    printf("file_pointer = %d\n", file_pointer);

    // 如果前端上传的文件指针与后端实际不符，则不存入文件，并且返回正确的文件指针
    if (file_pointer != req["file_pointer"])
    {
        fclose(out_file);
        res_data["msg"] = "success";
        res_data["file_pointer"] = to_string(file_pointer);
        string res = genJsonResponse(toJson(res_data));
        write(sock, res.c_str(), res.length());
        return 0;
    }
    ret = fwrite(file_content, sizeof(char), req["uploading_len"], out_file);
    fclose(out_file);

    //释放
    delete[] form_content;

    printf("file_pointer = %d, ret = %d\n", file_pointer, ret);
    file_pointer += ret;
    // 如果文件上传完毕，则维护数据库
    if (file_pointer >= file_len)
    {
        // 删除待上传信息
        sql = "delete from uploading_file where id = ";
        sql += to_string(req["uploading_id"]) + ";";
        ret = sqlChange(sql.c_str());
        if (ret)
        {
            printf("uploadFileNormal sqlChange err\n");
            respErrMsg(sock, "uploadFileNormal sqlChange err");
            return -1;
        }
        // 插入物理文件表
        sql = "insert into physic_file(md5,file_size,file_link_cnt) values (\'";
        sql += sql_query_res.at(0).at(3) + "\', ";
        sql += sql_query_res.at(0).at(4) + ",1);";
        ret = sqlChange(sql.c_str());
        if (ret)
        {
            printf("uploadFileNormal sqlChange err\n");
            respErrMsg(sock, "uploadFileNormal sqlChange err");
            return -1;
        }
        // 插入逻辑文件表
        ret = createLogicFile(sql_query_res.at(0).at(7),
                              sql_query_res.at(0).at(3),
                              sql_query_res.at(0).at(6),
                              "0");
        if (ret)
        {
            printf("uploadFileNormal sqlChange err\n");
            respErrMsg(sock, "uploadFileNormal sqlChange err");
            return -1;
        }
        // 如果是随着文件夹上传，还需要维护文件夹信息
        if (sql_query_res.at(0).at(8) != "-1")
        {
            sql = "update uploading_dir set file_now=file_now+1 where id = ";
            sql += sql_query_res.at(0).at(8);
            ret = sqlChange(sql.c_str());
            if (ret)
            {
                printf("uploadFileNormal update uploading_dir err\n");
                respErrMsg(sock, "uploadFileNormal update uploading_dir err");
                return -1;
            }
        }
    }
    else
    {
        // 否则，更新file_pointer
        sql = "update uploading_file set file_pointer = ";
        sql += to_string(file_pointer) + " where id = ";
        sql += to_string(req["uploading_id"]) + ";";
        ret = sqlChange(sql.c_str());
        if (ret)
        {
            printf("uploadFileNormal sqlChange err\n");
            respErrMsg(sock, "uploadFileNormal sqlChange err");
            return -1;
        }
    }

    res_data["msg"] = "success";
    res_data["file_pointer"] = to_string(file_pointer);
    string res = genJsonResponse(toJson(res_data));
    write(sock, res.c_str(), res.length());

    return 0;
}

int uploadFileNormalOld(int sock, char *buf, int body_start, int total_len, int content_len)
{
    int ret;
    char *form_content = new char[SLICE_LEN + BUF_SIZE];
    ret = getAllFormData(sock, buf, body_start, total_len, content_len, form_content);
    // 处理post请求出错的情况
    if (ret)
    {
        printf("uploadFileNormal post err\n");
        respErrMsg(sock, "uploadFileNormal post err");
        //释放
        delete[] form_content;
        return -1;
    }
    // string content = getAllContent(sock, buf, body_start, total_len, content_len);
    map<string, string> res_data;
    map<string, int> req;
    vector<vector<string>> sql_query_res;

    //文件内容
    const char *file_content;
    // 取得请求内容
    ret = analyseFormDataBinary(form_content, req, file_content);
    // printf("upload clen = %lu, ul = %d\n", content.size(), req["uploading_len"]);

    // cout << "uploading_id: " << form_data["uploading_id"] << endl;
    // cout << "uploading_len: " << form_data["uploading_len"] << endl;
    // cout << "file_content: " << file_content << endl;
    // cout << "get:" << endl
    //      << content << endl;

    // 查询待上传文件表信息
    string sql = "select * from uploading_file where id = ";
    sql += to_string(req["uploading_id"]) + ";";
    ret = sqlQuery(sql_query_res, sql.c_str());
    if (ret)
    {
        printf("uploadFileNormal sqlQuery err\n");
        respErrMsg(sock, "uploadFileNormal sqlQuery err");
        //释放
        delete[] form_content;
        return -1;
    }

    string file_path = SKY_REAL_PATH + sql_query_res.at(0).at(3);
    int file_pointer = str2int(sql_query_res.at(0).at(5));
    int file_len = str2int(sql_query_res.at(0).at(4));

    FILE *out_file;
    out_file = fopen(file_path.c_str(), "rb+");
    if (out_file == NULL)
    {
        printf("open %s err\n", file_path.c_str());
        respErrMsg(sock, "uploadFileNormal openfile err");
        //释放
        delete[] form_content;
        return -1;
    }
    fseek(out_file, file_pointer, SEEK_SET);
    ret = fwrite(file_content, sizeof(char), req["uploading_len"], out_file);
    fclose(out_file);

    //释放
    delete[] form_content;

    printf("file_pointer = %d, ret = %d\n", file_pointer, ret);
    file_pointer += ret;
    // 如果文件上传完毕，则维护数据库
    if (file_pointer >= file_len)
    {
        // 删除待上传信息
        sql = "delete from uploading_file where id = ";
        sql += to_string(req["uploading_id"]) + ";";
        ret = sqlChange(sql.c_str());
        if (ret)
        {
            printf("uploadFileNormal sqlChange err\n");
            respErrMsg(sock, "uploadFileNormal sqlChange err");
            return -1;
        }
        // 插入物理文件表
        sql = "insert into physic_file(md5,file_size,file_link_cnt) values (\'";
        sql += sql_query_res.at(0).at(3) + "\', ";
        sql += sql_query_res.at(0).at(4) + ",1);";
        ret = sqlChange(sql.c_str());
        if (ret)
        {
            printf("uploadFileNormal sqlChange err\n");
            respErrMsg(sock, "uploadFileNormal sqlChange err");
            return -1;
        }
        // 插入逻辑文件表
        ret = createLogicFile(sql_query_res.at(0).at(7),
                              sql_query_res.at(0).at(3),
                              sql_query_res.at(0).at(6),
                              "0");
        if (ret)
        {
            printf("uploadFileNormal sqlChange err\n");
            respErrMsg(sock, "uploadFileNormal sqlChange err");
            return -1;
        }
        // 如果是随着文件夹上传，还需要维护文件夹信息
        if (sql_query_res.at(0).at(8) != "-1")
        {
            sql = "update uploading_dir set file_now=file_now+1 where id = ";
            sql += sql_query_res.at(0).at(8);
            ret = sqlChange(sql.c_str());
            if (ret)
            {
                printf("uploadFileNormal update uploading_dir err\n");
                respErrMsg(sock, "uploadFileNormal update uploading_dir err");
                return -1;
            }
        }
    }
    else
    {
        // 否则，更新file_pointer
        sql = "update uploading_file set file_pointer = ";
        sql += to_string(file_pointer) + " where id = ";
        sql += to_string(req["uploading_id"]) + ";";
        ret = sqlChange(sql.c_str());
        if (ret)
        {
            printf("uploadFileNormal sqlChange err\n");
            respErrMsg(sock, "uploadFileNormal sqlChange err");
            return -1;
        }
    }

    res_data["msg"] = "success";
    res_data["file_pointer"] = to_string(file_pointer);
    string res = genJsonResponse(toJson(res_data));
    write(sock, res.c_str(), res.length());

    return 0;
}

int uploadMkdir(int sock, char *buf, int body_start, int total_len, int content_len)
{
    string content = getAllContent(sock, buf, body_start, total_len, content_len);
    map<string, string> res_data;

    // 解析json参数
    auto req = getJson(content.c_str());
    res_data["id"] = mkDir(utf8_to_gbk(req["file_name"]), req["parent_id"]);
    if (res_data["id"] == "")
        res_data["msg"] = "mkdir err";
    else
    {
        res_data["msg"] = "success";
        if (req.count("uploading_id")) //如果有这个参数
        {
            int ret;
            string sql = "update uploading_dir set file_now=file_now+1 where id = ";
            sql += req["uploading_id"];
            ret = sqlChange(sql.c_str());
            if (ret)
            {
                res_data["msg"] = "update file_now error";
            }
        }
    }

    string res = genJsonResponse(toJson(res_data));
    write(sock, res.c_str(), res.length());

    return 0;
}

int checkFileExist(int sock, char *buf, int body_start, int total_len, int content_len)
{
    int ret;
    string content = getAllContent(sock, buf, body_start, total_len, content_len);
    map<string, string> res_data;

    // 解析json参数
    auto req = getJson(content.c_str());
    vector<vector<string>> retData;
    string sql = "select md5 from physic_file where md5 = \'" + req["md5"] + "\';";
    printf("sql = %s\n", sql.c_str());
    ret = sqlQuery(retData, sql.c_str());
    if (ret == -1)
    {
        printf("sql query err\n");
        respErrMsg(sock, "checkFileExist sqlQuery err");
        return -1;
    }

    if (retData.size() == 0)
        res_data["is_exist"] = "0";
    else
        res_data["is_exist"] = "1";

    string retJson = toJson(res_data);
    string res = genJsonResponse(retJson);
    write(sock, res.c_str(), res.length());

    return 0;
}

int uploadFileFast(int sock, char *buf, int body_start, int total_len, int content_len)
{
    // 调用此接口以前，已经确定可以秒传了
    int ret;
    string content = getAllContent(sock, buf, body_start, total_len, content_len);
    map<string, string> res_data;
    vector<vector<string>> sql_query_res;

    // 解析json参数
    auto req = getJson(content.c_str());
    // 插入逻辑文件表
    ret = createLogicFile(utf8_to_gbk(req["file_name"]),
                          req["md5"],
                          req["parent_id"],
                          "0");
    if (ret)
    {
        printf("uploadFileFast sqlChange err\n");
        respErrMsg(sock, "uploadFileFast sqlChange err");
        return -1;
    }
    // 维护link_cnt
    string sql = "update physic_file set file_link_cnt=file_link_cnt+1 where md5 = \'";
    sql += req["md5"] + "\';";
    ret = sqlChange(sql.c_str());
    if (ret)
    {
        printf("uploadFileFast update physic_file err\n");
        respErrMsg(sock, "uploadFileFast update physic_file err");
        return -1;
    }

    res_data["msg"] = "success";
    string res = genJsonResponse(toJson(res_data));
    write(sock, res.c_str(), res.length());

    return 0;
}

int uploadFileFirst(int sock, char *buf, int body_start, int total_len, int content_len)
{
    int ret;
    string content = getAllContent(sock, buf, body_start, total_len, content_len);
    map<string, string> res_data;
    vector<vector<string>> sql_query_res;

    // 解析json参数
    string sql;
    auto req = getJson(content.c_str());
    req["file_real_path"] = utf8_to_gbk(req["file_real_path"]);
    req["file_name"] = utf8_to_gbk(req["file_name"]);

    // 对空文件的特判
    if (req["file_len"] == "0")
    {
        // 插入物理文件表
        sql = "insert into physic_file(md5,file_size,file_link_cnt) values (\'";
        sql += req["md5"] + "\',0,1);";
        ret = sqlChange(sql.c_str());
        if (ret)
        {
            printf("uploadFileFirst sqlChange err\n");
            respErrMsg(sock, "uploadFileFirst sqlChange err");
            return -1;
        }
        // 插入逻辑文件表
        ret = createLogicFile(req["file_name"],
                              req["md5"],
                              req["parent_id"],
                              "0");
        if (ret)
        {
            printf("uploadFileFirst sqlChange err\n");
            respErrMsg(sock, "uploadFileFirst sqlChange err");
            return -1;
        }
        // 如果是随着文件夹上传，还需要维护文件夹信息
        if (req["uploading_dir_id"] != "-1")
        {
            sql = "update uploading_dir set file_now=file_now+1 where id = ";
            sql += req["uploading_dir_id"];
            ret = sqlChange(sql.c_str());
            if (ret)
            {
                printf("uploadFileFirst update uploading_dir err\n");
                respErrMsg(sock, "uploadFileFirst update uploading_dir err");
                return -1;
            }
        }
        // 创建文件
        string file_path = SKY_REAL_PATH;
        file_path += req["md5"];
        fclose(fopen(file_path.c_str(), "w"));

        res_data["msg"] = "success";
        res_data["file_pointer"] = "0";
        res_data["uploading_id"] = "0";
        res_data["slice_len"] = to_string(SLICE_LEN);
        string res = genJsonResponse(toJson(res_data));
        write(sock, res.c_str(), res.length());
        return 0;
    }
    // 查询待上传文件表，如果已经存在，则返回filepointer，否则新建
    sql = "select * from uploading_file where u_id = ";
    sql += req["u_id"] + " and file_real_path = \'";
    sql += req["file_real_path"] + "\' and uploading_dir_id = ";
    sql += req["uploading_dir_id"] + ";";
    ret = sqlQuery(sql_query_res, sql.c_str());
    if (ret)
    {
        printf("uploadFileFirst sqlQuery err\n");
        respErrMsg(sock, "uploadFileFirst select * from uploading_file err");
        return -1;
    }

    if (sql_query_res.size() == 0)
    {
        // 新建
        // 插入待上传文件表
        sql = "insert into uploading_file(u_id,file_real_path,md5,file_len,file_pointer,parent_id,file_name,uploading_dir_id) values(";
        sql += req["u_id"] + ",\'";
        sql += req["file_real_path"] + "\',\'";
        sql += req["md5"] + "\',";
        sql += req["file_len"] + ",";
        sql += "0,";
        sql += req["parent_id"] + ",\'";
        sql += req["file_name"] + "\',";
        sql += req["uploading_dir_id"] + ");";
        ret = sqlChange(sql.c_str());
        if (ret)
        {
            printf("uploadFileFirst sqlChange err\n");
            respErrMsg(sock, "uploadFileFirst insert into uploading_file err");
            return -1;
        }
        // 取刚刚上传的那条表项的id
        sql = "select * from uploading_file where u_id = ";
        sql += req["u_id"] + " and file_real_path = \'";
        sql += req["file_real_path"] + "\';";
        ret = sqlQuery(sql_query_res, sql.c_str());
        if (ret)
        {
            printf("uploadFileFirst sqlQuery err\n");
            respErrMsg(sock, "uploadFileFirst err");
            return -1;
        }
        // 创建文件
        string file_path = SKY_REAL_PATH;
        file_path += req["md5"];
        fclose(fopen(file_path.c_str(), "w"));
    }

    res_data["msg"] = "success";
    res_data["file_pointer"] = sql_query_res.at(0).at(5);
    res_data["uploading_id"] = sql_query_res.at(0).at(0);
    res_data["slice_len"] = to_string(SLICE_LEN);
    string res = genJsonResponse(toJson(res_data));
    write(sock, res.c_str(), res.length());

    return 0;
}

int uploadDirFirst(int sock, char *buf, int body_start, int total_len, int content_len)
{
    int ret;
    string content = getAllContent(sock, buf, body_start, total_len, content_len);
    map<string, string> res_data;
    vector<vector<string>> sql_query_res;

    // 解析json参数
    auto req = getJson(content.c_str());
    req["file_real_path"] = utf8_to_gbk(req["file_real_path"]);
    req["dir_name"] = utf8_to_gbk(req["dir_name"]);

    // 检查文件夹是否已经在上传，如果是则直接返回
    string sql = "select * from uploading_dir where u_id = ";
    sql += req["u_id"] + " and file_real_path = \'";
    sql += req["file_real_path"] + "\';";
    ret = sqlQuery(sql_query_res, sql.c_str());
    if (ret)
    {
        printf("uploadDirFirst sqlQuery err\n");
        respErrMsg(sock, "uploadDirFirst err");
        return -1;
    }

    if (sql_query_res.size() > 0)
    {
        res_data["msg"] = "success";
        res_data["uploading_id"] = sql_query_res.at(0).at(0);
        res_data["logic_id"] = sql_query_res.at(0).at(4);
        res_data["file_now"] = sql_query_res.at(0).at(5);
        string res = genJsonResponse(toJson(res_data));
        write(sock, res.c_str(), res.length());
        return 0;
    }
    // 否则，需要新建待上传文件夹表项
    // 首先在逻辑文件表插入该文件夹信息
    sql = "insert into logic_file(file_name,md5,parent_id,is_dir) values(\'";
    sql += req["dir_name"] + "\',\'" + req["md5"] + "\',";
    sql += req["parent_id"] + ",1);";
    ret = sqlChange(sql.c_str());
    if (ret)
    {
        printf("uploadDirFirst sqlChange err\n");
        respErrMsg(sock, "uploadDirFirst err");
        return -1;
    }
    // 查出刚刚插入的文件夹id
    sql = "select id from logic_file where file_name = \'";
    sql += req["dir_name"] + "\' and parent_id = " + req["parent_id"];
    ret = sqlQuery(sql_query_res, sql.c_str());
    if (ret || sql_query_res.size() == 0)
    {
        printf("uploadDirFirst sqlQuery err\n");
        respErrMsg(sock, "uploadDirFirst err");
        return -1;
    }
    string logic_id = sql_query_res.at(0).at(0);
    // 插入待上传文件夹表
    sql = "insert into uploading_dir(u_id,file_real_path,md5,logic_id,file_now,file_count) values(";
    sql += req["u_id"] + ",\'" + req["file_real_path"] + "\',\'";
    sql += req["md5"] + "\'," + logic_id + ",0,";
    sql += req["file_count"] + ");";
    ret = sqlChange(sql.c_str());
    if (ret)
    {
        printf("uploadDirFirst sqlChange err\n");
        respErrMsg(sock, "uploadDirFirst err");
        return -1;
    }
    // 查出刚刚插入的待上传id
    sql = "select id from uploading_dir where u_id = ";
    sql += req["u_id"] + " and file_real_path = \'";
    sql += req["file_real_path"] + "\';";
    ret = sqlQuery(sql_query_res, sql.c_str());
    if (ret || sql_query_res.size() == 0)
    {
        printf("uploadDirFirst sqlQuery err\n");
        respErrMsg(sock, "uploadDirFirst err");
        return -1;
    }

    res_data["msg"] = "success";
    res_data["uploading_id"] = sql_query_res.at(0).at(0);
    res_data["logic_id"] = logic_id;
    res_data["file_now"] = to_string(0);
    string res = genJsonResponse(toJson(res_data));
    write(sock, res.c_str(), res.length());

    return 0;
}

int uploadDirOver(int sock, char *buf, int body_start, int total_len, int content_len)
{
    int ret;
    string content = getAllContent(sock, buf, body_start, total_len, content_len);
    map<string, string> res_data;

    auto req = getJson(content.c_str());
    //删除表项
    string sql = "delete from uploading_dir where id = ";
    sql += req["uploading_id"];
    cout << sql << endl;
    ret = sqlChange(sql.c_str());
    if (ret)
        res_data["msg"] = "del uploading dir error!";
    else
        res_data["msg"] = "success";

    string res = genJsonResponse(toJson(res_data));
    write(sock, res.c_str(), res.length());
    return 0;
}

int getAllFileUnderDir(int sock, char *buf, int body_start, int total_len, int content_len)
{
    int ret;
    string content = getAllContent(sock, buf, body_start, total_len, content_len);
    vector<map<string, string>> res_data;

    // 解析json参数
    auto req = getJson(content.c_str());
    vector<vector<string>> retData;
    string sql = "select * from logic_file where parent_id = " + req["file_node"] + ";";

    printf("sql = %s\n", sql.c_str());

    ret = sqlQuery(retData, sql.c_str());
    if (ret == -1)
    {
        printf("sql query err\n");
        return -1;
    }

    for (unsigned int i = 0; i < retData.size(); i++)
    {
        map<string, string> nowRow;
        nowRow["id"] = retData.at(i).at(0);
        nowRow["file_name"] = gbk_to_utf8(retData.at(i).at(1));
        nowRow["md5"] = retData.at(i).at(2);
        nowRow["parent_id"] = retData.at(i).at(3);
        nowRow["is_dir"] = retData.at(i).at(4);
        res_data.push_back(nowRow);
    }

    string retJson = toJsonList(res_data);
    string res = genJsonResponse(retJson);

    write(sock, res.c_str(), res.length());

    return 0;
}

int changeName(int sock, char *buf, int body_start, int total_len, int content_len)
{
    int ret;
    string content = getAllContent(sock, buf, body_start, total_len, content_len);
    map<string, string> res_data;

    // 解析json参数
    auto req = getJson(content.c_str());
    // vector<vector<string>> retData;
    string sql = "update logic_file set file_name = \'";
    sql += utf8_to_gbk(req["newfilename"]) + "\' where id = " + req["id"] + ";";
    printf("sql = %s\n", sql.c_str());
    // ret = sqlQuery(retData, sql.c_str());
    ret = sqlChange(sql.c_str());

    //记得commit！！！！！！！！！！

    if (ret == -1)
    {
        res_data["msg"] = "error";
    }
    else
    {
        res_data["msg"] = "success";
    }

    string retJson = toJson(res_data);
    string res = genJsonResponse(retJson);
    write(sock, res.c_str(), res.length());

    return 0;
}

int copyFile(int sock, char *buf, int body_start, int total_len, int content_len)
{
    int ret;
    string content = getAllContent(sock, buf, body_start, total_len, content_len);
    map<string, string> res_data;

    // 解析json参数
    auto req = getJson(content.c_str());
    vector<vector<string>> retData;

    //先在数据库里查询这个节点
    string sql = "select * from logic_file where id = ";
    sql += req["id"];
    cout << sql << endl;
    ret = sqlQuery(retData, sql.c_str());
    if (retData.size() == 0 || ret == -1) //出错
    {
        res_data["msg"] = "query error";
        string res = genJsonResponse(toJson(res_data));
        write(sock, res.c_str(), res.length());
        return 0;
    }

    //插入新的节点
    sql = "insert into logic_file(file_name,md5,parent_id,is_dir) values (\'";
    sql += retData.at(0).at(1) + "\',\'" + retData.at(0).at(2) + "\'," + req["new_parent_id"] + ",0)";
    printf("sql = %s\n", sql.c_str());
    // ret = sqlQuery(retData, sql.c_str());
    ret = sqlChange(sql.c_str());
    if (retData.size() == 0 || ret == -1) //出错
    {
        res_data["msg"] = "insert error";
        string res = genJsonResponse(toJson(res_data));
        write(sock, res.c_str(), res.length());
        return 0;
    }
    //记得commit！！！！！！！！！！
    //维护link
    sql = "update physic_file set file_link_cnt=file_link_cnt+1 where md5 = \'";
    sql += retData.at(0).at(2) + "\';";
    ret = sqlChange(sql.c_str());

    if (ret == -1)
    {
        res_data["msg"] = "error";
    }
    else
    {
        res_data["msg"] = "success";
    }

    string retJson = toJson(res_data);
    string res = genJsonResponse(retJson);
    write(sock, res.c_str(), res.length());

    return 0;
}

// DFS遍历拷贝
int recursion_copy(string parent_id, string new_parent_id)
{
    //查询所有以此父节点为父节点的文件/文件夹
    int ret;
    vector<vector<string>> retData;
    string sql = "select * from logic_file where parent_id = ";
    sql += parent_id;
    cout << sql << endl;
    ret = sqlQuery(retData, sql.c_str());
    if (ret == -1)
        return -1;
    if (retData.size() == 0) //没了
        return 0;
    for (unsigned int i = 0; i < retData.size(); i++)
    {
        //插入一个新的文件/文件夹
        sql = "insert into logic_file (file_name,md5,parent_id,is_dir) values (\'";
        sql += retData.at(i).at(1) + "\',\'" + retData.at(i).at(2) + "\'," + new_parent_id + "," + retData.at(i).at(4) + ")";
        cout << sql << endl;
        ret = sqlChange(sql.c_str());
        if (ret == -1) //出错
            return -1;
        if (retData.at(i).at(4) == "1") //是文件夹
        {
            //查询这个新的文件夹的id
            vector<vector<string>> retData0;
            string parent_id0 = retData.at(i).at(0);
            sql = "select * from logic_file where file_name = \'";
            sql += retData.at(i).at(1) + "\' and parent_id = " + new_parent_id;
            cout << sql << endl;
            ret = sqlQuery(retData0, sql.c_str());
            if (retData0.size() == 0 || ret == -1) //出错
            {
                return -1;
            }
            string new_parent_id0 = retData0.at(0).at(0);
            ret = recursion_copy(parent_id0, new_parent_id0);
            if (ret == -1)
                return -1;
        }
        else //是文件
        {
            //维护link
            sql = "update physic_file set file_link_cnt=file_link_cnt+1 where md5 = \'";
            sql += retData.at(i).at(2) + "\';";
            ret = sqlChange(sql.c_str());
            if (ret == -1)
                return -1;
        }
    }
    return 0;
}
int copyDir(int sock, char *buf, int body_start, int total_len, int content_len)
{
    int ret;
    string content = getAllContent(sock, buf, body_start, total_len, content_len);
    map<string, string> res_data;

    auto req = getJson(content.c_str());

    vector<vector<string>> retData;
    //先在数据库里查询这个节点
    string sql = "select * from logic_file where id = ";
    sql += req["id"];
    cout << sql << endl;
    ret = sqlQuery(retData, sql.c_str());
    if (retData.size() == 0 || ret == -1) //出错
    {
        res_data["msg"] = "error";
        string res = genJsonResponse(toJson(res_data));
        write(sock, res.c_str(), res.length());
        return 0;
    }

    //插入一个新的文件夹
    sql = "insert into logic_file (file_name,parent_id,is_dir) values (\'";
    sql += retData.at(0).at(1) + "\'," + req["new_parent_id"] + ",1)";
    cout << sql << endl;
    ret = sqlChange(sql.c_str());
    if (ret == -1) //出错
    {
        res_data["msg"] = "error";
        string res = genJsonResponse(toJson(res_data));
        write(sock, res.c_str(), res.length());
        return 0;
    }
    //查询这个新的文件夹的id
    sql = "select * from logic_file where file_name = \'";
    sql += retData.at(0).at(1) + "\' and parent_id = " + req["new_parent_id"];
    cout << sql << endl;
    ret = sqlQuery(retData, sql.c_str());
    if (retData.size() == 0) //出错
    {
        res_data["msg"] = "error";
        string res = genJsonResponse(toJson(res_data));
        write(sock, res.c_str(), res.length());
        return 0;
    }
    string new_father_id = retData.at(0).at(0);
    cout << "new id" << new_father_id << endl;
    // DFS
    ret = recursion_copy(req["id"], new_father_id);

    if (ret == -1) //出错
    {
        res_data["msg"] = "error";
        string res = genJsonResponse(toJson(res_data));
        write(sock, res.c_str(), res.length());
        return 0;
    }
    else
    {
        res_data["msg"] = "success";
        string res = genJsonResponse(toJson(res_data));
        write(sock, res.c_str(), res.length());
        return 0;
    }
}

int moveFile(int sock, char *buf, int body_start, int total_len, int content_len)
{
    int ret;
    string content = getAllContent(sock, buf, body_start, total_len, content_len);
    map<string, string> res_data;

    // 解析json参数
    auto req = getJson(content.c_str());
    // vector<vector<string>> retData;
    string sql = "update logic_file set parent_id = \'";
    sql += req["new_parent_id"] + "\' where id = " + req["id"] + ";";
    printf("sql = %s\n", sql.c_str());
    // ret = sqlQuery(retData, sql.c_str());
    ret = sqlChange(sql.c_str());

    if (ret)
        res_data["msg"] = "error";
    else
        res_data["msg"] = "success";

    string res = genJsonResponse(toJson(res_data));
    write(sock, res.c_str(), res.length());

    return 0;
}

// DFS遍历删除
int recursion_del(string parent_id)
{
    //查询所有以此父节点为父节点的文件/文件夹
    int ret;
    vector<vector<string>> retData;
    string sql = "select * from logic_file where parent_id = ";
    sql += parent_id;
    cout << sql << endl;
    ret = sqlQuery(retData, sql.c_str());
    if (ret == -1)
        return -1;
    if (retData.size() == 0) //没了
        return 0;
    for (unsigned int i = 0; i < retData.size(); i++)
    {
        //删除一个文件/文件夹
        sql = "delete from logic_file where id = " + retData.at(i).at(0) + ";";
        cout << sql << endl;
        ret = sqlChange(sql.c_str());
        if (ret == -1) //出错
            return -1;
        if (retData.at(i).at(4) == "1") //是文件夹
        {
            ret = recursion_del(retData.at(i).at(0));
            if (ret == -1)
                return -1;
        }
        else //是文件
        {
            //维护link
            sql = "update physic_file set file_link_cnt=file_link_cnt-1 where md5 = \'";
            sql += retData.at(i).at(2) + "\';";
            ret = sqlChange(sql.c_str());
            if (ret == -1)
                return -1;
        }
    }
    return 0;
}

int delFile(int sock, char *buf, int body_start, int total_len, int content_len)
{
    int ret;
    string content = getAllContent(sock, buf, body_start, total_len, content_len);
    map<string, string> res_data;
    vector<vector<string>> retData;
    // 解析json参数
    auto req = getJson(content.c_str());

    //先在数据库里查询这个节点
    string sql = "select * from logic_file where id = ";
    sql += req["id"];
    cout << sql << endl;
    ret = sqlQuery(retData, sql.c_str());
    if (retData.size() == 0 || ret == -1) //出错
    {
        res_data["msg"] = "error";
        string res = genJsonResponse(toJson(res_data));
        write(sock, res.c_str(), res.length());
        return 0;
    }

    sql = "delete from logic_file where id = " + req["id"] + ";";
    printf("sql = %s\n", sql.c_str());
    ret = sqlChange(sql.c_str());
    if (ret == -1) //出错
    {
        res_data["msg"] = "error";
        string res = genJsonResponse(toJson(res_data));
        write(sock, res.c_str(), res.length());
        return 0;
    }

    if (retData.at(0).at(4) == "0") //是文件
    {
        //维护link
        sql = "update physic_file set file_link_cnt=file_link_cnt-1 where md5 = \'";
        sql += retData.at(0).at(2) + "\';";
        ret = sqlChange(sql.c_str());
        if (ret == -1)
            res_data["msg"] = "link error";
        else
            res_data["msg"] = "success";
        string res = genJsonResponse(toJson(res_data));
        write(sock, res.c_str(), res.length());
        return 0;
    }
    else
    {
        // DFS删除
        ret = recursion_del(retData.at(0).at(0));
        if (ret == -1)
            res_data["msg"] = "error";
        else
            res_data["msg"] = "success";

        string res = genJsonResponse(toJson(res_data));
        write(sock, res.c_str(), res.length());

        return 0;
    }
}

int downloadNormalFile(int sock, char *buf, int body_start, int total_len, int content_len)
{
    int ret;
    string content = getAllContent(sock, buf, body_start, total_len, content_len);
    char *download_content = new char[SLICE_LEN + 1];

    // 解析json参数
    auto req = getJson(content.c_str());
    vector<vector<string>> retData;

    string &downloading_id = req["downloading_id"];
    int file_pointer = str2int(req["file_pointer"]);

    // update downloading_file file_now
    string sql_downloading_dir = "update downloading_file set file_pointer = " + req["file_pointer"] + " where id = " + downloading_id + ";";
    printf("sql = %s\n", sql_downloading_dir.c_str());
    // ret = sqlQuery(retData, sql.c_str());
    ret = sqlChange(sql_downloading_dir.c_str());

    // find downloading_file, get logic_id
    string sql_find_path = "select * from downloading_file where id = " + downloading_id + ";";
    printf("sql = %s\n", sql_find_path.c_str());
    retData.clear();
    ret = sqlQuery(retData, sql_find_path.c_str());
    string logic_id = retData.at(0).at(2);

    // find logic_file, get md5
    string sql_logic_file = "select * from logic_file where id = " + logic_id + ";";
    printf("sql = %s\n", sql_logic_file.c_str());
    retData.clear();
    ret = sqlQuery(retData, sql_logic_file.c_str());

    string file_name = SKY_REAL_PATH;
    file_name += retData.at(0).at(2);

    cout << "filename: " << file_name << endl;

    FILE *file_in = fopen(file_name.c_str(), "rb");
    if (file_in == NULL)
    {
        cout << "file open err" << endl;
        getBinaryResponseHead(buf, 0);
        write(sock, buf, strlen(buf));
        delete[] download_content;
        return -1;
    }

    ret = fseek(file_in, file_pointer, SEEK_SET);
    if (ret != 0)
    {
        cout << "seek error" << endl;
        getBinaryResponseHead(buf, 0);
        write(sock, buf, strlen(buf));
        delete[] download_content;
        return -1;
    }

    ret = fread(download_content, sizeof(char), SLICE_LEN, file_in);
    // cout << "read: " << download_content << ", totally " << ret << "bytes" << endl;
    cout << "read: " << ret << "bytes" << endl;

    fclose(file_in);

    getBinaryResponseHead(buf, ret);
    write(sock, buf, strlen(buf));
    ret = write(sock, download_content, ret);
    cout << "write: " << ret << "bytes" << endl;

    delete[] download_content;

    return 0;
}

int downloadFileFirst(int sock, char *buf, int body_start, int total_len, int content_len)
{
    int ret;
    string content = getAllContent(sock, buf, body_start, total_len, content_len);
    map<string, string> res_data;
    vector<vector<string>> retData;

    // 解析json参数
    auto req = getJson(content.c_str());

    // find downloading_file, get downloading_id
    string sql_find_id = "select * from downloading_file where u_id = " + req["u_id"] + " and logic_id = " + req["logic_id"] + ";";
    printf("sql = %s\n", sql_find_id.c_str());
    // retData.clear();
    ret = sqlQuery(retData, sql_find_id.c_str());

    int flag = 1;

    if (retData.size() != 0)
    {
        cout << "downloading file id already exist, continue..." << endl;
        flag = 0;
    }

    // find logic_file, get md5
    string sql_logic_file = "select * from logic_file where id = " + req["logic_id"] + ";";
    printf("sql = %s\n", sql_logic_file.c_str());
    ret = sqlQuery(retData, sql_logic_file.c_str());

    // find physic_file, get file_size
    string sql_physic_file = "select * from physic_file where md5 = \'" + retData.at(0).at(2) + "\';";
    printf("sql = %s\n", sql_physic_file.c_str());
    retData.clear();
    ret = sqlQuery(retData, sql_physic_file.c_str());

    // save file_len, down there we need this.
    string file_len = retData.at(0).at(2);

    if (flag == 1)
    {
        // insert downloading_file a new row
        string sql_downloading_file = "insert into downloading_file(u_id, logic_id, file_len, file_real_path, downloading_dir_id, file_pointer) values (";
        sql_downloading_file += req["u_id"] + ", " + req["logic_id"] + ", " + retData.at(0).at(2) + ", \'" + utf8_to_gbk(req["file_real_path"]) + "\', " + req["downloading_dir_id"] + ", 0);";
        printf("sql = %s\n", sql_downloading_file.c_str());
        // ret = sqlQuery(retData, sql.c_str());
        ret = sqlChange(sql_downloading_file.c_str());
    }

    // find downloading_file, get downloading_id
    string sql_find_id2 = "select * from downloading_file where u_id = " + req["u_id"] + " and logic_id = " + req["logic_id"] + ";";
    printf("sql = %s\n", sql_find_id2.c_str());
    // retData.clear();
    ret = sqlQuery(retData, sql_find_id2.c_str());

    if (ret == -1)
    {
        res_data["msg"] = "error";
        res_data["downloading_id"] = "-1";
        res_data["file_len"] = "-1";
        res_data["slice_len"] = to_string(SLICE_LEN);
        res_data["file_pointer"] = "-1";
    }
    else
    {
        res_data["msg"] = "success";
        res_data["downloading_id"] = retData.at(0).at(0);
        res_data["file_len"] = file_len;
        res_data["slice_len"] = to_string(SLICE_LEN);
        res_data["file_pointer"] = retData.at(0).at(6);
    }

    string retJson = toJson(res_data);
    string res = genJsonResponse(retJson);
    write(sock, res.c_str(), res.length());

    return 0;
}

int getFileCount(string downloading_dir_id, int &cnt)
{
    int ret;
    vector<vector<string>> retData;
    vector<string> dir_list;
    //计算文件夹下的文件数量
    string sql = "select * from logic_file where parent_id = " + downloading_dir_id + ";";
    printf("sql = %s\n", sql.c_str());
    ret = sqlQuery(retData, sql.c_str());

    for (unsigned int i = 0; i < retData.size(); i++)
    {
        //文件夹
        if (retData.at(i).at(4) == "1")
            dir_list.push_back(retData.at(i).at(0));
        else
            cnt++;
    }
    for (unsigned int k = 0; k < dir_list.size(); k++)
    {
        getFileCount(dir_list[k], cnt);
    }
    return ret;
}

int downloadDirFirst(int sock, char *buf, int body_start, int total_len, int content_len)
{
    int ret;
    string content = getAllContent(sock, buf, body_start, total_len, content_len);
    map<string, string> res_data;

    // 解析json参数
    auto req = getJson(content.c_str());
    vector<vector<string>> retData;

    // find downloading_dir
    string sql_find_dir = "select * from downloading_dir where u_id = " + req["u_id"] + " and logic_id = " + req["logic_id"] + ";";
    printf("sql = %s\n", sql_find_dir.c_str());
    ret = sqlQuery(retData, sql_find_dir.c_str());

    //首次下载
    if (retData.size() == 0)
    {
        //获取文件数量
        int dir_file_cnt = 0;
        getFileCount(req["logic_id"], dir_file_cnt);

        // insert downloading_dir a new row
        string sql_downloading_dir = "insert into downloading_dir(u_id, file_real_path, logic_id, file_now, file_count) values (";
        sql_downloading_dir += req["u_id"] + ", \'" + utf8_to_gbk(req["file_real_path"]) + "\', " + req["logic_id"] + ", 0, " + to_string(dir_file_cnt) + ");";
        printf("sql = %s\n", sql_downloading_dir.c_str());
        // ret = sqlQuery(retData, sql.c_str());
        ret = sqlChange(sql_downloading_dir.c_str());

        // find downloading_dir, get downloading_id
        string sql_find_id = "select * from downloading_dir where u_id = " + req["u_id"] + " and logic_id = " + req["logic_id"] + ";";
        printf("sql = %s\n", sql_find_id.c_str());
        ret = sqlQuery(retData, sql_find_id.c_str());
    }

    if (ret == -1)
    {
        res_data["msg"] = "error";
        res_data["downloading_id"] = "-1";
        res_data["file_now"] = "-1";
        res_data["file_count"] = "-1";
    }
    else
    {
        res_data["msg"] = "success";
        res_data["downloading_id"] = retData.at(0).at(0);
        res_data["file_now"] = retData.at(0).at(5);
        res_data["file_count"] = retData.at(0).at(6);
    }

    string retJson = toJson(res_data);
    string res = genJsonResponse(retJson);
    write(sock, res.c_str(), res.length());

    return 0;
}

int downloadingFileList(int sock, char *buf, int body_start, int total_len, int content_len)
{
    int ret;
    string content = getAllContent(sock, buf, body_start, total_len, content_len);
    vector<map<string, string>> res_data;

    // 解析json参数
    auto req = getJson(content.c_str());
    vector<vector<string>> retData;

    // find downloading_file, get downloading_file_list
    string sql_find_list = "select * from downloading_file where u_id = " + req["u_id"] + ";";
    printf("sql = %s\n", sql_find_list.c_str());
    ret = sqlQuery(retData, sql_find_list.c_str());
    cout << "sql return " << ret << endl;
    // cout << "user has " << retData.size() << "downloading files!" << endl;

    for (unsigned int i = 0; i < retData.size(); i++)
    {
        map<string, string> nowRow;
        nowRow["id"] = retData.at(i).at(0);
        nowRow["logic_id"] = retData.at(i).at(2);
        nowRow["file_len"] = retData.at(i).at(3);
        nowRow["file_real_path"] = gbk_to_utf8(retData.at(i).at(4));
        nowRow["downloading_dir_id"] = retData.at(i).at(5);
        nowRow["file_pointer"] = retData.at(i).at(6);

        // find logic_file, get all info
        vector<vector<string>> logicData;
        string sql_logic_file = "select * from logic_file where id = " + nowRow["logic_id"] + ";";
        printf("sql = %s\n", sql_logic_file.c_str());
        ret = sqlQuery(logicData, sql_logic_file.c_str());

        nowRow["file_name"] = gbk_to_utf8(logicData.at(0).at(1));
        nowRow["md5"] = logicData.at(0).at(2);
        nowRow["parent_id"] = logicData.at(0).at(3);
        nowRow["is_dir"] = logicData.at(0).at(4);

        res_data.push_back(nowRow);
    }

    string retJson = toJsonList(res_data);
    string res = genJsonResponse(retJson);

    write(sock, res.c_str(), res.length());

    return 0;
}

int downloadingDirList(int sock, char *buf, int body_start, int total_len, int content_len)
{
    int ret;
    string content = getAllContent(sock, buf, body_start, total_len, content_len);
    vector<map<string, string>> res_data;

    // 解析json参数
    auto req = getJson(content.c_str());
    vector<vector<string>> retData;

    // find downloading_dir, get downloading_dir_list
    string sql_find_list = "select * from downloading_dir where u_id = " + req["u_id"] + ";";
    printf("sql = %s\n", sql_find_list.c_str());
    ret = sqlQuery(retData, sql_find_list.c_str());
    cout << "sql return " << ret << endl;
    // cout << "user has " << retData.size() << "downloading dirs!" << endl;

    for (unsigned int i = 0; i < retData.size(); i++)
    {
        map<string, string> nowRow;
        nowRow["id"] = retData.at(i).at(0);
        nowRow["file_real_path"] = gbk_to_utf8(retData.at(i).at(2));
        // nowRow["md5"] = retData.at(i).at(3);
        nowRow["logic_id"] = retData.at(i).at(4);
        nowRow["file_now"] = retData.at(i).at(5);
        nowRow["file_count"] = retData.at(i).at(6);

        // find logic_file, get all info
        vector<vector<string>> logicData;
        string sql_logic_file = "select * from logic_file where id = " + nowRow["logic_id"] + ";";
        printf("sql = %s\n", sql_logic_file.c_str());
        ret = sqlQuery(logicData, sql_logic_file.c_str());

        nowRow["file_name"] = gbk_to_utf8(logicData.at(0).at(1));
        // nowRow["md5"] = logicData.at(0).at(2);
        nowRow["parent_id"] = logicData.at(0).at(3);
        nowRow["is_dir"] = logicData.at(0).at(4);

        res_data.push_back(nowRow);
    }

    string retJson = toJsonList(res_data);
    string res = genJsonResponse(retJson);

    write(sock, res.c_str(), res.length());

    return 0;
}

int uploadingFileList(int sock, char *buf, int body_start, int total_len, int content_len)
{
    int ret;
    string content = getAllContent(sock, buf, body_start, total_len, content_len);
    vector<map<string, string>> res_data;

    // 解析json参数
    auto req = getJson(content.c_str());
    vector<vector<string>> retData;

    // find uploading_file, get uploading_file_list
    string sql_find_list = "select * from uploading_file where u_id = " + req["u_id"] + ";";
    printf("sql = %s\n", sql_find_list.c_str());
    ret = sqlQuery(retData, sql_find_list.c_str());
    cout << "sql return " << ret << endl;
    // cout << "user has " << ret << "uploading files!" << endl;

    for (unsigned int i = 0; i < retData.size(); i++)
    {
        map<string, string> nowRow;
        nowRow["id"] = retData.at(i).at(0);
        nowRow["file_real_path"] = gbk_to_utf8(retData.at(i).at(2));
        nowRow["md5"] = retData.at(i).at(3);
        nowRow["file_len"] = retData.at(i).at(4);
        nowRow["file_pointer"] = retData.at(i).at(5);
        nowRow["parent_id"] = retData.at(i).at(6);
        nowRow["file_name"] = gbk_to_utf8(retData.at(i).at(7));
        nowRow["uploading_dir_id"] = retData.at(i).at(8);

        res_data.push_back(nowRow);
    }

    string retJson = toJsonList(res_data);
    string res = genJsonResponse(retJson);

    write(sock, res.c_str(), res.length());

    return 0;
}

int uploadingDirList(int sock, char *buf, int body_start, int total_len, int content_len)
{
    int ret;
    string content = getAllContent(sock, buf, body_start, total_len, content_len);
    vector<map<string, string>> res_data;

    // 解析json参数
    auto req = getJson(content.c_str());
    vector<vector<string>> retData;

    // find uploading_dir, get uploading_dir_list
    string sql_find_list = "select * from uploading_dir where u_id = " + req["u_id"] + ";";
    printf("sql = %s\n", sql_find_list.c_str());
    ret = sqlQuery(retData, sql_find_list.c_str());
    cout << "sql return " << ret << endl;
    // cout << "user has " << ret << "uploading dirs!" << endl;

    for (unsigned int i = 0; i < retData.size(); i++)
    {
        map<string, string> nowRow;
        nowRow["id"] = retData.at(i).at(0);
        nowRow["file_real_path"] = gbk_to_utf8(retData.at(i).at(2));
        // nowRow["md5"] = retData.at(i).at(3);
        nowRow["logic_id"] = retData.at(i).at(4);
        nowRow["file_now"] = retData.at(i).at(5);
        nowRow["file_count"] = retData.at(i).at(6);

        // find logic_file, get all info
        vector<vector<string>> logicData;
        string sql_logic_file = "select * from logic_file where id = " + nowRow["logic_id"] + ";";
        printf("sql = %s\n", sql_logic_file.c_str());
        ret = sqlQuery(logicData, sql_logic_file.c_str());

        nowRow["file_name"] = gbk_to_utf8(logicData.at(0).at(1));
        // nowRow["md5"] = logicData.at(0).at(2);
        nowRow["parent_id"] = logicData.at(0).at(3);
        nowRow["is_dir"] = logicData.at(0).at(4);

        res_data.push_back(nowRow);
    }

    string retJson = toJsonList(res_data);
    string res = genJsonResponse(retJson);

    write(sock, res.c_str(), res.length());

    return 0;
}

int downloadFileOver(int sock, char *buf, int body_start, int total_len, int content_len)
{
    int ret;
    string content = getAllContent(sock, buf, body_start, total_len, content_len);
    map<string, string> res_data;

    // 解析json参数
    auto req = getJson(content.c_str());
    vector<vector<string>> retData;

    // find downloading_dir_id
    string sql_find_dir_id = "select * from downloading_file where id = " + req["downloading_id"] + ";";
    printf("sql = %s\n", sql_find_dir_id.c_str());
    ret = sqlQuery(retData, sql_find_dir_id.c_str());

    if (retData.size() == 0)
    {
        cout << "err! downloading id not found!" << endl;
        return -1;
    }

    string ret_file_now = "0";

    //若是文件夹下的待下载文件则更新file_now
    if (retData.at(0).at(5) != "-1")
    {
        // update downloading_dir file_now
        string sql_downloading_dir = "update downloading_dir set file_now=file_now+1 where id = " + retData.at(0).at(5) + ";";
        printf("sql = %s\n", sql_downloading_dir.c_str());
        // ret = sqlQuery(retData, sql.c_str());
        ret = sqlChange(sql_downloading_dir.c_str());

        string sql_downloading_now = "select * from downloading_dir where id = " + retData.at(0).at(5) + ";";
        printf("sql = %s\n", sql_downloading_now.c_str());
        ret = sqlQuery(retData, sql_downloading_now.c_str());
        // ret = sqlChange(sql_downloading_now.c_str());
        ret_file_now = retData.at(0).at(5);
    }

    // delete from downloading_file a row
    string sql_downloading_file = "delete from downloading_file where id = " + req["downloading_id"] + ";";
    printf("sql = %s\n", sql_downloading_file.c_str());
    // ret = sqlQuery(retData, sql.c_str());
    ret = sqlChange(sql_downloading_file.c_str());

    if (ret == -1)
    {
        res_data["msg"] = "error";
        res_data["file_now"] = ret_file_now;
    }
    else
    {
        res_data["msg"] = "success";
        res_data["file_now"] = ret_file_now;
    }

    string retJson = toJson(res_data);
    string res = genJsonResponse(retJson);
    write(sock, res.c_str(), res.length());

    return 0;
}

int downloadDirOver(int sock, char *buf, int body_start, int total_len, int content_len)
{
    int ret;
    string content = getAllContent(sock, buf, body_start, total_len, content_len);
    map<string, string> res_data;

    // 解析json参数
    auto req = getJson(content.c_str());
    vector<vector<string>> retData;

    // delete from downloading_dir a row
    string sql_downloading_dir = "delete from downloading_dir where id = " + req["downloading_id"] + ";";
    printf("sql = %s\n", sql_downloading_dir.c_str());
    // ret = sqlQuery(retData, sql.c_str());
    ret = sqlChange(sql_downloading_dir.c_str());

    if (ret == -1)
    {
        res_data["msg"] = "error";
    }
    else
    {
        res_data["msg"] = "success";
    }

    string retJson = toJson(res_data);
    string res = genJsonResponse(retJson);
    write(sock, res.c_str(), res.length());

    return 0;
}

int getFileLogicId(int sock, char *buf, int body_start, int total_len, int content_len)
{
    int ret;
    string content = getAllContent(sock, buf, body_start, total_len, content_len);
    map<string, string> res_data;

    // 解析json参数
    auto req = getJson(content.c_str());
    vector<vector<string>> retData;

    // delete from downloading_dir a row
    string sql = "select * from logic_file where file_name = \'";
    sql += utf8_to_gbk(req["file_name"]) + "\' and parent_id = " + req["parent_id"] + " and is_dir = " + req["is_dir"];
    cout << sql << endl;
    ret = sqlQuery(retData, sql.c_str());
    // ret = sqlChange(sql_downloading_dir.c_str());
    if (ret == -1)
    {
        res_data["msg"] = "error";
        res_data["id"] = "-1";
    }
    else
    {
        res_data["msg"] = "success";
        res_data["id"] = retData.at(0).at(0);
    }

    string retJson = toJson(res_data);
    string res = genJsonResponse(retJson);
    write(sock, res.c_str(), res.length());

    return 0;
}

// 解析formdata 二进制
int analyseWebFormDataBinary(const char *content, map<string, string> &M, const char *&buf)
{
    //查找uploading_id
    char tmp[256];

    const char *ptr = strstr(content, "name=\"md5\"");
    if (ptr == NULL)
        return -1;
    sscanf(ptr, "name=\"md5\"\r\n\r\n%s", tmp);

    M["md5"] = tmp;

    //查找uploading_len
    ptr = strstr(content, "name=\"file_name\"");
    if (ptr == NULL)
        return -1;
    sscanf(ptr, "name=\"file_name\"\r\n\r\n%s", tmp);

    M["file_name"] = tmp;

    //查找file_pointer
    ptr = strstr(content, "name=\"file_size\"");
    if (ptr == NULL)
        return -1;
    sscanf(ptr, "name=\"file_size\"\r\n\r\n%s", tmp);

    M["file_size"] = tmp;

    //查找file_pointer
    ptr = strstr(content, "name=\"parent_id\"");
    if (ptr == NULL)
        return -1;
    sscanf(ptr, "name=\"parent_id\"\r\n\r\n%s", tmp);

    M["parent_id"] = tmp;

    //查找file_content
    ptr = strstr(ptr, "Content-Type: ");
    if (ptr == NULL)
        return -1;
    ptr = strstr(ptr, "\r\n\r\n");
    if (ptr == NULL)
        return -1;
    ptr += strlen("\r\n\r\n");
    buf = ptr;

    return 0;
}

int webUpload(int sock, char *buf, int body_start, int total_len, int content_len)
{
    int ret;
    const int MAX_LEN = 3000000;
    if (content_len >= MAX_LEN)
    {
        printf("file big err\n");
        respErrMsg(sock, "file big err");
        return -1;
    }
    char *form_content = new char[MAX_LEN];
    ret = getAllFormData(sock, buf, body_start, total_len, content_len, form_content);
    // 处理post请求出错的情况
    if (ret)
    {
        printf("webUpload post err\n");
        respErrMsg(sock, "webUpload post err");
        //释放
        delete[] form_content;
        return -1;
    }

    // string content = getAllContent(sock, buf, body_start, total_len, content_len);
    map<string, string> res_data;
    map<string, string> req;
    vector<vector<string>> sql_query_res;

    //文件内容
    const char *file_content;
    // 取得请求内容
    ret = analyseWebFormDataBinary(form_content, req, file_content);

    req["file_name"] = utf8_to_gbk(req["file_name"]);
    // cout << "file_name:" << file_name << endl;
    // cout << "md5" << md5 << endl;
    // cout << "file_content" << file_content << endl;
    // cout << "cnt" << cnt << endl;

    string file_path = SKY_REAL_PATH + req["md5"];
    FILE *out_file = fopen(file_path.c_str(), "w");
    if (out_file == NULL)
    {
        printf("open %s err\n", file_path.c_str());
        respErrMsg(sock, "webUpload openfile err");
        delete[] form_content;
        return -1;
    }
    // 循环读写
    fwrite(file_content, sizeof(char), str2int(req["file_size"]), out_file);
    fclose(out_file);

    // 插入物理文件表
    string sql = "insert into physic_file(md5,file_size,file_link_cnt) values (\'";
    sql += req["md5"] + "\', ";
    sql += req["file_size"] + ",1);";
    cout << "webUpload sql = " << sql << endl;
    ret = sqlChange(sql.c_str());
    if (ret)
    {
        printf("uploadFileNormal sqlChange err\n");
        respErrMsg(sock, "uploadFileNormal sqlChange err");
        delete[] form_content;
        return -1;
    }
    // 插入逻辑文件表
    ret = createLogicFile(req["file_name"],
                          req["md5"],
                          req["parent_id"],
                          "0");
    if (ret)
    {
        printf("webUpload sqlChange err\n");
        respErrMsg(sock, "webUpload sqlChange err");
        delete[] form_content;
        return -1;
    }

    res_data["msg"] = "success";
    string res = genJsonResponse(toJson(res_data));
    write(sock, res.c_str(), res.length());
    delete[] form_content;

    return 0;
}

int webDownload(int sock, char *buf, int body_start, int total_len, int content_len)
{
    int ret;
    string content = getAllContent(sock, buf, body_start, total_len, content_len);

    map<string, string> res_data;
    vector<vector<string>> retData;

    // 解析json参数
    auto req = getJson(content.c_str());

    // find logic_file, get md5
    string sql_logic_file = "select * from logic_file where id = " + req["id"] + ";";
    printf("sql = %s\n", sql_logic_file.c_str());
    retData.clear();
    ret = sqlQuery(retData, sql_logic_file.c_str());

    string file_name = SKY_REAL_PATH;
    file_name += retData.at(0).at(2);

    FILE *file_in = fopen(file_name.c_str(), "rb");
    if (file_in == NULL)
    {
        cout << "file open err" << endl;
        return -1;
    }

    ret = fseek(file_in, 0, SEEK_END);
    if (ret != 0)
    {
        cout << "seek error" << endl;
        return -1;
    }

    int file_len = ftell(file_in);

    ret = fseek(file_in, 0, SEEK_SET);
    if (ret != 0)
    {
        cout << "seek error" << endl;
        return -1;
    }

    char *download_content = new char[file_len + 1];

    ret = fread(download_content, sizeof(char), file_len, file_in);
    // cout << "read: " << download_content << ", totally " << ret << "bytes" << endl;
    cout << "read: " << ret << "bytes" << endl;

    fclose(file_in);

    getBinaryResponseHeadForWeb(buf, ret);
    write(sock, buf, strlen(buf));
    write(sock, download_content, ret);

    delete[] download_content;

    return 0;
}