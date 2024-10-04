#ifdef _WIN32
#include <windows.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "memory.h"
#include "database.h"

#define PRETTY_BUFFER_SIZE 16

// Interval of memory status update => upload cycle
const struct timespec UPDATE_INTERVAL = {
    .tv_sec = 1,
    .tv_nsec = 0,
};

// Get environment variable, exit the program if it isn't defined
char *get_env_check_null(const char *name)
{
    char *value = getenv(name);
    if (value == NULL)
    {
        printf("Must export environment variable: %s\n", name);
        exit(1);
    }
    return value;
}

// Convert memory, in bytes, to a pretty format e.g. 2.5GiB
void prettify_memory(char *result, int result_size, unsigned long long bytes)
{
    char *units[] = {"B", "KiB", "MiB", "GiB", "TiB", "PiB"}; // "future-proofing"
    int i = 0;
    double f_bytes = bytes;
    while (f_bytes >= 1024)
    {
        f_bytes /= 1024;
        i++;
    }
    snprintf(result, result_size, "%.3f %s", f_bytes, units[i]);
}

int main(void)
{
    char *DB_SERV = get_env_check_null("DB_SERV");
    char *DB_NAME = get_env_check_null("DB_NAME");
    char *DB_USER = get_env_check_null("DB_USER");
    char *DB_PASS = get_env_check_null("DB_PASS");

    const char DRIVER[] = "ODBC Driver 18 for SQL Server";

    const char connString[1024];
    snprintf((char *)connString, sizeof(connString), "Driver={%s};Server=%s;Database=%s;Uid=%s;Pwd=%s;Encrypt=yes;TrustServerCertificate=no;Connection Timeout=30;", DRIVER, DB_SERV, DB_NAME, DB_USER, DB_PASS);
    printf("Connecting to the database\n");

    DBConnection conn = {SQL_NULL_HENV, SQL_NULL_HDBC, SQL_NULL_HSTMT};

    initializeDBConnection(&conn, connString);

    executeSQLQuery(&conn, "DROP TABLE IF EXISTS my_table");
    executeSQLQuery(&conn, "CREATE TABLE my_table (id INT IDENTITY(1,1) PRIMARY KEY, free BIGINT, time DATETIME)");

    char avarage_buffer[PRETTY_BUFFER_SIZE];
    char used_buffer[PRETTY_BUFFER_SIZE];
    struct timespec start, end;

    while (1)
    {
        clock_gettime(CLOCK_MONOTONIC, &start);

        MemoryInfo mem = get_memory();
        char query[256];
        snprintf(query, sizeof(query), "INSERT INTO my_table (free, time) VALUES ('%llu', GETDATE())", mem.total - mem.free);
        executeSQLQuery(&conn, query);

        char *avg = executeSQLQuery(&conn, "SELECT AVG(free) FROM my_table");
        prettify_memory(avarage_buffer, PRETTY_BUFFER_SIZE, strtoull(avg, NULL, 10));
        prettify_memory(used_buffer, PRETTY_BUFFER_SIZE, mem.total - mem.free);

        printf("Current used memory: %s; Average used memory: %s\n", used_buffer, avarage_buffer);

        free(avg);

        clock_gettime(CLOCK_MONOTONIC, &end);

        struct timespec sleep_time = {
            .tv_sec = UPDATE_INTERVAL.tv_sec - (end.tv_sec - start.tv_sec),
            .tv_nsec = UPDATE_INTERVAL.tv_nsec - (end.tv_nsec - start.tv_nsec),
        };

        if (sleep_time.tv_nsec <= 0)
        {
            sleep_time.tv_nsec += 1000000000;
            sleep_time.tv_sec -= 1;
        }

        if (sleep_time.tv_sec >= 0)
            nanosleep(&sleep_time, NULL);
    }

    closeDBConnection(&conn);

    return 0;
}