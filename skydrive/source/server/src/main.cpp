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
// #include <daemon.h>

#include "../include/const.h"
#include "../include/tools.h"
#include "../include/interface.h"

using namespace std;

// ȫ�ֱ�������
ofstream server_log(S_LOG);

// ������������
int handleConn(int,string);

int main(int argc, char *argv[])
{
    /*******************************************
     * ��������
     * */
    int ret;
    struct sockaddr_in serv_addr, clnt_addr;
    socklen_t clnt_addr_size = sizeof(clnt_addr);

    /*******************************************
     *  ��ʼ��
     * */
    signal(SIGCHLD, SIG_IGN); // ��ֹ�ӽ��̱佩ʬ
    // �ѷ���������ػ�����
    daemon(1, 1);
    // server_log.open;
    if (!server_log.is_open())
    {
        printf("log file open err\n");
        exit(0);
    }
    // ��������ӿ�
    int serv_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    createIpv4ServAddr(S_IP, S_PORT, &serv_addr);
    // �˿ڿ����ظ�ʹ��
    int flag = 1;
    ret = setsockopt(serv_sock, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag));
    if (0 != ret)
    {
        perror("server setsockopt");
        exit(0);
    }
    // socket�������ַ
    ret = bind(serv_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    if (0 != ret)
    {
        perror("server bind");
        exit(0);
    }
    //�������״̬���ȴ��û���������
    ret = listen(serv_sock, 20);
    if (0 != ret)
    {
        perror("server listen");
        exit(0);
    }

    /*******************************************
     *  ��ʼ������������
     * */
    while (true)
    {
        int clnt_sock = accept(serv_sock, (struct sockaddr *)&clnt_addr, &clnt_addr_size);
        printf("new conn,");
        printf("client = %s:%d\n", inet_ntoa(clnt_addr.sin_addr), ntohs(clnt_addr.sin_port));
        string clientinfo=inet_ntoa(clnt_addr.sin_addr);
        clientinfo+=":";
        clientinfo+=to_string(ntohs(clnt_addr.sin_port));
        // �����ӽ���
        pid_t pid = fork();
        if (0 == pid)
        {
            // ��
            prctl(PR_SET_PDEATHSIG, SIGKILL);
            handleConn(clnt_sock,clientinfo);
            //�ر��׽���
            close(clnt_sock);
            exit(0);
        }
        else if (pid > 0)
        {
            // ��
            continue;
        }
        else
        {
            //����ʧ�ܷ��ظ���
            printf("fork err\n");
            exit(-1);
        }
    }

    close(serv_sock);

    return 0;
}

