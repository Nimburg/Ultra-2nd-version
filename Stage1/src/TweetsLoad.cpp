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

// rapid json
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

// function headers
#include "TweetsLoad.h"
#include "DBTable3Comd.h"
#include "DBTable1Comd.h"

// for convenience
using namespace std;
using std::string;

void stripUnicode(string&);
bool invalidChar(char);
string func_TagID(string);

void Tweets_Load(string filename, string server_name, string DBname, string user_name, string user_psw)
{
	/*---------------------------------------------------------------------------*/
	// dealing with data file
	string dist = "../Data/";
	dist.append(filename);
	cout << "\nReading file: " << dist << "\n";
	ifstream file(dist); 
	string tweet_line;
	int count_line = 0;
	bool flag_handle = TRUE; 
	bool flag_while = TRUE;
	int getline_fail_count = 0;
	char End1;
	///////////////////////////////////////////////////////////////////////////////
	while (flag_while)
	{
		count_line++;
		cout << count_line << "\n";
		bool flag_API = FALSE; // whether each tweet were taken complete by API 
		/*-------------------------------------------------------*/
		// tweet line pre-processing
		// check if: 1st, each tweet json message is complete; 2nd, in case there are empty lines in data file;
		if (getline(file, tweet_line))
		{
			getline_fail_count = 0;// reset count
			stripUnicode(tweet_line);// strip unicode
			// API DAQ level check; incomplete tweet
			End1 = tweet_line.back();
			if (End1 == '}')
				flag_API = TRUE;
		}
		else
		{
			getline_fail_count++;
			if (getline_fail_count > 10) // allowance of 10 empty lines; otherwise end of data file
				flag_while = FALSE; // End while loop; End of file reached
		}

		/*-------------------------------------------------------*/
		// tweet main body
		// MUST have data (excluding userID, tweetID, tweetTime) for a tweet to be uploaded to SQL server:
		// further restriction: must have at least one of the following
		// in_reply_to_user_id_str, at least one #, at least one mentioned user
		bool flag_reply_user = FALSE;
		bool flag_Tag_Exist = FALSE;
		bool flag_MenUser_Exist = FALSE;

		// Important Note: rapidjson.GetType() returns 5 for string, 6 for int, 4 for list
		rapidjson::Document tMain; // tweetMain
		string s_tweetTime = "Data Missing";
		string s_tweetID = "Data Missing";
		string s_reply_to_ID = "-1"; // in SQL commands, default value is NULL, thus...
		std::size_t flag_tweet_time = tweet_line.substr(0, 15).find("\"created_at\":");
		std::size_t flag_tweet_idstr = tweet_line.substr(0, 100).find("\"id_str\":");
		if (flag_tweet_time != std::string::npos && flag_tweet_idstr != std::string::npos && flag_API)
		{
			try {
				// load each tweet into json format
				const char * cc_line = tweet_line.c_str(); // string to char* type
				tMain.Parse(cc_line);
				// tweet created time
				rapidjson::Value& tweetTime = tMain["created_at"]; // 
				if (tweetTime.GetType() == 5)
					s_tweetTime = tweetTime.GetString();
				// tweet ID 
				rapidjson::Value& tweetID = tMain["id_str"];
				if (tweetID.GetType() == 5)
					s_tweetID = tweetID.GetString();
				// in_reply_to_user_id
				rapidjson::Value& reply_to_ID = tMain["in_reply_to_user_id_str"];
				if (reply_to_ID.GetType() == 5)
				{
					s_reply_to_ID = reply_to_ID.GetString();
					if (s_reply_to_ID.compare("-1") != 0)  // against default
						flag_reply_user = TRUE;
				}
			}
			catch (...) { cout << "\ntweet main load failure\n"; }
		} // End of tweet main

		/*-------------------------------------------------------*/
		// tweet user information
		// user information is a json format within the tweet main json format
		// rapidjson cannot handle this kind of overlapping json formats
		// thus one has to extract entire user information, convert to const char*, then parse it;
		rapidjson::Document tUser;
		string s_userID = "Data Missing";
		string s_user_created = "Data Missing";
		string s_user_time_zone = "Data Missing";
		int user_followers_count = 0;
		int user_friends_count = 0;
		int user_favourites_count = 0;
		std::size_t flag_user = tweet_line.find("\"user\":{");
		std::size_t flag_count = tweet_line.find("\"followers_count\":");
		if (flag_user != std::string::npos && flag_count != std::string::npos  && flag_API)
		{
			try {
				rapidjson::Value& infor_user = tMain["user"];
				// use the Stringify function to write sub-json to const char*, then parse it
				rapidjson::StringBuffer buffer;
				rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
				infor_user.Accept(writer);
				const char* cc_user = buffer.GetString();
				tUser.Parse(cc_user);
			}
			catch (...) { cout << "\nUser load failure\n"; }
			// user ID
			rapidjson::Value& userID = tUser["id_str"];
			if (userID.GetType() == 5)
				s_userID = userID.GetString();
			// user created at
			rapidjson::Value& user_created = tUser["created_at"];
			if (user_created.GetType() == 5)
				s_user_created = user_created.GetString();
			// user TZ
			rapidjson::Value& user_time_zone = tUser["time_zone"];
			if (user_time_zone.GetType() == 5)
				s_user_time_zone = user_time_zone.GetString();
			// user followers_count
			rapidjson::Value& followers_count = tUser["followers_count"];
			if (followers_count.GetType() == 6)
				user_followers_count = followers_count.GetInt();
			// user friends_count
			rapidjson::Value& friends_count = tUser["friends_count"];
			if (friends_count.GetType() == 6)
				user_friends_count = friends_count.GetInt();
			// user favourites_count
			rapidjson::Value& favourites_count = tUser["favourites_count"];
			if (favourites_count.GetType() == 6)
				user_favourites_count = favourites_count.GetInt();
		}

		/*-------------------------------------------------------*/
		// place information
		// this is also a "json within json", like user information
		// thus one has to extract entire place information, convert to const char*, then parse it;
		rapidjson::Document tPlace;
		string s_placeType = "Data Missing";
		string s_countryCode = "Data Missing";
		std::size_t flag_place = tweet_line.find("\"place\":{");
		std::size_t flag_countryCode = tweet_line.find("\"country_code\":");
		if (flag_place != std::string::npos && flag_countryCode != std::string::npos  && flag_API)
		{
			try {
				rapidjson::Value& infor_place = tMain["place"];
				// use the Stringify function to write sub-json to const char*, then parse it
				rapidjson::StringBuffer buffer;
				rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
				infor_place.Accept(writer);
				const char* cc_place = buffer.GetString();
				tPlace.Parse(cc_place);
			}
			catch (...) {}
			// place_type
			rapidjson::Value& place_type = tPlace["place_type"];
			if (place_type.GetType() == 5)
				s_placeType = place_type.GetString();
			// country_code
			rapidjson::Value& country_code = tPlace["country_code"];
			if (country_code.GetType() == 5)
				s_countryCode = country_code.GetString();
		}

		/*-------------------------------------------------------*/
		// Entities
		// this is also a "json within json"
		// thus one has to extract entire Entities information, convert to const char*, then parse it;
		// Below the "Entities" json format, there are several parts giving tag and mentioning information;
		// Each one of them has json format as well as list format; thus need seperate parsing;
		std::list<string> TagList;
		std::list<string>::iterator Tagit;
		std::list<string> TagIDList;
		std::list<string>::iterator TagIDit;
		std::list<string> MenUserList;
		std::list<string>::iterator Menit;
		rapidjson::Document tEntities;
		std::size_t flag_Entities = tweet_line.find("\"entities\":{");
		if (flag_Entities != std::string::npos  && flag_API)
		{
			try {
				rapidjson::Value& infor_entities = tMain["entities"];
				// use the Stringify function to write sub-json to const char*, then parse it
				rapidjson::StringBuffer buffer;
				rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
				infor_entities.Accept(writer);
				const char* cc_entities = buffer.GetString();
				tEntities.Parse(cc_entities);
			}
			catch (...) {}
			// get tag list
			rapidjson::Document tTags;
			std::size_t flag_Tags = tweet_line.find("\"hashtags\":[{\""); // when it is empty: "hashtags":[],
			if (flag_Tags != std::string::npos)
			{
				// Extract a list<string> of hashtags
				try {
					const rapidjson::Value& List_Tags = tEntities["hashtags"];
					if (List_Tags.GetType() == 4)
					{
						assert(List_Tags.IsArray());
						if (List_Tags.Size() > 0)
							flag_Tag_Exist = TRUE;
						// list<string> to hold each tag
						for (rapidjson::SizeType i = 0; i < List_Tags.Size(); i++)
						{
							// use the Stringify function to write sub-json to const char*, then parse it
							rapidjson::StringBuffer buffer;
							rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
							List_Tags[i].Accept(writer);
							const char* cc_Elem_Tag = buffer.GetString();
							rapidjson::Document Elem_Tag;
							Elem_Tag.Parse(cc_Elem_Tag);
							// now get tag string from {} of each tag
							rapidjson::Value& v_Elem_Tag = Elem_Tag["text"];
							if (v_Elem_Tag.GetType() == 5)
							{
								string s_Elem_Tag = v_Elem_Tag.GetString();
								TagList.push_back(s_Elem_Tag);
							}
						} // End of for loop for hashtags list of entities
						// create ID for tags
						for (TagIDit = TagList.begin(); TagIDit != TagList.end(); ++TagIDit)
						{
							string Cal_ID_Input = *TagIDit;
							string eachTag_ID = func_TagID(Cal_ID_Input);
							TagIDList.push_back(eachTag_ID);
						}
					} // End of if (List_Tags.GetType() != NULL)
				}
				catch (...) {}
			} // End of flag_Tags != std::string::npos
			// get mentioned user list
			rapidjson::Document tMenUsers;
			std::size_t flag_MenUsers = tweet_line.find("\"user_mentions\":[{\"");
			if (flag_MenUsers != std::string::npos)
			{
				// Extract a list<string> of Mentioned Users
				try {
					rapidjson::Value& v_MenUsers = tEntities["user_mentions"];
					if (v_MenUsers.GetType() == 4)
					{
						assert(v_MenUsers.IsArray());
						if (v_MenUsers.Size() > 0)
							flag_MenUser_Exist = TRUE;
						// list<string> to hold each tag
						for (rapidjson::SizeType i = 0; i < v_MenUsers.Size(); i++)
						{
							// use the Stringify function to write sub-json to const char*, then parse it
							rapidjson::StringBuffer buffer;
							rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
							v_MenUsers[i].Accept(writer);
							const char* cc_Men_User = buffer.GetString();
							rapidjson::Document Men_User;
							Men_User.Parse(cc_Men_User);
							// now get tag string from {} of each tag
							rapidjson::Value& v_Men_User = Men_User["id_str"];
							if (v_Men_User.GetType() == 5)
							{
								string s_Men_User = v_Men_User.GetString();
								MenUserList.push_back(s_Men_User);
							}
						} // End of for loop for hashtags list of entities
					}//End of v_MenUsers.GetType() != NULL
				}
				catch (...) {}
			} // End of flag_MenUsers != std::string::npos
		} // End of Entities

		/*-------------------------------------------------------*/
		// End of analyzing each tweet information; End of "json load"-related;

		/*-------------------------------------------------------*/
		// Start loading data into SQL data base
	
		// Condition check for whether to start SQL command or not
		bool flag_tweet_Main = FALSE; // basic tweet check
		bool flag_User_Main = FALSE;  // basic user infor check
		bool flag_User_List = FALSE; // at least one MenUser
		bool flag_Tag_List = FALSE; // at least one tag
		if (flag_API) // each tweet json format is complete;
		{
			if (flag_reply_user || flag_Tag_Exist || flag_MenUser_Exist) // OR condition; at least one type of connection
			{
				if (flag_tweet_time != std::string::npos && flag_tweet_idstr != std::string::npos)
					if (s_tweetTime.compare(s_tweetID) != 0) // default value check
						flag_tweet_Main = TRUE;// tweet_ID, tweet_Time
				if (flag_user != std::string::npos && flag_count != std::string::npos)
					if (s_userID.compare("Data Missing") != 0)
						flag_User_Main = TRUE; // user_ID for tweet		
				// if there is list to upload
				if (flag_Entities != std::string::npos)
				{
					if (MenUserList.size() > 0)
						flag_User_List = TRUE; // user_ID for tweet	
					if (TagList.size() > 0 && TagIDList.size() > 0 && TagList.size() == TagIDList.size())
						flag_Tag_List = TRUE;
				} // list check
			} // 3 condition OR check
		} // API check

		// uploads the rutine comds; Tweet_Stack; User_Unique; UserID_Call
		if (flag_tweet_Main && flag_User_Main)
		{
			cout << "\n    Routine Loading    \n";
			// Commands
			// Tweet_Stack insert
			stringstream Comd1;
			Comd1 << "\
INSERT INTO Tweet_Stack (tweetID, tweetTime, userID, in_reply_to_user_id)\n\
VALUES (" << s_tweetID << ", '" << s_tweetTime << "', " << s_userID << ", " << s_reply_to_ID << " )\n";
			// INSERT User_Unique
			stringstream Comd2;
			Comd2 << "\
INSERT INTO User_Unique (userID, user_created_at, user_followers_count, user_friends_count, user_favourites_count, user_time_zone)\n\
VALUES (" << s_userID << ", '" << s_user_created << "', " << user_followers_count << ", " << user_friends_count << ", " << user_favourites_count << ", '";
			Comd2 << s_user_time_zone << "' )\n";
			// UserID_Call
			stringstream userID_Call;
			userID_Call << "User_" << s_userID;
			stringstream Comd3;
			Comd3 << "\
IF OBJECT_ID(N'dbo." << userID_Call.str() << "', N'U') IS NOT NULL\n\
BEGIN\n\
INSERT INTO " << userID_Call.str() << " (tweetID, UserCall, ReplyID)\n\
VALUES( " << s_tweetID << ", '" << s_tweetTime << "', " << s_reply_to_ID << " )\n\
END\n\
IF OBJECT_ID(N'dbo." << userID_Call.str() << "', N'U') IS NULL\n\
BEGIN\n\
CREATE TABLE " << userID_Call.str() << "\n\
(\n\
tweetID bigint PRIMARY KEY NOT NULL,\n\
UserCall varchar(64) DEFAULT 'Data Missing',\n\
ReplyID bigint DEFAULT - 1,\n\
)\n\
INSERT INTO " << userID_Call.str() << " (tweetID, UserCall, ReplyID)\n\
VALUES ( " << s_tweetID << ", '" << s_tweetTime << "', " << s_reply_to_ID << " )\nEND\n";

			// convert stringstream to string, and load into function
			SQL_DB_3Comd(Comd1.str(), Comd2.str(), Comd3.str(), server_name, DBname, user_name, user_psw);
		} // End of uploads the rutine comds; Tweet_Stack; User_Unique; UserID_Call

		//IF has MenUser
		if (flag_tweet_Main && flag_User_Main && flag_User_List)
		{
			cout << "\n    Mentioned Users List Loading    \n";
			// Commands
			// tweetID_MenUserList Create
			
			// create ID for tags
			stringstream MenUserListValues;
			for (Menit = MenUserList.begin(); Menit != MenUserList.end(); ++Menit)
			{
				MenUserListValues << "( " << *Menit << " ),";
			}
			string MenUserValues = MenUserListValues.str();
			MenUserValues.back() = ' '; // get rid of the last ,
			stringstream MenUserList_tweetID;
			MenUserList_tweetID << "MenUserList_" << s_tweetID;
			stringstream Comd21;
			Comd21 << "\
CREATE TABLE " << MenUserList_tweetID.str() << "\n\
(\n\
tID_MU_ID int idENTITY(1, 1) NOT NULL,\n\
PRIMARY KEY(tID_MU_ID),\n\
MenUserID bigint NOT NULL, \n\
)\n\
INSERT INTO " << MenUserList_tweetID.str() << " (MenUserID)\n\
VALUES " << MenUserValues << "\n";
			// convert stringstream to string, and load into function
			SQL_DB_1Comd(Comd21.str(), server_name, DBname, user_name, user_psw);
		} // END of if (flag_tweet_Main && flag_User_Main && flag_User_List)

		//IF has tag
		// 3 commands for tweetID_TagList, Tag_Unique and TagID_Call;
		// 3 commands within a for loop
		if (flag_tweet_Main && flag_User_Main && flag_Tag_List)
		{
			cout << "\n    Tag List Loading    \n";
			// for loop on TagIDList AMD TagList
			Tagit = TagList.begin();
			TagIDit = TagIDList.begin();
			while (TagIDit != TagIDList.end() && Tagit != TagList.end())
			{
				string current_Tag = *Tagit; // text
				string current_ID = *TagIDit; // bigint in string format
				// comd1
				stringstream Comd31; // tweetID_TagList
				Comd31 << "\
IF NOT EXISTS (SELECT " << current_ID << " FROM Tag_Unique WHERE TagID = " << current_ID << " )\n\
BEGIN\n\
INSERT INTO Tag_Unique(TagID, tag_Text)\n\
VALUES ( " << current_ID << ", '" << current_Tag << "')\n\
END\n";
				// Comd2
				stringstream Tag_TagID;
				Tag_TagID << "Tag_" << current_ID;
				stringstream Comd32;
				Comd32 << "\
IF OBJECT_ID(N'dbo." << Tag_TagID.str() << "', N'U') IS NOT NULL\n\
BEGIN\n\
INSERT INTO " << Tag_TagID.str() << " (tweetID, TagCall, userID, TagText)\n\
VALUES( " << s_tweetID << ", '" << s_tweetTime << "', " << s_userID << ", '" << current_Tag << "' )\n\
END\n\
IF OBJECT_ID(N'dbo." << Tag_TagID.str() << "', N'U') IS NULL\n\
BEGIN\n\
CREATE TABLE " << Tag_TagID.str() << "\n\
(\n\
tweetID bigint PRIMARY KEY NOT NULL,\n\
TagCall varchar(64) DEFAULT 'Data Missing',\n\
userID bigint NOT NULL,\n\
TagText varchar(64),\n\
)\n\
INSERT INTO " << Tag_TagID.str() << " (tweetID, TagCall, userID, TagText)\n\
VALUES( " << s_tweetID << ", '" << s_tweetTime << "', " << s_userID << ", '" << current_Tag << "' )\n\
END\n";
				//Comd3
				stringstream tweetID_TagList;
				tweetID_TagList << "TagList_" << s_tweetID;
				stringstream Comd33;
				Comd33 << "\
IF OBJECT_ID(N'dbo." << tweetID_TagList.str() << "', N'U') IS NOT NULL\n\
BEGIN\n\
INSERT INTO " << tweetID_TagList.str() << " (TagID, TagText)\n\
VALUES (" << current_ID << ", '" << current_Tag << "')\n\
END\n\
IF OBJECT_ID(N'dbo." << tweetID_TagList.str() << "', N'U') IS NULL\n\
BEGIN\n\
CREATE TABLE " << tweetID_TagList.str() << "\n\
(\n\
tID_TL_ID int idENTITY(1, 1) NOT NULL,\n\
PRIMARY KEY(tID_TL_ID),\n\
TagID bigint NOT NULL,\n\
TagText varchar(64),\n\
)\n\
INSERT INTO " << tweetID_TagList.str() << " (TagID, TagText)\n\
VALUES (" << current_ID << ", '" << current_Tag << "')\n\
END\n";
				// convert stringstream to string, and load into function
				SQL_DB_3Comd(Comd31.str(), Comd32.str(), Comd33.str(), server_name, DBname, user_name, user_psw);
				++TagIDit;
				++Tagit;
			}//End of while loop for both taglists
		} // END of if (flag_tweet_Main && flag_User_Main && flag_Tag_List)
		/*-------------------------------------------------------*/
		// End of SQL loading

	} // End of getline while loop
}// End of TweetsLoad();

// strip Unicode
bool invalidChar(char c)
{
	return !(c >= 0 && c <128);
}
void stripUnicode(string & str)
{	
	str.erase(std::remove_if(str.begin(), str.end(), invalidChar), str.end());
}

// calculate each tag's ID, 
// ASC char -> corresponding int on chart -> devide by 64 (thus each letter with at most 2 digits) -> stringstream together
// SQL bigint has 19 digits; usable: 18; means at most convert 9 chars. 
// if the first 9 letters of two tags are same, it is VERY likely they are the same stuff. 
// also converting lower cases to upper cases; Thus tags like jobs and JOBS would share same ID
string func_TagID(string S_Tag)
{
	stringstream Output_Tag;
	for (int i = 0; i < S_Tag.length(); i++)
	{
		int add = (int)S_Tag[i] % 64;
		if (add >= 33 && add <= 58) // convert lower case to upper case; 
			add = add - 32;
		Output_Tag << add; // Int to ASC and append
		if (i == 8) // after converting 9th char
			break;
	}
	return Output_Tag.str();
}

/////////////////////////////////////////////////////////////////////////
