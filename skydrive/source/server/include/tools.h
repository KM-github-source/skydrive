#pragma once
/**************************************************************
 * 存放工具函数
 */

// str2int
int str2int(const std::string &);

// 域名转ip
int hostToIp(const std::string &, std::string &);

// 网址或点分十进制ip地址转网络序ip
int addrToNs(const std::string &, int &);

// 创建servaddr结构体，传入点分十进制ip和主机序端口号
int createIpv4ServAddr(const std::string &, const int, void *);

// 获取content-len
int getContentLength(const char *);

// 获取请求的url
std::string getUrlPath(const char *);

// 获取请求体（json格式）
std::map<std::string, std::string> getJson(const char *);

// map转JSON
std::string toJson(std::map<std::string, std::string> &);

// vec转JSON
std::string toJsonList(std::vector<std::map<std::string, std::string>> &);

// gbk与utf8互转
std::string utf8_to_gbk(const std::string &);

// gbk转utf8
std::string gbk_to_utf8(const std::string &);

// 解析formdata
int analyseFormData(const std::string &, std::map<std::string, int> &, const char *&);

// 解析formdata 二进制
int analyseFormDataBinary(const char* content, std::map<std::string, int> &M, const char *&buf);
