#ifndef SQL1Comd1List_H    // To make sure you don't declare the function more than once by including the header multiple times.
#define SQL1Comd1List_H

#include <fstream>
#include <string>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <windows.h>
#include <unordered_map>
#include <list>
#include <chrono>
#include <thread>

// sql server interfacing
#include <sqlext.h>
#include <sqltypes.h>
#include <sql.h>

using namespace std;
using std::string;

list<long long int> SQL_1Comd_1List(string Comd1, list<long long int> &input, string server_name, string DBname, string user_name, string user_psw);

#endif
