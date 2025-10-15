#pragma once

#include <string>
#include <memory>
#include "ConsoleCMD.h"
#include "coord.h"
#include "cursor.h"

namespace mz {

    /**
     * @class TerminalManager
     * @brief Cross-platform terminal configuration and management
     *
     * This class provides functionality to configure and manage terminal settings
     * across different platforms (Windows, macOS, Linux) with a unified interface.
     * It handles terminal size, font, styling, and other platform-specific settings.
     */
    class TerminalManager {
    public:
        // Forward declaration of platform-specific implementation
        class PlatformImpl;

        /**
         * @brief Constructor initializes the platform-specific implementation
         */
        TerminalManager() noexcept;

        /**
         * @brief Destructor ensures proper cleanup of platform resources
         */
        ~TerminalManager() noexcept;

        /**
         * @brief Set the terminal font (when supported by platform)
         *
         * @param fontFamily Font family name (platform-specific)
         * @param fontSize Font size in points
         * @return 0 on success, error code on failure
         */
        int set_font(std::wstring_view fontFamily = L"", int fontSize = 0) noexcept;

        /**
         * @brief Set the terminal window size
         *
         * @param numRows Number of rows (lines)
         * @param numCols Number of columns (characters)
         * @return 0 on success, error code on failure
         */
        int set_console_size(int numRows, int numCols) noexcept;

        /**
         * @brief Get the screen size in pixels
         *
         * @return 0 on success, error code on failure
         */
        int get_screen_size() noexcept;

        /**
         * @brief Get the console size in character cells
         *
         * @return 0 on success, error code on failure
         */
        int get_console_size() noexcept;

        /**
         * @brief Initialize standard input/output handles
         *
         * @return 0 on success, error code on failure
         */
        int get_standard_handles() noexcept;

        /**
         * @brief Configure console window style
         *
         * @return 0 on success, error code on failure
         */
        int set_console_style() noexcept;

        /**
         * @brief Configure console input/output modes
         *
         * @return 0 on success, error code on failure
         */
        int set_console_mode() noexcept;

        /**
         * @brief Configure character encoding support
         *
         * @return 0 on success, error code on failure
         */
        int set_code_page() noexcept;

        /**
         * @brief Setup the terminal with default settings
         *
         * @param numRows Number of rows
         * @param numCols Number of columns
         * @return 0 on success, error code on failure
         */
        int setup(int numRows, int numCols) noexcept;

        /**
         * @brief Clear the screen
         *
         * @param mode Clear mode (0: from cursor to end, 1: from start to cursor, 2: entire screen)
         */
        void cls(int mode = 2) noexcept;

        /**
         * @brief Clear the entire terminal including scrollback buffer
         */
        void clear_all() noexcept;

        /**
         * @brief Get current terminal size
         *
         * @return coord object containing current rows and columns
         */
        coord get_terminal_size() const noexcept;

        /**
         * @brief Get the usable window area as a box
         *
         * @return coord_box representing the usable terminal area
         */
        const coord_box& get_window() const noexcept;

        /**
         * @brief Check if the terminal supports ANSI escape sequences
         *
         * @return true if ANSI escape sequences are supported
         */
        bool supports_ansi() const noexcept;

        /**
         * @brief Check if the terminal supports color
         *
         * @return true if color is supported
         */
        bool supports_color() const noexcept;

        /**
         * @brief Check if the terminal supports cursor positioning
         *
         * @return true if cursor positioning is supported
         */
        bool supports_cursor_positioning() const noexcept;

    private:
        // Platform-specific implementation
        std::unique_ptr<PlatformImpl> pImpl;

        // Terminal size information
        coord maxTerminalSize;
        coord_box oldWindow;
        coord_box window;

        // Default cursor settings
        cursor defaults;

        // Screen size in pixels (when available)
        int screenPixelWidth{ 0 };
        int screenPixelHeight{ 0 };

        // No copying allowed
        TerminalManager(const TerminalManager&) = delete;
        TerminalManager& operator=(const TerminalManager&) = delete;
    };

} // namespace mz
