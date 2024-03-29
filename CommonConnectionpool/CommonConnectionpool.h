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
ʵ�����ӳع��ܵ�ģ��
*/

//�̰߳�ȫ������ʽ����ģʽ

class Connectionpool
{
public:
	static Connectionpool* getInstance();
	//���������̴߳����ӳ��л�ÿ��ÿ�������(ʹ������ָ��)
	shared_ptr<Connection> getConnection();
private:
	Connectionpool();

	bool loadConfigFile();

	//�����ڶ������߳��У�ר�Ÿ�������������
	void produceConnectionTask();
	//����һ���µĶ�ʱ�̣߳�ɨ�賬��������ʱ������Ӳ����
	void scannerConnectionTask();

	string _ip;//IP��ַ
	unsigned short _port;//�˿ں�
	string _username;//�û���
	string _password;//����
	string _dbname;//���ӵ����ݿ�����
	int initSize;//���ӳس�ʼ��
	int _maxSize;//���ӳ����������
	int _maxIdleTime;//���ӳ�������ʱ��
	int _connectionTimeout;//���ӳػ�ȡ���ӵĳ�ʱʱ��

	queue<Connection*> _connectionQue;//��mysql���ӵĶ��б�
	mutex _queueMutex;//��֤�����̰߳�ȫ�Ļ�����
	atomic_int _connectionCnt; //��¼��������Connection��������atomic�Ǳ�֤�̰߳�ȫ������
	condition_variable cv;//����������������������������֮���ͨ��
	
};

