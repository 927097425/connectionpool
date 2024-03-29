#pragma once
#include<string>
#include<queue>
#include<mutex>
#include<atomic>
#include "Connection.h"
#include<thread>
#include<memory>
#include<functional>
#include<condition_variable>
using namespace std;

/*
实现连接池功能的模块
*/

//线程安全的懒汉式单例模式

class Connectionpool
{
public:
	static Connectionpool* getInstance();
	//给消费者线程从连接池中获得可用空闲链接(使用智能指针)
	shared_ptr<Connection> getConnection();
private:
	Connectionpool();

	bool loadConfigFile();

	//运行在独立的线程中，专门负责生产新链接
	void produceConnectionTask();
	//启动一个新的定时线程，扫描超过最大空闲时间的链接并清除
	void scannerConnectionTask();

	string _ip;//IP地址
	unsigned short _port;//端口号
	string _username;//用户名
	string _password;//密码
	string _dbname;//链接的数据库名称
	int initSize;//连接池初始量
	int _maxSize;//连接池最大链接量
	int _maxIdleTime;//连接池最大空闲时间
	int _connectionTimeout;//连接池获取链接的超时时间

	queue<Connection*> _connectionQue;//存mysql链接的队列表
	mutex _queueMutex;//保证队列线程安全的互斥锁
	atomic_int _connectionCnt; //记录所创建的Connection链接总数atomic是保证线程安全的类型
	condition_variable cv;//设置条件变量用于生产者消费者之间的通信
	
};

