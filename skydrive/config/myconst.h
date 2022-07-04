#pragma once
/**************************************************************
 * 定义所有常量
 */

// 数据库常量定义
// 请修改这里的密码DB_PASSWD，否则无法正常连接!!!
#define DB_USER "root"
#define DB_PASSWD ""

//在install.sh里安装好的数据库，请不要更换名称
#define DB_NAME "skydrive"

// 定义服务器相关常量
//请修改成本机ip后再执行install.sh!
#define S_IP "192.168.0.28"
// #define S_IP "139.9.243.37"
#define S_PORT 40000
#define S_LOG "./log"
#define BUF_SIZE 8192

// 定义网盘相关常量
//物理文件根目录，请不要更换路径
#define SKY_REAL_PATH "/home/skydrive_real_path/"
//一次传输的大小
#define SLICE_LEN 102400