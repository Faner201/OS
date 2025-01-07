#ifndef HANDLER_HPP
#define HANDLER_HPP

#include "../lib/httplib.h"

void HandleStatisticsHourlyRequest(const httplib::Request& req, httplib::Response& res);
void HandleStatisticsDailyRequest(const httplib::Request& req, httplib::Response& res);
void HandleTemperatureRequest(const httplib::Request& req, httplib::Response& res);

#endif