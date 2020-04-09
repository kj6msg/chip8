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
// File Name : chip8.cpp
// Project   : CHIP-8
// Author    : Ryan Clarke
// E-mail    : kj6msg@icloud.com
// -----------------------------------------------------------------------------
// Purpose   : Class definition and helper functions for the CHIP-8 emulator.
////////////////////////////////////////////////////////////////////////////////

#include <algorithm>
#include <array>
#include "chip8.h"
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <iterator>
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <utility>
#include <vector>


////////////////////////////////////////////////////////////////////////////////
/// \brief Generate tone
///
/// This function generates a square wave to be used when the sound timer is
/// active in the CHIP8.
//
/// \param frequency Frequency of square wave
/// \param sample_rate Sample rate of sound buffer
///
/// \return Vector holding one period of samples
///
////////////////////////////////////////////////////////////////////////////////
static std::vector<sf::Int16> square_wave(unsigned int frequency,
                                          unsigned int sample_rate);

////////////////////////////////////////////////////////////////////////////////
/// \brief Extract type from opcode
///
/// This function extracts the type of operation from an \a opcode.
///
/// \param opcode Opcode
///
/// \return Opcode type
///
////////////////////////////////////////////////////////////////////////////////
static uint8_t type(uint16_t opcode);

////////////////////////////////////////////////////////////////////////////////
/// \brief Extract NNN from opcode
///
/// This function extracts the 12-bit NNN value from an \a opcode.
///
/// \param opcode Opcode
///
/// \return NNN
///
////////////////////////////////////////////////////////////////////////////////
static uint16_t NNN(uint16_t opcode);

////////////////////////////////////////////////////////////////////////////////
/// \brief Extract NN from opcode
///
/// This function extracts the 8-bit NN value from an \a opcode.
///
/// \param opcode Opcode
///
/// \return NN
///
////////////////////////////////////////////////////////////////////////////////
static uint8_t NN(uint16_t opcode);

////////////////////////////////////////////////////////////////////////////////
/// \brief Extract N from opcode
///
/// This function extracts the 4-bit N value from an \a opcode.
///
/// \param opcode Opcode
///
/// \return N
///
////////////////////////////////////////////////////////////////////////////////
static uint8_t N(uint16_t opcode);

////////////////////////////////////////////////////////////////////////////////
/// \brief Extract Vx from opcode
///
/// This function extracts 4-bit register index from an \a opcode.
///
/// \param opcode Opcode
///
/// \return Register index
///
////////////////////////////////////////////////////////////////////////////////
static uint8_t VX(uint16_t opcode);

////////////////////////////////////////////////////////////////////////////////
/// \brief Extract Vy from opcode
///
/// This function extracts 4-bit register index from an \a opcode.
///
/// \param opcode Opcode
///
/// \return Register index
///
////////////////////////////////////////////////////////////////////////////////
static uint8_t VY(uint16_t opcode);


////////////////////////////////////////////////////////////////////////////////
// Static Data
////////////////////////////////////////////////////////////////////////////////

// default keyboard map for CHIP8 hexademical keypad
static const std::array<sf::Keyboard::Key, 16> default_keymap
{
    sf::Keyboard::X,
    sf::Keyboard::Num1,
    sf::Keyboard::Num2,
    sf::Keyboard::Num3,
    sf::Keyboard::Q,
    sf::Keyboard::W,
    sf::Keyboard::E,
    sf::Keyboard::A,
    sf::Keyboard::S,
    sf::Keyboard::D,
    sf::Keyboard::Z,
    sf::Keyboard::C,
    sf::Keyboard::Num4,
    sf::Keyboard::R,
    sf::Keyboard::F,
    sf::Keyboard::V
};


// 8x5 font for each hexadecimal digit
static const std::array<uint8_t, 80> font
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
CHIP8::CHIP8(const std::vector<uint8_t>& program)
{
    // copy font and program to CHIP8 memory
    memory.resize(mem_size);
    std::copy(font.begin(), font.end(), memory.begin());
    std::copy(program.begin(), program.end(), memory.begin() + org);

    // set default key mapping
    m_keymap.resize(16);
    for(std::size_t i = 0; i != m_keymap.size(); ++i)
        m_keymap[i] = std::make_pair(default_keymap[i], KeyState::Released);
    
    // create the CHIP8 display and clear it
    m_texture.create(width, height);
    m_pixels.assign(width * height, 0);
    m_texture.update(reinterpret_cast<const sf::Uint8*>(m_pixels.data()));
    m_sprite.setTexture(m_texture);

    // generate the sound data for the sound timer
    auto samples = square_wave(1050, 44100);
    m_soundbuffer.loadFromSamples(samples.data(), samples.size(), 1, 44100);
    m_sound.setBuffer(m_soundbuffer);
    m_sound.setLoop(true);

    // reset the DT and ST clock
    timer_clock.restart();
}


