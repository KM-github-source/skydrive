#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <netdb.h>
#include <string>
#include <sys/types.h>
#include <ifaddrs.h>
#include <vector>
#include <fcntl.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <map>
#include <sstream>
#include <codecvt>
#include <locale>

using namespace std;

int str2int(const string &str)
{
    return atoi(str.c_str());
}

int hostToIp(const string &host, string &ip)
{
    const char *phost = host.c_str();
    struct sockaddr_in sa;
    sa.sin_family = AF_INET;
    if (inet_aton(phost, &sa.sin_addr) == 0)
    {
        struct hostent *he;
        he = gethostbyname(phost);
        if (he == NULL)
            return -1;
        memcpy(&sa.sin_addr, he->h_addr, sizeof(struct in_addr));
    }
    ip = inet_ntoa(sa.sin_addr);
    return 0;
}

int addrToNs(const string &addr, int &ns)
{
    string ip;
    if (-1 == hostToIp(addr, ip))
        return -1;
    ns = inet_addr(ip.c_str());
    if (-1 == ns)
        return -1;
    return 0;
}

int createIpv4ServAddr(const string &ip, const int port, void *serv_addr_p)
{
    struct sockaddr_in &serv_addr = *(struct sockaddr_in *)serv_addr_p;

    //向服务器（特定的IP和端口）发起请求
    memset(&serv_addr, 0, sizeof(serv_addr)); //每个字节都用0填充
    serv_addr.sin_family = AF_INET;           //使用IPv4地址
    if (ip.length() == 0 || ip == "0.0.0.0")
        serv_addr.sin_addr.s_addr = INADDR_ANY;
    else
        serv_addr.sin_addr.s_addr = inet_addr(ip.c_str());
    printf("ip = %s, port = %d\n", ip.c_str(), port);
    serv_addr.sin_port = htons(port); //端口

    return 0;
}

int getContentLength(const char *head)
{
    int len;
    const char *ptr = strstr(head, "Content-Length: ");
    if (ptr == NULL)
        return -1;
    sscanf(ptr, "Content-Length: %d", &len);
    return len;
}

string getUrlPath(const char *head)
{
    char url[30];
    const char *ptr = strstr(head, "POST ");
    if (ptr == NULL)
        return "";
    sscanf(ptr, "POST %s", url);
    return string(url);
}

//解析json
map<string, string> getJson_old(const char *buf)
{
    map<string, string> result;
    stringstream sstream;
    stringstream sstream2;
    sstream << buf;
    string strres;
    while (getline(sstream, strres))
    {
        if (strres[0] == '{' || strres[0] == '}')
            continue;
        else
        {
            unsigned int len = strres.length();
            // int flag = 0;
            //处理先导空格
            unsigned int i;
            string key = "", value = "";
            for (i = 0; (strres[i] == ' ' || strres[i] == '\t') && i < len; i++)
            {
                ;
            }
            i++; //略过第一个双引号
            for (; strres[i] != '\"' && i < len; i++)
            {
                key += strres[i];
            }
            i++; //略过第二个双引号
            //处理先导空格
            for (; (strres[i] == ' ' || strres[i] == '\t' || strres[i] == ':') && i < len; i++)
            {
                ;
            }
            //略过可能的第三个双引号
            if (strres[i] == '\"')
                i++;
            //开始读value
            for (; strres[i] != '\"' && strres[i] != ',' && i < len; i++)
            {
                value += strres[i];
            }
            result[key] = value;
            // printf("key = %s value = %s\n", key.c_str(), value.c_str());
        }
    }
    return result;
}
map<string, string> getJson(const char *buf)
{
    map<string, string> result;
    const char *chat = buf;
    int string_flag = 0;
    int keystart = 0;

    int valuestart = 0;

    int colon_flag = 0;
    string mykey;
    string myvalue;
    for (; chat; chat++)
    {
        // char testlog = *chat;
        if (string_flag == 1)
        {
            if (*chat == '\"' && *(chat - 1) != '\\')
            {
                if (string_flag == 0)
                    string_flag = 1;
                else
                    string_flag = 0;

                if (keystart == 0 && colon_flag == 0)
                {
                    keystart = 1;
                }
                else if (keystart == 1)
                {
                    keystart = 0;
                }
            }
            else
            {
                if (keystart == 1)
                {
                    mykey += *chat;
                }
                else if (valuestart == 1)
                {
                    myvalue += *chat;
                }
                else
                    printf("json error\n");
            }
        }
        else if (*chat == ' ' || *chat == '\t' || *chat == '\r' || *chat == '\n' || *chat == '{')
        {
            continue;
        }
        else if (*chat == '\"')
        {
            if (string_flag == 0)
                string_flag = 1;
            else
                string_flag = 0;

            if (keystart == 0 && colon_flag == 0)
            {
                keystart = 1;
            }
            else if (keystart == 1)
            {
                keystart = 0;
            }
        }
        else if (*chat == ':')
        {
            colon_flag = 1;
            valuestart = 1;
        }
        else if (*chat == ',')
        {
            colon_flag = 0;
            valuestart = 0;
            result[mykey] = myvalue;
            mykey.clear();
            myvalue.clear();
        }
        else if (*chat == '}')
        {
            colon_flag = 0;
            valuestart = 0;
            result[mykey] = myvalue;
            mykey.clear();
            myvalue.clear();
            break;
        }
        else
        {
            if (keystart == 1)
            {
                mykey += *chat;
            }
            else if (valuestart == 1)
            {
                myvalue += *chat;
            }
            else
                continue;
        }
    }
    return result;
}
// map转JSON
string toJson(map<string, string> &M)
{
    string ret = "{";
    map<string, string>::iterator it;
    for (it = M.begin(); it != M.end(); it++)
    {
        if (std::next(it) == M.end())
            ret += "\"" + it->first + "\":\"" + it->second + "\"";
        else
            ret += "\"" + it->first + "\":\"" + it->second + "\", ";
    }
    ret += "}";

    return ret;
}

