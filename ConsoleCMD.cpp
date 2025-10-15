/*
* MIT License
*
* Copyright (c) 2025 Meysam Zare
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

/**
 * @file ConsoleCMD.cpp
 * @brief Implementation of terminal utility functions
 *
 * This file contains the implementation of various terminal utility functions
 * for character classification, string conversion, formatting, and screen
 * manipulation. These functions are designed to work across different platforms
 * with appropriate cross-platform adaptations.
 *
 * @author Meysam Zare
 * @date October 14, 2025
 */

#include "ConsoleCMD.h"
#include <format>
#include <climits>  // For LLONG_MIN, LLONG_MAX
#include <cstdint>  // For int64_t, uint64_t

 // Platform-specific includes and defines
#if defined(MZ_PLATFORM_WINDOWS)
#include <Windows.h>
#elif defined(MZ_PLATFORM_MACOS) || defined(MZ_PLATFORM_UNIX)
#include <sys/ioctl.h>
#include <unistd.h>
#endif

namespace mz {

    // Thread-local storage for Windows API calls (only used on Windows)
#ifdef MZ_PLATFORM_WINDOWS
    static thread_local DWORD L;
#endif

    /**
     * @brief Checks if a character is valid in filenames
     *
     * Determines whether a character is allowed in standard file names
     * across most file systems. This includes alphanumeric characters
     * and a subset of safe special characters.
     *
     * @param c Character code to check
     * @return true if the character is valid in filenames, false otherwise
     */
    bool IsFilenameCharacter(int c) noexcept
    {
        // Alphanumeric characters are always allowed
        if (IsAlphaNumeric(c)) return true;

        // Check specific allowed special characters
        switch (c)
        {
        case '~':   // Tilde
        case '`':   // Backtick
        case '!':   // Exclamation mark
        case '@':   // At sign
        case '#':   // Hash/pound sign
        case '$':   // Dollar sign
        case '%':   // Percent sign
        case '^':   // Caret
        case '&':   // Ampersand
        case '(':   // Opening parenthesis
        case ')':   // Closing parenthesis
        case '-':   // Hyphen/minus
        case '+':   // Plus sign
        case '_':   // Underscore
        case '=':   // Equals sign
        case ';':   // Semicolon
        case '\'':  // Single quote
        case ',':   // Comma
        case '.':   // Period
        case ' ':   // Space (though can cause issues in some contexts)
        {
            return true;
        }
        default:
            return false;
        }
    }

    /**
     * @brief Checks if a character is suitable for display
     *
     * Determines whether a character is appropriate for display in terminal
     * output. This includes all filename characters plus additional
     * characters that might be used in user interfaces.
     *
     * @param c Character code to check
     * @return true if the character can be displayed, false otherwise
     */
    bool IsDisplayCharacter(int c) noexcept
    {
        // All filename characters are displayable
        if (IsFilenameCharacter(c)) return true;

        // Additional displayable characters that aren't recommended for filenames
        switch (c)
        {
        case '\\':  // Backslash
        case '/':   // Forward slash
        case ':':   // Colon
        case '*':   // Asterisk
        case '?':   // Question mark
        case '"':   // Double quote
        case '<':   // Less than
        case '>':   // Greater than
        case '|':   // Vertical bar/pipe
        {
            return true;
        }
        default:
            return false;
        }
    }

    /**
     * @brief Converts a wide string to an unsigned integer
     *
     * Parses a wide string containing only digits to an unsigned long long.
     * Returns LLONG_MIN if the string is invalid or too large.
     *
     * @param sv Wide string view containing the number to parse
     * @return Parsed unsigned value, or LLONG_MIN if invalid
     */
    long long string_to_unsigned(std::wstring_view sv) noexcept
    {
        int ndigits{ 0 };
        long long Res{ 0 };

        // Empty string is invalid
        if (sv.empty()) { return LLONG_MIN; }

        // Process each character
        for (auto c : sv) {
            // Limit digits to 17 (to avoid overflow) and ensure it's a digit
            if (++ndigits < 18 && c >= '0' && c <= '9') {
                Res *= 10;  // Shift existing value
                Res += long long(c - '0');  // Add new digit
            }
            else {
                // Not a valid digit or too many digits
                return LLONG_MIN;
            }
        }
        return Res;
    }

