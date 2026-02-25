cmake_minimum_required(VERSION 2.8.12)
project(WobBT CXX)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
elseif(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++17")
endif()

add_executable(WobBT
    src/WobBT.cpp
    src/Binance.cpp
    src/Cerebro.cpp
    src/OHLC.cpp
)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/Dependencies
    ${CMAKE_CURRENT_SOURCE_DIR}/Dependencies/python/include
)

if(UNIX AND NOT APPLE)
    find_package(Threads REQUIRED)
    find_package(CURL REQUIRED)
    find_package(OpenSSL REQUIRED)

    include_directories(${CURL_INCLUDE_DIRS} ${OPENSSL_INCLUDE_DIR})
    target_link_libraries(WobBT ${CURL_LIBRARIES} ${OPENSSL_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
endif()
