#ifdef _WIN32
#include <windows.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <sql.h>
#include <sqlext.h>
#include <string.h>
#include "database.h"

#define BUFFER_SIZE 256

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

    ret = SQLDriverConnect(conn->dbc, NULL, (SQLCHAR *)connString, SQL_NTS, NULL, 0, NULL, SQL_DRIVER_COMPLETE);
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

char *executeSQLQuery(DBConnection *conn, const char *query)
{
    SQLRETURN ret = SQLExecDirect(conn->stmt, (SQLCHAR *)query, SQL_NTS);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO)
    {
        handleError(conn->stmt, SQL_HANDLE_STMT, conn, "Failed to execute query");
        return NULL;
    }

    char *result = (char *)malloc(BUFFER_SIZE);
    if (!result)
    {
        fprintf(stderr, "Failed to allocate memory for result\n");
        return NULL;
    }
    result[0] = '\0';

    if (strstr(query, "SELECT") != NULL)
    {
        ret = SQLFetch(conn->stmt);
        if (ret == SQL_NO_DATA)
        {
            free(result);
            return NULL;
        }
        else if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO)
        {
            handleError(conn->stmt, SQL_HANDLE_STMT, conn, "Failed to fetch data");
            free(result);
            return NULL;
        }

        SQLCHAR buffer[BUFFER_SIZE];
        SQLLEN indicator;
        ret = SQLGetData(conn->stmt, 1, SQL_C_CHAR, buffer, sizeof(buffer), &indicator);
        if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO)
        {
            handleError(conn->stmt, SQL_HANDLE_STMT, conn, "Failed to fetch data");
            free(result);
            return NULL;
        }

        strncpy(result, (char *)buffer, BUFFER_SIZE - 1);
        result[BUFFER_SIZE - 1] = '\0';

        SQLCloseCursor(conn->stmt);
    }

    return result;
}