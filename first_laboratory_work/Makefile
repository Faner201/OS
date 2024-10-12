TARGET = main

CC = g++

CFLAGS = -Wall -g

SRC = main.cpp

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -rf *.dSYM

full_clean: clean
	rm -f $(TARGET)

build_and_clean: all clean

build_and_full_clean: all full_clean
