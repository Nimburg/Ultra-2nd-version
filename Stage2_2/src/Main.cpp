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
#include "TagNetwork.h"

using namespace std;
using std::string;

int main();

int main()
{
	// IMPORTANT !! exception 0xC0000005 disabled
	// As I understand it, this is due to a bug of ODBC package. 
	
	// You should have created the Data Base that you are tring connecting to!
	// You should also have set up your SQL server properly, according to this:
	// http://superuser.com/questions/730994/default-sa-password-of-sql-server-automatically-installed-by-sharepoint-server-2

	// This part of program establishs user network (expressed by links) of a specific hashtag
	// Several tables (User_Unique, Tweet_Stack, Tag_Unique) are transferred to the 2nd data base MANUALLY
	// Tag_Unique is transfered AFTER TagRecal() updates call_freq and user_degree
	
	string server_name = "";
	string DBname1 = "";
	string DBname2 = "";
	string user_name = "";
	string user_psw = "";
	
	// tagID = 205053454850484954; tagText = Trump2016
	string tagID = "205053454850484954";
	TagNetwork(tagID, server_name, DBname1, DBname2, user_name, user_psw);

	//  Terminate.
	std::cout << "\n";
	std::cout << "  Normal end of execution.\n";
	std::cin.get();
	return 0;
}