////////////////////////////////////////////////////////////////////////////////
void CHIP8::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
    target.draw(m_sprite, states);
}


////////////////////////////////////////////////////////////////////////////////
void CHIP8::emulate()
{
    constexpr int T_60Hz = 16;      // 60 Hz ~= 16 ms

    // Update 60 Hz delay and sound timers
    if(timer_clock.getElapsedTime().asMilliseconds() > T_60Hz)
    {
        if(DT > 0)
            --DT;
        if(ST > 0)
            --ST;
        
        if(!ST)
            m_sound.stop();

        timer_clock.restart();
    }
    
    // extract two bytes from memory and increment program counter
    uint16_t opcode = (memory.at(PC) << 8) | memory.at(PC + 1);
    PC += 2;

    // opcode decoder
    switch(type(opcode))
    {
        case 0x0:
            switch(NNN(opcode))
            {
                case 0x0E0:
                    // CLR
                    // clear screen
                    std::fill(m_pixels.begin(), m_pixels.end(), 0);
                    break;
                
                case 0x0EE:
                    // RET
                    // return from subroutine
                    --SP;
                    PC = stack.at(SP);
                    break;
                
                default:
                    // call RCA 1802 program ... treat as NOP
                    break;
            }
            break;
        
        case 0x1:
            // JP addr
            // jump to address NNN
            PC = NNN(opcode);
            break;
        
        case 0x2:
            // CALL addr
            // call subroutine at NNN
            stack.at(SP) = PC;
            ++SP;
            PC = NNN(opcode);
            break;
        
        case 0x3:
            // SE Vx, byte
            // skip next instruction if VX == NN
            if(V[VX(opcode)] == NN(opcode))
                PC += 2;
            break;
        
        case 0x4:
            // SNE Vx, byte
            // skip next instruction if VX != NN
            if(V[VX(opcode)] != NN(opcode))
                PC += 2;
            break;
        
        case 0x5:
            // SE Vx, Vy
            // skip next instruction if VX == VY
            if(V[VX(opcode)] == V[VY(opcode)])
                PC += 2;
            break;
        
        case 0x6:
            // LD Vx, byte
            // VX = NN
            V[VX(opcode)] = NN(opcode);
            break;
        
        case 0x7:
            // ADD Vx, byte
            // VX = VX + NN (VF carry not set)
            V[VX(opcode)] += NN(opcode);
            break;
        
        case 0x8:
            switch(N(opcode))
            {
                case 0x0:
                    // LD Vx, Vy
                    // VX = VY
                    V[VX(opcode)] = V[VY(opcode)];
                    break;
                
                case 0x1:
                    // OR Vx, Vy
                    // VX = VX | VY
                    V[VX(opcode)] |= V[VY(opcode)];
                    break;
                
                case 0x2:
                    // AND Vx, Vy
                    // VX = VX & VY
                    V[VX(opcode)] &= V[VY(opcode)];
                    break;
                
                case 0x3:
                    // XOR Vx, Vy
                    // VX = VX ^ VY
                    V[VX(opcode)] ^= V[VY(opcode)];
                    break;
                
                case 0x4:
                {
                    // ADD Vx, Vy
                    // VX = VX + VY (VF set to 1 if carry)
                    uint16_t temp = V[VX(opcode)] + V[VY(opcode)];
                    V[15] = (temp > 255) ? 1 : 0;
                    V[VX(opcode)] = temp & 0xFF;
                    break;
                }
                
                case 0x5:
                    // SUB Vx, Vy
                    // VX = VX - VY (VF set to 0 if borrow)
                    V[15] = (V[VY(opcode)] > V[VX(opcode)]) ? 0 : 1;
                    V[VX(opcode)] -= V[VY(opcode)];
                    break;
                
                case 0x6:
                    // SHR Vx
                    // VX = VX >> 1 (VF set to LSb)
                    V[15] = (V[VX(opcode)] & 0x01) ? 1 : 0;
                    V[VX(opcode)] >>= 1;
                    break;
                
                case 0x7:
                    // SUBN Vx, Vy
                    // VX = VY - VX (VF set to 0 if borrow)
                    V[15] = (V[VX(opcode)] > V[VY(opcode)]) ? 0 : 1;
                    V[VX(opcode)] = V[VY(opcode)] - V[VX(opcode)];
                    break;
                
                case 0xE:
                    // SHL Vx
                    // VX = VX << 1 (VF set to MSb)
                    V[15] = (V[VX(opcode)] & 0x80) ? 1 : 0;
                    V[VX(opcode)] <<= 1;
                    break;
                
                default:
                    // illegal opcode
                    std::cerr << "Illegal opcode 0x"
                              << std::hex
                              << opcode
                              << " at address 0x"
                              << PC - 2
                              << std::dec
                              << '\n';
                    break;
            }
            break;
        
        case 0x9:
            // SNE Vx, Vy
            // skip next instruction if VX != VY
            if(V[VX(opcode)] != V[VY(opcode)])
                PC += 2;
            break;
        
        case 0xA:
            // LD I, addr
            // I = NNN
            I = NNN(opcode);
            break;
        
        case 0xB:
            // JP V0
            // PC = V0 + NNN
            PC = V[0] + NNN(opcode);
            break;
        
        case 0xC:
            // RND Vx, byte
            // VX = random & NN
            V[VX(opcode)] = (std::rand() % 256) & NN(opcode);
            break;
        
        case 0xD:
            // DRW Vx, Vy, nibble
            // draw sprite pointed to by I of height N at (VX,VY)
            // VF set to 1 if pixels flipped from set/unset (I unchanged)
            V[15] = 0;

            for(uint8_t y_sprite = 0; y_sprite < N(opcode); ++y_sprite)
            {
                for(uint8_t x_sprite = 0; x_sprite < 8; ++x_sprite)
                {
                    if(memory.at(I + y_sprite) & (0x80 >> x_sprite))
                    {
                        // compute offset into pixel data, wrapping x and y
                        // values if they're out of bounds.
                        uint8_t x = (V[VX(opcode)] + x_sprite) & (width - 1);
                        uint8_t y = (V[VY(opcode)] + y_sprite) & (height - 1);
                        uint16_t offset = x + (width * y);
                        
                        if(m_pixels[offset])
                            V[15] = 1;
                        
                        m_pixels[offset] ^= 0xFFFF'FFFF;
                    }
                }
            }

            m_texture.update(reinterpret_cast<const sf::Uint8*>(m_pixels.data()));
            break;

        case 0xE:
            switch(NN(opcode))
            {
                case 0x9E:
                    // SKP Vx
                    // skip next instruction if VX == key pressed
                    if(m_keymap.at(V[VX(opcode)]).second == KeyState::Pressed)
                        PC += 2;
                    break;
                
                case 0xA1:
                    // SKNP Vx
                    // skip next instruction if VX != key pressed
                    if(m_keymap.at(V[VX(opcode)]).second != KeyState::Pressed)
                        PC += 2;
                    break;
                
                default:
                    // illegal opcode
                    std::cerr << "Illegal opcode 0x"
                              << std::hex
                              << opcode
                              << " at address 0x"
                              << PC - 2
                              << std::dec
                              << '\n';
                    break;
            }
            break;
        
        case 0xF:
            switch(NN(opcode))
            {
                case 0x07:
                    // LD Vx, DT
                    // VX = DT
                    V[VX(opcode)] = DT;
                    break;
                
                case 0x0A:
                    // LD Vx, K
                    // VX = keypress (blocking)
                    if(!m_key_captured)
                    {
                        auto key_it = std::find_if(m_keymap.begin(),
                                                   m_keymap.end(),
                                                   [](const auto& k){return k.second == KeyState::Pressed;});
                        
                        if(key_it != m_keymap.end())
                        {
                            V[VX(opcode)] = std::distance(m_keymap.begin(), key_it);
                            m_key_captured = true;
                        }

                        PC -= 2;
                    }
                    else
                    {
                        if(m_keymap.at(V[VX(opcode)]).second == KeyState::Released)
                            m_key_captured = false;
                        else
                            PC -= 2;
                    }
                    break;
                
                case 0x15:
                    // LD DT, Vx
                    // DT = VX
                    DT = V[VX(opcode)];
                    break;
                
                case 0x18:
                    // LD ST, Vx
                    // ST = VX
                    ST = V[VX(opcode)];

                    if(ST)
                        m_sound.play();
                    
                    break;
                
                case 0x1E:
                    // ADD I, Vx
                    // I = I + VX (VF set to 1 if overflow)
                    I += V[VX(opcode)];
                    V[15] = (I > 0x0FFF) ? 1 : 0;
                    break;
                
                case 0x29:
                    // LD F, Vx
                    // set I to sprite address for character in VX
                    I = V[VX(opcode)] * 5;
                    break;
                
                case 0x33:
                    // LD B, Vx
                    // set I to BCD representation of VX, MSd at I, LSd at I+2
                    memory.at(I) = V[VX(opcode)] / 100;
                    memory.at(I + 1) = (V[VX(opcode)] / 10) % 10;
                    memory.at(I + 2) = V[VX(opcode)] % 10;
                    break;
                
                case 0x55:
                    // LD [I], Vx
                    // store V0-VX at address I (I auto incremented)
                    for(std::size_t i = 0; i <= VX(opcode); ++i)
                    {
                        memory.at(I) = V[i];
                        ++I;
                    }
                    break;
                
                case 0x65:
                    // LD Vx, [I]
                    // fill V0-VX with values at address I (I auto incremented)
                    for(std::size_t i = 0; i <= VX(opcode); ++i)
                    {
                        V[i] = memory.at(I);
                        ++I;
                    }
                    break;
                
                default:
                    // illegal opcode
                    std::cerr << "Illegal opcode 0x"
                              << std::hex
                              << opcode
                              << " at address 0x"
                              << PC - 2
                              << std::dec
                              << '\n';
                    break;
            }
            break;

        default:
            // illegal opcode
            std::cerr << "Illegal opcode 0x"
                      << std::hex
                      << opcode
                      << " at address 0x"
                      << PC - 2
                      << std::dec
                      << '\n';
            break;
    }
}


