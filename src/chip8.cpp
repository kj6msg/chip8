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

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstdio>
#include <exception>
#include <random>
#include <utility>
#include <vector>
#include <gsl/util>
#include <fmt/core.h>
#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include "chip8.hpp"


////////////////////////////////////////////////////////////////////////////////
// Static Data
////////////////////////////////////////////////////////////////////////////////

// default keyboard map for CHIP8 hexademical keypad
static const std::array<sf::Keyboard::Key, 16> default_keys
{
    sf::Keyboard::X,        // 0
    sf::Keyboard::Num1,     // 1
    sf::Keyboard::Num2,     // 2
    sf::Keyboard::Num3,     // 3
    sf::Keyboard::Q,        // 4
    sf::Keyboard::W,        // 5
    sf::Keyboard::E,        // 6
    sf::Keyboard::A,        // 7
    sf::Keyboard::S,        // 8
    sf::Keyboard::D,        // 9
    sf::Keyboard::Z,        // A
    sf::Keyboard::C,        // B
    sf::Keyboard::Num4,     // C
    sf::Keyboard::R,        // D
    sf::Keyboard::F,        // E
    sf::Keyboard::V         // F
};


// 8x5 font for each hexadecimal digit
static const std::array<std::uint8_t, 80> font
{
    0b1111'0000,         // 0
    0b1001'0000,
    0b1001'0000,
    0b1001'0000,
    0b1111'0000,

    0b0010'0000,         // 1
    0b0110'0000,
    0b0010'0000,
    0b0010'0000,
    0b0111'0000,

    0b1111'0000,         // 2
    0b0001'0000,
    0b1111'0000,
    0b1000'0000,
    0b1111'0000,

    0b1111'0000,         // 3
    0b0001'0000,
    0b1111'0000,
    0b0001'0000,
    0b1111'0000,

    0b1001'0000,         // 4
    0b1001'0000,
    0b1111'0000,
    0b0001'0000,
    0b0001'0000,

    0b1111'0000,         // 5
    0b1000'0000,
    0b1111'0000,
    0b0001'0000,
    0b1111'0000,

    0b1111'0000,         // 6
    0b1000'0000,
    0b1111'0000,
    0b1001'0000,
    0b1111'0000,

    0b1111'0000,         // 7
    0b0001'0000,
    0b0010'0000,
    0b0100'0000,
    0b0100'0000,

    0b1111'0000,         // 8
    0b1001'0000,
    0b1111'0000,
    0b1001'0000,
    0b1111'0000,

    0b1111'0000,         // 9
    0b1001'0000,
    0b1111'0000,
    0b0001'0000,
    0b1111'0000,

    0b1111'0000,         // A
    0b1001'0000,
    0b1111'0000,
    0b1001'0000,
    0b1001'0000,

    0b1111'0000,         // B
    0b1001'0000,
    0b1110'0000,
    0b1001'0000,
    0b1111'0000,

    0b1111'0000,         // C
    0b1000'0000,
    0b1000'0000,
    0b1000'0000,
    0b1111'0000,

    0b1110'0000,         // D
    0b1001'0000,
    0b1001'0000,
    0b1001'0000,
    0b1110'0000,

    0b1111'0000,         // E
    0b1000'0000,
    0b1111'0000,
    0b1000'0000,
    0b1111'0000,

    0b1111'0000,         // F
    0b1000'0000,
    0b1111'0000,
    0b1000'0000,
    0b1000'0000
};


