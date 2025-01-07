#include <ctime>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>
#include <memory>
#include "../services/database_manager.hpp"

#ifdef _WIN32
#include <windows.h>
#else
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#endif

namespace {

#ifdef _WIN32
HANDLE OpenSerialPort(const char* port_name) {
  HANDLE h_serial = CreateFile(port_name, GENERIC_READ, 0, 0, OPEN_EXISTING,
                               FILE_ATTRIBUTE_NORMAL, 0);
  if (h_serial == INVALID_HANDLE_VALUE) {
    std::cerr << "Error opening COM port\n";
  }
  return h_serial;
}

void ConfigureSerialPort(HANDLE h_serial) {
  DCB dcb_serial_params = {0};
  dcb_serial_params.DCBlength = sizeof(dcb_serial_params);
  if (!GetCommState(h_serial, &dcb_serial_params)) {
    std::cerr << "Error getting COM state\n";
    return;
  }

  dcb_serial_params.BaudRate = CBR_9600;
  dcb_serial_params.ByteSize = 8;
  dcb_serial_params.StopBits = ONESTOPBIT;
  dcb_serial_params.Parity = NOPARITY;

  if (!SetCommState(h_serial, &dcb_serial_params)) {
    std::cerr << "Error setting COM state\n";
  }
}

void CloseSerialPort(HANDLE h_serial) {
  CloseHandle(h_serial);
}

void ReadTemperature(HANDLE h_serial, double* temperature) {
  char buffer[256];
  DWORD bytes_read;
  if (ReadFile(h_serial, buffer, sizeof(buffer), &bytes_read, NULL) &&
      bytes_read > 0) {
    buffer[bytes_read] = '\0';
    *temperature = atof(buffer);
  }
}

#else

int OpenSerialPort(const char* port_name) {
  int serial_port = open(port_name, O_RDONLY);
  if (serial_port < 0) {
    std::cerr << "Error opening serial port\n";
  }
  return serial_port;
}

void ConfigureSerialPort(int serial_port) {
  struct termios tty;
  memset(&tty, 0, sizeof tty);

  if (tcgetattr(serial_port, &tty) != 0) {
    std::cerr << "Error getting tty attributes\n";
    return;
  }

  cfsetospeed(&tty, B9600);
  cfsetispeed(&tty, B9600);

  tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;
  tty.c_iflag &= ~IGNBRK;
  tty.c_lflag = 0;
  tty.c_oflag = 0;
  tty.c_cc[VMIN] = 1;
  tty.c_cc[VTIME] = 5;

  tty.c_iflag &= ~(IXON | IXOFF | IXANY);
  tty.c_cflag |= (CLOCAL | CREAD);
  tty.c_cflag &= ~(PARENB | PARODD);
  tty.c_cflag |= 0;
  tty.c_cflag &= ~CSTOPB;
  tty.c_cflag &= ~CRTSCTS;

  if (tcsetattr(serial_port, TCSANOW, &tty) != 0) {
    std::cerr << "Error setting tty attributes\n";
  }
}

void CloseSerialPort(int serial_port) {
  close(serial_port);
}

void ReadTemperature(int serial_port, double* temperature) {
  char buffer[256];
  int n = read(serial_port, buffer, sizeof(buffer));
  if (n > 0) {
    buffer[n] = '\0';
    *temperature = atof(buffer);
  }
}

#endif

double CalculateAverage(const std::vector<double>& temperatures) {
  double sum = 0.0;
  for (size_t i = 0; i < temperatures.size(); ++i) {
    sum += temperatures[i];
  }
  return sum / temperatures.size();
}

void RunTemperatureMonitor(pqxx::connection& conn) {
  std::vector<double> hourly_temperatures;
  std::vector<double> daily_averages;

  std::chrono::steady_clock::time_point start_time =
      std::chrono::steady_clock::now();

  while (true) {
    double current_temperature;
#ifdef _WIN32
    HANDLE h_serial;
    do {
      h_serial = OpenSerialPort("COM4");
      if (h_serial == INVALID_HANDLE_VALUE) {
        std::cerr << "Waiting for COM port...\n";
        Sleep(1500);
      }
    } while (h_serial == INVALID_HANDLE_VALUE);
    ConfigureSerialPort(h_serial);
    ReadTemperature(h_serial, &current_temperature);
    CloseSerialPort(h_serial);
#else
    int serial_port;
    std::chrono::steady_clock::time_point wait_start =
        std::chrono::steady_clock::now();
    int count_reload = 10;
    do {
      serial_port = OpenSerialPort("/dev/ttys010");
      if (serial_port < 0) {
        count_reload--;
        std::cerr << "Waiting for serial port...\n";
        usleep(1500000);
      }
      if (count_reload == 0) {
        std::cerr << "Sorry, the waiting time has expired";
        return;
      }
    } while (serial_port < 0);
    ConfigureSerialPort(serial_port);
    ReadTemperature(serial_port, &current_temperature);
    CloseSerialPort(serial_port);
#endif

    std::chrono::system_clock::time_point now =
        std::chrono::system_clock::now();
    std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);
    std::tm now_tm = *std::localtime(&now_time_t);
    std::ostringstream oss;
    oss << std::put_time(&now_tm, "%Y-%m-%d %H:%M:%S");
    std::string timestamp = oss.str();

    std::cout << "Reading temperature..." << std::endl;

    WriteToDatabase(conn, "temperature_log", timestamp, current_temperature);
    std::cout << "Temperature written to database." << std::endl;

    hourly_temperatures.push_back(current_temperature);

    std::chrono::steady_clock::time_point current_time =
        std::chrono::steady_clock::now();
    if (std::chrono::duration_cast<std::chrono::hours>(current_time -
                                                       start_time)
            .count() >= 1) {
      double hourly_average = CalculateAverage(hourly_temperatures);
      std::cout << "Hourly average calculated: " << hourly_average << std::endl;
      WriteToDatabase(conn, "hourly_average_log", timestamp, hourly_average);
      std::cout << "Hourly average written to database." << std::endl;

      daily_averages.push_back(hourly_average);
      hourly_temperatures.clear();
      start_time = current_time;
    }

    if (daily_averages.size() == 24) {
      double daily_average = CalculateAverage(daily_averages);
      std::cout << "Daily average calculated: " << daily_average << std::endl;
      WriteToDatabase(conn, "daily_average_log", timestamp, daily_average);
      std::cout << "Daily average written to database." << std::endl;
      daily_averages.clear();
    }
  }
}

}

void RunTemperatureReader() {
    initializeDatabase();

    auto conn = ConnectToDatabase();
    if (!conn) {
        std::cerr << "Failed to establish database connection. Exiting..." << std::endl;
        return;
    }

    RunTemperatureMonitor(*conn);
}
