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

#include "TerminalManager.h"
#include <iostream>
#include <string>

// Platform detection
#if defined(_WIN32) || defined(_WIN64) || defined(_MSC_VER)
#define MZ_PLATFORM_WINDOWS
#include <windows.h>
#elif defined(__APPLE__) || defined(__MACH__)
#define MZ_PLATFORM_MACOS
#include <sys/ioctl.h>
#include <unistd.h>
#include <termios.h>
#include <signal.h>
#elif defined(__linux__) || defined(__unix__) || defined(__unix)
#define MZ_PLATFORM_UNIX
#include <sys/ioctl.h>
#include <unistd.h>
#include <termios.h>
#include <signal.h>
#else
#define MZ_PLATFORM_UNKNOWN
// Minimal implementation for unknown platforms
#endif

namespace mz {

    //=========================================================================
    // PLATFORM-SPECIFIC IMPLEMENTATION CLASSES
    //=========================================================================

#ifdef MZ_PLATFORM_WINDOWS
    // Windows implementation
    class TerminalManager::PlatformImpl {
    public:
        PlatformImpl() noexcept :
            oldOutputCodePage(0),
            oldConsoleCodePage(0),
            newOutputCodePage(CP_UTF8),
            newConsoleCodePage(CP_UTF8),
            winConsoleWindow(NULL),
            stdInHandle(INVALID_HANDLE_VALUE),
            stdOutHandle(INVALID_HANDLE_VALUE),
            stdErrHandle(INVALID_HANDLE_VALUE),
            oldWindowStyle(0),
            newWindowStyle(~WS_MAXIMIZEBOX & ~WS_SIZEBOX & ~WS_VSCROLL),
            oldConsoleMode(0),
            newConsoleMode(0)
        {
            // Initialize font info structures
            oldFontInfoEx.cbSize = sizeof(oldFontInfoEx);
            newFontInfoEx.cbSize = sizeof(newFontInfoEx);
        }

        ~PlatformImpl() noexcept {
            // Restore original settings
            if (stdOutHandle != INVALID_HANDLE_VALUE) {
                // Restore console mode
                if (oldConsoleMode != 0) {
                    ::SetConsoleMode(stdOutHandle, oldConsoleMode);
                }

                // Restore code page
                if (oldOutputCodePage != 0) {
                    SetConsoleOutputCP(oldOutputCodePage);
                }

                if (oldConsoleCodePage != 0) {
                    SetConsoleCP(oldConsoleCodePage);
                }

                // Restore font if we changed it
                if (oldFontInfoEx.cbSize != 0) {
                    SetCurrentConsoleFontEx(stdOutHandle, FALSE, &oldFontInfoEx);
                }
            }

            // Restore window style
            if (winConsoleWindow != NULL && oldWindowStyle != 0) {
                SetWindowLong(winConsoleWindow, GWL_STYLE, oldWindowStyle);
            }
        }

        // Windows-specific data members
        UINT oldOutputCodePage;
        UINT oldConsoleCodePage;
        UINT newOutputCodePage;
        UINT newConsoleCodePage;
        HWND winConsoleWindow;
        HANDLE stdInHandle;
        HANDLE stdOutHandle;
        HANDLE stdErrHandle;
        LONG oldWindowStyle;
        LONG newWindowStyle;
        DWORD oldConsoleMode;
        DWORD newConsoleMode;
        CONSOLE_FONT_INFOEX oldFontInfoEx;
        CONSOLE_FONT_INFOEX newFontInfoEx;

        // Windows-specific implementation methods
        int GetStandardHandles(TerminalManager* mgr) noexcept {
            int error = 0;

            stdInHandle = GetStdHandle(STD_INPUT_HANDLE);
            if (stdInHandle == INVALID_HANDLE_VALUE) {
                std::wcerr << L"ERROR: Failed to retrieve handle to standard input." << std::endl;
                error += 1;
            }

            stdOutHandle = GetStdHandle(STD_OUTPUT_HANDLE);
            if (stdOutHandle == INVALID_HANDLE_VALUE) {
                std::wcerr << L"ERROR: Failed to retrieve handle to standard output." << std::endl;
                error += 2;
            }

            stdErrHandle = GetStdHandle(STD_ERROR_HANDLE);
            if (stdErrHandle == INVALID_HANDLE_VALUE) {
                std::wcerr << L"ERROR: Failed to retrieve handle to standard error." << std::endl;
                error += 4;
            }

            return error;
        }