    /**
     * @brief Converts a wide string to a signed integer
     *
     * Parses a wide string representing an integer (with optional sign)
     * to a signed long long. Returns LLONG_MIN if the string is invalid.
     *
     * @param sv Wide string view containing the number to parse
     * @return Parsed signed value, or LLONG_MIN if invalid
     */
    long long string_to_signed(std::wstring_view sv) noexcept
    {
        long long Res{ 0 };
        long long Sign{ 1 };

        // Empty string is invalid
        if (sv.empty()) { return LLONG_MIN; }

        // Check for negative sign
        if (sv[0] == '-') {
            Sign = -1;  // Set negative flag
            Res = string_to_unsigned(sv.substr(1));  // Parse rest without sign
        }
        else {
            // Parse as unsigned
            Res = string_to_unsigned(sv);
        }

        // Return signed result if valid
        if (Res != LLONG_MIN) {
            return Res * Sign;
        }
        return LLONG_MIN;
    }

    /**
     * @brief Validates a username against common requirements
     *
     * Checks if a username meets standard requirements:
     * - Length between MinLen and MaxLen
     * - Contains at least one digit
     * - Contains at least one lowercase letter
     * - Contains at least one uppercase letter
     * - Contains only alphanumeric characters
     *
     * @param Text Username to validate
     * @param MinLen Minimum allowed length
     * @param MaxLen Maximum allowed length
     * @return Bitmask of errors (0 if valid):
     *         bit 0: Length outside allowed range
     *         bit 1: Missing digit
     *         bit 2: Missing lowercase letter
     *         bit 3: Missing uppercase letter
     *         bit 4: Contains invalid characters
     */
    int username_error(std::wstring const& Text, long long MinLen, long long MaxLen) noexcept
    {
        int nL = 0;  // Count of lowercase letters
        int nU = 0;  // Count of uppercase letters
        int nD = 0;  // Count of digits
        int Errors = 0;  // Error bitmap

        // Check length constraints
        if (Text.size() < size_t(MinLen)) Errors |= 1;
        if (Text.size() > size_t(MaxLen)) Errors |= 1;

        // Check character types
        for (auto const& w : Text) {
            if (w >= '0' && w <= '9') ++nD;  // Digit
            else if (w >= 'a' && w <= 'z') ++nL;  // Lowercase
            else if (w >= 'A' && w <= 'Z') ++nU;  // Uppercase
            else Errors |= 16;  // Invalid character
        }

        // Check required character types
        if (!nD) Errors |= 2;  // Missing digit
        if (!nL) Errors |= 4;  // Missing lowercase
        if (!nU) Errors |= 8;  // Missing uppercase

        return Errors;
    }

    /**
     * @brief Formats a file size with appropriate units
     *
     * Converts a file size in bytes to a human-readable string with
     * appropriate units (B, KB/KiB, MB/MiB, GB/GiB). Can use either
     * binary (1024-based) or decimal (1000-based) units.
     *
     * @param SignedLength File size in bytes (signed for compatibility)
     * @param Binary Whether to use binary units (true) or decimal units (false)
     * @return Formatted file size string (e.g., "1.2 MB" or "1.2 MiB")
     */
    std::wstring FileLengthString(int64_t SignedLength, bool Binary) noexcept
    {
        // Convert to unsigned to handle negative values (though file sizes shouldn't be negative)
        uint64_t Length = SignedLength;

        if (Binary) {
            // Binary units (powers of 1024)
            if (Length < 1024) {
                return std::format(L"{:d} B", Length);
            }
            if (Length < 1048576) { // Less than 1 MiB
                return std::format(L"{:d} KiB", (Length >> 10));
            }
            if (Length < 1073741824) { // Less than 1 GiB
                // Format with one decimal place
                return std::format(L"{:d}.{:d} MiB",
                    (Length >> 20), // Whole MiB
                    (((Length - ((Length >> 20) << 20)) * 10) >> 20)); // Decimal part
            }
            // GiB and larger
            return std::format(L"{:d}.{:d} GiB",
                (Length >> 30), // Whole GiB
                (((Length - ((Length >> 30) << 30)) * 100) >> 30)); // Decimal part
        }
        else {
            // Decimal units (powers of 1000)
            if (Length < 1000) {
                return std::format(L"{:d} B", Length);
            }
            if (Length < 1000000) { // Less than 1 MB
                return std::format(L"{:d} KB", (Length / 1000));
            }
            if (Length < 1000000000) { // Less than 1 GB
                // Format with one decimal place
                return std::format(L"{:d}.{:d} MB",
                    (Length / 1000000), // Whole MB
                    ((Length % 1000000) / 100000)); // Decimal part
            }
            // GB and larger
            return std::format(L"{:d}.{:d} GB",
                (Length / 1000000000), // Whole GB
                ((Length % 1000000000) / 10000000)); // Decimal part
        }
    }

