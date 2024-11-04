CC = gcc
CFLAGS = -lpthread
TARGET = multi_process_launcher

SRC = multi_process_launcher.c test_multi_process_launcher.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET) log.txt

full_clean: clean
	rm -rf *.dSYM

build_and_clean: all clean

build_and_full_clean: all full_clean