////////////////////////////////////////////////////////////////////////////////
// Public Member Functions
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
CHIP8::CHIP8(const std::vector<std::uint8_t>& program)
    : m_memory(mem_size), m_engine(std::random_device{}()), m_dist(0U, 255U),
      m_window(sf::VideoMode::getFullscreenModes().front(), "CHIP-8",
            sf::Style::Fullscreen)
{
    std::copy(font.begin(), font.end(), m_memory.begin());
    std::copy(program.begin(), program.end(), m_memory.begin() + org);

    // Set default key states
    for(const auto key : default_keys)
        m_keymap.emplace_back(key, KeyState::Released);

    m_pixels.create(width, height);
    m_texture.create(width, height);
    m_texture.update(m_pixels);
    m_sprite.setTexture(m_texture);

    m_window.setView(sf::View(sf::FloatRect(0.0f, 0.0f,
        static_cast<float>(CHIP8::width), static_cast<float>(CHIP8::height))));
    m_window.setKeyRepeatEnabled(false);
    m_window.setMouseCursorVisible(false);

    // generate the sound data for the sound timer
    constexpr auto num_samples = Fs / tone;
    std::vector<sf::Int16> samples(num_samples);
    auto halfway = std::fill_n(samples.begin(), num_samples / 2, 0);
    std::fill(halfway, samples.end(), 24500);

    m_soundbuffer.loadFromSamples(samples.data(), samples.size(), 1, 44100);
    m_sound.setBuffer(m_soundbuffer);
    m_sound.setLoop(true);

    // reset the DT and ST clock
    m_timer_clock.restart();

    // instruction decoding maps
    m_opcodes[0x0000U] = [this](std::uint16_t opcode)
    {
        if(m_opcodes_0.contains(opcode))
            m_opcodes_0[opcode]();
    };
    m_opcodes[0x1000U] = [this](std::uint16_t opcode){ JUMP(opcode); };
    m_opcodes[0x2000U] = [this](std::uint16_t opcode){ CALL(opcode); };
    m_opcodes[0x3000U] = [this](std::uint16_t opcode){ SKE(opcode); };
    m_opcodes[0x4000U] = [this](std::uint16_t opcode){ SKNE(opcode); };
    m_opcodes[0x5000U] = [this](std::uint16_t opcode){ SKRE(opcode); };
    m_opcodes[0x6000U] = [this](std::uint16_t opcode){ LOAD(opcode); };
    m_opcodes[0x7000U] = [this](std::uint16_t opcode){ ADD(opcode); };
    m_opcodes[0x8000U] = [this](std::uint16_t opcode)
    {
        m_opcodes_8.at(opcode & 0xF00FU)(opcode);
    };
    m_opcodes[0x9000U] = [this](std::uint16_t opcode){ SKRNE(opcode); };
    m_opcodes[0xA000U] = [this](std::uint16_t opcode){ LOADI(opcode); };
    m_opcodes[0xB000U] = [this](std::uint16_t opcode){ JUMPI(opcode); };
    m_opcodes[0xC000U] = [this](std::uint16_t opcode){ RAND(opcode); };
    m_opcodes[0xD000U] = [this](std::uint16_t opcode){ DRAW(opcode); };
    m_opcodes[0xE000U] = [this](std::uint16_t opcode)
    {
        m_opcodes_E.at(opcode & 0xF0FFU)(opcode);
    };
    m_opcodes[0xF000U] = [this](std::uint16_t opcode)
    {
        m_opcodes_F.at(opcode & 0xF0FFU)(opcode);
    };

    // 0 opcodes
    m_opcodes_0[0x00E0U] = [this](){ CLS(); };
    m_opcodes_0[0x00EEU] = [this](){ RET(); };

    // 8 opcodes
    m_opcodes_8[0x8000U] = [this](std::uint16_t opcode){ MOVE(opcode); };
    m_opcodes_8[0x8001U] = [this](std::uint16_t opcode){ OR(opcode); };
    m_opcodes_8[0x8002U] = [this](std::uint16_t opcode){ AND(opcode); };
    m_opcodes_8[0x8003U] = [this](std::uint16_t opcode){ XOR(opcode); };
    m_opcodes_8[0x8004U] = [this](std::uint16_t opcode){ ADDR(opcode); };
    m_opcodes_8[0x8005U] = [this](std::uint16_t opcode){ SUB(opcode); };
    m_opcodes_8[0x8006U] = [this](std::uint16_t opcode){ SHR(opcode); };
    m_opcodes_8[0x8007U] = [this](std::uint16_t opcode){ SUBN(opcode); };
    m_opcodes_8[0x800EU] = [this](std::uint16_t opcode){ SHL(opcode); };

    // E opcodes
    m_opcodes_E[0xE09EU] = [this](std::uint16_t opcode){ SKPR(opcode); };
    m_opcodes_E[0xE0A1U] = [this](std::uint16_t opcode){ SKUP(opcode); };

    // F opcodes
    m_opcodes_F[0xF007U] = [this](std::uint16_t opcode){ MOVED(opcode); };
    m_opcodes_F[0xF00AU] = [this](std::uint16_t opcode){ KEYD(opcode); };
    m_opcodes_F[0xF015U] = [this](std::uint16_t opcode){ LOADD(opcode); };
    m_opcodes_F[0xF018U] = [this](std::uint16_t opcode){ LOADS(opcode); };
    m_opcodes_F[0xF01EU] = [this](std::uint16_t opcode){ ADDI(opcode); };
    m_opcodes_F[0xF029U] = [this](std::uint16_t opcode){ LDSPR(opcode); };
    m_opcodes_F[0xF033U] = [this](std::uint16_t opcode){ BCD(opcode); };
    m_opcodes_F[0xF055U] = [this](std::uint16_t opcode){ STOR(opcode); };
    m_opcodes_F[0xF065U] = [this](std::uint16_t opcode){ READ(opcode); };
}


