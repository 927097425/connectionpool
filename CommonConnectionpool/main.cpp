#include "test.h"
#include<iostream>
using namespace std;

int main()
{

	cout << "Single thread Time without pool:" << test_for_SingleThread_withoutpool() <<"ms" << endl;
	delete_sql();
	cout << "Single thread Time with pool:" << test_for_SingleThread_withpool() << "ms" << endl;
	delete_sql();
    cout<< "Four thread Time without pool:"  << test_for_FourThread_withoutpool() <<"ms"<< endl;
	delete_sql();
	cout << "Four thread Time with pool:" << test_for_FourThread_withpool() <<"ms"<< endl;
	delete_sql();

	cout << "TestdataSize: " << TestDatasize << endl;
	return 0;

}