    /**
     * @brief Formats a monetary value as a dollar string
     *
     * Converts a numeric value to a properly formatted dollar string
     * with currency symbol, thousands separators, and decimal places
     * if needed.
     *
     * @param SignedValue Value to format (in cents if InCents=true)
     * @param InCents Whether the value is in cents (true) or dollars (false)
     * @return Formatted dollar string (e.g., "$1,234.56")
     */
    std::wstring DollarString(int64_t SignedValue, bool InCents) noexcept
    {
        // Buffer large enough for even the largest values
        static thread_local wchar_t Letters[32];

        int Index{ 32 };  // Start at the end
        int SepCount{ 0 };  // Counter for thousands separator

        // Convert negative values to positive for processing
        uint64_t Value = (SignedValue < 0) ? -SignedValue : SignedValue;

        if (InCents)
        {
            // Add decimal places for cents
            Letters[--Index] = '0' + Value % 10; Value /= 10;  // Last digit
            Letters[--Index] = '0' + Value % 10; Value /= 10;  // Second digit
            Letters[--Index] = '.';  // Decimal point
        }

        // Process the whole number part
        do
        {
            if (SepCount != 3)
            {
                // Add a digit
                Letters[--Index] = '0' + (Value % 10);
                Value /= 10;
                ++SepCount;
            }
            else if (Value)
            {
                // Add a thousands separator
                Letters[--Index] = ',';
                SepCount = 0;
            }
        } while (Index > 2 && Value);  // Ensure we don't overflow the buffer

        // Add the dollar sign
        Letters[--Index] = '$';

        // Add negative sign if needed
        if (SignedValue < 0) {
            Letters[--Index] = '-';
        }

        // Return the formatted string
        return std::wstring{ &Letters[Index], 32ull - Index };
    }

    /**
     * @brief Clears the entire screen
     *
     * Uses ANSI escape sequence to clear the entire screen.
     * Cross-platform implementation compatible with modern terminals.
     */
    void cls() { mz::Write(L"\x1b[2J\x1b[H"); }  // Clear screen and move to home position

    /**
     * @brief Clears the screen with specified mode
     *
     * Uses ANSI escape sequence to clear the screen with a specific mode:
     * - Mode 0: Clear from cursor to end of screen
     * - Mode 1: Clear from beginning of screen to cursor
     * - Mode 2: Clear entire screen
     *
     * @param Mode The clearing mode (0, 1, or 2)
     */
    void cls(int Mode) {
        switch (Mode) {
        case 0: mz::Write(L"\x1b[0J"); break;  // Clear from cursor to end
        case 1: mz::Write(L"\x1b[1J"); break;  // Clear from beginning to cursor
        default: mz::Write(L"\x1b[2J\x1b[H"); break;  // Clear entire screen and home
        };
    }

#if defined(MZ_PLATFORM_WINDOWS) || defined(MZ_PLATFORM_MACOS) || defined(MZ_PLATFORM_UNIX)
    /**
     * @brief Gets the terminal window dimensions
     *
     * Retrieves the current dimensions of the terminal window.
     * Cross-platform implementation for Windows, macOS, and Unix-like systems.
     *
     * @param width Reference to store the width (in characters)
     * @param height Reference to store the height (in characters)
     * @return true if successful, false otherwise
     */
    bool GetConsoleSize(int& width, int& height) noexcept {
#ifdef MZ_PLATFORM_WINDOWS
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi)) {
            width = csbi.srWindow.Right - csbi.srWindow.Left + 1;
            height = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
            return true;
        }
        return false;