////////////////////////////////////////////////////////////////////////////////
void CHIP8::run()
{
    sf::Clock emulate_clock;                // 500 Hz (default) emulation clock

    while(m_window.isOpen())
    {
        for(sf::Event event; m_window.pollEvent(event);)
        {
            switch(event.type)
            {
                case sf::Event::Closed:
                    m_window.close();
                    break;

                case sf::Event::KeyPressed:
                    update_key(event.key.code, KeyState::Pressed);
                    break;

                case sf::Event::KeyReleased:
                    update_key(event.key.code, KeyState::Released);
                    break;

                default:
                    break;
            }
        }

        constexpr sf::Int64 T_500Hz{2000};  // 2000 us

        // 500 Hz emulation clock
        if(emulate_clock.getElapsedTime().asMicroseconds() >= T_500Hz)
        {
            emulate_clock.restart();
            emulate();
        }

        m_window.clear();
        m_window.draw(m_sprite);
        m_window.display();
    }
}


////////////////////////////////////////////////////////////////////////////////
// Private Member Functions
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
void CHIP8::emulate()
{
    constexpr sf::Int32 T_60Hz{16};      // 60 Hz ~= 16 ms

    // Update 60 Hz delay and sound timers
    if(m_timer_clock.getElapsedTime().asMilliseconds() > T_60Hz)
    {
        if(m_DT > 0U)
            --m_DT;

        if(m_ST > 0U)
            --m_ST;
        else
            m_sound.stop();

        m_timer_clock.restart();
    }

    // extract two bytes from memory and increment program counter
    std::uint16_t opcode = (static_cast<std::uint16_t>(m_memory.at(m_PC) << 8U)
        | static_cast<std::uint16_t>(m_memory.at(m_PC + 1)));
    m_PC += 2;

    // decode opcode
    try
    {
        m_opcodes.at(opcode & 0xF000U)(opcode);
    }
    catch(const std::out_of_range& e)
    {
        fmt::print(stderr, "Illegal opcode {:#04x} at {:#05x}\n", opcode,
            m_PC - 2);
    }
}


