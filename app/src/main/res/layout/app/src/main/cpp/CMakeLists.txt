cmake_minimum_required(VERSION 3.22)
project(voicechanger)

add_library(voicechanger SHARED native-lib.cpp)
find_library(log-lib log)
target_link_libraries(voicechanger ${log-lib})
