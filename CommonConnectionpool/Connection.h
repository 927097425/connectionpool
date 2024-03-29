#pragma once
/*
实现mysql数据库的增删改查
*/
#include <mysql.h>
#include <string>
#include<ctime>
using namespace std;
#include "public.h"
// 数据库操作类
class Connection
{
public:
	// 初始化数据库连接
	Connection();
	// 释放数据库连接资源
	~Connection();

	// 连接数据库
	bool connect(string ip, unsigned short port, string user, string password,
		string dbname);

	// 更新操作 insert、delete、update
	bool update(string sql);
	// 查询操作 select
	MYSQL_RES* query(string sql);
	//刷新链接的起始的空闲时间点
	void refreshAliveTime() { _alivetime = clock(); }
	clock_t getAliveTime() const{ return clock() - _alivetime; }
private:
	MYSQL* _conn; // 表示和MySQL Server的一条连接
	clock_t _alivetime;//记录进入空闲状态（连接池）的起始时间
};