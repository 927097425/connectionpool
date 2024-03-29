#include "CommonConnectionpool.h"

Connectionpool* Connectionpool::getInstance()
{
	static Connectionpool pool;
	return &pool;
}

bool Connectionpool::loadConfigFile()
{
	FILE* pf = fopen("mysql.ini", "r");
	if (pf == NULL)
	{
		LOG("mysql.ini file is not exist!");
		return false;
	}
	
	while (!feof(pf))
	{
		char line[1024] = { 0 };
		fgets(line, 1024, pf);
		string str = line;
		int idx = str.find('=', 0);
		if (idx == -1) continue;//无效配置项
		int endidx = str.find('\n', idx);
		string key = str.substr(0, idx);
		string value = str.substr(idx + 1, endidx - idx - 1);
		
		if (key == "ip")this->_ip = value;
		else if (key == "port") this->_port = stoi(value);
		else if (key == "username") this->_username = value;
		else if (key == "password") this->_password = value;
		else if (key == "databasename") this->_dbname = value;
		else if (key == "initSize") this->initSize = stoi(value);
		else if (key == "maxSize") this->_maxSize = stoi(value);
		else if (key == "maxIdleTime") this->_maxIdleTime = stoi(value);
		else if (key == "maxConnectionTimeout") this->_maxIdleTime = stoi(value);

	}
}

Connectionpool::Connectionpool()
{
	if (!loadConfigFile())return;
	//创建初始数量的链接
	for (int i = 0; i < initSize; i++)
	{
		Connection* p = new Connection();
		p->connect(_ip, _port, _username, _password, _dbname);
		p->refreshAliveTime();
		_connectionQue.push(p);
		_connectionCnt++;
	}

	//启动一个新的线程，作为链接的生产者
	thread produce(std::bind(&Connectionpool::produceConnectionTask, this));
	produce.detach();//设置成分离线程
	//启动一个新的定时线程，扫描超过最大空闲时间的链接并清除
	thread scanner(std::bind(&Connectionpool::scannerConnectionTask, this));
	scanner.detach();
	

}

void Connectionpool::produceConnectionTask()
{
	for (;;)
	{
		unique_lock<mutex> lock(_queueMutex);
		while (!_connectionQue.empty())//空的时候生产
		{
			cv.wait(lock);
		}
		//链接数量没有到达上限，继续创建新的链接

		if (_connectionCnt < _maxSize)
		{
			Connection* p = new Connection();
			p->connect(_ip, _port, _username, _password, _dbname);
			p->refreshAliveTime(); 
			_connectionQue.push(p);
			_connectionCnt++;
		}
		//通知消费者线程可以消费链接了
		cv.notify_all();
	}
}

shared_ptr<Connection> Connectionpool::getConnection()
{
	unique_lock<mutex> lock(_queueMutex);
	while(_connectionQue.empty())
	{
		if(cv_status::timeout == cv.wait_for(lock, chrono::milliseconds(_connectionTimeout)))
		{
			if (_connectionQue.empty())
			{
				LOG("获取空闲链接超时，获取失败");
				return NULL;
			}
		}
	}
	//使用lambda表达式写匿名函数重写析构函数让智能指针使用完后不释放链接而是归还到连接池中
	shared_ptr<Connection> sp(_connectionQue.front(),
		[&](Connection* pcon) {
			unique_lock<mutex> lock(_queueMutex);
			pcon->refreshAliveTime();
			_connectionQue.push(pcon);
		});
	_connectionQue.pop();
	cv.notify_all();//通知生产者检测链接数量
	return sp;
}


void Connectionpool::scannerConnectionTask()
{
	for (;;)
	{
		//模拟睡眠定时效果
		this_thread::sleep_for(chrono::seconds(_maxIdleTime));

		//扫描整个队列，释放多余的链接
		unique_lock<mutex> lock(_queueMutex);
		while (_connectionCnt > this->initSize)
		{
			Connection* p = _connectionQue.front();
			if (p->getAliveTime() > (this->_maxIdleTime * 1000))
			{
				_connectionQue.pop();
				_connectionCnt--;
				delete p;
			}
			else break;//队头链接没有超过_maxIdleTime其余肯定也没超过
		}
	}
}