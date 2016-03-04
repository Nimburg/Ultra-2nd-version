#ifndef SQL1Comd1Res_H    // To make sure you don't declare the function more than once by including the header multiple times.
#define SQL1Comd1Res_H

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

long long int SQL_1Comd_1Res(string Comd1, string server_name, string DBname, string user_name, string user_psw);

#endif
