## Ram Usage Monitor 
Written in C using system APIs ```<sysinfoapi.h>/<sys/sysinfo.h>``` depending on the platform and uploading data to a cloud DB.

```main.c``` - Main program

```main_old.c``` - Main program, without SQL

```database.c|h``` - Database logic

```memory.c|h``` - RAM reading logic

## Requirements
1. gcc compiler
2. ODBC driver (e.g. ```ODBC Driver 18 for SQL Server```) | ```unixODBC``` (for linux target)
3. mingw-w64 (for windows target)

## How to use
1. Compile the code using ```make``` on linux. If you want to target windows, you can use ```PLATFORM=WINDOWS```. If you want older version without SQL, use ```OLD=1```.
2. Export the environment variables ```DB_SERV```, ```DB_USER```, ```DB_PASS```, ```DB_USER``` with the appropriate values. The database is an SQL database driven by ODBC.
3. Run the executable using ```./ram_monitor```