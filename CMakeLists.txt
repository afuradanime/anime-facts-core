cmake_minimum_required(VERSION 3.0.0)

project(anime_facts_core C)

# Create sqlite library
add_library(sqlite3 STATIC sqlite3/sqlite3.c)
set_target_properties(sqlite3 PROPERTIES POSITION_INDEPENDENT_CODE ON)

set(
    SOURCES
    main.c
    src/dynamic_array.c 
    src/anime.c 
    src/printer.c
)

# Compile sources into a shared library
add_library(${PROJECT_NAME} SHARED ${SOURCES})
target_link_libraries(${PROJECT_NAME} PRIVATE
    sqlite3
)
target_include_directories(${PROJECT_NAME} PRIVATE
    ${PROJECT_SOURCE_DIR}/include
)