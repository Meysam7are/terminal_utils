/*
* MIT License
*
* Copyright (c) 2021-2024 Meysam Zare
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*/

#ifndef MZ_CONSOLE_CMD_H
#define MZ_CONSOLE_CMD_H
#pragma once

/**
 * @file ConsoleCMD.h
 * @brief Cross-platform terminal manipulation utilities
 *
 * This header provides a comprehensive set of functions and constants for
 * terminal control, including cursor positioning, text formatting, and
 * special character rendering. It uses ANSI escape sequences that work on
 * modern terminals across Windows, macOS, and Linux systems.
 *
 * @author Meysam Zare
 */


#include <vector>
#include <string>
#include <string_view>
#include <format>
#include <cstdint>

 // Platform detection
#if defined(_WIN32) || defined(_WIN64) || defined(_MSC_VER)
#define MZ_PLATFORM_WINDOWS
#include <conio.h>
#elif defined(__APPLE__) || defined(__MACH__)
#define MZ_PLATFORM_MACOS
#include <termios.h>
#include <unistd.h>
#elif defined(__linux__) || defined(__unix__) || defined(__unix)
#define MZ_PLATFORM_UNIX
#include <termios.h>
#include <unistd.h>
#else
#define MZ_PLATFORM_UNKNOWN
#endif

// Define wide character support
#define WIDECHARS

namespace mz {

    /**
     * @struct console_picture
     * @brief Represents a bitmap or graphical element for console display
     *
     * This structure stores a simple bitmap with dimensions and pixel data
     * that can be rendered in a terminal using block characters or other
     * rendering techniques.
     */
    struct console_picture {
        int Width{ 0 };      ///< Width of the picture in character cells
        int Height{ 0 };     ///< Height of the picture in character cells
        std::vector<int> Pixels;  ///< Pixel data (color/character information)
    };

    /**
     * @struct control_keys
     * @brief Tracks the state of various control keys
     *
     * This structure uses bit fields to efficiently store and check
     * the state of control keys like Return, Escape, arrows, etc.
     * It provides both named access via bit fields and raw access via an integer value.
     */
    struct control_keys {
        // Bit flag constants for each control key
        static constexpr int RET{ 1 };          ///< Return/Enter key flag
        static constexpr int ESC{ 2 };          ///< Escape key flag
        static constexpr int INS{ 4 };          ///< Insert key flag
        static constexpr int DEL{ 8 };          ///< Delete key flag
        static constexpr int TAB{ 16 };         ///< Tab key flag
        static constexpr int SPACE{ 32 };       ///< Space key flag
        static constexpr int UPDOWN{ 64 };      ///< Up/Down arrows flag
        static constexpr int HOMEEND{ 128 };    ///< Home/End keys flag
        static constexpr int LEFTRIGHT{ 256 };  ///< Left/Right arrows flag
        static constexpr int PAGEUPDOWN{ 512 }; ///< Page Up/Down keys flag

        // Union for dual access to the key states
        union {
            // Bit field structure for named access
            struct {
                int Return : 1;  ///< Return/Enter key state
                int Escape : 1;  ///< Escape key state
                int Insert : 1;  ///< Insert key state
                int Delete : 1;  ///< Delete key state
                int Tab : 1;  ///< Tab key state
                int Space : 1;  ///< Space key state
                int UpDown : 1;  ///< Up/Down arrows state
                int HomeEnd : 1;  ///< Home/End keys state
                int LeftRight : 1;  ///< Left/Right arrows state
                int PageUpDown : 1;  ///< Page Up/Down keys state
            };
            int value{ 0 };  ///< Combined value for all key states
        };

        /**
         * @brief Default constructor
         */
        constexpr control_keys() noexcept = default;

        /**
         * @brief Constructor from integer value
         * @param Value Integer representing key states
         */
        constexpr control_keys(int Value) noexcept : value{ Value } {}
    };

    /**
     * @struct sym
     * @brief Represents a Unicode character or symbol pair
     *
     * This structure encapsulates Unicode symbols, particularly those
     * that may require surrogate pairs or other multi-code-unit representations.
     */
    struct sym {
        wchar_t Symbol[2]{ 0,0 };  ///< Array to store the symbol (up to 2 wide characters)

        /**
         * @brief Constructor from a 32-bit integer
         *
         * Creates a symbol from a 32-bit integer where the low 16 bits
         * represent the first code unit and the high 16 bits represent
         * the second code unit.
         *
         * @param X32 32-bit integer encoding two 16-bit code units
         */
        explicit constexpr sym(int X32) noexcept : Symbol{ wchar_t(X32), wchar_t(X32 >> 16) } {}

        /**
         * @brief Constructor from two integers
         *
         * Creates a symbol from two separate integers representing
         * the low and high code units.
         *
         * @param XL Low code unit
         * @param XH High code unit
         */
        explicit constexpr sym(int XL, int XH) noexcept : Symbol{ wchar_t(XL), wchar_t(XH) } {}
    };

