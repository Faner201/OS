#include <ctime>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>

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
  std::string data;
  int n;

  while ((n = read(serial_port, buffer, sizeof(buffer) - 1)) > 0) {
    buffer[n] = '\0';
    data += buffer;

    if (n < sizeof(buffer) - 1) {
      break;
    }
  }

  if (!data.empty()) {
    *temperature = atof(data.c_str());
  }
}

#endif

std::unordered_map<std::string, int> log_line_count;

void WriteToLog(const std::string& filename, const std::string& data) {
  std::ofstream log_file(filename, std::ios::app);
  if (log_file.is_open()) {
    log_file << data << std::endl;
    log_file.close();
    log_line_count[filename]++;
  }
}

double CalculateAverage(const std::vector<double>& temperatures) {
  double sum = 0.0;
  for (size_t i = 0; i < temperatures.size(); ++i) {
    sum += temperatures[i];
  }
  return sum / temperatures.size();
}

void ManageLogFile(const std::string& filename, int max_entries) {
  if (log_line_count[filename] > max_entries) {
    std::ifstream file(filename);
    if (!file.is_open()) return;

    std::vector<std::string> lines;
    std::string line;
    while (std::getline(file, line)) {
      lines.push_back(line);
    }
    file.close();

    std::ofstream out_file(filename);
    for (size_t i = lines.size() - max_entries; i < lines.size(); ++i) {
      out_file << lines[i] << std::endl;
    }
    out_file.close();

    log_line_count[filename] = max_entries;
  }
}

void ClearLogFile(const std::string& filename,
                  const std::chrono::hours& duration) {
  static std::map<std::string, std::chrono::steady_clock::time_point>
      last_clear_time;

  std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
  if (last_clear_time.find(filename) == last_clear_time.end()) {
    last_clear_time[filename] = now;
  }

  if (now - last_clear_time[filename] >= duration) {
    std::ofstream log_file(filename, std::ios::trunc);
    if (log_file.is_open()) {
      log_file.close();
      std::cout << "Cleared log file: " << filename << std::endl;
    }
    last_clear_time[filename] = now;
  }
}

void RunTemperatureMonitor() {
  std::vector<double> hourly_temperatures;
  std::vector<double> daily_averages;
  std::string temp_log_filename = "temperature_log.txt";
  std::string hourly_average_log_filename = "hourly_average_log.txt";
  std::string daily_average_log_filename = "daily_average_log.txt";

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
      serial_port = OpenSerialPort("/dev/ttys006");
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
    oss << std::put_time(&now_tm, "%Y:%m:%d %H:%M:%S") << ": "
        << current_temperature << "°C";
    std::string temp_log_entry = oss.str();

    WriteToLog(temp_log_filename, temp_log_entry);
    ManageLogFile(temp_log_filename, 3601);

    hourly_temperatures.push_back(current_temperature);

    std::chrono::steady_clock::time_point current_time =
        std::chrono::steady_clock::now();
    if (std::chrono::duration_cast<std::chrono::hours>(current_time -
                                                       start_time)
            .count() >= 1) {
      double hourly_average = CalculateAverage(hourly_temperatures);
      std::string hourly_log_entry = oss.str() + ": " +
                                     std::to_string(hourly_average) + "°C";
      WriteToLog(hourly_average_log_filename, hourly_log_entry);
      ManageLogFile(hourly_average_log_filename, 720);

      daily_averages.push_back(hourly_average);
      hourly_temperatures.clear();
      start_time = current_time;
    }

    if (daily_averages.size() == 24) {
      double daily_average = CalculateAverage(daily_averages);
      std::string daily_log_entry = oss.str() + ": " +
                                    std::to_string(daily_average) + "°C";
      WriteToLog(daily_average_log_filename, daily_log_entry);
      daily_averages.clear();
    }

    ClearLogFile(temp_log_filename, std::chrono::hours(24));
    ClearLogFile(hourly_average_log_filename, std::chrono::hours(24 * 30));
    ClearLogFile(daily_average_log_filename, std::chrono::hours(24 * 365));
  }
}

}

int main() {
  RunTemperatureMonitor();
  return 0;
}
