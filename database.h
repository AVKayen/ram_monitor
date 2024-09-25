#ifndef DATABASE_H
#define DATABASE_H

#include <sql.h>

typedef struct DBConnection
{
    SQLHENV env;
    SQLHDBC dbc;
    SQLHSTMT stmt;
} DBConnection;

void initializeDBConnection(DBConnection *conn, const char *connString);
void closeDBConnection(DBConnection *conn);
void executeSQLQuery(DBConnection *conn, const char *query);

#endif