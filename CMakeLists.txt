cmake_minimum_required(VERSION 3.17)

project(CHIP8 VERSION 1.0.0 LANGUAGES CXX)

set(SOURCE_FILES src/main.cpp src/chip8.cpp)

find_package(SFML 2.5 COMPONENTS graphics audio REQUIRED)

add_executable(chip8 ${SOURCE_FILES})
target_include_directories(chip8 PRIVATE /usr/local/include)
target_compile_options(chip8 PRIVATE -O2 -Wall -Wextra)
target_compile_features(chip8 PRIVATE cxx_std_17)
target_link_libraries(chip8 PRIVATE sfml-system sfml-window sfml-graphics sfml-audio)
