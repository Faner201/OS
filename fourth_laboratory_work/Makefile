TARGETS = temperature_reader temperature_simulator

CC = g++

CFLAGS = -Wall -g

all: $(TARGETS)

temperature_reader: temperature_reader.cpp
	$(CC) $(CFLAGS) -o temperature_reader temperature_reader.cpp

temperature_simulator: temperature_simulator.cpp
	$(CC) $(CFLAGS) -o temperature_simulator temperature_simulator.cpp

clean:
	rm -rf *.dSYM

full_clean: clean
	rm -f $(TARGETS)

build_and_clean: all clean

build_and_full_clean: all full_clean
