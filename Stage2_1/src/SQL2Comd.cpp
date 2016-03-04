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
#include <tuple>

// sql server interfacing
#include <sqlext.h>
#include <sqltypes.h>
#include <sql.h>

#include "SQL2Comd.h"

using namespace std;

// IMPORTANT !! exception 0xC0000005 disabled


void SQL_2Comd(string Comd1, string Comd2, string server_name, string DBname, string user_name, string user_psw)
{

	bool flag_Connection = TRUE;
	bool flag_Error = FALSE;
	
	SQLHANDLE sqlenvhandle;
	SQLHANDLE sqlconnectionhandle;
	SQLHANDLE sqlstatementhandle;
	SQLRETURN retcode;

	/*---------------------------------------------------------------------*/
	// SQL Commands
	// Re-Connect
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
			SQL_DRIVER_NOPROMPT))
		{
		case SQL_SUCCESS_WITH_INFO:
			//std::cout << "  1. SQL_SUCCESS_WITH_INFO\n";
			break;
		case SQL_INVALID_HANDLE:
			//std::cout << "  2. SQL_INVALID_HANDLE\n";
			flag_Error = TRUE;
		case SQL_ERROR:
			//std::cout << "  3. SQL_ERROR\n";
			flag_Error = TRUE;
		default:
			break;
		}// End of connection switch
	} // End of if (flag_Connection = TRUE)
	if (SQL_SUCCESS != SQLAllocHandle(SQL_HANDLE_STMT, sqlconnectionhandle, &sqlstatementhandle))
	{
		//std::cout << "  4. SQLAllocHandle\n";
		flag_Error = TRUE;
	}
	// Command 1
	int SQLreturn1_int;
	if (SQL_SUCCESS != SQLExecDirect(sqlstatementhandle, (SQLCHAR*)Comd1.c_str(), SQL_NTS)) {
		std::cout << "  Command 1\n";
		flag_Error = TRUE;
	}
	// Command 2
	SQLCHAR retconstring2[1024];
	stringstream Comd_connection2;
	Comd_connection2 << "DRIVER={SQL Server};SERVER={" << server_name << "};DATABASE={" << DBname << "\
};Trusted_connection={yes};user_id={" << user_name << "};password={" << user_psw << "};";
	if (flag_Connection = TRUE)
	{
		switch (SQLDriverConnect(sqlconnectionhandle,
			NULL,
			/*---------------------------------------------------------------------*/
			(SQLCHAR*)(Comd_connection2.str()).c_str(),
			/*---------------------------------------------------------------------*/
			SQL_NTS,
			retconstring2,
			1024,
			NULL,
			SQL_DRIVER_NOPROMPT))
		{
		case SQL_SUCCESS_WITH_INFO:
			//std::cout << "  1. SQL_SUCCESS_WITH_INFO\n";
			break;
		case SQL_INVALID_HANDLE:
			//std::cout << "  2. SQL_INVALID_HANDLE\n";
			flag_Error = TRUE;
		case SQL_ERROR:
			//std::cout << "  3. SQL_ERROR\n";
			flag_Error = TRUE;
		default:
			break;
		}// End of connection switch
	} // End of if (flag_Connection = TRUE)
	if (SQL_SUCCESS != SQLAllocHandle(SQL_HANDLE_STMT, sqlconnectionhandle, &sqlstatementhandle))
	{
		//std::cout << "  4. SQLAllocHandle\n";
		flag_Error = TRUE;
	}
	int SQLreturn2_int;
	if (SQL_SUCCESS != SQLExecDirect(sqlstatementhandle, (SQLCHAR*)Comd2.c_str(), SQL_NTS)) {
		std::cout << "  Command 2\n";
		flag_Error = TRUE;
	}
	// clear handles
	if (flag_Connection == TRUE)
	{
		SQLFreeHandle(SQL_HANDLE_STMT, sqlstatementhandle);
		SQLDisconnect(sqlconnectionhandle);
		SQLFreeHandle(SQL_HANDLE_DBC, sqlconnectionhandle);
		SQLFreeHandle(SQL_HANDLE_ENV, sqlenvhandle);
	}
}