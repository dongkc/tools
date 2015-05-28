cmake_minimum_required(VERSION 2.8)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

PROJECT(zebra)

INCLUDE_DIRECTORIES(src)
INCLUDE_DIRECTORIES(/usr/include/apr-1)
INCLUDE_DIRECTORIES(/usr/local/include/activemq-cpp-3.8.3)
LINK_DIRECTORIES(/usr/local/lib)

ADD_SUBDIRECTORY(src)
