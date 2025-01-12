#include <iostream>
#include <vector>
#include <string>
#include <pqxx/pqxx>
#include "gnuplot-iostream.h"
#include "database_manager.hpp"

bool checkDataAvailability(const pqxx::result& r) {
    if (r.empty()) {
        std::cerr << "No data available for the specified range." << std::endl;
        return false;
    }
    return true;
}

pqxx::result executeQuery(pqxx::connection& conn, const std::string& query) {
    pqxx::work txn(conn);
    pqxx::result r = txn.exec(query);
    if (!checkDataAvailability(r)) throw std::runtime_error("No data available");
    return r;
}

void plotGraph(const std::vector<std::string>& x, const std::vector<double>& y, const std::string& title) {
    Gnuplot gp;
    gp << "set title '" << title << "'\n";
    gp << "set xdata time\n";
    gp << "set timefmt '%Y-%m-%d %H:%M:%S'\n";
    gp << "set format x '%H:%M'\n";
    gp << "plot '-' using 1:2 with lines title 'Temperature'\n";
    gp.send1d(boost::make_tuple(x, y));
}

void generateCurrentTemperatureGraph() {
    auto conn = ConnectToDatabase();
    if (!conn) return;

    try {
        pqxx::result r = executeQuery(*conn, "SELECT timestamp, temperature FROM temperature_log ORDER BY timestamp DESC LIMIT 10");

        std::vector<std::string> x;
        std::vector<double> y;
        for (const auto& row : r) {
            x.push_back(row[0].as<std::string>());
            y.push_back(row[1].as<double>());
        }

        plotGraph(x, y, "Current Temperature");
    } catch (const std::exception &e) {
        std::cerr << "Error fetching current temperature data: " << e.what() << std::endl;
    }
}

void generateDailyAverageTemperatureGraph(const std::string& startDate, const std::string& endDate) {
    auto conn = ConnectToDatabase();
    if (!conn) return;

    try {
        pqxx::result r = executeQuery(*conn, "SELECT timestamp, average_temperature FROM daily_average_log WHERE timestamp BETWEEN " + conn->quote(startDate) + " AND " + conn->quote(endDate));

        std::vector<std::string> x;
        std::vector<double> y;
        for (const auto& row : r) {
            x.push_back(row[0].as<std::string>());
            y.push_back(row[1].as<double>());
        }

        plotGraph(x, y, "Average Temperature from " + startDate + " to " + endDate);
    } catch (const std::exception &e) {
        std::cerr << "Error fetching average temperature data: " << e.what() << std::endl;
    }
}

void generateHourlyAverageTemperatureGraph(const std::string& startDate, const std::string& endDate) {
    auto conn = ConnectToDatabase();
    if (!conn) return;

    try {
        pqxx::result r = executeQuery(*conn, "SELECT timestamp, average_temperature FROM hourly_average_log WHERE timestamp BETWEEN " + conn->quote(startDate) + " AND " + conn->quote(endDate));

        std::vector<std::string> x;
        std::vector<double> y;
        for (const auto& row : r) {
            x.push_back(row[0].as<std::string>());
            y.push_back(row[1].as<double>());
        }

        plotGraph(x, y, "Hourly Average Temperature from " + startDate + " to " + endDate);
    } catch (const std::exception &e) {
        std::cerr << "Error fetching hourly average temperature data: " << e.what() << std::endl;
    }
}