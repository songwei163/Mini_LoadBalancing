cmake_minimum_required(VERSION 3.14)
project(Mini_LoadBalancing)

set(CMAKE_CXX_STANDARD 11)

add_executable(log_1_test log_1/test.cc log_1/log.cc log_1/log.h)

add_executable(log_2_test log_2/test.cc log_2/log.h log_2/log.cc log_2/macro_define.h)
target_link_libraries(log_2_test pthread)

add_executable(tinyxml_test tinyxml/src/main.cc
        tinyxml/include/tinyxml.h tinyxml/include/tinystr.h
        tinyxml/src/tinyxml.cpp tinyxml/src/tinystr.cpp
        tinyxml/src/tinyxmlerror.cpp tinyxml/src/tinyxmlparser.cpp
        )

add_executable(IO_test_cli IO/cli.cc IO/common.h)
add_executable(IO_epoll_test_ser IO/epoll.cc IO/common.h)
