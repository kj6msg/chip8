////////////////////////////////////////////////////////////////////////////////
// CHIP-8 Emulator
// Copyright (C) 2022 Ryan Clarke <kj6msg@icloud.com>
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// MIT License
//
// Copyright (c) 2020 Ryan Clarke
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
////////////////////////////////////////////////////////////////////////////////

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <vector>
#include <fmt/core.h>
#include "chip8.hpp"


////////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
{
    // CHIP8 requires a file name as an argument
    if(argc < 2)
    {
        fmt::print(stderr, "usage: chip8 filename\n");
        return EXIT_FAILURE;
    }

    std::ifstream file(argv[1], std::ifstream::in | std::ifstream::binary);

    if(!file.is_open())
    {
        fmt::print(stderr, "error reading [{}]\n", argv[1]);
        return EXIT_FAILURE;
    }

    // The maximum program size is defined by the CHIP-8 memory size and the
    // program start address
    constexpr std::size_t max_size{CHIP8::mem_size - CHIP8::org};
    std::vector<std::uint8_t> data(max_size);
    file.read(reinterpret_cast<char*>(data.data()), data.size());
    file.close();

    CHIP8 chip8(data);
    chip8.run();
}
