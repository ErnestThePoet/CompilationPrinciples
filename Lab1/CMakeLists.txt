cmake_minimum_required(VERSION 3.22)
project(CPLab1)
aux_source_directory(./bits/ BITS_SRCS)
aux_source_directory(./generated/ GENERAETD_SRCS)

add_executable(parser ${GENERAETD_SRCS} ${BITS_SRCS} ./main.c)

# set(CMAKE_BUILD_TYPE Debug)
# set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "-rdynamic")
set(CMAKE_C_FLAGS ${CMAKE_C_FLAGS} "-O2 -Wall")

target_link_libraries(parser fl)
target_link_libraries(parser y)