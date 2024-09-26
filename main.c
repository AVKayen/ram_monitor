#ifdef _WIN32
#include <windows.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "memory.h"
#include "database.h"

#define PRETTY_BUFFER_SIZE 16

struct timespec UPDATE_INTERVAL = {
    .tv_sec = 1,
    .tv_nsec = 0,
};

void prettify_memory(char *result, int result_size, double bytes)
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
    executeSQLQuery(&conn, "CREATE TABLE my_table (id INT IDENTITY(1,1) PRIMARY KEY, free BIGINT, time DATETIME)");

    char pretty_buffer[PRETTY_BUFFER_SIZE];

    while (1)
    {
        struct memory mem = get_memory();
        char query[256];
        snprintf(query, sizeof(query), "INSERT INTO my_table (free, time) VALUES ('%llu', GETDATE())", mem.free);
        executeSQLQuery(&conn, query);

        char *avg = executeSQLQuery(&conn, "SELECT AVG(free) FROM my_table");

        prettify_memory(pretty_buffer, PRETTY_BUFFER_SIZE, strtod(avg, NULL));

        printf("Average free memory: %s\n", pretty_buffer);

        free(avg);
    }

    closeDBConnection(&conn);

    return 0;
}