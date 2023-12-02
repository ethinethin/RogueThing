TARGET=nrl
CC?=clang
CFLAGS=-I./lib -Wall -Wextra -Wpedantic -lm -lncurses
CFILES=./src/*.c

default:
	$(CC) -o $(TARGET) $(CFILES) $(CFLAGS)
clean:
	rm -f $(TARGET)
debug:
	$(CC) -o $(TARGET) -g $(CFILES) $(CFLAGS)
run: default
	./$(TARGET)
all: default
