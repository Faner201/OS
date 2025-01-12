#ifndef DATABASE_MANAGER_HPP
#define DATABASE_MANAGER_HPP

#include <pqxx/pqxx>
#include <string>
#include <memory>

void LoadEnv(const std::string& filename);
void CreateTables(pqxx::connection& conn);
pqxx::connection CreateDatabaseAndConnect(const std::string& dbname);
void initializeDatabase();
void WriteToDatabase(pqxx::connection& conn, const std::string& table, const std::string& timestamp, double temperature);
std::unique_ptr<pqxx::connection> ConnectToDatabase();

#endif