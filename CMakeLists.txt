cmake_minimum_required (VERSION 2.6)

project(imgui-fps-hud LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

find_library(log-lib log)

add_library(imgui-fps-hud SHARED src/main.cpp)
target_link_libraries(imgui-fps-hud PUBLIC ${log-lib})
