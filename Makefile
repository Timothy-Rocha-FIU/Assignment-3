# Makefile for building the PA3 project

CC = gcc
CFLAGS = -Wall -static -lm
TARGET = PA3
SRC = PA3.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)