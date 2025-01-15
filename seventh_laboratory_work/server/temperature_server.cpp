#include <iostream>
#include <fstream>
#include <string>
#include "../lib/httplib.h"
#include "handler.hpp"
#include "../middleware/temperature_reader.hpp"
#include <thread>

void RunHttpServer() {
  httplib::Server svr;

  svr.Get("/temperature", HandleTemperatureRequest);
  svr.Get("/statistics/hourly", HandleStatisticsHourlyRequest);
  svr.Get("/statistics/daily", HandleStatisticsDailyRequest);
  svr.listen("localhost", 8080);
}

int main() {
  std::thread temperature_reader_thread(RunTemperatureReader);
  std::thread http_server_thread(RunHttpServer);

  temperature_reader_thread.join();
  http_server_thread.join();

  return 0;
} 