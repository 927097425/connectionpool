#pragma once
/*
ʵ��mysql���ݿ����ɾ�Ĳ�
*/
#include <mysql.h>
#include <string>
#include<ctime>
using namespace std;
#include "public.h"
// ���ݿ������
class Connection
{
public:
	// ��ʼ�����ݿ�����
	Connection();
	// �ͷ����ݿ�������Դ
	~Connection();

	// �������ݿ�
	bool connect(string ip, unsigned short port, string user, string password,
		string dbname);

	// ���²��� insert��delete��update
	bool update(string sql);
	// ��ѯ���� select
	MYSQL_RES* query(string sql);
	//ˢ�����ӵ���ʼ�Ŀ���ʱ���
	void refreshAliveTime() { _alivetime = clock(); }
	clock_t getAliveTime() const{ return clock() - _alivetime; }
private:
	MYSQL* _conn; // ��ʾ��MySQL Server��һ������
	clock_t _alivetime;//��¼�������״̬�����ӳأ�����ʼʱ��
};