    //=========================================================================
    // KEY CODE CONSTANTS
    //=========================================================================

    /**
     * @name Key Code Constants
     * @brief Virtual key codes for special keys
     *
     * These constants define the virtual key codes for various special keys,
     * encoded in a cross-platform manner. For extended keys, the high 16 bits
     * contain the extended code and the low 16 bits contain the base code.
     */
     ///@{
    static constexpr int RETURNKEY{ 13 };                 ///< Return/Enter key
    static constexpr int ESCAPEKEY{ 27 };                 ///< Escape key
    static constexpr int SPACEKEY{ 32 };                  ///< Space key
    static constexpr int BACKSPACEKEY{ 8 };               ///< Backspace key
    static constexpr int UPKEY{ (72 << 16) | 224 };       ///< Up arrow key
    static constexpr int DOWNKEY{ (80 << 16) | 224 };     ///< Down arrow key
    static constexpr int HOMEKEY{ (71 << 16) | 224 };     ///< Home key
    static constexpr int ENDKEY{ (79 << 16) | 224 };      ///< End key
    static constexpr int LEFTKEY{ (75 << 16) | 224 };     ///< Left arrow key
    static constexpr int RIGHTKEY{ (77 << 16) | 224 };    ///< Right arrow key
    static constexpr int INSERTKEY{ (82 << 16) | 224 };   ///< Insert key
    static constexpr int DELETEKEY{ (83 << 16) | 224 };   ///< Delete key
    static constexpr int PAGEUPKEY{ (73 << 16) | 224 };   ///< Page Up key
    static constexpr int PAGEDOWNKEY{ (81 << 16) | 224 }; ///< Page Down key
    ///@}

    //=========================================================================
    // UNICODE SYMBOL CONSTANTS
    //=========================================================================

    /**
     * @name Unicode Symbol Constants
     * @brief Predefined Unicode symbols for terminal graphics
     *
     * These constants define various Unicode symbols for box drawing,
     * block elements, arrows, and other special characters useful for
     * creating terminal user interfaces.
     */
     ///@{
    static constexpr wchar_t const LEFTHALF[2]{ 0x96E2, 0x8C };    ///< Left half block
    static constexpr wchar_t const RIGHTHALF[2]{ 0x96E2, 0x90 };   ///< Right half block
    static constexpr wchar_t const LHEAD{ 0x82cb };                ///< Left-pointing head
    static constexpr wchar_t const RHEAD{ 0x83cb };                ///< Right-pointing head
    static constexpr wchar_t const LLQUOTE[1]{ 0xABC2 };           ///< Left double quote
    static constexpr wchar_t const RRQUOTE[1]{ 0xBBC2 };           ///< Right double quote
    static constexpr wchar_t const ELLIPSIS[2]{ 0x80E2, 0xA6 };    ///< Ellipsis (...)
    static constexpr wchar_t const LFARROW[2]{ 0x86E2, 0x90 };     ///< Left arrow
    static constexpr wchar_t const UPARROW[2]{ 0x86E2, 0x91 };     ///< Up arrow
    static constexpr wchar_t const RTARROW[2]{ 0x86E2, 0x92 };     ///< Right arrow
    static constexpr wchar_t const DNARROW[2]{ 0x86E2, 0x93 };     ///< Down arrow
    static constexpr wchar_t const LRARROW[2]{ 0x86E2, 0x94 };     ///< Left-right arrow
    static constexpr wchar_t const UDARROW[2]{ 0x86E2, 0x95 };     ///< Up-down arrow
    static constexpr wchar_t const TRILEFT[2]{ 0x97E2, 0x80 };     ///< Left-pointing triangle
    static constexpr wchar_t const ENDLINE2[2]{ L'\n', 0 };        ///< Newline character
    static constexpr wchar_t const BLOCK00[2]{ L' ', 0 };          ///< Empty block (space)
    static constexpr wchar_t const BLOCK25[2]{ 0x96E2, 0x91 };     ///< 25% filled block
    static constexpr wchar_t const BLOCK50[2]{ 0x96E2, 0x92 };     ///< 50% filled block
    static constexpr wchar_t const BLOCK75[2]{ 0x96E2, 0x93 };     ///< 75% filled block
    static constexpr wchar_t const BLOCK100[2]{ 0x96E2, 0x88 };    ///< 100% filled block
    static constexpr wchar_t const LOWERHALF[2]{ 0x96E2, 0x84 };   ///< Lower half block
    static constexpr wchar_t const LEFTBOTTOMCORNER[2]{ 0x96E2, 0x99 }; ///< Bottom-left corner