        int SetFont(std::wstring_view fontFamily, int fontSize) noexcept {
            if (stdOutHandle == INVALID_HANDLE_VALUE) {
                return 1;
            }

            // Clamp font size to reasonable limits
            fontSize = fontSize < 6 ? 6 : fontSize > 32 ? 32 : fontSize;

            // Save current font info
            if (!GetCurrentConsoleFontEx(stdOutHandle, TRUE, &oldFontInfoEx)) {
                std::wcerr << L"ERROR: Failed to retrieve current font info." << std::endl;
            }

            // Set up new font info
            newFontInfoEx.cbSize = sizeof(newFontInfoEx);
            newFontInfoEx.nFont = oldFontInfoEx.nFont;
            newFontInfoEx.dwFontSize.X = 0; // Let system choose the best width
            newFontInfoEx.dwFontSize.Y = fontSize;
            newFontInfoEx.FontFamily = oldFontInfoEx.FontFamily;
            newFontInfoEx.FontWeight = oldFontInfoEx.FontWeight;

            // Copy font name if provided, otherwise keep current
            if (!fontFamily.empty()) {
                wcsncpy_s(newFontInfoEx.FaceName, fontFamily.data(), LF_FACESIZE - 1);
            }
            else {
                wcsncpy_s(newFontInfoEx.FaceName, oldFontInfoEx.FaceName, LF_FACESIZE - 1);
            }

            if (!SetCurrentConsoleFontEx(stdOutHandle, FALSE, &newFontInfoEx)) {
                std::wcerr << L"ERROR: Failed to set console font." << std::endl;
                return 2;
            }

            return 0;
        }

        int GetScreenSize(TerminalManager* mgr) noexcept {
            int error = 0;

            mgr->screenPixelWidth = GetSystemMetrics(SM_CXSCREEN);
            if (mgr->screenPixelWidth == 0) {
                std::wcerr << L"ERROR: Failed to retrieve screen width." << std::endl;
                error += 1;
            }

            mgr->screenPixelHeight = GetSystemMetrics(SM_CYSCREEN);
            if (mgr->screenPixelHeight == 0) {
                std::wcerr << L"ERROR: Failed to retrieve screen height." << std::endl;
                error += 2;
            }

            return error;
        }

        int GetConsoleSize(TerminalManager* mgr) noexcept {
            if (stdOutHandle == INVALID_HANDLE_VALUE) {
                return 1;
            }

            CONSOLE_SCREEN_BUFFER_INFO csbi;
            if (!GetConsoleScreenBufferInfo(stdOutHandle, &csbi)) {
                std::wcerr << L"ERROR: Failed to retrieve console size." << std::endl;
                return 1;
            }

            mgr->oldWindow.Top.Row = csbi.srWindow.Top;
            mgr->oldWindow.Top.Col = csbi.srWindow.Left;
            mgr->oldWindow.Bottom.Row = csbi.srWindow.Bottom;
            mgr->oldWindow.Bottom.Col = csbi.srWindow.Right;

            COORD maxSize = GetLargestConsoleWindowSize(stdOutHandle);
            if (maxSize.X == 0 || maxSize.Y == 0) {
                std::wcerr << L"ERROR: Failed to retrieve largest console size." << std::endl;
                return 2;
            }

            mgr->maxTerminalSize.Col = maxSize.X;
            mgr->maxTerminalSize.Row = maxSize.Y;

            return 0;
        }

