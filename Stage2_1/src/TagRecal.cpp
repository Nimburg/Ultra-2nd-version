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
#include <algorithm>
#include <cctype>
#include <chrono>
#include <thread>
#include <tuple>

// function headers
#include "TagRecal.h"
#include "SQL2Comd2Res.h"
#include "SQL2Comd.h"

// for convenience
using namespace std;
using std::string;

void TagRecal(string server_name, string DBname1, string DBname2, string user_name, string user_psw, int thre_call, int thre_userdegree)
{
	/*---------------------------------------------------------------------------*/
	// Connecting to data base UStweets
	SQLHANDLE sqlenvhandle;
	SQLHANDLE sqlconnectionhandle;
	SQLHANDLE sqlstatementhandle;
	SQLRETURN retcode;

	bool flag_Connection = TRUE;
	bool flag_HandleClean = FALSE;

	if (SQL_SUCCESS != SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &sqlenvhandle))
		flag_Connection = FALSE;
	if (SQL_SUCCESS != SQLSetEnvAttr(sqlenvhandle, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0))
		flag_Connection = FALSE;
	if (SQL_SUCCESS != SQLAllocHandle(SQL_HANDLE_DBC, sqlenvhandle, &sqlconnectionhandle))
		flag_Connection = FALSE;
	// connecting to SQL server
	SQLCHAR retconstring[1024];
	stringstream Comd_connection;
	Comd_connection << "DRIVER={SQL Server};SERVER={" << server_name << "};DATABASE={" << DBname1 << "\
};Trusted_connection={yes};user_id={" << user_name << "};password={" << user_psw << "};";
	if (flag_Connection = TRUE)
	{
		switch (SQLDriverConnect(sqlconnectionhandle,
			NULL,
			/*---------------------------------------------------------------------*/
			(SQLCHAR*)(Comd_connection.str()).c_str(),
			/*---------------------------------------------------------------------*/
			SQL_NTS,
			retconstring,
			1024,
			NULL,
			SQL_DRIVER_NOPROMPT)) {
		case SQL_SUCCESS_WITH_INFO:
			std::cout << "  1. SQL_SUCCESS_WITH_INFO\n";
			flag_HandleClean = TRUE; // need to clear handles in the end
			break;
		case SQL_INVALID_HANDLE:
			std::cout << "  2. SQL_INVALID_HANDLE\n";
		case SQL_ERROR:
			std::cout << "  3. SQL_ERROR\n";
		default:
			break;
		}// End of connection switch
	} // End of if (flag_Connection = TRUE)
	//
	if (SQL_SUCCESS != SQLAllocHandle(SQL_HANDLE_STMT, sqlconnectionhandle, &sqlstatementhandle))
	{
		cout << "\nConnecting to data base UStweets failed" << endl;
		std::cin.get();
	}

	/*---------------------------------------------------------------------*/
	// get the list of TagIDs;
	std::list<long long int> TagIDList;
	std::list<long long int>::iterator TagIDit;
	long long int IDholder;
	stringstream Comd_TagList;
	Comd_TagList << "SELECT TagID FROM Tag_Unique";

	if (SQL_SUCCESS != SQLExecDirect(sqlstatementhandle, (SQLCHAR*)(Comd_TagList.str()).c_str(), SQL_NTS)) {
		cout << "\nConnecting to data base UStweets failed" << endl;
		// clear handles
		SQLFreeHandle(SQL_HANDLE_STMT, sqlstatementhandle);
		SQLDisconnect(sqlconnectionhandle);
		SQLFreeHandle(SQL_HANDLE_DBC, sqlconnectionhandle);
		SQLFreeHandle(SQL_HANDLE_ENV, sqlenvhandle);
	}
	/*---------------------------------------------------------------------*/
	else {
		while (SQLFetch(sqlstatementhandle) == SQL_SUCCESS) {
			SQLGetData(sqlstatementhandle, 1, SQL_C_SBIGINT, &IDholder, 0, NULL);
			TagIDList.push_back(IDholder);
		}//End of while
		// clear handles
		SQLFreeHandle(SQL_HANDLE_STMT, sqlstatementhandle);
		SQLDisconnect(sqlconnectionhandle);
		SQLFreeHandle(SQL_HANDLE_DBC, sqlconnectionhandle);
		SQLFreeHandle(SQL_HANDLE_ENV, sqlenvhandle);
	}// End of extracting tagID list
	cout << "\nNumber of Tags taken: " << TagIDList.size() << endl;

	/*---------------------------------------------------------------------------*/
	// Get Call_Freq and User_Degree for each tag
	// call_freq is the total number of calls a hashtag received
	// user_degree is the number of distinct users ever called this hashtag
	int count = 0;
	for (TagIDit = TagIDList.begin(); TagIDit != TagIDList.end(); ++TagIDit)
	{
		cout << ++count << endl;
		
		long long int current_ID = *TagIDit;
		// results
		long long int call_freq = 0; // default value
		long long int user_degree =0;
		// check if TagID_Table exists; if yes, get call_freq, user_degree;
		// 2 Comds; 1 call; 2 returns;
		stringstream TagID_table;
		TagID_table << "Tag_" << current_ID;
		stringstream Comd21; // get call freq;
		Comd21 << "\
IF OBJECT_ID(N'dbo." << TagID_table.str() << "', N'U') IS NOT NULL\n\
BEGIN\n\
SELECT COUNT(*)\n\
FROM " << TagID_table.str() << "\n\
END\n";
		stringstream Comd22; // get user degree; num of unique users
		Comd22 << "\
IF OBJECT_ID(N'dbo." << TagID_table.str() << "', N'U') IS NOT NULL\n\
BEGIN\n\
SELECT COUNT(DISTINCT userID)\n\
FROM " << TagID_table.str() << "\n\
END\n";
		// sent to query
		tie(call_freq, user_degree) = SQL_2Comd_2Res(Comd21.str(), Comd22.str(), server_name, DBname1, user_name, user_psw);
		if (call_freq >= thre_call && user_degree >= thre_userdegree)
		{
			cout << "Call Frequency: " << call_freq << endl;
			cout << "User degree: " << user_degree << endl;

			// update to Tag_Unique; AND Transfer tag_ID table into new data base
			// 2 commands, 1 call, no return
			stringstream Comd23;// update call_freq and user_degree
			Comd23 << "\
UPDATE Tag_Unique\n\
SET Tag_Call_Freq = " << call_freq << ", Tag_User_Degree = " << user_degree << "\n\
WHERE TagID = " << current_ID << "\n";
			stringstream Comd24;// Transfer tag_ID table into new data base
			Comd24 << "\
IF OBJECT_ID(N'dbo." << TagID_table.str() << "', N'U') IS NOT NULL\n\
BEGIN\n\
SELECT *\n\
INTO " << DBname2 << ".dbo." << TagID_table.str() << "\n\
FROM " << TagID_table.str() << "\n\
END\n";
			// send to query
			SQL_2Comd(Comd23.str(), Comd24.str(), server_name, DBname1, user_name, user_psw);
		}

	} // End of for loop; End of going through TagIDList 
	/*-------------------------------------------------------*/
	// End of SQL loading
}// End of TweetsLoad();

