cmake_minimum_required(VERSION 3.16)
project(WobBT LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(STATIC_LINK "Link curl, ssl, crypto statically into executable" OFF)

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
    if(STATIC_LINK)
        # Prefer static libraries (.a) for curl and openssl
        set(CMAKE_FIND_LIBRARY_SUFFIXES_BACKUP ${CMAKE_FIND_LIBRARY_SUFFIXES})
        set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
    endif()

    find_package(CURL REQUIRED)
    find_package(OpenSSL REQUIRED)
    if(STATIC_LINK)
        set(CMAKE_FIND_LIBRARY_SUFFIXES ${CMAKE_FIND_LIBRARY_SUFFIXES_BACKUP})
    endif()
    find_package(Python3 REQUIRED COMPONENTS Development)
    target_include_directories(WobBT PRIVATE ${Python3_INCLUDE_DIRS})

    if(STATIC_LINK)
        # Static linking: curl, ssl, crypto + dependencies (dl, z for OpenSSL)
        target_link_libraries(WobBT PRIVATE
            CURL::libcurl
            OpenSSL::SSL
            OpenSSL::Crypto
            Python3::Python
            pthread
            dl
            z
        )
    else()
        target_link_libraries(WobBT PRIVATE CURL::libcurl OpenSSL::SSL OpenSSL::Crypto Python3::Python pthread)
    endif()
else()
    target_include_directories(WobBT PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Dependencies/python/include
    )
endif()
