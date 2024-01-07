cmake_minimum_required (VERSION 2.6)

project(imgui-fps-hud LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

find_library(log-lib log)

add_library(imgui-fps-hud SHARED src/main.cpp
imgui/imgui.cpp
imgui/imgui_demo.cpp
imgui/imgui_draw.cpp
imgui/imgui_tables.cpp
imgui/imgui_widgets.cpp
imgui/backends/imgui_impl_opengl3.cpp
)

target_compile_definitions(imgui-fps-hud PRIVATE
  IMGUI_IMPL_OPENGL_ES2
)

target_include_directories(imgui-fps-hud PRIVATE
  imgui
  imgui/backends
  ${ANDROID_NDK}/sources/android/native_app_glue
)

target_link_libraries(imgui-fps-hud PUBLIC GLESv2 android ${log-lib})
