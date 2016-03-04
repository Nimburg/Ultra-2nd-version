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

// rapidjson libs for json read
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

// sql server interfacing
#include <sqlext.h>
#include <sqltypes.h>
#include <sql.h>

// function headers
#include "TweetsLoad.h"
#include "DBInit.h"
#include "DBTable3Comd.h"
#include "DBTable1Comd.h"

using namespace std;
using std::string;

int main();

int main()
{
	// Through all 3 stages of the program, you are going to need 2 seperate data bases
	// For Stage1, we only need the first one of the two data bases

	// You should have created the Data Base that you are tring connecting to!
	// You should also have set up your SQL server properly, according to this:
	// http://superuser.com/questions/730994/default-sa-password-of-sql-server-automatically-installed-by-sharepoint-server-2
	
	string server_name = "FanYu-HC\\SQLEXPRESS";
	string DBname1 = "DBdemo1";
	string user_name = "sa";
	string user_psw = "fanyu01";

	// IMPORTANT !! exception 0xC0000005 disabled
	// As I understand it, this is due to a bug of ODBC package. 

	// Initialize tabls of the data base you are going to use
	//// in order of: server_name, DB_name, user_name, user_pass word
	SQL_DB_Init(server_name, DBname1, user_name, user_psw);
	
	// read data file line by line
	// using Tabls of the first data base
	// data file should be located in the folder \Data
	string filename = "tweets.txt";
	Tweets_Load(filename, server_name, DBname1, user_name, user_psw);

	//  Terminate.
	std::cout << "\n";
	std::cout << "  Normal end of execution.\n";
	std::cin.get();
	return 0;
}

