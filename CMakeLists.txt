cmake_minimum_required(VERSION 3.16)
project(SmartHealthcareDBMS)

set(CMAKE_CXX_STANDARD 20)

find_package(unofficial-sqlite3 CONFIG REQUIRED)

add_executable(SmartHealthcareDBMS main.cpp)

target_link_libraries(SmartHealthcareDBMS PRIVATE unofficial::sqlite3::sqlite3)
