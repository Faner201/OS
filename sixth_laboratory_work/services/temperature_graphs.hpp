#ifndef TEMPERATURE_GRAPHS_HPP
#define TEMPERATURE_GRAPHS_HPP

#include <pqxx/pqxx>
#include <string>
#include <vector>
#include "../lib/gnuplot-iostream.h"
#include "database_manager.hpp"

void generateCurrentTemperatureGraph();
void generateDailyAverageTemperatureGraph(const std::string& startDate, const std::string& endDate);
void generateHourlyAverageTemperatureGraph(const std::string& startDate, const std::string& endDate);

#endif // TEMPERATURE_GRAPHS_HPP 