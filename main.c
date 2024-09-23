#include <stdio.h>
#include <stdlib.h>
#include <sql.h>
#include <sqlext.h>
#include <string.h>
#include <time.h>
#include "memory.h"

struct timespec UPDATE_INTERVAL = {
    .tv_sec = 1,
    .tv_nsec = 0,
};

void prettify_memory(double bytes, char *result, int result_size)
{
    char *units[] = {"B", "KiB", "MiB", "GiB", "TiB", "PiB"};
    int i = 0;
    while (bytes >= 1024)
    {
        bytes /= 1024;
        i++;
    }
    snprintf(result, result_size, "%.3f %s", bytes, units[i]);
}

// Define the connection string
#define CONNECTION_STRING "Driver={ODBC Driver 18 for SQL Server};Server={SERV};Database=azure_database;Uid={USER};Pwd={PASS};Encrypt=yes;TrustServerCertificate=no;Connection Timeout=30;"
// replace {SERV}, {USER}, {PASS} with your server, username, password from .env file


void handleError(SQLHANDLE handle, SQLSMALLINT handleType, SQLHENV env, SQLHDBC dbc, SQLHSTMT stmt, const char *customMessage)
{
    
    SQLCHAR sqlState[6], errorMsg[SQL_MAX_MESSAGE_LENGTH];
    SQLINTEGER nativeError;
    SQLSMALLINT msgLen;
    SQLGetDiagRec(handleType, handle, 1, sqlState, &nativeError, errorMsg, sizeof(errorMsg), &msgLen);
    printf("%s. SQL State: %s, Error Message: %s\n", customMessage, sqlState, errorMsg);

    if (handleType == SQL_HANDLE_STMT)
    {
        SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    }
    if (dbc != SQL_NULL_HDBC)
    {
        SQLDisconnect(dbc);
        SQLFreeHandle(SQL_HANDLE_DBC, dbc);
    }
    if (env != SQL_NULL_HENV)
    {
        SQLFreeHandle(SQL_HANDLE_ENV, env);
    }

    exit(1);
}

int main()
{
    char* serv = getenv("SERV");
    char* user = getenv("USER_SQL");
    char* pass = getenv("PASS");
    SQLCHAR connString[1024];
    snprintf((char *)connString, sizeof(connString), "Driver={ODBC Driver 18 for SQL Server};Server=%s;Database=azure_database;Uid=%s;Pwd=%s;Encrypt=yes;TrustServerCertificate=no;Connection Timeout=30;", serv, user, pass);
    printf("Connecting to the database\n");
    printf("Connection string: %s\n", connString);

    SQLHENV env = SQL_NULL_HENV;
    SQLHDBC dbc = SQL_NULL_HDBC;
    SQLHSTMT stmt = SQL_NULL_HSTMT;
    SQLRETURN ret;

    ret = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &env);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO)
    {
        handleError(SQL_NULL_HANDLE, SQL_HANDLE_ENV, env, dbc, stmt, "Failed to allocate environment handle");
    }

    ret = SQLSetEnvAttr(env, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO)
    {
        handleError(env, SQL_HANDLE_ENV, env, dbc, stmt, "Failed to set ODBC version");
    }

    ret = SQLAllocHandle(SQL_HANDLE_DBC, env, &dbc);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO)
    {
        handleError(dbc, SQL_HANDLE_DBC, env, dbc, stmt, "Failed to allocate connection handle");
    }

    ret = SQLDriverConnect(dbc, NULL, connString, SQL_NTS, NULL, 0, NULL, SQL_DRIVER_COMPLETE);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO)
    {
        handleError(dbc, SQL_HANDLE_DBC, env, dbc, stmt, "Failed to connect to the database");
    }
    printf("Connected to the database\n");

    ret = SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO)
    {
        handleError(stmt, SQL_HANDLE_STMT, env, dbc, stmt, "Failed to allocate statement handle");
    }

    // clear the database
    ret = SQLExecDirect(stmt, (SQLCHAR *)"DROP TABLE IF EXISTS my_table", SQL_NTS);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO)
    {
        handleError(stmt, SQL_HANDLE_STMT, env, dbc, stmt, "Failed to drop table");
    }

    ret = SQLExecDirect(stmt, (SQLCHAR *)"CREATE TABLE my_table (id INT IDENTITY(1,1) PRIMARY KEY, free VARCHAR(255), time DATETIME)", SQL_NTS);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO)
    {
        handleError(stmt, SQL_HANDLE_STMT, env, dbc, stmt, "Failed to create table");
    }

    
    char buffer[16];

    while (1)
    {
        struct memory mem = get_memory();
        prettify_memory((double)mem.free, buffer, sizeof(buffer));
        char query[256];
        snprintf(query, sizeof(query), "INSERT INTO my_table (free, time) VALUES ('%s', GETDATE())", buffer);
        ret = SQLExecDirect(stmt, (SQLCHAR *)query, SQL_NTS);
        if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO)
        {
            handleError(stmt, SQL_HANDLE_STMT, env, dbc, stmt, "Failed to insert data into the table");
        }
        nanosleep(&UPDATE_INTERVAL, NULL);
    }
    free(buffer);

    // get all the rows from the table
    ret = SQLExecDirect(stmt, (SQLCHAR *)"SELECT * FROM my_table", SQL_NTS);

    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    SQLDisconnect(dbc);
    SQLFreeHandle(SQL_HANDLE_DBC, dbc);
    SQLFreeHandle(SQL_HANDLE_ENV, env);

    return 0;
}