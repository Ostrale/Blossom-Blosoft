cmake_minimum_required(VERSION 3.0.0)
project(Blossom-Blosoft VERSION 0.1.0)

include_directories(${CMAKE_SOURCE_DIR}/include)
link_directories(${CMAKE_SOURCE_DIR}/lib)

find_library(tins packet ws2_32 phlpapi wpcap)
link_libraries(${ProjectName} tins packet ws2_32 phlpapi wpcap)

add_compile_definitions(TINS_STATIC)

include(CTest)
enable_testing()

add_executable(Blossom-Blosoft main.cpp)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