#else
        struct winsize w;
        if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) != -1) {
            width = w.ws_col;
            height = w.ws_row;
            return true;
        }
        return false;
#endif
    }
#endif

    /**
     * @brief Creates a centered box around text
     *
     * Formats a string as a centered box with borders. Useful for
     * creating dialog boxes, headers, or other UI elements.
     *
     * @param text Text to put in the box
     * @param padding Number of spaces around the text
     * @return Formatted string with the text in a box
     */
    std::wstring BoxedText(const std::wstring& text, int padding) noexcept {
        int width, height;
        if (!GetConsoleSize(width, height)) {
            width = 80; // Default width if console size can't be determined
        }

        // Calculate box dimensions
        int textWidth = static_cast<int>(text.length());
        int boxWidth = min(width - 4, textWidth + padding * 2 + 2);
        int leftPadding = (boxWidth - textWidth) / 2;

        std::wstring result;

        // Top border
        result += L"┌";
        for (int i = 0; i < boxWidth - 2; i++) {
            result += L"─";
        }
        result += L"┐\n";

        // Padding above text
        for (int i = 0; i < padding; i++) {
            result += L"│";
            for (int j = 0; j < boxWidth - 2; j++) {
                result += L" ";
            }
            result += L"│\n";
        }

        // Text line
        result += L"│";
        for (int i = 0; i < leftPadding - 1; i++) {
            result += L" ";
        }
        result += text;
        for (int i = 0; i < boxWidth - 2 - textWidth - leftPadding + 1; i++) {
            result += L" ";
        }
        result += L"│\n";

        // Padding below text
        for (int i = 0; i < padding; i++) {
            result += L"│";
            for (int j = 0; j < boxWidth - 2; j++) {
                result += L" ";
            }
            result += L"│\n";
        }

        // Bottom border
        result += L"└";
        for (int i = 0; i < boxWidth - 2; i++) {
            result += L"─";
        }
        result += L"┘\n";

        return result;
    }

    /**
     * @brief Gets the terminal window width
     *
     * Retrieves the current width of the terminal window.
     * Falls back to a default value if the width can't be determined.
     *
     * @return Terminal width in characters
     */
    int GetConsoleWidth() noexcept {
        int width, height;
        if (GetConsoleSize(width, height)) {
            return width;
        }
        return 80;  // Default width
    }

    /**
     * @brief Gets the terminal window height
     *
     * Retrieves the current height of the terminal window.
     * Falls back to a default value if the height can't be determined.
     *
     * @return Terminal height in characters
     */
    int GetConsoleHeight() noexcept {
        int width, height;
        if (GetConsoleSize(width, height)) {
            return height;
        }
        return 24;  // Default height
    }

    /**
     * @brief Centers text horizontally in the terminal
     *
     * Creates a string with the given text centered within the
     * current width of the terminal window.
     *
     * @param text Text to center
     * @return Centered text string
     */
    std::wstring CenterText(const std::wstring& text) noexcept {
        int width = GetConsoleWidth();
        int padding = (width - static_cast<int>(text.length())) / 2;
        if (padding < 0) padding = 0;

        std::wstring result;
        for (int i = 0; i < padding; i++) {
            result += L" ";
        }
        result += text;
        return result;
    }

    /**
     * @brief Creates a horizontal rule (divider line)
     *
     * Creates a horizontal divider line across the width of the terminal.
     *
     * @param lineChar Character to use for the line (default: '─')
     * @return String containing the horizontal rule
     */
    std::wstring HorizontalRule(wchar_t lineChar) noexcept {
        int width = GetConsoleWidth();
        std::wstring result;
        for (int i = 0; i < width; i++) {
            result += lineChar;
        }
        return result;
    }

} // namespace mz
