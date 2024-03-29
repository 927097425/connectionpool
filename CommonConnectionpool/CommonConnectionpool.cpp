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
		if (idx == -1) continue;//��Ч������
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
	//������ʼ����������
	for (int i = 0; i < initSize; i++)
	{
		Connection* p = new Connection();
		p->connect(_ip, _port, _username, _password, _dbname);
		p->refreshAliveTime();
		_connectionQue.push(p);
		_connectionCnt++;
	}

	//����һ���µ��̣߳���Ϊ���ӵ�������
	thread produce(std::bind(&Connectionpool::produceConnectionTask, this));
	produce.detach();//���óɷ����߳�
	//����һ���µĶ�ʱ�̣߳�ɨ�賬��������ʱ������Ӳ����
	thread scanner(std::bind(&Connectionpool::scannerConnectionTask, this));
	scanner.detach();
	

}

void Connectionpool::produceConnectionTask()
{
	for (;;)
	{
		unique_lock<mutex> lock(_queueMutex);
		while (!_connectionQue.empty())//�յ�ʱ������
		{
			cv.wait(lock);
		}
		//��������û�е������ޣ����������µ�����

		if (_connectionCnt < _maxSize)
		{
			Connection* p = new Connection();
			p->connect(_ip, _port, _username, _password, _dbname);
			p->refreshAliveTime(); 
			_connectionQue.push(p);
			_connectionCnt++;
		}
		//֪ͨ�������߳̿�������������
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
				LOG("��ȡ�������ӳ�ʱ����ȡʧ��");
				return NULL;
			}
		}
	}
	//ʹ��lambda���ʽд����������д��������������ָ��ʹ������ͷ����Ӷ��ǹ黹�����ӳ���
	shared_ptr<Connection> sp(_connectionQue.front(),
		[&](Connection* pcon) {
			unique_lock<mutex> lock(_queueMutex);
			pcon->refreshAliveTime();
			_connectionQue.push(pcon);
		});
	_connectionQue.pop();
	cv.notify_all();//֪ͨ�����߼����������
	return sp;
}


void Connectionpool::scannerConnectionTask()
{
	for (;;)
	{
		//ģ��˯�߶�ʱЧ��
		this_thread::sleep_for(chrono::seconds(_maxIdleTime));

		//ɨ���������У��ͷŶ��������
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
			else break;//��ͷ����û�г���_maxIdleTime����϶�Ҳû����
		}
	}
}