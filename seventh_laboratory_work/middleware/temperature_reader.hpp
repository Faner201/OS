#ifndef FIFTH_LABORATORY_WORK_MIDDLEWARE_TEMPERATURE_READER_HPP_
#define FIFTH_LABORATORY_WORK_MIDDLEWARE_TEMPERATURE_READER_HPP_

#include <pqxx/pqxx>
void RunTemperatureReader(const pqxx::connection& conn);

#endif
