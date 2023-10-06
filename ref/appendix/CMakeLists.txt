cmake_minimum_required(VERSION 3.5)
project(Algorithm VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

aux_source_directory(./src SRC_LIST)

add_executable(${PROJECT_NAME} ${SRC_LIST})