    // Block elements for gradient rendering
    static constexpr wchar_t const BBLOCK1[2]{ 0x96E2, 0x81 };     ///< Bottom 1/8 block
    static constexpr wchar_t const BBLOCK2[2]{ 0x96E2, 0x82 };     ///< Bottom 2/8 block
    static constexpr wchar_t const BBLOCK3[2]{ 0x96E2, 0x83 };     ///< Bottom 3/8 block
    static constexpr wchar_t const BBLOCK4[2]{ 0x96E2, 0x84 };     ///< Bottom 4/8 block
    static constexpr wchar_t const BBLOCK5[2]{ 0x96E2, 0x85 };     ///< Bottom 5/8 block
    static constexpr wchar_t const BBLOCK6[2]{ 0x96E2, 0x86 };     ///< Bottom 6/8 block
    static constexpr wchar_t const BBLOCK7[2]{ 0x96E2, 0x87 };     ///< Bottom 7/8 block
    static constexpr wchar_t const FULLBLOCK[2]{ 0x96E2, 0x88 };   ///< Full block

    // Left-side blocks
    static constexpr wchar_t const LBLOCK7[2]{ 0x96E2, 0x89 };     ///< Left 7/8 block
    static constexpr wchar_t const LBLOCK6[2]{ 0x96E2, 0x8A };     ///< Left 6/8 block
    static constexpr wchar_t const LBLOCK5[2]{ 0x96E2, 0x8B };     ///< Left 5/8 block
    static constexpr wchar_t const LBLOCK4[2]{ 0x96E2, 0x8C };     ///< Left 4/8 block
    static constexpr wchar_t const LBLOCK3[2]{ 0x96E2, 0x8D };     ///< Left 3/8 block
    static constexpr wchar_t const LBLOCK2[2]{ 0x96E2, 0x8E };     ///< Left 2/8 block
    static constexpr wchar_t const LBLOCK1[2]{ 0x96E2, 0x8F };     ///< Left 1/8 block

    // Other block types
    static constexpr wchar_t const TBLOCK1[2]{ 0x96E2, 0x94 };     ///< Top 1/8 block
    static constexpr wchar_t const RBLOCK1[2]{ 0x96E2, 0x95 };     ///< Right 1/8 block

    // Diagonal and corner characters
    static constexpr wchar_t const LRDIAG[2]{ 0x96E2, 0x9A };      ///< Diagonal (bottom-left to top-right)
    static constexpr wchar_t const RLDIAG[2]{ 0x96E2, 0x9E };      ///< Diagonal (top-left to bottom-right)
    static constexpr wchar_t const BLQUAD[2]{ 0x96E2, 0x96 };      ///< Bottom-left quadrant
    static constexpr wchar_t const BRQUAD[2]{ 0x96E2, 0x97 };      ///< Bottom-right quadrant
    static constexpr wchar_t const TLQUAD[2]{ 0x96E2, 0x98 };      ///< Top-left quadrant
    static constexpr wchar_t const TRQUAD[2]{ 0x96E2, 0x9D };      ///< Top-right quadrant
    static constexpr wchar_t const BLCORNER[2]{ 0x96E2, 0x99 };    ///< Bottom-left corner
    static constexpr wchar_t const TLCORNER[2]{ 0x96E2, 0x9B };    ///< Top-left corner
    static constexpr wchar_t const TRCORNER[2]{ 0x96E2, 0x9C };    ///< Top-right corner
    static constexpr wchar_t const BRCORNER[2]{ 0x96E2, 0x9F };    ///< Bottom-right corner

    // Square characters
    static constexpr wchar_t const FSQUARE[2]{ 0x96E2, 0xA0 };     ///< Full square
    static constexpr wchar_t const ESQUARE[2]{ 0x96E2, 0xA1 };     ///< Empty square
    static constexpr wchar_t const RSQUARE[2]{ 0x96E2, 0xA2 };     ///< Rounded square
    static constexpr wchar_t const DSQUARE[2]{ 0x96E2, 0xA3 };     ///< Dotted square
    static constexpr wchar_t const FULLSQUARE[2]{ 0x96E2, 0xa0 };  ///< Full square (alias)

    // Triangle characters
    static constexpr wchar_t const TRIUP[2]{ 0x96E2, 0xb2 };       ///< Up-pointing triangle
    static constexpr wchar_t const TRIRIGHT[2]{ 0x96E2, 0xb6 };    ///< Right-pointing triangle
    static constexpr wchar_t const TRIDOWN[2]{ 0x96E2, 0xbc };     ///< Down-pointing triangle
    ///@}

