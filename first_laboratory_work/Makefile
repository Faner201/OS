TARGET = main

CC = g++

CFLAGS = -Wall -g

SRC = main.cpp

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)
	rm -rf *.dSYM

build_and_clean: all clean