int handleConn(int sock,string clientinfo)
{
    int ret;
    char *buf = new char[BUF_SIZE + 1];

    int contentLen;   //�����峤��
    int totalLen = 0; //Ŀǰ�յ����ܳ���
    string urlPath;   //����·��
    int body_start = 0;
    ret = read(sock, buf, BUF_SIZE);
    if (ret <= 0)
    {
        perror("sock read");
        return -1;
    }
    buf[ret] = '\0';
    server_log << buf << endl
               << endl;

    // ��OPTIONS��������⴦��
    // ע�⣺�û����ᷢ��OPTIONS����ֻ��POST̽��ʱ�ᷢ��
    // �ʴ�����֮�󣬲����˳���Ӧ�ü�����ȡ��������POST����
    if (buf[0] == 'O')
    {
        sprintf(buf, "HTTP/1.1 200 OK\r\n"
                     "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n"
                     "Access-Control-Allow-Headers: X-Requested-With,X-PINGOTHER,Content-Type,Accept\r\n"
                     "Access-Control-Allow-Origin: *\r\n"
                     "Vary: Accept-Encoding, Origin\r\n"
                     "Content-Length: 0\r\n"
                     "Content-Type: text/plain\r\n"
                     "Connection: Keep-Alive\r\n"
                     "Keep-Alive: timeout=2, max=100\r\n"
                     "Content-Encoding: gzip\r\n"
                     "Access-Control-Max-Age: 86400\r\n\r\n");
        ret = write(sock, buf, strlen(buf));
        if (ret <= 0)
        {
            perror("sock write");
            return -1;
        }
        ret = read(sock, buf, BUF_SIZE);
        if (ret <= 0)
        {
            perror("sock read");
            return -1;
        }
        buf[ret] = '\0';
        server_log << buf << endl
                   << endl;
    }

    // printf("req = \n%s\n", buf);
    contentLen = getContentLength(buf);
    if (contentLen == -1)
    {
        printf("no content-len in req head\n");
        return -1;
    }
    urlPath = getUrlPath(buf);
    if (urlPath == "")
    {
        printf("no url in req head\n");
        return -1;
    }
    // printf("len = %d, url = %s\n", contentLen, urlPath.c_str());
    char last_ch = 0;
    for (int i = 0; i < ret; i++)
    {
        if (last_ch == '\n' && buf[i] == '\r')
        {
            body_start = i + 2;
            break;
        }
        last_ch = buf[i];
    }
    totalLen = ret - body_start;

    // ѡ����Ľӿ�
    if (urlPath == "/user/login")
        userLogin(sock, buf, body_start, totalLen, contentLen,clientinfo);
    else if (urlPath == "/user/register")
        userRegister(sock, buf, body_start, totalLen, contentLen);
    else if (urlPath == "/upload/normal")
        uploadFileNormal(sock, buf, body_start, totalLen, contentLen);
    else if (urlPath == "/upload/mkdir")
        uploadMkdir(sock, buf, body_start, totalLen, contentLen);
    else if (urlPath == "/upload/check")
        checkFileExist(sock, buf, body_start, totalLen, contentLen);
    else if (urlPath == "/upload/fast")
        uploadFileFast(sock, buf, body_start, totalLen, contentLen);
    else if (urlPath == "/upload/file-first")
        uploadFileFirst(sock, buf, body_start, totalLen, contentLen);
    else if (urlPath == "/upload/dir-first")
        uploadDirFirst(sock, buf, body_start, totalLen, contentLen);
    else if (urlPath == "/upload/dir-over")
        uploadDirOver(sock, buf, body_start, totalLen, contentLen);
    else if (urlPath == "/operate/all-file")
        getAllFileUnderDir(sock, buf, body_start, totalLen, contentLen);
    else if (urlPath == "/operate/change-name")
        changeName(sock, buf, body_start, totalLen, contentLen);
    else if (urlPath == "/operate/copy-file")
        copyFile(sock, buf, body_start, totalLen, contentLen);
    else if (urlPath == "/operate/copy-dir")
        copyDir(sock, buf, body_start, totalLen, contentLen);
    else if (urlPath == "/operate/move-file")
        moveFile(sock, buf, body_start, totalLen, contentLen);
    else if (urlPath == "/operate/del-file")
        delFile(sock, buf, body_start, totalLen, contentLen);
    else if (urlPath == "/download/file-normal")
        downloadNormalFile(sock, buf, body_start, totalLen, contentLen);
    else if (urlPath == "/download/file-first")
        downloadFileFirst(sock, buf, body_start, totalLen, contentLen);
    else if (urlPath == "/download/dir-first")
        downloadDirFirst(sock, buf, body_start, totalLen, contentLen);
    else if (urlPath == "/download/file-over")
        downloadFileOver(sock, buf, body_start, totalLen, contentLen);
    else if (urlPath == "/download/dir-over")
        downloadDirOver(sock, buf, body_start, totalLen, contentLen);
    else if (urlPath == "/getinfo/downloading-file-list")
        downloadingFileList(sock, buf, body_start, totalLen, contentLen);
    else if (urlPath == "/getinfo/downloading-dir-list")
        downloadingDirList(sock, buf, body_start, totalLen, contentLen);
    else if (urlPath == "/getinfo/uploading-file-list")
        uploadingFileList(sock, buf, body_start, totalLen, contentLen);
    else if (urlPath == "/getinfo/uploading-dir-list")
        uploadingDirList(sock, buf, body_start, totalLen, contentLen);        
    else if (urlPath == "/getinfo/single-file")
        getFileLogicId(sock, buf, body_start, totalLen, contentLen);      
    else if (urlPath == "/web/upload")
        webUpload(sock, buf, body_start, totalLen, contentLen);      
    else if (urlPath == "/web/download")
        webDownload(sock, buf, body_start, totalLen, contentLen);  
    // else if (urlPath == "/web/download")
    //     getFileLogicId(sock, buf, body_start, totalLen, contentLen);      
    else
    {
        printf("unknown request\n");
        server_log << "unknown request, " << urlPath << endl;
        return -1;
    }

    delete[] buf;
    return 0;
}