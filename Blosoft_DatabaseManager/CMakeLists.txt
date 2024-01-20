cmake_minimum_required(VERSION 3.10)
project(Blosoft_DatabaseManager_library)

# Change add_executable to add_library
add_library(Blosoft_DatabaseManager_library
    DatabaseManager.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/sqlite/shell.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/sqlite/sqlite3.c
)

# Set any target-specific properties or configurations if needed

# Link libraries if needed
target_include_directories(Blosoft_DatabaseManager_library PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include/sqlite)
target_link_libraries(Blosoft_DatabaseManager_library PRIVATE)

# Set any other target-specific properties or configurations if needed

# Add the command to copy sqlite3.dll
add_custom_command(
    TARGET Blosoft_DatabaseManager_library POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
    ${CMAKE_CURRENT_SOURCE_DIR}/dll/sqlite3.dll
    $<TARGET_FILE_DIR:Blosoft_DatabaseManager_library>
)
