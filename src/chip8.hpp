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

#ifndef CHIP8_H
#define CHIP8_H

#include <array>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <map>
#include <random>
#include <utility>
#include <vector>
#include <gsl/util>
#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>


////////////////////////////////////////////////////////////////////////////////
/// \brief CHIP8 emulator
///
////////////////////////////////////////////////////////////////////////////////
class CHIP8
{
public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Construct a new CHIP8 emulator and load it with program data
    /// \param program Program data
    ////////////////////////////////////////////////////////////////////////////
    explicit CHIP8(const std::vector<std::uint8_t>& program);

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Main emulation loop for the CHIP8
    ////////////////////////////////////////////////////////////////////////////
    void run();

    static constexpr std::size_t mem_size{4096};    // Memory size
    static constexpr std::size_t org{512};          // Program start address
    static constexpr int width{64};                 // Screen width
    static constexpr int height{32};                // Screen height
    static constexpr int Fs{44100};                 // Sample rate
    static constexpr int tone{1050};                // Tone frequency

private:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Key states
    ////////////////////////////////////////////////////////////////////////////
    enum class KeyState
    {
        Released,
        Pressed
    };

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Update the DT and ST timers, and fetch and decode an opcode
    ////////////////////////////////////////////////////////////////////////////
    void emulate();

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Update key state of pressed/released key
    /// \param key Key code
    /// \param state Key state
    ////////////////////////////////////////////////////////////////////////////
    void update_key(sf::Keyboard::Key key, KeyState state);

    ////////////////////////////////////////////////////////////////////////////////
    /// \brief Extract NNN from opcode
    /// \param opcode Opcode
    /// \return NNN
    ////////////////////////////////////////////////////////////////////////////////
    std::size_t NNN(std::uint16_t opcode) const noexcept
    {
        return static_cast<std::size_t>(opcode & 0x0FFFU);
    }

    ////////////////////////////////////////////////////////////////////////////////
    /// \brief Extract NN from opcode
    /// \param opcode Opcode
    /// \return NN
    ////////////////////////////////////////////////////////////////////////////////
    std::uint8_t NN(std::uint16_t opcode) const noexcept
    {
        return static_cast<std::uint8_t>(opcode & 0x00FFU);
    }

    ////////////////////////////////////////////////////////////////////////////////
    /// \brief Extract N from opcode
    /// \param opcode Opcode
    /// \return N
    ////////////////////////////////////////////////////////////////////////////////
    std::uint8_t N(std::uint16_t opcode) const noexcept
    {
        return static_cast<std::uint8_t>(opcode & 0x000FU);
    }

    ////////////////////////////////////////////////////////////////////////////////
    /// \brief Extract Vx from opcode
    /// \param opcode Opcode
    /// \return Register index
    ////////////////////////////////////////////////////////////////////////////////
    gsl::index X(std::uint16_t opcode) const noexcept
    {
        return static_cast<gsl::index>((opcode & 0x0F00U) >> 8U);
    }

    ////////////////////////////////////////////////////////////////////////////////
    /// \brief Extract Vy from opcode
    /// \param opcode Opcode
    /// \return Register index
    ////////////////////////////////////////////////////////////////////////////////
    gsl::index Y(std::uint16_t opcode) const noexcept
    {
        return static_cast<gsl::index>((opcode & 0x00F0U) >> 4U);
    }

