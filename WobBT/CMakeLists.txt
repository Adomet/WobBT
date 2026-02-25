cmake_minimum_required(VERSION 3.16)
project(WobBT LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(WobBT
    src/WobBT.cpp
    src/Binance.cpp
    src/Cerebro.cpp
    src/OHLC.cpp
)

target_include_directories(WobBT PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/Dependencies
)

if(UNIX AND NOT APPLE)
    find_package(CURL REQUIRED)
    find_package(OpenSSL REQUIRED)
    find_package(Python3 REQUIRED COMPONENTS Development)
    target_include_directories(WobBT PRIVATE ${Python3_INCLUDE_DIRS})
    target_link_libraries(WobBT PRIVATE CURL::libcurl OpenSSL::SSL OpenSSL::Crypto Python3::Python pthread)
else()
    target_include_directories(WobBT PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Dependencies/python/include
    )
endif()
