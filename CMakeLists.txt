cmake_minimum_required(VERSION 3.10)
project(Abimon)
include_directories(
src/UI
src/user-profiling
)
add_executable(Abimon
    src/main.cpp
    src/UI/banner.cpp
    src/UI/mainmenu.cpp
    src/UI/abimonstats
    src/user-profiling/userinput.cpp
)
