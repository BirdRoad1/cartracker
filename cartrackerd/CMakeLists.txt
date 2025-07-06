cmake_minimum_required(VERSION 3.20)

# Define the project name and optionally the version and languages
project(cartrackerd VERSION 1.0 LANGUAGES CXX)

set(CMAKE_C_COMPILER aarch64-linux-gnu-gcc)
set(CMAKE_CXX_COMPILER aarch64-linux-gnu-g++)

set(CMAKE_SYSROOT /home/user/Documents/cartracker/cartrackerd/sysroot)
set(CMAKE_FIND_ROOT_PATH /home/user/Documents/cartracker/cartrackerd/sysroot)

# Add an executable target
# The first argument is the name of the executable
# Subsequent arguments are the source files that make up the executable
add_executable(cartrackerd src src/main.cpp src/wifi_helper.cpp src/api.cpp src/config.cpp)

find_library(lib_curl NAMES libcurl.so.4.8.0)

target_include_directories(cartrackerd PRIVATE include)
target_link_libraries(cartrackerd PRIVATE wpa_client ${lib_curl})

# (Optional) Set the C++ standard for the target
# This ensures a specific C++ standard is used during compilation
target_compile_features(cartrackerd PRIVATE cxx_std_23)