////////////////////////////////////////////////////////////////////////////////
void CHIP8::key_press(sf::Keyboard::Key key)
{
    // Find the key in the keymap
    auto key_iter = std::find_if(m_keymap.begin(), m_keymap.end(),
                                 [key](const auto& k){return k.first == key;});
    
    if(key_iter != m_keymap.end())
    {
        // Compute the index into the keymap and update its state
        auto index = std::distance(m_keymap.begin(), key_iter);
        m_keymap[index].second = KeyState::Pressed;
    }
}


////////////////////////////////////////////////////////////////////////////////
void CHIP8::key_release(sf::Keyboard::Key key)
{
    // Find the key in the keymap
    auto key_iter = std::find_if(m_keymap.begin(), m_keymap.end(),
                                 [key](const auto& k){return k.first == key;});
    
    if(key_iter != m_keymap.end())
    {
        // Compute the indeo into the keymap and update its state
        auto index = std::distance(m_keymap.begin(), key_iter);
        m_keymap[index].second = KeyState::Released;
    }
}


////////////////////////////////////////////////////////////////////////////////
static std::vector<sf::Int16> square_wave(unsigned int frequency,
                                          unsigned int sample_rate)
{
    unsigned int num_samples = sample_rate / frequency;
    std::vector<sf::Int16> samples(num_samples);
    
    for(unsigned int n = 0; n != num_samples / 2; ++n)
        samples[n] = 0;
    
    for(unsigned int n = num_samples / 2; n != num_samples; ++n)
        samples[n] = 24500;

    return samples;
}


////////////////////////////////////////////////////////////////////////////////
static inline uint8_t type(uint16_t opcode)
{
    return (opcode & 0xF000) >> 12;
}


////////////////////////////////////////////////////////////////////////////////
static inline uint16_t NNN(uint16_t opcode)
{
    return opcode & 0x0FFF;
}


////////////////////////////////////////////////////////////////////////////////
static inline uint8_t NN(uint16_t opcode)
{
    return opcode & 0x00FF;
}


////////////////////////////////////////////////////////////////////////////////
static inline uint8_t N(uint16_t opcode)
{
    return opcode & 0x000F;
}


////////////////////////////////////////////////////////////////////////////////
static inline uint8_t VX(uint16_t opcode)
{
    return (opcode & 0x0F00) >> 8;
}


////////////////////////////////////////////////////////////////////////////////
static inline uint8_t VY(uint16_t opcode)
{
    return (opcode & 0x00F0) >> 4;
}
