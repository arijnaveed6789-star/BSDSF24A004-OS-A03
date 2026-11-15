LDLIBS = -lreadline
CC = gcc
CFLAGS = -Wall -Wextra -Iinclude

SRC = src/main.c src/shell.c src/execute.c src/builtins.c src/history.c src/jobs.c
OBJ = obj/main.o obj/shell.o obj/execute.o obj/builtins.o obj/history.o obj/jobs.o
TARGET = bin/psh

all: $(TARGET)

$(TARGET): $(OBJ)
	mkdir -p bin
	$(CC) $(OBJ) -o $(TARGET) $(LDLIBS)

obj/%.o: src/%.c include/shell.h
	mkdir -p obj
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf obj/*.o bin/psh

.PHONY: all clean