    /**
     * @namespace cmd
     * @brief ANSI terminal command sequences
     *
     * This namespace contains ANSI escape sequences for terminal control,
     * including screen clearing, cursor movement, and visibility settings.
     */
    namespace cmd {
        static constexpr wchar_t const CLS_0[5]{ L"\x1b[0J" };      ///< Clear from cursor to end of screen
        static constexpr wchar_t const CLS_1[5]{ L"\x1b[1J" };      ///< Clear from cursor to beginning of screen
        static constexpr wchar_t const CLS_2[5]{ L"\x1b[2J" };      ///< Clear entire screen
        static constexpr wchar_t const SAVE_POS[3]{ L"\x1b""7" };   ///< Save cursor position
        static constexpr wchar_t const LOAD_POS[3]{ L"\x1b""8" };   ///< Restore cursor position

        static constexpr wchar_t const MOVE_UP[4]{ L"\x1b[A" };     ///< Move cursor up one line
        static constexpr wchar_t const MOVE_DN[4]{ L"\x1b[B" };     ///< Move cursor down one line
        static constexpr wchar_t const MOVE_LT[4]{ L"\x1b[D" };     ///< Move cursor left one column
        static constexpr wchar_t const MOVE_RT[4]{ L"\x1b[C" };     ///< Move cursor right one column

        static constexpr wchar_t const SET_SHOW[7]{ L"\x1b[?25h" }; ///< Show cursor
        static constexpr wchar_t const CLR_SHOW[7]{ L"\x1b[?25l" }; ///< Hide cursor
        static constexpr wchar_t const SET_HIDE[7]{ L"\x1b[?12h" }; ///< Enable cursor blinking
        static constexpr wchar_t const CLR_HIDE[7]{ L"\x1b[?12l" }; ///< Disable cursor blinking

        static constexpr wchar_t const CLR_COLOR[5]{ L"\x1b[0m" };  ///< Reset all text attributes

        static constexpr int RgbLength{ 19 };                       ///< Length of an RGB color escape sequence
        static constexpr int ColorLength{ RgbLength * 2 };          ///< Length of foreground+background color sequence
        static constexpr int CoordLength{ 12 };                     ///< Length of a coordinate escape sequence
    }

    //=========================================================================
    // CHARACTER CLASSIFICATION FUNCTIONS
    //=========================================================================

    /**
     * @brief Check if character is a digit
     * @param c Character to check
     * @return true if character is a digit (0-9)
     */
    inline constexpr bool IsDigitCharacter(int c) noexcept { return c >= '0' && c <= '9'; }

    /**
     * @brief Check if character is a lowercase letter
     * @param c Character to check
     * @return true if character is a lowercase letter (a-z)
     */
    inline constexpr bool IsLowerCase(int c) noexcept { return c >= 'a' && c <= 'z'; }

    /**
     * @brief Check if character is an uppercase letter
     * @param c Character to check
     * @return true if character is an uppercase letter (A-Z)
     */
    inline constexpr bool IsUpperCase(int c) noexcept { return c >= 'A' && c <= 'Z'; }

    /**
     * @brief Check if character is alphanumeric
     * @param c Character to check
     * @return true if character is a letter or digit
     */
    inline constexpr bool IsAlphaNumeric(int c) noexcept {
        return IsLowerCase(c) || IsUpperCase(c) || IsDigitCharacter(c);
    }

    /**
     * @brief Check if character is suitable for display
     * @param c Character to check
     * @return true if character can be displayed in the terminal
     */
    bool IsDisplayCharacter(int c) noexcept;

    /**
     * @brief Check if character is valid in a filename
     * @param c Character to check
     * @return true if character is allowed in filenames
     */
    bool IsFilenameCharacter(int c) noexcept;

    //=========================================================================
    // STRING CONVERSION FUNCTIONS
    //=========================================================================

    /**
     * @brief Convert wide string to signed integer
     * @param Text String to convert
     * @return Signed integer value
     */
    long long string_to_signed(std::wstring_view Text) noexcept;

    /**
     * @brief Convert wide string to unsigned integer
     * @param Text String to convert
     * @return Unsigned integer value
     */
    long long string_to_unsigned(std::wstring_view Text) noexcept;

    /**
     * @brief Validate username constraints
     * @param Text Username to validate
     * @param MinLen Minimum allowed length
     * @param MaxLen Maximum allowed length
     * @return Error code (0 if valid, non-zero for specific errors)
     */
    int username_error(std::wstring const& Text, long long MinLen, long long MaxLen) noexcept;

    /**
     * @brief Cross-platform wide character input function
     *
     * Reads a wide character from the console, handling extended keys
     * that produce multiple characters or special key codes.
     *
     * @return Key code (possibly with extended key info in high 16 bits)
     */
    inline int wgetch() noexcept {
#ifdef MZ_PLATFORM_WINDOWS
        int w = _getwch();
        if (!w || w == 224) {
            w |= int(_getwch()) << 16;
        }
        return w;
#else
        // Non-Windows implementation using termios for raw input
        struct termios oldSettings, newSettings;
        tcgetattr(STDIN_FILENO, &oldSettings);
        newSettings = oldSettings;
        newSettings.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &newSettings);

