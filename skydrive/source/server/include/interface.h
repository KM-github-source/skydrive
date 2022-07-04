#pragma once
/**************************************************************
 * 定义所有http请求接口函数
 */

int userLogin(int sock, char *buf, int body_start, int total_len, int content_len,std::string clientinfo);

int userRegister(int sock, char *buf, int body_start, int total_len, int content_len);

int checkFileExist(int sock, char *buf, int body_start, int total_len, int content_len);

int uploadFileNormal(int sock, char *buf, int body_start, int total_len, int content_len);

int uploadMkdir(int sock, char *buf, int body_start, int total_len, int content_len);

int uploadFileFast(int sock, char *buf, int body_start, int total_len, int content_len);

int uploadFileFirst(int sock, char *buf, int body_start, int total_len, int content_len);

int uploadDirFirst(int sock, char *buf, int body_start, int total_len, int content_len);

int uploadDirOver(int sock, char *buf, int body_start, int total_len, int content_len);

int getAllFileUnderDir(int sock, char *buf, int body_start, int total_len, int content_len);

int changeName(int sock, char *buf, int body_start, int total_len, int content_len);

int copyFile(int sock, char *buf, int body_start, int total_len, int content_len);

int copyDir(int sock, char *buf, int body_start, int total_len, int content_len);

int moveFile(int sock, char *buf, int body_start, int total_len, int content_len);

int delFile(int sock, char *buf, int body_start, int total_len, int content_len);

int downloadNormalFile(int sock, char *buf, int body_start, int total_len, int content_len);

int downloadFileFirst(int sock, char *buf, int body_start, int total_len, int content_len);

int downloadDirFirst(int sock, char *buf, int body_start, int total_len, int content_len);

int downloadFileOver(int sock, char *buf, int body_start, int total_len, int content_len);

int downloadDirOver(int sock, char *buf, int body_start, int total_len, int content_len);

int downloadingFileList(int sock, char *buf, int body_start, int total_len, int content_len);

int downloadingDirList(int sock, char *buf, int body_start, int total_len, int content_len);

int uploadingFileList(int sock, char *buf, int body_start, int total_len, int content_len);

int uploadingDirList(int sock, char *buf, int body_start, int total_len, int content_len);

int getFileLogicId(int sock, char *buf, int body_start, int total_len, int content_len);

int webUpload(int sock, char *buf, int body_start, int total_len, int content_len);

int webDownload(int sock, char *buf, int body_start, int total_len, int content_len);