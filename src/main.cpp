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

////////////////////////////////////////////////////////////////////////////////
// Program   : chip8
// File Name : main.cpp
// Project   : CHIP-8
// Author    : Ryan Clarke
// E-mail    : kj6msg@icloud.com
// -----------------------------------------------------------------------------
// Purpose   : CHIP-8 emulator written in C++ using the SFML library. The
//           : program accepts a program file as a command line argument and
//           : emulates at 500 Hz.
////////////////////////////////////////////////////////////////////////////////

#include <algorithm>
#include <array>
#include "chip8.h"
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iostream>
#include <iterator>
#include <sstream>
#include <vector>
#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>


////////////////////////////////////////////////////////////////////////////////
/// \brief Scale the CHIP-8 display
///
/// This function retrieves the current video mode and scales it to a 1440x900
/// resolution. It is designed around the Apple Retina display which is not
/// handled properly by SFML. If the resolution is already 1440x900, no scaling
/// is performed. Additionally, a scaling factor of 10 is also applied to
/// compensate for the low resolution CHIP-8 display.
///
/// \return Screen resolution scale
///
////////////////////////////////////////////////////////////////////////////////
sf::Vector2f display_scale();


////////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
{
    // CHIP8 requires a file name as an argument
    if(argc < 2)
    {
        std::cerr << "usage: chip8 file\n";
        return EXIT_FAILURE;
    }

    std::ifstream file(argv[1], std::ifstream::in | std::ifstream::binary);

    if(!file.is_open())
    {
        std::cerr << "error reading [" << argv[1] << "]\n";
        return EXIT_FAILURE;
    }

    // The maximum program size is defined by the CHIP-8 memory size and the
    // program start address
    std::vector<uint8_t> data(CHIP8::mem_size - CHIP8::org);
    file.read(reinterpret_cast<char*>(data.data()), data.size());
    file.close();

    CHIP8 chip8(data);
    std::srand(std::time(nullptr));

    // Create the program window, scale it as required, and set a view for the
    // logical resolution of the CHIP-8. Also, disable key repeating, since it
    // doesn't make sense for the CHIP-8.
    auto scale = display_scale();
    auto mode = sf::VideoMode(CHIP8::width * scale.x, CHIP8::height * scale.y);
    sf::RenderWindow window(mode, "CHIP-8 1.0.0");
    sf::View view(sf::FloatRect(0.0f, 0.0f, CHIP8::width, CHIP8::height));
    window.setView(view);
    window.setKeyRepeatEnabled(false);

    // Variables for the main event loop
    sf::Event event;
    sf::Clock emulate_clock;            // 500 Hz (default) emulation clock
    constexpr sf::Int64 T_500Hz{2000};  // 2000 us

    while(window.isOpen())
    {   
        // 500 Hz emulation clock
        if(emulate_clock.getElapsedTime().asMicroseconds() >= T_500Hz)
        {
            emulate_clock.restart();
            chip8.emulate();
        }

        while(window.pollEvent(event))
        {
            switch(event.type)
            {
                case sf::Event::Closed:
                    window.close();
                    break;
                
                case sf::Event::KeyPressed:
                    chip8.key_press(event.key.code);
                    break;
                
                case sf::Event::KeyReleased:
                    chip8.key_release(event.key.code);
                    break;

                default:
                    break;
            }
        }

        window.clear(sf::Color::Black);
        window.draw(chip8);
        window.display();
    }
    
    return EXIT_SUCCESS;
}


////////////////////////////////////////////////////////////////////////////////
sf::Vector2f display_scale()
{
    auto desktop = sf::VideoMode::getDesktopMode();
    
    // Compute a scaling factor for a 1440x900 display
    sf::Vector2f scale(10.0f, 10.0f);
    scale.x *= (desktop.width > 1440) ? desktop.width / 1440.0f : 1.0f;
    scale.y *= (desktop.height > 900) ? desktop.height / 900.0f : 1.0f;

    return scale;
}