        int SetConsoleSize(TerminalManager* mgr, int numRows, int numCols) noexcept {
            if (stdOutHandle == INVALID_HANDLE_VALUE) {
                return 1;
            }

            // Update our information about the console
            GetScreenSize(mgr);
            GetConsoleSize(mgr);

            // Limit to maximum allowable size
            if (numRows > mgr->maxTerminalSize.Row) {
                numRows = mgr->maxTerminalSize.Row;
            }
            if (numCols > mgr->maxTerminalSize.Col) {
                numCols = mgr->maxTerminalSize.Col;
            }

            // Calculate minimum window size (to avoid errors)
            coord minWindowSize = mgr->oldWindow.get_size();
            if (minWindowSize.Row > numRows) {
                minWindowSize.Row = numRows;
            }
            if (minWindowSize.Col > numCols) {
                minWindowSize.Col = numCols;
            }

            // First, minimize the window
            SMALL_RECT windowRect{ 0, 0,
                static_cast<SHORT>(minWindowSize.Col - 1),
                static_cast<SHORT>(minWindowSize.Row - 1)
            };

            if (!SetConsoleWindowInfo(stdOutHandle, TRUE, &windowRect)) {
                std::wcerr << L"ERROR: Failed to minimize window." << std::endl;
                return 1;
            }

            // Set the buffer size
            COORD bufferSize{
                static_cast<SHORT>(numCols),
                static_cast<SHORT>(numRows)
            };

            if (!SetConsoleScreenBufferSize(stdOutHandle, bufferSize)) {
                std::wcerr << L"ERROR: Failed to set output buffer size." << std::endl;
                return 2;
            }

            // Now set the window size
            windowRect.Bottom = static_cast<SHORT>(numRows - 1);
            windowRect.Right = static_cast<SHORT>(numCols - 1);

            if (!SetConsoleWindowInfo(stdOutHandle, TRUE, &windowRect)) {
                std::wcerr << L"ERROR: Failed to set window size." << std::endl;
                return 3;
            }

            // Update our window tracking
            mgr->window.Top.Row = windowRect.Top;
            mgr->window.Top.Col = windowRect.Left;
            mgr->window.Bottom.Row = windowRect.Bottom;
            mgr->window.Bottom.Col = windowRect.Right;
            mgr->window = mgr->window.shift(1, 1);

            return 0;
        }

        int SetConsoleStyle() noexcept {
            int error = 0;

            winConsoleWindow = GetConsoleWindow();
            if (winConsoleWindow == NULL) {
                std::wcerr << L"ERROR: Failed to retrieve handle to console window." << std::endl;
                error += 1;
            }
            else {
                // Hide scrollbars
                if (ShowScrollBar(winConsoleWindow, SB_BOTH, FALSE) == 0) {
                    std::wcerr << L"ERROR: Failed to hide scroll bar." << std::endl;
                    error += 2;
                }

                // Modify window style
                oldWindowStyle = GetWindowLong(winConsoleWindow, GWL_STYLE);
                if (oldWindowStyle == 0) {
                    std::wcerr << L"ERROR: Failed to retrieve console window style." << std::endl;
                    error += 4;
                }

                // Apply our style by removing unwanted style bits
                newWindowStyle &= oldWindowStyle;
                if (SetWindowLong(winConsoleWindow, GWL_STYLE, newWindowStyle) == 0) {
                    std::wcerr << L"ERROR: Failed to set console window style." << std::endl;
                    error += 8;
                }
            }

            return error;
        }

        int SetConsoleMode() noexcept {
            int error = 0;

            if (stdOutHandle == INVALID_HANDLE_VALUE) {
                return 1;
            }

            // Get current console mode
            if (!GetConsoleMode(stdOutHandle, &oldConsoleMode)) {
                std::wcerr << L"ERROR: Failed to retrieve console mode." << std::endl;
                error += 1;
            }
            else {
                // Enable ANSI escape sequence processing
                newConsoleMode = oldConsoleMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING;
                if (!::SetConsoleMode(stdOutHandle, newConsoleMode)) {
                    std::wcerr << L"ERROR: Failed to assign console mode." << std::endl;
                    error += 2;
                }
            }

            return error;
        }

        int SetCodePage() noexcept {
            int error = 0;

            // Save current code pages
            oldConsoleCodePage = GetConsoleCP();
            if (oldConsoleCodePage == 0) {
                std::wcerr << L"ERROR: Failed to retrieve console code page." << std::endl;
                error += 1;
            }

            oldOutputCodePage = GetConsoleOutputCP();
            if (oldOutputCodePage == 0) {
                std::wcerr << L"ERROR: Failed to retrieve console output code page." << std::endl;
                error += 2;
            }

            // Set to UTF-8
            if (!SetConsoleCP(newConsoleCodePage)) {
                std::wcerr << L"ERROR: Failed to set console code page." << std::endl;
                error += 4;
            }

            if (!SetConsoleOutputCP(newOutputCodePage)) {
                std::wcerr << L"ERROR: Failed to set console output code page." << std::endl;
                error += 8;
            }

            return error;
        }