////////////////////////////////////////////////////////////////////////////////
void CHIP8::update_key(sf::Keyboard::Key key, KeyState state)
{
    auto it = std::find_if(m_keymap.begin(), m_keymap.end(),
        [key](const auto &k)
        {
            return (k.first == key);
        });

    if(it != m_keymap.end())
    {
        // Compute the index into the keymap and update its state
        auto i = static_cast<gsl::index>(std::distance(m_keymap.begin(), it));
        m_keymap[i].second = state;
    }
}


////////////////////////////////////////////////////////////////////////////////
// Private Member Functions - OPCODES
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
void CHIP8::CLS()
{
    // 00E0 - clear the display
    for(int y{0}; y != height; ++y)
    {
        for(int x{0}; x != width; ++x)
        {
            m_pixels.setPixel(x, y, sf::Color::Black);
        }
    }
}


////////////////////////////////////////////////////////////////////////////////
void CHIP8::RET()
{
    // 00EE - return from a subroutine
    --m_SP;
    m_PC = m_stack.at(m_SP);
}


////////////////////////////////////////////////////////////////////////////////
void CHIP8::JUMP(std::uint16_t opcode)
{
    // 1nnn - jump to location nnn
    m_PC = NNN(opcode);
}


////////////////////////////////////////////////////////////////////////////////
void CHIP8::CALL(std::uint16_t opcode)
{
    // 2nnn - call subroutine at nnn
    m_stack.at(m_SP) = m_PC;
    ++m_SP;
    m_PC = NNN(opcode);
}


////////////////////////////////////////////////////////////////////////////////
void CHIP8::SKE(std::uint16_t opcode)
{
    // 3xnn - skip next instruction if Vx == nn
    if(m_V[X(opcode)] == NN(opcode))
        m_PC += 2;
}


////////////////////////////////////////////////////////////////////////////////
void CHIP8::SKNE(std::uint16_t opcode)
{
    // 4xnn - skip next instruction if Vx != nn
    if(m_V[X(opcode)] != NN(opcode))
        m_PC += 2;
}


////////////////////////////////////////////////////////////////////////////////
void CHIP8::SKRE(std::uint16_t opcode)
{
    // 5xy0 - skip next instruction if Vx == Vy
    if(m_V[X(opcode)] == m_V[Y(opcode)])
        m_PC += 2;
}


////////////////////////////////////////////////////////////////////////////////
void CHIP8::LOAD(std::uint16_t opcode)
{
    // 6xnn - set Vx = nn
    m_V[X(opcode)] = NN(opcode);
}


////////////////////////////////////////////////////////////////////////////////
void CHIP8::ADD(std::uint16_t opcode)
{
    // 7xnn - set Vx = Vx + nn (carry flag not set)
    m_V[X(opcode)] += NN(opcode);
}


////////////////////////////////////////////////////////////////////////////////
void CHIP8::MOVE(std::uint16_t opcode)
{
    // 8xy0 - set Vx = Vy
    m_V[X(opcode)] = m_V[Y(opcode)];
}


////////////////////////////////////////////////////////////////////////////////
void CHIP8::OR(std::uint16_t opcode)
{
    // 8xy1 - Set Vx = Vx OR Vy
    m_V[X(opcode)] |= m_V[Y(opcode)];
}


////////////////////////////////////////////////////////////////////////////////
void CHIP8::AND(std::uint16_t opcode)
{
    // 8xy2 - set Vx = Vx AND Vy
    m_V[X(opcode)] &= m_V[Y(opcode)];
}


////////////////////////////////////////////////////////////////////////////////
void CHIP8::XOR(std::uint16_t opcode)
{
    // 8xy3 - set Vx = Vx XOR Vy
    m_V[X(opcode)] ^= m_V[Y(opcode)];
}


