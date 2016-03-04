#ifndef SQL2Comd2Res_H    // To make sure you don't declare the function more than once by including the header multiple times.
#define SQL2Comd2Res_H

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

tuple<long long int, long long int> SQL_2Comd_2Res(string Comd1, string Comd2, string server_name, string DBname, string user_name, string user_psw);

#endif