    ////////////////////////////////////////////////////////////////////////////
    // Opcode Functions
    ////////////////////////////////////////////////////////////////////////////
    void CLS();                         // 00E0 - CLS
    void RET();                         // 00EE - RET
    void JUMP(std::uint16_t opcode);    // 1nnn - JP addr
    void CALL(std::uint16_t opcode);    // 2nnn - CALL addr
    void SKE(std::uint16_t opcode);     // 3xnn - SE Vx, byte
    void SKNE(std::uint16_t opcode);    // 4xnn - SNE Vx, byte
    void SKRE(std::uint16_t opcode);    // 5xy0 - SE Vx, Vy
    void LOAD(std::uint16_t opcode);    // 6xnn - LD Vx, byte
    void ADD(std::uint16_t opcode);     // 7xnn - ADD Vx, byte
    void MOVE(std::uint16_t opcode);    // 8xy0 - LD Vx, Vy
    void OR(std::uint16_t opcode);      // 8xy1 - OR Vx, Vy
    void AND(std::uint16_t opcode);     // 8xy2 - AND Vx, Vy
    void XOR(std::uint16_t opcode);     // 8xy3 - XOR Vx, Vy
    void ADDR(std::uint16_t opcode);    // 8xy4 - ADD Vx, Vy
    void SUB(std::uint16_t opcode);     // 8xy5 - SUB Vx, Vy
    void SHR(std::uint16_t opcode);     // 8xy6 - SHR Vx
    void SUBN(std::uint16_t opcode);    // 8xy7 - SUBN Vx, Vy
    void SHL(std::uint16_t opcode);     // 8xyE - SHL Vx
    void SKRNE(std::uint16_t opcode);   // 9xy0 - SNE Vx, Vy
    void LOADI(std::uint16_t opcode);   // Annn - LD I, addr
    void JUMPI(std::uint16_t opcode);   // Bnnn - JP V0, addr
    void RAND(std::uint16_t opcode);    // Cxnn - RND Vx, byte
    void DRAW(std::uint16_t opcode);    // Dxyn - DRW Vx, Vy, nibble
    void SKPR(std::uint16_t opcode);    // Ex9E - SKP Vx
    void SKUP(std::uint16_t opcode);    // ExA1 - SKNP Vx
    void MOVED(std::uint16_t opcode);   // Fx07 - LD Vx, DT
    void KEYD(std::uint16_t opcode);    // Fx0A - LD Vx, K
    void LOADD(std::uint16_t opcode);   // Fx15 - LD DT, Vx
    void LOADS(std::uint16_t opcode);   // Fx18 - LD ST, Vx
    void ADDI(std::uint16_t opcode);    // Fx1E - ADD I, Vx
    void LDSPR(std::uint16_t opcode);   // Fx29 - LD F, Vx
    void BCD(std::uint16_t opcode);     // Fx33 - LD B, Vx
    void STOR(std::uint16_t opcode);    // Fx55 - LD [I], Vx
    void READ(std::uint16_t opcode);    // Fx65 - LD Vx, [I]

    ////////////////////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////////////////////
    std::size_t m_PC{org};                      // Program counter
    std::size_t m_SP{0};                        // Stack pointer
    std::size_t m_I{0};                         // Index register
    std::array<std::uint8_t, 16> m_V{};         // General purpose registers

    std::vector<std::uint8_t> m_memory;         // Program and data memory
    std::array<std::size_t, 16> m_stack{};      // Call stack

    std::uint8_t m_DT{0};                       // Delay timer
    std::uint8_t m_ST{0};                       // Sound timer
    sf::Clock m_timer_clock;                    // Clock for DT and ST timers

    std::default_random_engine m_engine;        // Random number engine
    std::uniform_int_distribution<std::uint8_t> m_dist; // 0-255 distribution

    std::vector<std::pair<sf::Keyboard::Key, KeyState>> m_keymap; // Key map
    bool m_key_captured{false};                 // Key press state variable

    sf::Image m_pixels;                         // Pixel data
    sf::Texture m_texture;                      // CHIP8 texture surface
    sf::Sprite m_sprite;                        // Sprite used for drawing
    sf::RenderWindow m_window;                  // Application window

    sf::SoundBuffer m_soundbuffer;              // Buffer for sound timer
    sf::Sound m_sound;                          // Sound for sound buffer

    // Opcode function maps
    std::map<std::uint16_t, std::function<void(std::uint16_t)>> m_opcodes;
    std::map<std::uint16_t, std::function<void()>> m_opcodes_0;
    std::map<std::uint16_t, std::function<void(std::uint16_t)>> m_opcodes_8;
    std::map<std::uint16_t, std::function<void(std::uint16_t)>> m_opcodes_E;
    std::map<std::uint16_t, std::function<void(std::uint16_t)>> m_opcodes_F;
};


#endif  // CHIP8_H