// vec转JSON
string toJsonList(vector<map<string, string>> &ML)
{
    string ret = "[";

    for (unsigned int i = 0; i < ML.size(); i++)
    {
        if (i < ML.size() - 1)
            ret += toJson(ML[i]) + ", ";
        else
            ret += toJson(ML[i]);
    }
    ret += "]";
    return ret;
}

// 解析formdata
int analyseFormData(const string &content, map<string, int> &M, const char *&buf)
{
    //查找uploading_id
    const char *ptr = strstr(content.c_str(), "name=\"uploading_id\"");
    if (ptr == NULL)
        return -1;
    int uploading_id;
    sscanf(ptr, "name=\"uploading_id\"\r\n\r\n%d", &uploading_id);

    M["uploading_id"] = uploading_id;

    //查找uploading_len
    ptr = strstr(content.c_str(), "name=\"uploading_len\"");
    if (ptr == NULL)
        return -1;
    int uploading_len;
    sscanf(ptr, "name=\"uploading_len\"\r\n\r\n%d", &uploading_len);

    M["uploading_len"] = uploading_len;

    //查找file_content
    ptr = strstr(content.c_str(), "Content-Type: application/octet-stream");
    if (ptr == NULL)
        return -1;
    ptr += strlen("Content-Type: application/octet-stream\r\n\r\n");
    buf = ptr;

    return 0;
}

// // 解析formdata 二进制
// int analyseFormDataBinary(const char* content, map<string, int> &M, const char *&buf)
// {
//     //查找uploading_id
//     const char *ptr = strstr(content, "name=\"uploading_id\"");
//     if (ptr == NULL)
//         return -1;
//     int uploading_id;
//     sscanf(ptr, "name=\"uploading_id\"\r\n\r\n%d", &uploading_id);

//     M["uploading_id"] = uploading_id;

//     //查找uploading_len
//     ptr = strstr(content, "name=\"uploading_len\"");
//     if (ptr == NULL)
//         return -1;
//     int uploading_len;
//     sscanf(ptr, "name=\"uploading_len\"\r\n\r\n%d", &uploading_len);

//     M["uploading_len"] = uploading_len;

//     //查找file_content
//     ptr = strstr(content, "Content-Type: application/octet-stream");
//     if (ptr == NULL)
//         return -1;
//     ptr += strlen("Content-Type: application/octet-stream\r\n\r\n");
//     buf = ptr;

//     return 0;
// }
// 解析formdata 二进制
int analyseFormDataBinary(const char *content, map<string, int> &M, const char *&buf)
{
    //查找uploading_id
    const char *ptr = strstr(content, "name=\"uploading_id\"");
    if (ptr == NULL)
        return -1;
    int uploading_id;
    sscanf(ptr, "name=\"uploading_id\"\r\n\r\n%d", &uploading_id);

    M["uploading_id"] = uploading_id;

    //查找uploading_len
    ptr = strstr(content, "name=\"uploading_len\"");
    if (ptr == NULL)
        return -1;
    int uploading_len;
    sscanf(ptr, "name=\"uploading_len\"\r\n\r\n%d", &uploading_len);

    M["uploading_len"] = uploading_len;

    //查找file_pointer
    ptr = strstr(content, "name=\"file_pointer\"");
    if (ptr == NULL)
        return -1;
    int file_pointer;
    sscanf(ptr, "name=\"file_pointer\"\r\n\r\n%d", &file_pointer);

    M["file_pointer"] = file_pointer;

    //查找file_content
    ptr = strstr(content, "Content-Type: application/octet-stream");
    if (ptr == NULL)
        return -1;
    ptr += strlen("Content-Type: application/octet-stream\r\n\r\n");
    buf = ptr;

    return 0;
}

std::wstring utf8_to_wstr(const std::string &_utf8)
{
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    return converter.from_bytes(_utf8);
}
// 包装 wstring/wbuffer_convert 所用的绑定本地环境平面的工具
template <class Facet>
struct deletable_facet : Facet
{
    template <class... Args>
    explicit deletable_facet(Args &&...args) : Facet(std::forward<Args>(args)...) {}
    ~deletable_facet() override = default;
};
const char *GBK_LOCALE_NAME = "zh_CN.GBK";
string wstr_to_gbk(const std::wstring &_wstr)
{
    using codecvt = deletable_facet<std::codecvt_byname<wchar_t, char, mbstate_t>>;
    std::wstring_convert<codecvt> convert(new codecvt(GBK_LOCALE_NAME));
    return convert.to_bytes(_wstr);
}
// utf8转gbk
std::string utf8_to_gbk(const std::string &_utf8)
{
    std::wstring tmp_wstr = utf8_to_wstr(_utf8);
    return wstr_to_gbk(tmp_wstr);
}
std::string wstr_to_utf8(const std::wstring &_wstr)
{
    std::wstring_convert<std::codecvt_utf8<wchar_t>> convert;
    return convert.to_bytes(_wstr);
}
std::wstring gbk_to_wstr(const std::string &_gbk)
{
    using codecvt = deletable_facet<std::codecvt_byname<wchar_t, char, mbstate_t>>;
    std::wstring_convert<codecvt> convert(new codecvt(GBK_LOCALE_NAME));
    return convert.from_bytes(_gbk);
}
// gbk转utf8
std::string gbk_to_utf8(const std::string &_gbk)
{
    std::wstring tmp_wstr = gbk_to_wstr(_gbk);
    return wstr_to_utf8(tmp_wstr);
}