        int ch = getchar();
        if (ch == 27) { // Escape sequence
            // Handle arrow keys and other escape sequences
            ch = getchar();
            if (ch == '[') {
                ch = getchar();
                switch (ch) {
                case 'A': return UPKEY;
                case 'B': return DOWNKEY;
                case 'C': return RIGHTKEY;
                case 'D': return LEFTKEY;
                case 'H': return HOMEKEY;
                case 'F': return ENDKEY;
                    // Extended sequences like page up/down, insert, delete
                case '2':
                    ch = getchar(); // Get '~'
                    return INSERTKEY;
                case '3':
                    ch = getchar(); // Get '~'
                    return DELETEKEY;
                case '5':
                    ch = getchar(); // Get '~'
                    return PAGEUPKEY;
                case '6':
                    ch = getchar(); // Get '~'
                    return PAGEDOWNKEY;
                }
            }
            return ESCAPEKEY;
        }

        tcsetattr(STDIN_FILENO, TCSANOW, &oldSettings);
        return ch;
#endif
    }

    /**
     * @brief Format a monetary value as a dollar string
     * @param Value Value to format (in cents if InCents=true)
     * @param InCents Whether Value is in cents
     * @return Formatted dollar string (e.g., "$12.34")
     */
    [[nodiscard]] std::wstring DollarString(int64_t Value, bool InCents) noexcept;

    /**
     * @brief Format a file size with appropriate units
     * @param Length Size in bytes
     * @param Binary Whether to use binary units (KiB, MiB) or decimal units (KB, MB)
     * @return Formatted file size string (e.g., "1.23 MB")
     */
    [[nodiscard]] std::wstring FileLengthString(int64_t Length, bool Binary) noexcept;

    //=========================================================================
    // CONSOLE OUTPUT FUNCTIONS
    //=========================================================================

    /**
     * @brief Write a single character to stdout
     * @param c Character to write
     */
    inline void Write(char c) noexcept { fwrite(&c, 1, 1, stdout); }

    /**
     * @brief Write a wide character to stdout
     * @param c Wide character to write
     */
    inline void Write(wchar_t c) noexcept { fwrite(&c, 2, 1, stdout); }

    /**
     * @brief Write a string view to stdout
     * @param sv String view to write
     */
    inline void Write(std::string_view sv) noexcept { fwrite(sv.data(), 1, sv.size(), stdout); }

    /**
     * @brief Write a wide string view to stdout
     * @param sv Wide string view to write
     */
    inline void Write(std::wstring_view sv) noexcept { fwrite(sv.data(), 2, sv.size(), stdout); }

    /**
     * @brief Write a character buffer to stdout
     * @param Ptr Pointer to the character buffer
     * @param Size Number of characters to write
     */
    inline void Write(char const* Ptr, size_t Size) noexcept { fwrite(Ptr, 1, Size, stdout); }

    /**
     * @brief Write a wide character buffer to stdout
     * @param Ptr Pointer to the wide character buffer
     * @param Size Number of wide characters to write
     */
    inline void Write(wchar_t const* Ptr, size_t Size) noexcept { fwrite(Ptr, 2, Size, stdout); }

    /**
     * @brief Write a symbol to stdout
     * @param S Symbol to write
     */
    inline void Write(sym S) noexcept { fwrite(S.Symbol, 2, 2, stdout); }

    //=========================================================================
    // STRING MANIPULATION HELPERS
    //=========================================================================

    /**
     * @brief Append a wide character array to a string
     * @tparam N Size of the character array
     * @param Buff String buffer to append to
     * @param L Character array to append
     */
    template <size_t N>
        requires (N > 0 && N < 6)
    inline void PushBack(std::wstring& Buff, const wchar_t(&L)[N]) noexcept { Buff.append(L, N); }

    /**
     * @brief Write a wide character array to stdout
     * @tparam N Size of the character array
     * @param L Character array to write
     */
    template <size_t N>
        requires (N > 0 && N < 6)
    inline void PushBack(const wchar_t(&L)[N]) noexcept { Write(L, N); }

    //=========================================================================
    // CURSOR CONTROL FUNCTIONS - DIRECT OUTPUT
    //=========================================================================

    /**
     * @brief Make the cursor visible
     */
    inline void SetShow() noexcept { Write(L"\x1b[?25h"); }

    /**
     * @brief Make the cursor invisible
     */
    inline void SetHide() noexcept { Write(L"\x1b[?25l"); }

    /**
     * @brief Enable cursor blinking
     */
    inline void SetBlink() noexcept { Write(L"\x1b[?12h"); }

    /**
     * @brief Disable cursor blinking
     */
    inline void ClrBlink() noexcept { Write(L"\x1b[?12l"); }

    /**
     * @brief Reset cursor shape to default
     */
    inline void ResetShape() noexcept { Write(L"\x1b[0 q"); }

    /**
     * @brief Set cursor shape
     * @param Shape Shape code (1-6)
     *
     * Shape codes:
     * 1: Blinking block
     * 2: Steady block
     * 3: Blinking underscore
     * 4: Steady underscore
     * 5: Blinking vertical bar
     * 6: Steady vertical bar
     */
    inline void SetShape(int Shape) noexcept { Write(std::format(L"\x1b[{} q", Shape)); }

    /**
     * @brief Set cursor to blinking block shape
     */
    inline void SetShapeBBLOCK() noexcept { Write(L"\x1b[1 q"); }

    /**
     * @brief Set cursor to steady block shape
     */
    inline void SetShapeSBLOCK() noexcept { Write(L"\x1b[2 q"); }

    /**
     * @brief Set cursor to blinking underscore shape
     */
    inline void SetShapeBUNDER() noexcept { Write(L"\x1b[3 q"); }

    /**
     * @brief Set cursor to steady underscore shape
     */
    inline void SetShapeSUNDER() noexcept { Write(L"\x1b[4 q"); }

    /**
     * @brief Set cursor to blinking vertical bar shape
     */
    inline void SetShapeBBAR() noexcept { Write(L"\x1b[5 q"); }

    /**
     * @brief Set cursor to steady vertical bar shape
     */
    inline void SetShapeSBAR() noexcept { Write(L"\x1b[6 q"); }

    //=========================================================================
    // CURSOR CONTROL FUNCTIONS - STRING BUFFERED
    //=========================================================================

    /**
     * @brief Append cursor show command to a string buffer
     * @param Buff String buffer to append to
     */
    inline void SetShow(std::wstring& Buff) noexcept { Buff.append(L"\x1b[?25h"); }

    /**
     * @brief Append cursor hide command to a string buffer
     * @param Buff String buffer to append to
     */
    inline void SetHide(std::wstring& Buff) noexcept { Buff.append(L"\x1b[?25l"); }

    /**
     * @brief Append cursor blink enable command to a string buffer
     * @param Buff String buffer to append to
     */
    inline void SetBlink(std::wstring& Buff) noexcept { Buff.append(L"\x1b[?12h"); }

    /**
     * @brief Append cursor blink disable command to a string buffer
     * @param Buff String buffer to append to
     */
    inline void ClrBlink(std::wstring& Buff) noexcept { Buff.append(L"\x1b[?12l"); }

    /**
     * @brief Append cursor shape reset command to a string buffer
     * @param Buff String buffer to append to
     */
    inline void ResetShape(std::wstring& Buff) noexcept { Buff.append(L"\x1b[0 q"); }

    /**
     * @brief Append cursor shape set command to a string buffer
     * @param Buff String buffer to append to
     * @param Shape Shape code (1-6)
     */
    inline void SetShape(std::wstring& Buff, int Shape) noexcept {
        Buff.append(std::format(L"\x1b[{} q", Shape));
    }

    //=========================================================================
    // TEXT FORMATTING FUNCTIONS - STRING BUFFERED
    //=========================================================================

    /**
     * @brief Append color reset command to a string buffer
     * @param Buff String buffer to append to
     */
    inline void ResetColor(std::wstring& Buff) noexcept { Buff.append(L"\x1b[0m"); }

    /**
     * @brief Append background color set command to a string buffer
     * @param Buf String buffer to append to
     * @param R Red component (0-255)
     * @param G Green component (0-255)
     * @param B Blue component (0-255)
     */
    inline void SetBackColor(std::wstring& Buf, int R, int G, int B) noexcept {
        Buf.append(std::format(L"\x1b[48;2;{:0>3d};{:0>3d};{:0>3d}m", R, G, B));
    }

    /**
     * @brief Append foreground color set command to a string buffer
     * @param Buf String buffer to append to
     * @param R Red component (0-255)
     * @param G Green component (0-255)
     * @param B Blue component (0-255)
     */
    inline void SetFrontColor(std::wstring& Buf, int R, int G, int B) noexcept {
        Buf.append(std::format(L"\x1b[38;2;{:0>3d};{:0>3d};{:0>3d}m", R, G, B));
    }

    /**
     * @brief Append bold text enable command to a string buffer
     * @param Buff String buffer to append to
     */
    inline void SetBold(std::wstring& Buff) noexcept { Buff.append(L"\x1b[1m"); Buff.push_back(0); }

    /**
     * @brief Append bold text disable command to a string buffer
     * @param Buff String buffer to append to
     */
    inline void ClrBold(std::wstring& Buff) noexcept { Buff.append(L"\x1b[22m"); }

    /**
     * @brief Append underline enable command to a string buffer
     * @param Buff String buffer to append to
     */
    inline void SetUnderline(std::wstring& Buff) noexcept { Buff.append(L"\x1b[4m"); Buff.push_back(0); }

    /**
     * @brief Append underline disable command to a string buffer
     * @param Buff String buffer to append to
     */
    inline void ClrUnderline(std::wstring& Buff) noexcept { Buff.append(L"\x1b[24m"); }

    /**
     * @brief Disable underline (direct output)
     */
    inline void ClrUnderline() noexcept { Write(L"\x1b[24m"); }

    /**
     * @brief Append inverse video (negative) enable command to a string buffer
     * @param Buff String buffer to append to
     */
    inline void SetNegative(std::wstring& Buff) noexcept { Buff.append(L"\x1b[7m"); Buff.push_back(0); }

    /**
     * @brief Append inverse video (negative) disable command to a string buffer
     * @param Buff String buffer to append to
     */
    inline void ClrNegative(std::wstring& Buff) noexcept { Buff.append(L"\x1b[27m"); }

    /**
     * @brief Enable inverse video (direct output)
     */
    inline void SetNegative() noexcept { mz::Write(L"\x1b[7m"); }

    /**
     * @brief Disable inverse video (direct output)
     */
    inline void ClrNegative() noexcept { mz::Write(L"\x1b[27m"); }

    //=========================================================================
    // CURSOR MOVEMENT FUNCTIONS - STRING BUFFERED
    //=========================================================================

    /**
     * @brief Append cursor up command to a string buffer
     * @param Buff String buffer to append to
     */
    inline void MoveUp(std::wstring& Buff) noexcept { Buff.append(L"\x1b[A"); }

    /**
     * @brief Append cursor down command to a string buffer
     * @param Buff String buffer to append to
     */
    inline void MoveDown(std::wstring& Buff) noexcept { Buff.append(L"\x1b[B"); }

    /**
     * @brief Append cursor left command to a string buffer
     * @param Buff String buffer to append to
     */
    inline void MoveLeft(std::wstring& Buff) noexcept { Buff.append(L"\x1b[D"); }

    /**
     * @brief Append cursor right command to a string buffer
     * @param Buff String buffer to append to
     */
    inline void MoveRight(std::wstring& Buff) noexcept { Buff.append(L"\x1b[C"); }

    /**
     * @brief Append multiple cursor up commands to a string buffer
     * @param Buff String buffer to append to
     * @param Count Number of positions to move
     */
    inline void MoveUp(std::wstring& Buff, int Count) noexcept { Buff.append(std::format(L"\x1b[{:0>4d}A", Count)); }

    /**
     * @brief Append multiple cursor down commands to a string buffer
     * @param Buff String buffer to append to
     * @param Count Number of positions to move
     */
    inline void MoveDown(std::wstring& Buff, int Count) noexcept { Buff.append(std::format(L"\x1b[{:0>4d}B", Count)); }

    /**
     * @brief Append multiple cursor left commands to a string buffer
     * @param Buff String buffer to append to
     * @param Count Number of positions to move
     */
    inline void MoveLeft(std::wstring& Buff, int Count) noexcept { Buff.append(std::format(L"\x1b[{:0>4d}D", Count)); }

    /**
     * @brief Append multiple cursor right commands to a string buffer
     * @param Buff String buffer to append to
     * @param Count Number of positions to move
     */
    inline void MoveRight(std::wstring& Buff, int Count) noexcept { Buff.append(std::format(L"\x1b[{:0>4d}C", Count)); }

    //=========================================================================
    // CURSOR MOVEMENT FUNCTIONS - DIRECT OUTPUT
    //=========================================================================

    /**
     * @brief Move cursor up one position (direct output)
     */
    inline void MoveUp() noexcept { Write(L"\x1b[A"); }

    /**
     * @brief Move cursor down one position (direct output)
     */
    inline void MoveDown() noexcept { Write(L"\x1b[B"); }

    /**
     * @brief Move cursor left one position (direct output)
     */
    inline void MoveLeft() noexcept { Write(L"\x1b[D"); }

    /**
     * @brief Move cursor right one position (direct output)
     */
    inline void MoveRight() noexcept { Write(L"\x1b[C"); }

    /**
     * @brief Move cursor up multiple positions (direct output)
     * @param Count Number of positions to move
     */
    inline void MoveUp(int Count) noexcept { Write(std::format(L"\x1b[{}A", Count)); }

    /**
     * @brief Move cursor down multiple positions (direct output)
     * @param Count Number of positions to move
     */
    inline void MoveDown(int Count) noexcept { Write(std::format(L"\x1b[{}B", Count)); }

    /**
     * @brief Move cursor left multiple positions (direct output)
     * @param Count Number of positions to move
     */
    inline void MoveLeft(int Count) noexcept { Write(std::format(L"\x1b[{}D", Count)); }

    /**
     * @brief Move cursor right multiple positions (direct output)
     * @param Count Number of positions to move
     */
    inline void MoveRight(int Count) noexcept { Write(std::format(L"\x1b[{}C", Count)); }

    //=========================================================================
    // CURSOR POSITIONING FUNCTIONS - STRING BUFFERED
    //=========================================================================

    /**
     * @brief Append set row command to a string buffer
     * @param Buff String buffer to append to
     * @param Row Target row (1-based)
     */
    inline void SetRow(std::wstring& Buff, unsigned Row) noexcept { Buff.append(std::format(L"\x1b[{:0>4d}G", Row)); }

    /**
     * @brief Append set column command to a string buffer
     * @param Buff String buffer to append to
     * @param Col Target column (1-based)
     */
    inline void SetCol(std::wstring& Buff, unsigned Col) noexcept { Buff.append(std::format(L"\x1b[{:0>4d}b", Col)); }

    /**
     * @brief Append set position command to a string buffer
     * @param Buff String buffer to append to
     * @param Row Target row (1-based)
     * @param Col Target column (1-based)
     */
    inline void SetPos(std::wstring& Buff, unsigned Row, unsigned Col) noexcept {
        Buff.append(std::format(L"\x1b[{:0>4d};{:0>4d}H", Row, Col));
    }

    //=========================================================================
    // TERMINAL CONTROL FUNCTIONS - DIRECT OUTPUT
    //=========================================================================

    /**
     * @brief Move cursor to absolute position (direct output)
     * @param Row Target row (1-based)
     * @param Col Target column (1-based)
     */
    inline void MoveTo(int Row, int Col) noexcept { Write(std::format(L"\x1b[{:0>4d};{:0>4d}H", Row, Col)); }

    /**
     * @brief Set terminal window title (wide string)
     * @param Title New window title
     */
    inline void SetTitle(std::wstring const& Title) noexcept { Write(std::format(L"\x1b]2;{}\x1b\\", Title)); }

    /**
     * @brief Set terminal window title (narrow string)
     * @param Title New window title
     */
    inline void SetTitle(std::string const& Title) noexcept { Write(std::format("\x1b]2;{}\x1b\\", Title)); }

    /**
     * @brief Switch to default screen buffer
     */
    inline void DefaultScreenBuffer() noexcept { Write(L"\x1b[?1049l"); }

    /**
     * @brief Switch to alternate screen buffer
     */
    inline void AlternateScreenBuffer() noexcept { Write(L"\x1b[?1049h"); }

    /**
     * @brief Save cursor position (direct output)
     */
    inline void SavePos() noexcept { Write(cmd::SAVE_POS); }

    /**
     * @brief Restore cursor position (direct output)
     */
    inline void LoadPos() noexcept { Write(cmd::LOAD_POS); }

    /**
     * @brief Set cursor position (direct output)
     * @param Row Target row (1-based)
     * @param Col Target column (1-based)
     */
    inline void SetPos(int Row, int Col) { Write(std::format(L"\x1b[{:0>4d};{:0>4d}H", Row, Col)); }

    //=========================================================================
    // POSITION MANIPULATION FUNCTIONS - STRING BUFFERED
    //=========================================================================

    /**
     * @brief Append save position command to a string buffer
     * @param W String buffer to append to
     */
    inline void SavePos(std::wstring& W) noexcept { W.append(cmd::SAVE_POS); }

    /**
     * @brief Append restore position command to a string buffer
     * @param W String buffer to append to
     */
    inline void LoadPos(std::wstring& W) noexcept { W.append(cmd::LOAD_POS); }

    //=========================================================================
    // COMPOSITE FUNCTIONS
    //=========================================================================

    /**
     * @brief Print text at specific position and restore cursor position
     * @param Row Target row (1-based)
     * @param Col Target column (1-based)
     * @param W Text to print
     */
    inline void PrintAt(int Row, int Col, std::wstring_view W) noexcept {
        SavePos();
        SetPos(Row, Col);
        Write(W);
        LoadPos();
    }

    /**
     * @brief Append text to a string buffer
     * @param bf String buffer to append to
     * @param wv Text to append
     * @return Number of characters appended
     */
    inline int Append(std::wstring& bf, std::wstring_view wv) noexcept {
        bf.append(wv);
        return int(wv.size());
    }

    /**
     * @brief Append text to a string buffer and move cursor back to start of text
     * @param bf String buffer to append to
     * @param wv Text to append
     * @return Number of characters appended
     *
     * This function is useful for overwriting text in place.
     */
    inline int InPlace(std::wstring& bf, std::wstring_view wv) noexcept {
        int Size = int(wv.size());
        bf.append(wv);
        MoveLeft(bf, Size);
        return Size;
    }

} // namespace mz

#endif // MZ_CONSOLE_CMD_H