////////////////////////////////////////////////////////////////////////////////
void CHIP8::ADDR(std::uint16_t opcode)
{
    // 8xy4 - set Vx = Vx + Vy, set VF = carry
    std::uint16_t temp = static_cast<std::uint16_t>(m_V[X(opcode)])
        + static_cast<std::uint16_t>(m_V[Y(opcode)]);
    m_V[15] = (temp > 255U) ? 1U : 0U;
    m_V[X(opcode)] = static_cast<std::uint8_t>(temp & 0x00FFU);
}


////////////////////////////////////////////////////////////////////////////////
void CHIP8::SUB(std::uint16_t opcode)
{
    // 8xy5 - set Vx = Vx - Vy, set VF = NOT borrow
    m_V[15] = (m_V[Y(opcode)] > m_V[X(opcode)]) ? 0U : 1U;
    m_V[X(opcode)] -= m_V[Y(opcode)];
}


////////////////////////////////////////////////////////////////////////////////
void CHIP8::SHR(std::uint16_t opcode)
{
    // 8xy6 - set Vx = Vx >> 1, set VF = LSb
    m_V[15] = (m_V[X(opcode)] & 0x01U) ? 1U : 0U;
    m_V[X(opcode)] >>= 1;
}


////////////////////////////////////////////////////////////////////////////////
void CHIP8::SUBN(std::uint16_t opcode)
{
    // 8xy7 - set Vx = Vy - Vx, set VF = NOT borrow
    m_V[15] = (m_V[X(opcode)] > m_V[Y(opcode)]) ? 0U : 1U;
    m_V[X(opcode)] = m_V[Y(opcode)] - m_V[X(opcode)];
}


////////////////////////////////////////////////////////////////////////////////
void CHIP8::SHL(std::uint16_t opcode)
{
    // 8xyE - set Vx = Vx << 1, set VF = MSb
    m_V[15] = (m_V[X(opcode)] & 0x80U) ? 1U : 0U;
    m_V[X(opcode)] <<= 1;
}


////////////////////////////////////////////////////////////////////////////////
void CHIP8::SKRNE(std::uint16_t opcode)
{
    // 9xy0 - skip next instruction if Vx != Vy
    if(m_V[X(opcode)] != m_V[Y(opcode)])
        m_PC += 2;
}


////////////////////////////////////////////////////////////////////////////////
void CHIP8::LOADI(std::uint16_t opcode)
{
    // Annn - set I = nnn
    m_I = NNN(opcode);
}


////////////////////////////////////////////////////////////////////////////////
void CHIP8::JUMPI(std::uint16_t opcode)
{
    // Bnnn - jump to location nnn + V0
    m_PC = static_cast<std::size_t>(m_V[0]) + NNN(opcode);
}


////////////////////////////////////////////////////////////////////////////////
void CHIP8::RAND(std::uint16_t opcode)
{
    // Cxnn - set Vx = random byte and nn
    m_V[X(opcode)] = m_dist(m_engine) & NN(opcode);
}


////////////////////////////////////////////////////////////////////////////////
void CHIP8::DRAW(std::uint16_t opcode)
{
    // Dxyn - display n-byte sprite starting at memory location I at (Vx, Vy),
    //        set VF = collision
    m_V[15] = 0U;

    for(std::uint8_t y_sprite{0}; y_sprite != N(opcode); ++y_sprite)
    {
        for(std::uint8_t x_sprite{0}; x_sprite != 8U; ++x_sprite)
        {
            std::size_t addr = m_I + static_cast<std::size_t>(y_sprite);

            if(m_memory.at(addr) & (0x80U >> x_sprite))
            {
                // compute offset into pixel data, wrapping x and y
                // values if they're out of bounds.
                int x = static_cast<int>(m_V[X(opcode)] + x_sprite);
                int y = static_cast<int>(m_V[Y(opcode)] + y_sprite);

                if(m_pixels.getPixel(x, y) == sf::Color::White)
                {
                    m_V[15] = 1U;
                    m_pixels.setPixel(x, y, sf::Color::Black);
                }
                else
                {
                    m_pixels.setPixel(x, y, sf::Color::White);
                }
            }
        }
    }

    m_texture.update(m_pixels);
}


