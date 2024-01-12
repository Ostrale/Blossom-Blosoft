cmake_minimum_required(VERSION 3.10)

project(Blosoft_DatabaseManager)

# Ajoute l'exécutable de ton projet
add_executable(Blosoft_DatabaseManager
    DatabaseManager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/sqlite/shell.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/sqlite/sqlite3.c
    test.cpp
)

# Inclut le dossier où se trouve sqlite3.h
target_include_directories(Blosoft_DatabaseManager PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include/sqlite)

# Ajoute le fichier sqlite3.dll à copier lors de la construction
add_custom_command(
    TARGET Blosoft_DatabaseManager POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
    ${CMAKE_CURRENT_SOURCE_DIR}/dll/sqlite3.dll
    $<TARGET_FILE_DIR:Blosoft_DatabaseManager>
)
