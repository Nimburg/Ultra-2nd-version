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
#include "TagNetwork.h"
#include "SQL1Comd1Res.h"
#include "SQL1Comd1List.h"
#include "SQL1Comd.h"

// for convenience
using namespace std;
using std::string;
using namespace std::this_thread; // sleep_for, sleep_until
using namespace std::chrono; // nanoseconds, system_clock, seconds

void TagNetwork(string Input_tagID, string server_name, string DBname1, string DBname2, string user_name, string user_psw)
{
	/*---------------------------------------------------------------------------*/
	// Connecting to the 2nd data base
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
	Comd_connection << "DRIVER={SQL Server};SERVER={" << server_name << "};DATABASE={" << DBname2 << "\
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
	// get the list of TagIDs, userIDs, timestamps from the 2nd data base just connected to;
	std::list<long long int> UserIDList;
	std::list<long long int>::iterator UserIDit;
	long long int userIDholder;
	std::list<long long int> tweetIDList;
	std::list<long long int>::iterator tweetIDit;
	long long int tweetIDholder;
	std::list<string> timeList;
	std::list<string>::iterator timeit;
	char timeholder[64];
	// extract userID and tweetID lists for the tag given
	stringstream Comd_Tag;
	Comd_Tag << "\
SELECT userID, tweetID, TagCall\n\
FROM Tag_" << Input_tagID << "\n";
	if (SQL_SUCCESS != SQLExecDirect(sqlstatementhandle, (SQLCHAR*)(Comd_Tag.str()).c_str(), SQL_NTS)) {
		cout << "\nFailed to collect ID information" << endl;
		// clear handles
		SQLFreeHandle(SQL_HANDLE_STMT, sqlstatementhandle);
		SQLDisconnect(sqlconnectionhandle);
		SQLFreeHandle(SQL_HANDLE_DBC, sqlconnectionhandle);
		SQLFreeHandle(SQL_HANDLE_ENV, sqlenvhandle);
	}
	else {
		while (SQLFetch(sqlstatementhandle) == SQL_SUCCESS) {
			SQLGetData(sqlstatementhandle, 1, SQL_C_SBIGINT, &userIDholder, 0, NULL);
			UserIDList.push_back(userIDholder);
			SQLGetData(sqlstatementhandle, 2, SQL_C_SBIGINT, &tweetIDholder, 0, NULL);
			tweetIDList.push_back(tweetIDholder);
			SQLGetData(sqlstatementhandle, 3, SQL_C_CHAR, timeholder, 64, NULL);
			timeList.push_back(timeholder);
		}//End of while
		 // clear handles
		SQLFreeHandle(SQL_HANDLE_STMT, sqlstatementhandle);
		SQLDisconnect(sqlconnectionhandle);
		SQLFreeHandle(SQL_HANDLE_DBC, sqlconnectionhandle);
		SQLFreeHandle(SQL_HANDLE_ENV, sqlenvhandle);
	}// End of extracting tagID list
	cout << "\nNumber of userIDs taken: " << UserIDList.size() << endl;

	/*---------------------------------------------------------------------*/
	// create a new table; each row as a link of network; for the tag given
	stringstream Comd_TagNetwork;
	Comd_TagNetwork << "\
CREATE TABLE Network_" << Input_tagID << "\n\
(\n\
link_ID int idENTITY (1,1) NOT NULL,\n\
PRIMARY KEY(link_ID), \n\
LStart bigint NOT NULL,\n\
LEnd bigint DEFAULT -1,\n\
LTime varchar(64) DEFAULT 'Data Missing',\n\
LinkType int DEFAULT -1,\n\
)\n";
	stringstream NetworkTable;
	NetworkTable << "Network_" << Input_tagID;
	// SQL; 1 comd no return
	SQL_1Comd(Comd_TagNetwork.str(), server_name, DBname2, user_name, user_psw);

	/*---------------------------------------------------------------------------*/
	// Get Call_Freq for each tag
	int count = 0;
	tweetIDit = tweetIDList.begin();
	timeit = timeList.begin();
	for (UserIDit = UserIDList.begin(); UserIDit != UserIDList.end(); ++UserIDit)
	{
		cout << ++count << endl;
		long long int current_user_ID = *UserIDit;
		long long int current_tweet_ID = *tweetIDit;
		string current_time = *timeit;
		// next tweetID
		++tweetIDit;
		++timeit;

		/*---------------------------------------------------------------------------*/
		// check Tweet_Stack for reply_to_userID
		// if got it, update to table "Network_tagID"
		long long int reply_to_userID = -1;
		stringstream Comd11;
		Comd11 << "\
SELECT in_reply_to_user_id\n\
FROM Tweet_Stack\n\
WHERE tweetID = " << current_tweet_ID << "\n";
		// SQL; return bigint
		reply_to_userID = SQL_1Comd_1Res(Comd11.str(), server_name, DBname2, user_name, user_psw);
		// update to Network table
		if (reply_to_userID != -1 && reply_to_userID != 0)
		{
			cout << "handling reply userID" << endl;
			stringstream Comd12;
			Comd12 << "\
INSERT INTO " << NetworkTable.str() << " (LStart, LEnd, LTime, LinkType)\n\
VALUES (" << current_user_ID << ", " << reply_to_userID << ", '" << current_time << "', 2)\n";
			// SQL; 1 comd no return
			SQL_1Comd(Comd12.str(), server_name, DBname2, user_name, user_psw);
		}//End of updating network table

		/*---------------------------------------------------------------------------*/
		// check MenUserList_"tagID"  for Mentioned user;
		// This part uses the first data base
		stringstream Comd21;
		Comd21 << "\
SELECT MenUserID\n\
FROM MenUserList_" << current_tweet_ID << "\n";
		// SQL; return list of bigint
		std::list<long long int> MenUserList;
		std::list<long long int>::iterator MenUserit;
		MenUserList = SQL_1Comd_1List(Comd21.str(), MenUserList, server_name, DBname1, user_name, user_psw);
		// check the list and update
		if (MenUserList.size() > 0)
		{
			cout << "handling mentioned userID" << endl;
			for (MenUserit = MenUserList.begin(); MenUserit != MenUserList.end(); ++MenUserit)
			{
				stringstream Comd22;
				long long int MenIDholder = *MenUserit;
				Comd22 << "\
INSERT INTO " << NetworkTable.str() << " (LStart, LEnd, LTime, LinkType)\n\
VALUES (" << current_user_ID << ", " << MenIDholder << ", '" << current_time << "', 1)\n";
				//SQL 1 comd no return
				SQL_1Comd(Comd22.str(), server_name, DBname2, user_name, user_psw);
			}// End of for loop updating MenUserLinks
		}//End of if (MenUserList.size() > 0)

		// If neither reply nor mentioned other users
		if (reply_to_userID == 0  && MenUserList.size() == 0)
		{
			cout << "handling standalone userID" << endl;
			stringstream Comd31;
			Comd31 << "\
INSERT INTO " << NetworkTable.str() << " (LStart, LEnd, LTime, LinkType)\n\
VALUES (" << current_user_ID << ", -1, '" << current_time << "', 0)\n";
			//SQL 1 comd no return
			SQL_1Comd(Comd31.str(), server_name, DBname2, user_name, user_psw);
		}// End of standalone case
	}//End of for loop on userID list
}