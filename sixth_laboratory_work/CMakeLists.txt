cmake_minimum_required(VERSION 3.10)

project(TemperatureMonitor)

set(CMAKE_CXX_STANDARD 17)

set(SOURCES
    server/temperature_server.cpp
    server/handler.cpp
    middleware/temperature_reader.cpp
    services/database_manager.cpp
    services/temperature_simulator.cpp
)

include_directories("${CMAKE_SOURCE_DIR}/lib/httplib")

if(APPLE)
    include_directories("/opt/homebrew/opt/libpqxx/include")
    link_directories("/opt/homebrew/opt/libpqxx/lib" "/opt/homebrew/opt/libpq/lib")
    set(PLATFORM_LIBS pqxx pq)
elseif(WIN32)
    include_directories("C:/path/to/libpqxx/include")
    link_directories("C:/path/to/libpqxx/lib" "C:/path/to/libpq/lib")
    set(PLATFORM_LIBS pqxx pq)
elseif(UNIX)
    include_directories("/usr/include/pqxx")
    link_directories("/usr/lib")
    set(PLATFORM_LIBS pqxx pq)
endif()

add_executable(TemperatureMonitor ${SOURCES})

target_link_libraries(TemperatureMonitor ${PLATFORM_LIBS} pthread)