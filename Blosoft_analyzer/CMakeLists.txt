cmake_minimum_required(VERSION 3.0.0)
project(Blossom_analyzer VERSION 0.1.0)

include_directories(${CMAKE_SOURCE_DIR}/Blosoft_analyzer/include
    ${CMAKE_SOURCE_DIR}/Blosoft_DatabaseManager/include
)
link_directories(${CMAKE_SOURCE_DIR}/Blosoft_analyzer/lib
    ${CMAKE_SOURCE_DIR}/build/Blosoft_DatabaseManager/Debug
)


find_library(tins packet ws2_32 phlpapi wpcap nDPI Blosoft_DatabaseManager_library)
link_libraries(${ProjectName} tins packet ws2_32 phlpapi wpcap nDPI Blosoft_DatabaseManager_library)

add_compile_definitions(TINS_STATIC)

include(CTest)
enable_testing()

add_executable(Blossom_analyzer 
    main.cpp 
    src/toby.cpp 
    ../Blosoft_DatabaseManager/DatabaseManager.hpp
)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
