#pragma once
/**************************************************************
 * 定义数据库操作函数
 */
int sqlChange(const char *);
int sqlQuery(std::vector<std::vector<std::string>> &, const char *);