# CHIP-8

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to
[Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [2.0.0] - 2022-02-24
### Changed
- Complete rewrite. All SFML code moved inside CHIP8 class. Emulation uses maps
of `std::function` vs switch statements. Random number generator updated to C++
standard, vice C functions. File structure of project streamlined.

## [1.0.0] - 2020-04-09
### Added
- Fully functional CHIP-8 emulator.
