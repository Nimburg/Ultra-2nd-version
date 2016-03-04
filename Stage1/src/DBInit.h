#ifndef DBInit_H    // To make sure you don't declare the function more than once by including the header multiple times.
#define DBInit_H

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

// sql server interfacing
#include <sqlext.h>
#include <sqltypes.h>
#include <sql.h>

using namespace std;
using std::string;

void SQL_DB_Init(string server_name, string DBname, string user_name, string user_psw);

#endif
