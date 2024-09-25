#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "memory.h"
#include "database.h"

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

int main()
{
    char *serv = getenv("DB_SERV");
    char *name = getenv("DB_NAME");
    char *user = getenv("DB_USER");
    char *pass = getenv("DB_PASS");
    const char connString[1024];
    snprintf((char *)connString, sizeof(connString), "Driver={ODBC Driver 18 for SQL Server};Server=%s;Database=%s;Uid=%s;Pwd=%s;Encrypt=yes;TrustServerCertificate=no;Connection Timeout=30;", serv, name, user, pass);
    printf("Connecting to the database\n");

    DBConnection conn = {SQL_NULL_HENV, SQL_NULL_HDBC, SQL_NULL_HSTMT};

    initializeDBConnection(&conn, connString);

    executeSQLQuery(&conn, "DROP TABLE IF EXISTS my_table");
    executeSQLQuery(&conn, "CREATE TABLE my_table (id INT IDENTITY(1,1) PRIMARY KEY, free VARCHAR(255), time DATETIME)");

    char buffer[16];

    while (1)
    {
        struct memory mem = get_memory();
        prettify_memory((double)mem.free, buffer, sizeof(buffer));
        char query[256];
        snprintf((char *)query, sizeof(query), "INSERT INTO my_table (free, time) VALUES ('%s', GETDATE())", buffer);
        executeSQLQuery(&conn, query);
        nanosleep(&UPDATE_INTERVAL, NULL);
    }

    closeDBConnection(&conn);

    return 0;
}