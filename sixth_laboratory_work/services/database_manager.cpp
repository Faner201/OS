#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <pqxx/pqxx>

void LoadEnv(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error opening .env file" << std::endl;
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        size_t pos = line.find('=');
        if (pos != std::string::npos) {
            std::string key = line.substr(0, pos);
            std::string value = line.substr(pos + 1);
            setenv(key.c_str(), value.c_str(), 1);
        }
    }
    file.close();
}

void CreateTables(pqxx::connection& conn) {
    try {
        pqxx::work txn(conn);
        txn.exec("CREATE TABLE IF NOT EXISTS temperature_log (\n"
                 "    id SERIAL PRIMARY KEY,\n"
                 "    timestamp TIMESTAMP NOT NULL,\n"
                 "    temperature DOUBLE PRECISION NOT NULL\n);");
        txn.exec("CREATE TABLE IF NOT EXISTS hourly_average_log (\n"
                 "    id SERIAL PRIMARY KEY,\n"
                 "    timestamp TIMESTAMP NOT NULL,\n"
                 "    average_temperature DOUBLE PRECISION NOT NULL\n);");
        txn.exec("CREATE TABLE IF NOT EXISTS daily_average_log (\n"
                 "    id SERIAL PRIMARY KEY,\n"
                 "    timestamp TIMESTAMP NOT NULL,\n"
                 "    average_temperature DOUBLE PRECISION NOT NULL\n);");
        txn.commit();
        std::cout << "Tables created successfully." << std::endl;
    } catch (const std::exception &e) {
        std::cerr << "Error creating tables: " << e.what() << std::endl;
    }
}

pqxx::connection CreateDatabaseAndConnect(const std::string& dbname) {
    try {
        std::string user = std::getenv("DB_USER");
        std::string password = std::getenv("DB_PASSWORD");
        std::string host = std::getenv("DB_HOST");
        pqxx::connection conn("user=" + user + " password=" + password + " host=" + host);
        
        pqxx::nontransaction txn(conn);
        pqxx::result r = txn.exec("SELECT 1 FROM pg_database WHERE datname = '" + dbname + "'");
        if (r.empty()) {
            txn.exec("CREATE DATABASE " + dbname);
            std::cout << "Database created successfully." << std::endl;
        } else {
            std::cout << "Database already exists." << std::endl;
        }

        pqxx::connection db_conn("dbname=" + dbname + " user=" + user + " password=" + password + " host=" + host);
        return db_conn;
    } catch (const std::exception &e) {
        std::cerr << "Error creating or connecting to database: " << e.what() << std::endl;
        throw;
    }
}

void initializeDatabase() {
    LoadEnv(".env");
    pqxx::connection conn = CreateDatabaseAndConnect(std::getenv("DB_NAME"));
    if (conn.is_open()) {
        CreateTables(conn);
    } else {
        std::cerr << "Failed to connect to database." << std::endl;
    }
}

void WriteToDatabase(pqxx::connection& conn, const std::string& table, const std::string& timestamp, double temperature) {
  try {
    pqxx::work txn(conn);
    std::string query = "INSERT INTO " + table + " (timestamp, temperature) VALUES (" + txn.quote(timestamp) + ", " + std::to_string(temperature) + ")";
    txn.exec(query);
    txn.commit();
  } catch (const std::exception &e) {
    std::cerr << "Database error: " << e.what() << std::endl;
  }
}

std::unique_ptr<pqxx::connection> ConnectToDatabase() {
    std::string db_host = std::getenv("DB_HOST");
    std::string db_port = std::getenv("DB_PORT");
    std::string db_name = std::getenv("DB_NAME");
    std::string db_user = std::getenv("DB_USER");
    std::string db_password = std::getenv("DB_PASSWORD");

    if (!db_host.empty() && !db_port.empty() && !db_name.empty() && !db_user.empty() && !db_password.empty()) {
        try {
            std::string conn_str = "host=" + db_host +
                                   " port=" + db_port +
                                   " dbname=" + db_name +
                                   " user=" + db_user +
                                   " password=" + db_password;
            auto conn = std::make_unique<pqxx::connection>(conn_str);
            if (conn->is_open()) {
                std::cout << "Connected to database successfully" << std::endl;
                return conn;
            } else {
                std::cerr << "Failed to connect to database" << std::endl;
            }
        } catch (const std::exception &e) {
            std::cerr << "Database connection error: " << e.what() << std::endl;
        }
    } else {
        std::cerr << "Database configuration not set" << std::endl;
    }
    return nullptr;
} 