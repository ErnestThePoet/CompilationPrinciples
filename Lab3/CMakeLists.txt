cmake_minimum_required(VERSION 3.22)
project(CPLab3)
aux_source_directory(../Lab1/bits/ LAB1_BITS_SRCS)
aux_source_directory(../Lab1/generated/ LAB1_GENERATED_SRCS)
aux_source_directory(../Lab2/bits/ LAB2_BITS_SRCS)
aux_source_directory(./bits/ BITS_SRCS)

add_executable(parser ${LAB1_BITS_SRCS} ${LAB1_GENERATED_SRCS} ${LAB2_BITS_SRCS} ${BITS_SRCS} ./main.cpp)

# set(CMAKE_BUILD_TYPE Debug)
# set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "-rdynamic")
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-O2 -Wall -std=c++17")

target_link_libraries(parser fl)
target_link_libraries(parser y)