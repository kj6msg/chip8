################################################################################
# CHIP-8 Emulator
# Copyright (C) 2020 Ryan Clarke <kj6msg@icloud.com>
################################################################################

cmake_minimum_required(VERSION 3.22)
project(chip8 VERSION 2.0.0 DESCRIPTION "CHIP-8 Emulator" LANGUAGES CXX)

find_package(fmt REQUIRED)
find_package(Microsoft.GSL CONFIG REQUIRED)
find_package(SFML 2.5 COMPONENTS system window graphics audio REQUIRED)

add_executable(chip8 src/main.cpp src/chip8.cpp src/chip8.hpp)

target_compile_features(chip8 PRIVATE cxx_std_20)
target_compile_options(chip8 PRIVATE -Wall -Wextra)
target_link_libraries(chip8 PRIVATE fmt::fmt
                                    Microsoft.GSL::GSL
                                    sfml-system
                                    sfml-window
                                    sfml-graphics
                                    sfml-audio)
