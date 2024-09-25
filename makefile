ifeq ($(PLATFORM),WINDOWS)
	CC = x86_64-w64-mingw32-gcc
	CFLAGS = -Wall -Wextra -g
	LDFLAGS = -lodbc32
	TARGET = ram_monitor.exe
else
	CC = gcc
	CFLAGS = -Wall -Wextra -g -lodbc
	LDFLAGS = -lodbc
	TARGET = ram_monitor
endif

ifeq ($(OLD),1)
	SRCS = main_old.c memory.c database.c
else
	SRCS = main.c memory.c database.c
endif

OBJS = $(SRCS:.c=.o)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -f $(TARGET) $(OBJS)