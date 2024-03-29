#pragma once
#include"Connection.h"
#include"CommonConnectionpool.h"
#include<ctime>

#define TestDatasize 1000
using namespace std;

clock_t test_for_SingleThread_withoutpool();
clock_t test_for_SingleThread_withpool();
clock_t test_for_FourThread_withoutpool();
clock_t test_for_FourThread_withpool();
void delete_sql();
