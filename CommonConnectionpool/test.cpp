#include"test.h"


clock_t test_for_SingleThread_withoutpool()
{
	clock_t withoutpool_begin = clock();
	for (int i = 0; i < TestDatasize; i++)
	{
		Connection conn;
		conn.connect("127.0.0.1", 3306, "root", "524628796", "chat");
		char sql[1024] = { 0 };
		sprintf(sql, "insert into user(name,age,sex) values('%s','%d','%s')", "zhangsan", 19, "male");
		conn.update(sql);
	}
	clock_t withoutpool_end = clock();
	return withoutpool_end - withoutpool_begin;
}
clock_t test_for_SingleThread_withpool()
{
	clock_t pool_begin = clock();
	Connectionpool* cp = Connectionpool::getInstance();
	for (int i = 0; i < TestDatasize; i++)
	{
		shared_ptr<Connection> sp = cp->getConnection();
		char sql[1024] = { 0 };
		sprintf(sql, "insert into user(name,age,sex) values('%s','%d','%s')", "zhangsan", 19, "male");
		sp->update(sql);
	}
	clock_t pool_end = clock();
	return pool_end - pool_begin;
}
clock_t test_for_FourThread_withoutpool()
{
	clock_t withoutpool_begin = clock();
	mutex _connectionMutex;//防止重复用同个端口链接mysql
	thread t1([&]() {
		for (int i = 0; i < TestDatasize / 4; i++)
		{
			Connection conn;
			char sql[1024] = { 0 };
			sprintf(sql, "insert into user(name,age,sex) values('%s','%d','%s')", "zhangsan", 19, "male");
			if (true)
			{
				unique_lock<mutex> lock(_connectionMutex);
				conn.connect("127.0.0.1", 3306, "root", "524628796", "chat");
				conn.update(sql);
			}
		}
		});
	thread t2([&]() {

		for (int i = 0; i < TestDatasize / 4; i++)
		{
			Connection conn;
			char sql[1024] = { 0 };
			sprintf(sql, "insert into user(name,age,sex) values('%s','%d','%s')", "zhangsan", 19, "male");
			if (true)
			{
				unique_lock<mutex> lock(_connectionMutex);
				conn.connect("127.0.0.1", 3306, "root", "524628796", "chat");
				conn.update(sql);
			}
		}
		});

	thread t3([&]() {

		for (int i = 0; i < TestDatasize / 4; i++)
		{
			Connection conn;
			char sql[1024] = { 0 };
			sprintf(sql, "insert into user(name,age,sex) values('%s','%d','%s')", "zhangsan", 19, "male");
			if (true)
			{
				unique_lock<mutex> lock(_connectionMutex);
				conn.connect("127.0.0.1", 3306, "root", "524628796", "chat");
				conn.update(sql);
			}
		}
		});

	thread t4([&]() {

		for (int i = 0; i < TestDatasize / 4; i++)
		{
			Connection conn;
			char sql[1024] = { 0 };
			sprintf(sql, "insert into user(name,age,sex) values('%s','%d','%s')", "zhangsan", 19, "male");
			if (true)
			{
				unique_lock<mutex> lock(_connectionMutex);
				conn.connect("127.0.0.1", 3306, "root", "524628796", "chat");
				conn.update(sql);
			}
		}
		});

	t1.join();
	t2.join();
	t3.join();
	t4.join();

	clock_t withoutpool_end = clock();
	return withoutpool_end - withoutpool_begin;
}
clock_t test_for_FourThread_withpool()
{
	clock_t pool_begin = clock();
	Connectionpool* cp = Connectionpool::getInstance();
	thread t1([&]() {
		for (int i = 0; i < TestDatasize / 4; i++)
		{
			
			shared_ptr<Connection> sp = cp->getConnection();
			char sql[1024] = { 0 };
			sprintf(sql, "insert into user(name,age,sex) values('%s','%d','%s')", "zhangsan", 19, "male");
			sp->update(sql);
		}
	});
	thread t2([&]() {
		for (int i = 0; i < TestDatasize / 4; i++)
		{
	
			shared_ptr<Connection> sp = cp->getConnection();
			char sql[1024] = { 0 };
			sprintf(sql, "insert into user(name,age,sex) values('%s','%d','%s')", "zhangsan", 19, "male");
			sp->update(sql);
		}
		});

	thread t3([&]() {
		for (int i = 0; i < TestDatasize / 4; i++)
		{

			shared_ptr<Connection> sp = cp->getConnection();
			char sql[1024] = { 0 };
			sprintf(sql, "insert into user(name,age,sex) values('%s','%d','%s')", "zhangsan", 19, "male");
			sp->update(sql);
		}
		});

	thread t4([&]() {
		for (int i = 0; i < TestDatasize / 4; i++)
		{
			shared_ptr<Connection> sp = cp->getConnection();
			char sql[1024] = { 0 };
			sprintf(sql, "insert into user(name,age,sex) values('%s','%d','%s')", "zhangsan", 19, "male");
			sp->update(sql);
		}
		});

	t1.join();
	t2.join();
	t3.join();
	t4.join();

	clock_t pool_end = clock();
	return pool_end - pool_begin;
}

void delete_sql()
{
	Connectionpool* cp = Connectionpool::getInstance();
	shared_ptr<Connection> sp = cp->getConnection();
	char sql[1024] = { 0 };
	sprintf(sql, "truncate table user");
	sp->update(sql);
}