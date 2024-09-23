CC = gcc
CFLAGS = -Wall -Wextra -g -lodbc

SRCS = main.c memory.c
OBJS = $(SRCS:.c=.o)
TARGET = ram_monitor

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^
	rm -f $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -f $(TARGET)