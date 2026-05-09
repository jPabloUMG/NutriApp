cmake_minimum_required(VERSION 3.22.1)

project("nutriapp")

# SQLite incluido directamente
add_library(
        nutriapp
        SHARED
        native-lib.cpp
        database.cpp
)

# Busca las librerías de Android
find_library(log-lib log)
find_library(android-lib android)

# SQLite viene incluido en Android
find_library(sqlite-lib sqlite3)

# Vincula todo junto
target_link_libraries(
        nutriapp
        ${log-lib}
        ${android-lib}
        ${sqlite-lib}
)