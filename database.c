#include <stdio.h>
#include <stdlib.h>
#include <sql.h>
#include <sqlext.h>
#include "database.h"

void handleError(SQLHANDLE handle, SQLSMALLINT handleType, DBConnection *conn, const char *customMessage)
{
    SQLCHAR sqlState[6], errorMsg[SQL_MAX_MESSAGE_LENGTH];
    SQLINTEGER nativeError;
    SQLSMALLINT msgLen;
    SQLRETURN diagRet;

    diagRet = SQLGetDiagRec(handleType, handle, 1, sqlState, &nativeError, errorMsg, sizeof(errorMsg), &msgLen);

    if (diagRet == SQL_SUCCESS || diagRet == SQL_SUCCESS_WITH_INFO)
    {
        printf("%s. SQL State: %s, Error Message: %s\n", customMessage, sqlState, errorMsg);
    }
    else
    {
        printf("%s. Unable to retrieve detailed error information.\n", customMessage);
    }

    if (handleType == SQL_HANDLE_STMT)
    {
        SQLFreeHandle(SQL_HANDLE_STMT, conn->stmt);
    }
    if (conn->dbc != SQL_NULL_HDBC)
    {
        SQLDisconnect(conn->dbc);
        SQLFreeHandle(SQL_HANDLE_DBC, conn->dbc);
    }
    if (conn->env != SQL_NULL_HENV)
    {
        SQLFreeHandle(SQL_HANDLE_ENV, conn->env);
    }

    exit(1);
}

void initializeDBConnection(DBConnection *conn, const char *connString)
{
    SQLRETURN ret;

    ret = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &conn->env);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO)
    {
        handleError(SQL_NULL_HANDLE, SQL_HANDLE_ENV, conn, "Failed to allocate environment handle");
    }

    ret = SQLSetEnvAttr(conn->env, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO)
    {
        handleError(&conn->env, SQL_HANDLE_ENV, conn, "Failed to set ODBC version");
    }

    ret = SQLAllocHandle(SQL_HANDLE_DBC, conn->env, &conn->dbc);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO)
    {
        handleError(conn->dbc, SQL_HANDLE_DBC, conn, "Failed to allocate connection handle");
    }

    ret = SQLDriverConnect(conn->dbc, NULL, (SQLCHAR*)connString, SQL_NTS, NULL, 0, NULL, SQL_DRIVER_COMPLETE);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO)
    {
        handleError(conn->dbc, SQL_HANDLE_DBC, conn, "Failed to connect to the database");
    }
    printf("Connected to the database\n");

    ret = SQLAllocHandle(SQL_HANDLE_STMT, conn->dbc, &conn->stmt);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO)
    {
        handleError(conn->stmt, SQL_HANDLE_STMT, conn, "Failed to allocate statement handle");
    }
}

void closeDBConnection(DBConnection *conn)
{
    SQLFreeHandle(SQL_HANDLE_STMT, conn->stmt);
    SQLDisconnect(conn->dbc);
    SQLFreeHandle(SQL_HANDLE_DBC, conn->dbc);
    SQLFreeHandle(SQL_HANDLE_ENV, conn->env);
}

void executeSQLQuery(DBConnection *conn, const char *query)
{
    SQLRETURN ret = SQLExecDirect(conn->stmt, (SQLCHAR*)query, SQL_NTS);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO)
    {
        handleError(conn->stmt, SQL_HANDLE_STMT, conn, "Failed to execute query");
    }
}