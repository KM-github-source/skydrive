#pragma once
/**************************************************************
 * ��Ź��ߺ���
 */

// str2int
int str2int(const std::string &);

// ����תip
int hostToIp(const std::string &, std::string &);

// ��ַ����ʮ����ip��ַת������ip
int addrToNs(const std::string &, int &);

// ����servaddr�ṹ�壬������ʮ����ip��������˿ں�
int createIpv4ServAddr(const std::string &, const int, void *);

// ��ȡcontent-len
int getContentLength(const char *);

// ��ȡ�����url
std::string getUrlPath(const char *);

// ��ȡ�����壨json��ʽ��
std::map<std::string, std::string> getJson(const char *);

// mapתJSON
std::string toJson(std::map<std::string, std::string> &);

// vecתJSON
std::string toJsonList(std::vector<std::map<std::string, std::string>> &);

// gbk��utf8��ת
std::string utf8_to_gbk(const std::string &);

// gbkתutf8
std::string gbk_to_utf8(const std::string &);

// ����formdata
int analyseFormData(const std::string &, std::map<std::string, int> &, const char *&);

// ����formdata ������
int analyseFormDataBinary(const char* content, std::map<std::string, int> &M, const char *&buf);
