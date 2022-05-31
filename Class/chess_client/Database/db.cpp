#include <stdlib.h>
#include <stdio.h>
#include <windows.h>
#include <sqlext.h>
#include <iostream>

int main()
{
	RETCODE valid;
	HENV environment{};
	HDBC handle{};
	HSTMT statement{};

	valid = SQLAllocEnv(&environment);
	if (SQL_SUCCESS == valid || SQL_SUCCESS_WITH_INFO == valid)
	{
		std::cout << "The environment is allocated.\n";
	}

	valid = SQLAllocHandle(SQL_HANDLE_ENV, environment, &handle);
	if (SQL_SUCCESS == valid || SQL_SUCCESS_WITH_INFO == valid)
	{
		std::cout << "The handle is allocated.\n";
	}

	valid = SQLAllocConnect(environment, &handle);
	if (SQL_SUCCESS == valid || SQL_SUCCESS_WITH_INFO == valid)
	{
		std::cout << "The connection is done.\n";
	}
	else
	{
		std::cout << "The connection was not done.\n";
		return 1;
	}

	auto server_name = (SQLWCHAR*)(L"Classroom Server 2022-1");
	valid = SQLConnect(handle
		, server_name
		, SQL_NTS
		, NULL, 0, NULL, 0);

	if (SQL_SUCCESS == valid || SQL_SUCCESS_WITH_INFO == valid)
	{
		std::cout << "DB connected\n";

		valid = SQLAllocStmt(handle, &statement);
		if (SQL_SUCCESS == valid || SQL_SUCCESS_WITH_INFO == valid)
		{
			std::cout << "The statement is initialized.\n";
		}
		else
		{
			return 1;
		}
	}
	else
	{
		return 1;
	}
	auto my_query = (SQLWCHAR*)(L"SELECT user.id, user.nickname FROM Users;");
	// SQL_NTS: Null terminated string
	valid = SQLPrepare(statement, my_query, SQL_NTS);

	// ODBC 라이브러리에 결과 값이 전달된다.
	//auto my_execute = (SQLWCHAR*)(L"TestMethod()");

	SQLINTEGER my_order_id = 0;
	SQLCHAR my_id[10]{};
	SQLCHAR my_nickname[30]{};

	//valid = SQLExecDirect(statement, my_execute, 100);
	if (SQL_SUCCESS == valid || SQL_SUCCESS_WITH_INFO == valid)
	{
		SQLBindCol(statement, 0, SQL_C_CHAR, my_id, 10, 0);
		SQLBindCol(statement, 0, SQL_C_CHAR, my_nickname, 30, 0);
	}

	valid = SQLFetch(statement);
	if (SQL_SUCCESS == valid || SQL_SUCCESS_WITH_INFO == valid)
	{
		std::cout << "The query is fetched.\n";
	}
	else
	{
		std::cout << "The query was not fetched.\n";
	}

	std::cout << "id: " << my_id << "\n";
	std::cout << "nickname: " << my_nickname << "\n";


	return 0;
}