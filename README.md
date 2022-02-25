# CHIP-8 Emulator

CHIP-8 emulator written in C++20 using SFML. Fully functional at 500Hz. Several
public domain ROMs are included for use.

## Dependencies
* {fmt}
* Microsoft Guidelines Support Library
* SFML 2.5

## Compile and Run
Compile:
```
% mkdir build
% cd build
% cmake ..
% cmake --build .
```
Run:
```
% ./chip8 filename
```

## Keymap
```
 1 | 2 | 3 | C
(1)|(2)|(3)|(4)
---------------
 4 | 5 | 6 | D
(q)|(w)|(e)|(r)
---------------
 7 | 8 | 9 | E
(a)|(s)|(d)|(f)
---------------
 A | 0 | B | F
(z)|(x)|(c)|(v)
```

## License
Copyright (c) 2020 Ryan Clarke, licensed under the MIT License.