        void ClearScreen(int mode) noexcept {
            if (stdOutHandle == INVALID_HANDLE_VALUE) {
                return;
            }

            // Use ANSI escape sequences if supported
            if (newConsoleMode & ENABLE_VIRTUAL_TERMINAL_PROCESSING) {
                const wchar_t* clearCommand;
                switch (mode) {
                case 0: clearCommand = L"\x1b[0J"; break; // Clear from cursor to end
                case 1: clearCommand = L"\x1b[1J"; break; // Clear from start to cursor
                default: clearCommand = L"\x1b[2J\x1b[H"; break; // Clear screen and home
                }

                DWORD written;
                WriteConsoleW(stdOutHandle, clearCommand, wcslen(clearCommand), &written, NULL);
            }
            else {
                // Fallback for terminals without ANSI support
                CONSOLE_SCREEN_BUFFER_INFO csbi;
                GetConsoleScreenBufferInfo(stdOutHandle, &csbi);

                COORD homeCoords = { 0, 0 };
                DWORD cellCount = csbi.dwSize.X * csbi.dwSize.Y;
                DWORD charsWritten;

                // Fill the console with spaces
                FillConsoleOutputCharacterW(stdOutHandle, L' ', cellCount, homeCoords, &charsWritten);
                FillConsoleOutputAttribute(stdOutHandle, csbi.wAttributes, cellCount, homeCoords, &charsWritten);

                // Reset cursor position
                SetConsoleCursorPosition(stdOutHandle, homeCoords);
            }
        }

        void ClearAll() noexcept {
            // Clear screen
            ClearScreen(2);

            // Fill with spaces for a more thorough clear
            if (stdOutHandle != INVALID_HANDLE_VALUE) {
                CONSOLE_SCREEN_BUFFER_INFO csbi;
                if (GetConsoleScreenBufferInfo(stdOutHandle, &csbi)) {
                    COORD homeCoords = { 0, 0 };
                    DWORD cellCount = csbi.dwSize.X * csbi.dwSize.Y;
                    DWORD charsWritten;

                    FillConsoleOutputCharacterW(stdOutHandle, L' ', cellCount, homeCoords, &charsWritten);
                    FillConsoleOutputAttribute(stdOutHandle, csbi.wAttributes, cellCount, homeCoords, &charsWritten);

                    SetConsoleCursorPosition(stdOutHandle, homeCoords);
                }
            }
        }

        coord GetTerminalSize() const noexcept {
            coord size = { 80, 24 }; // Default fallback

            if (stdOutHandle != INVALID_HANDLE_VALUE) {
                CONSOLE_SCREEN_BUFFER_INFO csbi;
                if (GetConsoleScreenBufferInfo(stdOutHandle, &csbi)) {
                    size.Row = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
                    size.Col = csbi.srWindow.Right - csbi.srWindow.Left + 1;
                }
            }

            return size;
        }

        bool SupportsAnsi() const noexcept {
            return (newConsoleMode & ENABLE_VIRTUAL_TERMINAL_PROCESSING) != 0;
        }

        bool SupportsColor() const noexcept {
            return true; // Windows console supports color
        }

        bool SupportsCursorPositioning() const noexcept {
            return true; // Windows console supports cursor positioning
        }
    };

#elif defined(MZ_PLATFORM_MACOS) || defined(MZ_PLATFORM_UNIX)
    // Unix/macOS implementation
    class TerminalManager::PlatformImpl {
    public:
        PlatformImpl() noexcept :
            originalTermios{},
            rawTermios{},
            isRawMode(false),
            supportsAnsi(true), // Most Unix terminals support ANSI
            supportsColor(true) // Most Unix terminals support color
        {
            // Store original terminal settings
            if (isatty(STDOUT_FILENO)) {
                tcgetattr(STDIN_FILENO, &originalTermios);
                rawTermios = originalTermios;
            }
            else {
                // Not a terminal - limited functionality
                supportsAnsi = false;
                supportsColor = false;
            }
        }

        ~PlatformImpl() noexcept {
            // Restore terminal settings
            if (isRawMode) {
                tcsetattr(STDIN_FILENO, TCSANOW, &originalTermios);
            }
        }

        // UNIX-specific data members
        struct termios originalTermios;
        struct termios rawTermios;
        bool isRawMode;
        bool supportsAnsi;
        bool supportsColor;

        // UNIX-specific implementation methods
        int GetStandardHandles(TerminalManager*) noexcept {
            // No special handling needed for Unix
            return 0;
        }

