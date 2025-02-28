#include "fifth_laboratory_work/server/handler.hpp"
#include <iostream>
#include <pqxx/pqxx>
#include "../services/database_manager.hpp"
#include "../lib/httplib.h"
#include <string>

void HandleStatisticsRequest(const httplib::Request& req, httplib::Response* res, const std::string& table_name, const std::string& title) {
    std::string month = req.get_param_value("month");
    std::string day = req.get_param_value("day");
    std::string hour = req.get_param_value("hour");

    auto conn = ConnectToDatabase();
    if (!conn) {
        res->status = 500;
        res->set_content("<html><body><h1>Database connection error</h1></body></html>", "text/html");
        return;
    }

    try {
        pqxx::work txn(*conn);
        std::string query = "SELECT timestamp, average_temperature FROM " + table_name + " WHERE 1=1";
        if (!month.empty()) {
            query += " AND EXTRACT(MONTH FROM timestamp) = " + txn.quote(month);
        }
        if (!day.empty()) {
            query += " AND EXTRACT(DAY FROM timestamp) = " + txn.quote(day);
        }
        if (!hour.empty()) {
            query += " AND EXTRACT(HOUR FROM timestamp) = " + txn.quote(hour);
        }
        query += " ORDER BY timestamp";

        pqxx::result r = txn.exec(query);

        std::string content = "<html><body><h1>" + title + "</h1><ul>";
        for (const auto& row : r) {
            content += "<li>" + row["timestamp"].as<std::string>() + ": " + row["average_temperature"].as<std::string>() + "°C</li>";
        }
        content += "</ul></body></html>";

        if (r.empty()) {
            content = "<html><body><h1>No data available for the selected period</h1></body></html>";
        }

        res->set_content(content, "text/html");
    } catch (const std::exception &e) {
        res->status = 500;
        res->set_content("<html><body><h1>Error retrieving data: " + std::string(e.what()) + "</h1></body></html>", "text/html");
    }
}

void HandleStatisticsHourlyRequest(const httplib::Request& req, httplib::Response* res) {
    HandleStatisticsRequest(req, res, "hourly_average_log", "Hourly Statistics");
}

void HandleStatisticsDailyRequest(const httplib::Request& req, httplib::Response* res) {
    HandleStatisticsRequest(req, res, "daily_average_log", "Daily Statistics");
}

void HandleTemperatureRequest(const httplib::Request& req, httplib::Response* res) {
    auto conn = ConnectToDatabase();
    if (!conn) {
        res->status = 500;
        res->set_content("<html><body><h1>Database connection error</h1></body></html>", "text/html");
        return;
    }

    try {
        pqxx::work txn(*conn);
        pqxx::result r = txn.exec("SELECT timestamp, temperature FROM temperature_log ORDER BY timestamp DESC LIMIT 1");

        std::string content;
        if (r.empty()) {
            content = "<html><body><h1>No temperature data available</h1></body></html>";
        } else {
            const auto& row = r[0];
            content = "<html><body><h1>Current Temperature</h1><p>" + row["timestamp"].as<std::string>() + ": " + row["temperature"].as<std::string>() + "°C</p></body></html>";
        }
        res->set_content(content, "text/html");
    } catch (const std::exception &e) {
        res->status = 500;
        res->set_content("<html><body><h1>Error retrieving data: " + std::string(e.what()) + "</h1></body></html>", "text/html");
    }
}