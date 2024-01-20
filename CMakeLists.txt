cmake_minimum_required(VERSION 3.10)

project(Blosoft)

# Include sub-projects.
add_subdirectory("Blosoft_DatabaseManager")
add_subdirectory("Blosoft_analyzer")

# Spécifiez l'exécutable principal ici (par exemple, Blosoft_analyzer)
set(EXECUTABLE_PRINCIPAL Blosoft_analyzer)

# Spécifiez l'exécutable principal comme exécutable de sortie
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${EXECUTABLE_PRINCIPAL})