////////////////////////////////////////////////////////////////////////////////
void CHIP8::SKPR(std::uint16_t opcode)
{
    // Ex9E - skip next instruction if key with the value of Vx is pressed
    if(m_keymap.at(m_V[X(opcode)]).second == KeyState::Pressed)
        m_PC += 2;
}


////////////////////////////////////////////////////////////////////////////////
void CHIP8::SKUP(std::uint16_t opcode)
{
    // ExA1 - skip next instruction if key with the value of Vx is not pressed
    if(m_keymap.at(m_V[X(opcode)]).second != KeyState::Pressed)
        m_PC += 2;
}


////////////////////////////////////////////////////////////////////////////////
void CHIP8::MOVED(std::uint16_t opcode)
{
    // Fx07 - set Vx = delay time value
    m_V[X(opcode)] = m_DT;
}


////////////////////////////////////////////////////////////////////////////////
void CHIP8::KEYD(std::uint16_t opcode)
{
    // Fx0A - wait for key press, store the value of the key in Vx
    if(!m_key_captured)
    {
        auto it = std::find_if(m_keymap.begin(), m_keymap.end(),
            [](const auto& k)
            {
                return (k.second == KeyState::Pressed);
            });

        if(it != m_keymap.end())
        {
            m_V[X(opcode)] = static_cast<std::uint8_t>(
                std::distance(m_keymap.begin(), it));
            m_key_captured = true;
        }

        m_PC -= 2;
    }
    else
    {
        if(m_keymap.at(m_V[X(opcode)]).second == KeyState::Released)
            m_key_captured = false;
        else
            m_PC -= 2;
    }
}


////////////////////////////////////////////////////////////////////////////////
void CHIP8::LOADD(std::uint16_t opcode)
{
    // Fx15 - set delay time = Vx
    m_DT = m_V[X(opcode)];
}


////////////////////////////////////////////////////////////////////////////////
void CHIP8::LOADS(std::uint16_t opcode)
{
    // Fx18 - set sound time = Vx
    m_ST = m_V[X(opcode)];

    if(m_ST > 0)
        m_sound.play();
}


////////////////////////////////////////////////////////////////////////////////
void CHIP8::ADDI(std::uint16_t opcode)
{
    // Fx1E - set I = I + Vx (carry flag not set)
    m_I += static_cast<std::size_t>(m_V[X(opcode)]);
}


////////////////////////////////////////////////////////////////////////////////
void CHIP8::LDSPR(std::uint16_t opcode)
{
    // Fx29 - set I = location of sprite for digit Vx
    m_I = static_cast<std::size_t>(m_V[X(opcode)] * 5U);
}


////////////////////////////////////////////////////////////////////////////////
void CHIP8::BCD(std::uint16_t opcode)
{
    // Fx33 - store BCD representation of Vx in memory locations I, I+1, and I+2
    m_memory.at(m_I)     = m_V[X(opcode)] / 100U;
    m_memory.at(m_I + 1) = (m_V[X(opcode)] / 10U) % 10U;
    m_memory.at(m_I + 2) = m_V[X(opcode)] % 10U;
}


////////////////////////////////////////////////////////////////////////////////
void CHIP8::STOR(std::uint16_t opcode)
{
    // Fx55 - store registers V0 through Vx in memory starting at location I
    for(gsl::index i{0}; i <= X(opcode); ++i)
    {
        m_memory.at(m_I) = m_V[i];
        ++m_I;
    }
}


////////////////////////////////////////////////////////////////////////////////
void CHIP8::READ(std::uint16_t opcode)
{
    // Fx65 - read registers V0 through Vx from memory starting at location I
    for(gsl::index i{0}; i <= X(opcode); ++i)
    {
        m_V[i] = m_memory.at(m_I);
        ++m_I;
    }
}
