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
// File Name : chip8.h
// Project   : CHIP-8
// Author    : Ryan Clarke
// E-mail    : kj6msg@icloud.com
// -----------------------------------------------------------------------------
// Purpose   : Class declaration for the CHIP-8 emulator.
////////////////////////////////////////////////////////////////////////////////

#ifndef CHIP8_H
#define CHIP8_H


#include <array>
#include <cstdint>
#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <utility>
#include <vector>


////////////////////////////////////////////////////////////////////////////////
/// \brief CHIP8 emulator
///
////////////////////////////////////////////////////////////////////////////////
class CHIP8 : public sf::Drawable
{
public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Default constructor
    ///
    /// Deleted.
    ///
    ////////////////////////////////////////////////////////////////////////////
    CHIP8() = delete;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Copy constructor
    ///
    /// Deleted.
    ///
    ////////////////////////////////////////////////////////////////////////////
    CHIP8(const CHIP8&) = delete;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Move constructor
    ///
    /// Deleted.
    ///
    ////////////////////////////////////////////////////////////////////////////
    CHIP8(CHIP8&&) = delete;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Construct a new CHIP8 emulator
    ///
    /// This constructor creates the CHIP8 emulator and loads it with the
    /// program data passed in \a program.
    ///
    /// \param program Program data
    ///
    ////////////////////////////////////////////////////////////////////////////
    CHIP8(const std::vector<uint8_t>& program);
    
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Copy assignment
    ///
    /// Deleted.
    ///
    ////////////////////////////////////////////////////////////////////////////
    CHIP8& operator=(const CHIP8&) = delete;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Move assignment
    ///
    /// Deleted.
    ///
    ////////////////////////////////////////////////////////////////////////////
    CHIP8& operator=(CHIP8&&) = delete;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Emulate the CHIP8 for one emulation cycle
    ///
    /// This function updates the DT and ST timers, fetches an opcode from the
    /// CHIP8 memory, and decodes it.
    ///
    ////////////////////////////////////////////////////////////////////////////
    void emulate();

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Process a key press event
    ///
    /// This function finds the key \a key in the keymap and updates its status
    /// to pressed. If \a key does not exist, it does nothing.
    ///
    /// \param key Key code
    ///
    ////////////////////////////////////////////////////////////////////////////
    void key_press(sf::Keyboard::Key key);

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Process a key release event
    ///
    /// This function finds the key \a key in the keymap and updates its status
    /// to released. If \a key does not exist, it does nothing.
    ///
    /// \param key Key code
    ///
    ////////////////////////////////////////////////////////////////////////////
    void key_release(sf::Keyboard::Key key);

    ////////////////////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////////////////////
    static constexpr unsigned int mem_size{4096};   //!< CHIP8 memory size
    static constexpr unsigned int org{512};         //!< Program start address
    static constexpr unsigned int width{64};        //!< Screen width
    static constexpr unsigned int height{32};       //!< Screen height
    
private:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Key state enumeration
    ///
    ////////////////////////////////////////////////////////////////////////////
    enum class KeyState
    {
        Released,                       //!< Key released
        Pressed                         //!< Key pressed
    };
    
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Draw the CHIP8 screen to a render target
    ///
    /// This function is an override from the \a sf::Drawable base class and is
    /// used to draw the CHIP8 screen directly to a render target.
    ///
    /// \param target Render target to draw to
    /// \param states Current render states
    ///
    ////////////////////////////////////////////////////////////////////////////
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;

    ////////////////////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////////////////////
    uint16_t PC{org};                   //!< Program counter
    uint8_t  SP{0};                     //!< Stack pointer
    uint16_t I{0};                      //!< Index register
    std::array<uint8_t, 16> V{};        //!< General purpose registers

    std::vector<uint8_t> memory;        //!< Program and data memory
    std::array<uint16_t, 16> stack{};   //!< Call stack

    uint8_t DT{0};                      //!< Delay timer
    uint8_t ST{0};                      //!< Sound timer
    sf::Clock timer_clock;              //!< Clock for DT and ST timers

    std::vector<std::pair<sf::Keyboard::Key, KeyState>> m_keymap;   //!< Key map
    bool m_key_captured{false};         //!< Wait for key press state variable

    std::vector<uint32_t> m_pixels;     //!< Pixel data
    sf::Texture m_texture;              //!< CHIP8 texture surface
    sf::Sprite m_sprite;                //!< Sprite used for drawing

    sf::SoundBuffer m_soundbuffer;      //!< Sound buffer for sound timer
    sf::Sound m_sound;                  //!< Sound for sound buffer
};


#endif  // CHIP8_H
