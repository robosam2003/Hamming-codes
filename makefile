CC = gcc
CFLAGS = -Wall -Wextra -O2 -I./${HEADERS}
TARGET = hamming
SRCS = src/main.c 
HEADERS = src/colours.h


all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS)

clean:
	rm -f $(TARGET)

.PHONY: all clean