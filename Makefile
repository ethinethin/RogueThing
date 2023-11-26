TARGET=nrl
CC?=clang
CFLAGS=-I./lib -Wall -Wextra -Wpedantic -lm -lcurses
CFILES=./src/*.c

default:
	$(CC) $(CFLAGS) -o $(TARGET) $(CFILES)
clean:
	rm -f $(TARGET)
debug:
	$(CC) $(CFLAGS) -o $(TARGET) -g $(CFILES)
run: default
	./$(TARGET)
all: default
