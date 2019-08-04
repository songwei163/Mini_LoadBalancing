cmake_minimum_required(VERSION 3.5)

set(CMAKE_CXX_STANDARD 11)

project(loadBalance)

include_directories(include
        include/clog
        include/tinyxml)

aux_source_directory(src/. DIR_SRCS)

ADD_SUBDIRECTORY(src/clog)
ADD_SUBDIRECTORY(src/tinyxml)

add_executable(loadBalance ${DIR_SRCS})
target_link_libraries(loadBalance clog tinyxml)




