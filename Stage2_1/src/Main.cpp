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
#include <tuple>

// sql server interfacing
#include <sqlext.h>
#include <sqltypes.h>
#include <sql.h>

// function headers
#include "TagRecal.h"

using namespace std;
using std::string;

int main();

int main()
{
	// IMPORTANT !! exception 0xC0000005 disabled
	// As I understand it, this is due to a bug of ODBC package. 
	
	// recalculate statistical variables for all hashtags
	// This part of programs operates on the first data base, particularly the table "Tag_Unique"
	// But it output new tables into the second data base 
	
	// You should have created the Data Base that you are tring connecting to!
	// You should also have set up your SQL server properly, according to this:
	// http://superuser.com/questions/730994/default-sa-password-of-sql-server-automatically-installed-by-sharepoint-server-2
	
	string server_name = "";
	string DBname1 = "";
	string DBname2 = "";
	string user_name = "";
	string user_psw = "";

	// Note: under current setup, only part of tags are selected into 2nd data base
	int thre_call = 5;
	int thre_userdegree = 2;
	// select those tags into the second data base, which: has >= 5 calls && >= 2 user_degrees;
	TagRecal(server_name, DBname1, DBname2, user_name, user_psw, thre_call, thre_userdegree);

	//  Terminate.
	std::cout << "\n";
	std::cout << "  Normal end of execution.\n";
	std::cin.get();
	return 0;
}

