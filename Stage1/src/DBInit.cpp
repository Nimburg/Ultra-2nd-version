// load raw data as json, add to SQL tables
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

#include "DBInit.h"

using namespace std;


/*---------------------------------------------------------------------*/
void show_error(unsigned int handletype, const SQLHANDLE& handle) {
	SQLCHAR sqlstate[1024];
	SQLCHAR message[1024];
	if (SQL_SUCCESS == SQLGetDiagRec(handletype, handle, 1, sqlstate, NULL, message, 1024, NULL))
		cout << "Message: " << message << "\nSQLSTATE: " << sqlstate << endl;
}

void SQL_DB_Init(string server_name, string DBname, string user_name, string user_psw) 
{
	SQLHANDLE sqlenvhandle;
	SQLHANDLE sqlconnectionhandle;
	SQLHANDLE sqlstatementhandle;
	SQLRETURN retcode;

	bool flag_Connection = TRUE;
	bool flag_HandleClean  = FALSE;
	bool flag_Error = FALSE;

	if (SQL_SUCCESS != SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &sqlenvhandle))
		flag_Connection = FALSE;
	if (SQL_SUCCESS != SQLSetEnvAttr(sqlenvhandle, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0))
		flag_Connection = FALSE;
	if (SQL_SUCCESS != SQLAllocHandle(SQL_HANDLE_DBC, sqlenvhandle, &sqlconnectionhandle))
		flag_Connection = FALSE;
	// connecting to SQL server
	SQLCHAR retconstring[1024];
	stringstream Comd_connection;
	Comd_connection << "DRIVER={SQL Server};SERVER={" << server_name << "};DATABASE={" << DBname << "\
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
			show_error(SQL_HANDLE_DBC, sqlconnectionhandle);
			break;
		case SQL_INVALID_HANDLE:
			std::cout << "  2. SQL_INVALID_HANDLE\n";
			flag_Error = TRUE;
		case SQL_ERROR:
			show_error(SQL_HANDLE_DBC, sqlconnectionhandle);
			std::cout << "  3. SQL_ERROR\n";
			flag_Error = TRUE;
		default:
			break;
		}// End of connection switch
	} // End of if (flag_Connection = TRUE)
	
	//
	if (SQL_SUCCESS != SQLAllocHandle(SQL_HANDLE_STMT, sqlconnectionhandle, &sqlstatementhandle))
	{
		show_error(SQL_HANDLE_STMT, sqlstatementhandle);
		flag_Error = TRUE;
	}
	
	/*---------------------------------------------------------------------*/
	// SQL Commands
	SQLCHAR* SQL_Query; 
	// create table User_Unique
	SQL_Query = (SQLCHAR*)"\
CREATE TABLE User_Unique\n\
(\n\
userID bigint PRIMARY KEY NOT NULL,\n\
user_created_at varchar(64) DEFAULT 'user_created_at Missing',\n\
user_followers_count bigint DEFAULT 0,\n\
user_friends_count bigint DEFAULT 0,\n\
user_favourites_count bigint DEFAULT 0,\n\
user_time_zone varchar(64) DEFAULT 'user_time_zone Missing',\n\
user_Var_Tag int DEFAULT 0,\n\
user_Var_Connection int DEFAULT 0,\n\
user_Tweet_Freq bigint DEFAULT 0,\n\
user_Mention_Freq bigint DEFAULT 0,\n\
user_GotMention_Freq bigint DEFAULT 0,\n\
user_Reply_Freq bigint DEFAULT 0,\n\
user_Connection_Count bigint DEFAULT 0,\n\
user_Tag_Freq bigint DEFAULT 0,\n\
user_Tag_Count bigint DEFAULT 0,\n\
)\n\
CREATE INDEX index_userID\n\
ON User_Unique(userID)\n";
	if (SQL_SUCCESS != SQLExecDirect(sqlstatementhandle, SQL_Query, SQL_NTS)) {
		std::cout << "  Command 1\n";
		flag_Error = TRUE;
	}
	
	// create table Tag_Unique
	SQL_Query = (SQLCHAR*)"\
CREATE TABLE Tag_Unique\n\
(\n\
TagUnique_ID int idENTITY(1, 1) NOT NULL,\n\
PRIMARY KEY(TagUnique_ID),\n\
TagID bigint NOT NULL DEFAULT - 1,\n\
tag_Text varchar(64) DEFAULT 'Data Missing',\n\
Tag_Var_Connection bigint DEFAULT 0,\n\
Tag_Var_User bigint DEFAULT 0,\n\
Tag_Call_Freq bigint DEFAULT 0,\n\
Tag_Connection_Degree bigint DEFAULT 0,\n\
Tag_Connection_Freq bigint DEFAULT 0,\n\
Tag_User_Degree bigint DEFAULT 0,\n\
Tag_User_Freq bigint DEFAULT 0,\n\
)\n\
CREATE INDEX index_TagID\n\
ON Tag_Unique(TagID)\n";
	if (SQL_SUCCESS != SQLExecDirect(sqlstatementhandle, SQL_Query, SQL_NTS)) {
		std::cout << "  Command 2\n";
		flag_Error = TRUE;
	}

	// create table Tweet_Stack
	SQL_Query = (SQLCHAR*)"\
CREATE TABLE Tweet_Stack\n\
(\n\
tweetID bigint PRIMARY KEY NOT NULL,\n\
tweetTime varchar(64) NOT NULL,\n\
tweetText TEXT DEFAULT 'Data Missing',\n\
userID bigint NOT NULL,\n\
in_reply_to_user_id bigint DEFAULT - 1,\n\
)";
	// Last command, clear handle
	if (SQL_SUCCESS == SQLExecDirect(sqlstatementhandle, SQL_Query, SQL_NTS))
	{
		SQLFreeHandle(SQL_HANDLE_STMT, sqlstatementhandle);
		SQLDisconnect(sqlconnectionhandle);
		SQLFreeHandle(SQL_HANDLE_DBC, sqlconnectionhandle);
		SQLFreeHandle(SQL_HANDLE_ENV, sqlenvhandle);
	}
	else 
	{
		std::cout << "  Command 3\n";
		flag_Error = TRUE;
		// clear handles anyway
		SQLFreeHandle(SQL_HANDLE_STMT, sqlstatementhandle);
		SQLDisconnect(sqlconnectionhandle);
		SQLFreeHandle(SQL_HANDLE_DBC, sqlconnectionhandle);
		SQLFreeHandle(SQL_HANDLE_ENV, sqlenvhandle);	
	}
		
	if (flag_Error == TRUE)
		cout << "\n\n		Something went Wrong!!!			\n\n";

}