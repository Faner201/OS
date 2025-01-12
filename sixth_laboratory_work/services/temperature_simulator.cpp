#include <iostream>
#include <cstdlib>
#include <ctime>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#endif

#ifdef _WIN32
HANDLE openSerialPort(const char* portName) {
    HANDLE hSerial = CreateFile(portName, GENERIC_WRITE, 0, 0,
                                OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    if (hSerial == INVALID_HANDLE_VALUE) {
        std::cerr << "Error opening COM port\n";
    }
    return hSerial;
}

void configureSerialPort(HANDLE hSerial) {
    DCB dcbSerialParams = {0};
    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
    if (!GetCommState(hSerial, &dcbSerialParams)) {
        std::cerr << "Error getting COM state\n";
        return;
    }

    dcbSerialParams.BaudRate = CBR_9600;
    dcbSerialParams.ByteSize = 8;
    dcbSerialParams.StopBits = ONESTOPBIT;
    dcbSerialParams.Parity = NOPARITY;

    if (!SetCommState(hSerial, &dcbSerialParams)) {
        std::cerr << "Error setting COM state\n";
    }
}

void closeSerialPort(HANDLE hSerial) {
    CloseHandle(hSerial);
}

void writeTemperature(HANDLE hSerial, int temperature) {
    std::string tempStr = std::to_string(temperature) + "\n";
    DWORD bytesWritten;
    WriteFile(hSerial, tempStr.c_str(), tempStr.size(), &bytesWritten, NULL);
}

#else

int openSerialPort(const char* portName) {
    int serial_port = open(portName, O_WRONLY);
    if (serial_port < 0) {
        std::cerr << "Error opening serial port\n";
    }
    return serial_port;
}

void configureSerialPort(int serial_port) {
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

void closeSerialPort(int serial_port) {
    close(serial_port);
}

void writeTemperature(int serial_port, int temperature) {
    std::string tempStr = std::to_string(temperature) + "\n";
    std::cout << "Writing temperature: " << tempStr << std::endl;
    ssize_t bytesWritten = write(serial_port, tempStr.c_str(), tempStr.size());
}

#endif

int main() {
    std::srand(std::time(nullptr));

#ifdef _WIN32
    HANDLE hSerial = openSerialPort("COM3");
    if (hSerial == INVALID_HANDLE_VALUE) return -1;
    configureSerialPort(hSerial);
#else
    int serial_port = openSerialPort("/dev/ttys008");
    if (serial_port < 0) return -1;
    configureSerialPort(serial_port);
#endif

    while (true) {
        int temperature = std::rand() % 61 - 30;
#ifdef _WIN32
        writeTemperature(hSerial, temperature);
#else
        writeTemperature(serial_port, temperature);
#endif

#ifdef _WIN32
        Sleep(1000);
#else
        usleep(1000000);
#endif
    }

#ifdef _WIN32
    closeSerialPort(hSerial);
#else
    closeSerialPort(serial_port);
#endif

    return 0;
}
