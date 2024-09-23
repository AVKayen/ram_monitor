CC = gcc
CFLAGS = -Wall -Wextra -g -lodbc

SRCS = main.c memory.c
OBJS = $(SRCS:.c=.o)
TARGET = program

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -f $(OBJS) $(TARGET)