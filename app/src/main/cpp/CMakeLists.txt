cmake_minimum_required(VERSION 3.22.1)

project("nutriapp")

add_library(
        nutriapp
        SHARED
        native-lib.cpp
        database.cpp
        calculos.cpp
)

find_library(log-lib log)

target_link_libraries(
        nutriapp
        ${log-lib}
        android
)