        int SetFont(std::wstring_view, int) noexcept {
            // Changing fonts not directly supported in most Unix terminals
            return 0;
        }

        int GetScreenSize(TerminalManager* mgr) noexcept {
            // Not directly available for terminal apps on Unix
            mgr->screenPixelWidth = 0;
            mgr->screenPixelHeight = 0;
            return 0;
        }

        int GetConsoleSize(TerminalManager* mgr) noexcept {
            struct winsize ws;

            if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1) {
                std::cerr << "ERROR: Failed to retrieve terminal size." << std::endl;
                return 1;
            }

            mgr->oldWindow.Top.Row = 0;
            mgr->oldWindow.Top.Col = 0;
            mgr->oldWindow.Bottom.Row = ws.ws_row - 1;
            mgr->oldWindow.Bottom.Col = ws.ws_col - 1;

            mgr->maxTerminalSize.Row = ws.ws_row;
            mgr->maxTerminalSize.Col = ws.ws_col;

            return 0;
        }

        int SetConsoleSize(TerminalManager* mgr, int numRows, int numCols) noexcept {
            // Get current size
            GetConsoleSize(mgr);

            // On Unix, we can't reliably set terminal size from within the application
            // Instead, we'll use what we have and print a message if it's too small
            if (mgr->maxTerminalSize.Row < numRows || mgr->maxTerminalSize.Col < numCols) {
                std::cerr << "Terminal size is smaller than requested." << std::endl;
                std::cerr << "Current: " << mgr->maxTerminalSize.Row << " rows x "
                    << mgr->maxTerminalSize.Col << " columns" << std::endl;
                std::cerr << "Needed:  " << numRows << " rows x "
                    << numCols << " columns" << std::endl;
            }

            // Set up window tracking
            mgr->window.Top.Row = 0;
            mgr->window.Top.Col = 0;
            mgr->window.Bottom.Row = mgr->maxTerminalSize.Row - 1;
            mgr->window.Bottom.Col = mgr->maxTerminalSize.Col - 1;

            return 0;
        }

        int SetConsoleStyle() noexcept {
            // No direct window style control in Unix terminals
            return 0;
        }

        int SetConsoleMode() noexcept {
            if (!isatty(STDOUT_FILENO)) {
                return 1;
            }

            // Set up raw mode for better control
            rawTermios.c_lflag &= ~(ICANON | ECHO | ISIG);
            rawTermios.c_iflag &= ~(IXON | ICRNL);
            rawTermios.c_cc[VMIN] = 1;
            rawTermios.c_cc[VTIME] = 0;

            if (tcsetattr(STDIN_FILENO, TCSANOW, &rawTermios) == -1) {
                std::cerr << "ERROR: Failed to set terminal attributes." << std::endl;
                return 2;
            }

            isRawMode = true;
            return 0;
        }

        int SetCodePage() noexcept {
            // Unix terminals typically use environment variables for encoding
            // Nothing to do here as we assume UTF-8 is already set
            return 0;
        }

        void ClearScreen(int mode) noexcept {
            const char* clearCommand;
            switch (mode) {
            case 0: clearCommand = "\x1b[0J"; break; // Clear from cursor to end
            case 1: clearCommand = "\x1b[1J"; break; // Clear from start to cursor
            default: clearCommand = "\x1b[2J\x1b[H"; break; // Clear screen and home
            }

            write(STDOUT_FILENO, clearCommand, strlen(clearCommand));
        }

        void ClearAll() noexcept {
            // Clear screen and scrollback buffer
            const char* clearCommand = "\x1b[2J\x1b[3J\x1b[H";
            write(STDOUT_FILENO, clearCommand, strlen(clearCommand));
        }

        coord GetTerminalSize() const noexcept {
            coord size = { 80, 24 }; // Default fallback

            struct winsize ws;
            if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) != -1) {
                size.Row = ws.ws_row;
                size.Col = ws.ws_col;
            }

            return size;
        }

        bool SupportsAnsi() const noexcept {
            return supportsAnsi;
        }

        bool SupportsColor() const noexcept {
            return supportsColor;
        }

        bool SupportsCursorPositioning() const noexcept {
            return supportsAnsi;
        }
    };

