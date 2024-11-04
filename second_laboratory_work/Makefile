TARGET = process_launcher

CC = gcc

CFLAGS = -Wall -g

SRC = process_launcher.c test_process_launcher.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -rf *.dSYM

full_clean: clean
	rm -f $(TARGET)

build_and_clean: all clean

build_and_full_clean: all full_clean
