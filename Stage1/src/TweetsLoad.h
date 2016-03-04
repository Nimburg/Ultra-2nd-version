#ifndef TweetsLoad_H    // To make sure you don't declare the function more than once by including the header multiple times.
#define TweetsLoad_H 

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

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

// sql server interfacing
#include <sqlext.h>
#include <sqltypes.h>
#include <sql.h>


using namespace std;
using std::string;
//void Tweets_Load(string filename, SQLHANDLE&, SQLHANDLE&, SQLHANDLE&, SQLRETURN&);

void Tweets_Load(string filename, string server_name, string DBname, string user_name, string user_psw);

#endif