#else
    // Fallback implementation for unknown platforms
    class TerminalManager::PlatformImpl {
    public:
        PlatformImpl() noexcept {}
        ~PlatformImpl() noexcept {}

        int GetStandardHandles(TerminalManager*) noexcept { return 0; }
        int SetFont(std::wstring_view, int) noexcept { return 0; }
        int GetScreenSize(TerminalManager*) noexcept { return 0; }
        int GetConsoleSize(TerminalManager* mgr) noexcept {
            // Use standard defaults
            mgr->maxTerminalSize.Row = 24;
            mgr->maxTerminalSize.Col = 80;
            mgr->oldWindow.Top = { 0, 0 };
            mgr->oldWindow.Bottom = { 23, 79 };
            return 0;
        }

        int SetConsoleSize(TerminalManager* mgr, int, int) noexcept {
            mgr->window = mgr->oldWindow;
            return 0;
        }

        int SetConsoleStyle() noexcept { return 0; }
        int SetConsoleMode() noexcept { return 0; }
        int SetCodePage() noexcept { return 0; }

        void ClearScreen(int) noexcept {
            // Try ANSI clear screen, may or may not work
            std::cout << "\x1b[2J\x1b[H" << std::flush;
        }

        void ClearAll() noexcept {
            std::cout << "\x1b[2J\x1b[H" << std::flush;
        }

        coord GetTerminalSize() const noexcept {
            return coord{ 24, 80 }; // Default size
        }

        bool SupportsAnsi() const noexcept { return false; }
        bool SupportsColor() const noexcept { return false; }
        bool SupportsCursorPositioning() const noexcept { return false; }
    };
#endif

    //=========================================================================
    // IMPLEMENTATION OF TERMINALMANAGER CLASS METHODS
    //=========================================================================

    TerminalManager::TerminalManager() noexcept : pImpl(std::make_unique<PlatformImpl>()) {}

    TerminalManager::~TerminalManager() noexcept = default;

    int TerminalManager::set_font(std::wstring_view fontFamily, int fontSize) noexcept {
        return pImpl->SetFont(fontFamily, fontSize);
    }

    int TerminalManager::set_console_size(int numRows, int numCols) noexcept {
        return pImpl->SetConsoleSize(this, numRows, numCols);
    }

    int TerminalManager::get_screen_size() noexcept {
        return pImpl->GetScreenSize(this);
    }

    int TerminalManager::get_console_size() noexcept {
        return pImpl->GetConsoleSize(this);
    }

    int TerminalManager::get_standard_handles() noexcept {
        return pImpl->GetStandardHandles(this);
    }

    int TerminalManager::set_console_style() noexcept {
        return pImpl->SetConsoleStyle();
    }

    int TerminalManager::set_console_mode() noexcept {
        return pImpl->SetConsoleMode();
    }

    int TerminalManager::set_code_page() noexcept {
        return pImpl->SetCodePage();
    }

    int TerminalManager::setup(int numRows, int numCols) noexcept {
        int error = 0;

        // Get standard handles first
        if (int err = get_standard_handles(); err != 0) {
            return err;
        }

        // Try to set font (may not work on all platforms)
#ifdef MZ_PLATFORM_WINDOWS
        if (int err = set_font(L"Cascadia Code", 18); err != 0) {
            error += err;
        }
#endif

        // Configure console size
        if (int err = set_console_size(numRows, numCols); err != 0) {
            error += err;
        }

        // Set up UTF-8 support
        if (int err = set_code_page(); err != 0) {
            error += err;
        }

        // Configure console mode for ANSI escape sequences
        if (int err = set_console_mode(); err != 0) {
            error += err;
        }

        // Set window style if supported
        if (int err = set_console_style(); err != 0) {
            error += err;
        }

        // Clear the screen
        clear_all();

        return error;
    }

    void TerminalManager::cls(int mode) noexcept {
        pImpl->ClearScreen(mode);
    }

    void TerminalManager::clear_all() noexcept {
        pImpl->ClearAll();
    }

    coord TerminalManager::get_terminal_size() const noexcept {
        return pImpl->GetTerminalSize();
    }

    const coord_box& TerminalManager::get_window() const noexcept {
        return window;
    }

    bool TerminalManager::supports_ansi() const noexcept {
        return pImpl->SupportsAnsi();
    }

    bool TerminalManager::supports_color() const noexcept {
        return pImpl->SupportsColor();
    }

    bool TerminalManager::supports_cursor_positioning() const noexcept {
        return pImpl->SupportsCursorPositioning();
    }

} // namespace mz
