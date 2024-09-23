## Ram Usage Monitor 
Written in C using system APIs ```<sysinfoapi.h>/<sys/sysinfo.h>``` depending on the platform and uploading data to a cloud DB in an absolute **dumpster fire** of a way.

```main.c``` is the dumpster fire.
```main_old.c``` is the original code that was written in a more sane way :DDDDD

## How to use
1. Compile the code using ```make```
2. Export the environment variables ```DB_SERV```, ```DB_USER```, ```DB_PASS``` with the appropriate values. The database is an SQL database.
3. Run the executable using ```./ram_monitor```