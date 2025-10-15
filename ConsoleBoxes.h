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

#ifndef MZ_CONSOLE_BOXES_H
#define MZ_CONSOLE_BOXES_H
#pragma once

/**
 * @file ConsoleBoxes.h
 * @brief Terminal-based UI box components
 *
 * This file provides base classes for creating terminal UI components
 * such as message boxes, input boxes, and other screen elements.
 * It includes classes for basic rectangular UI elements with support
 * for colors, positioning, and text display.
 *
 * @author Meysam Zare
 */

#include "ConsoleCMD.h"
#include "coord.h"
#include "cursor.h"
#include <string>
#include <string_view>
#include <chrono>
#include <thread>
#include <algorithm>

namespace mz {

    /**
     * @class BasicBox
     * @brief Base class for all terminal UI box components
     *
     * Provides core functionality for terminal UI elements including
     * positioning, coloring, and buffer management. All specialized
     * box components inherit from this class.
     */
    class BasicBox {
    protected:
        /**
         * @brief Terminal command buffer
         *
         * This buffer stores all ANSI terminal commands and content
         * that will be written to the terminal when print() is called.
         * Pre-allocated to reduce memory reallocations.
         */
        std::wstring bf;

    public:
        /**
         * @brief Box position and dimensions
         *
         * Defines the rectangular area that this box occupies on the screen.
         */
        coord_box Area;

        /**
         * @brief Key control flags
         *
         * Specifies which control keys this box responds to.
         */
        control_keys Controls{ 0 };

        /**
         * @brief Box colors (foreground and background)
         *
         * Defines the colors used for this box.
         */
        color Color;

        /**
         * @brief Default constructor
         *
         * Initializes an empty box with default settings.
         */
        BasicBox() noexcept {
            // Pre-allocate buffer to reduce reallocations
            bf.reserve(256);
            bf.clear();
        }

        BasicBox(color Color) noexcept : Color{ Color } {
            // Pre-allocate buffer to reduce reallocations
            bf.reserve(256);
            bf.clear();
        }

        /**
         * @brief Virtual destructor
         *
         * Ensures proper cleanup for derived classes.
         */
        virtual ~BasicBox() noexcept = default;

        /**
         * @brief Output the box contents to the terminal
         *
         * Writes the entire buffer to the terminal at once,
         * minimizing output operations for better performance.
         */
        void print() const noexcept {
            mz::Write(bf);
        }

        /**
         * @brief Get a view of the buffer contents
         *
         * @return Non-modifiable view of the internal buffer
         */
        std::wstring_view view() const noexcept {
            return std::wstring_view{ bf };
        }

        /**
         * @brief Set box colors
         *
         * @param Colors New color pair for the box
         */
        void set(color Colors) noexcept {
            Color = Colors;
        }

        /**
         * @brief Set background color
         *
         * @param RGB New background color
         */
        void set_back(rgb RGB) noexcept {
            Color.B = RGB;
        }

        /**
         * @brief Set foreground color
         *
         * @param RGB New foreground color
         */
        void set_front(rgb RGB) noexcept {
            Color.F = RGB;
        }

        /**
         * @brief Get current box colors
         *
         * @return Color pair used by this box
         */
        color colors() const noexcept {
            return Color;
        }

        /**
         * @brief Reserve buffer space
         *
         * Pre-allocates buffer capacity to avoid reallocations.
         *
         * @param capacity Minimum capacity to reserve
         */
        void reserve_buffer(size_t capacity) noexcept {
			size_t Cap = bf.capacity();
			Cap = Cap < capacity ? capacity : Cap;
            bf.reserve(Cap);
        }

        /**
         * @brief Clear the buffer and reset state
         *
         * Empties the buffer while preserving its capacity.
         */
        virtual void clear() noexcept {
            const size_t currentCapacity = bf.capacity();
            bf.clear();
            bf.reserve(currentCapacity);
        }
    };

    /**
     * @class MultilineMessageBox
     * @brief Text display box with support for multiple lines
     *
     * This class provides a way to display multiple lines of text within
     * a defined rectangular area. It supports different colors for each
     * line and visual effects like blinking.
     */
    class MultilineMessageBox : public BasicBox {
    public:
        /**
         * @brief Current line index for next insertion
         *
         * Tracks which line will be written to next.
         */
        int NextLine{ 0 };

        /**
         * @brief Buffer position for next insertion
         *
         * Tracks the position in the buffer where new content starts.
         */
        int NextSize{ 0 };

        /**
         * @brief Clear the box and reset state
         *
         * Clears the display area and resets internal tracking.
         */
        void clear() noexcept override {
            // Start with a fresh buffer but maintain capacity
            const size_t currentCapacity = bf.capacity();
            bf.clear();
            bf.reserve(currentCapacity);

            // Apply colors and position cursor at the top of the box
            Color.apply(bf);
            Area.Top.apply(bf);

            // Hide cursor and disable underline for cleaner appearance
            SetHide(bf);
            ClrUnderline(bf);

            // Reset tracking variables
            NextLine = 0;
            NextSize = static_cast<int>(bf.size());

            // Clear the entire area with spaces
            Area.clear(bf);
        }

        /**
         * @brief Initialize the message box
         *
         * Prepares the message box for use. Currently an alias for clear().
         */
        void create() noexcept {
            clear();
        }

        /**
         * @brief Insert a line of text
         *
         * Adds a new line of text to the box using the current foreground color.
         *
         * @param Msg Text to display
         * @return The line number after insertion (or the current line if full)
         */
        int insert_line(std::wstring_view Msg) noexcept {
            return insert_line(Msg, Color.F);
        }

        /**
         * @brief Insert a line of text with specific color
         *
         * Adds a new line of text to the box with the specified text color.
         *
         * @param Msg Text to display
         * @param FrontColor Text color to use
         * @return The line number after insertion (or the current line if full)
         */
        int insert_line(std::wstring_view Msg, rgb FrontColor) noexcept {
            // Check if we've reached the bottom of the box
            if (NextLine >= Area.num_rows())
                return NextLine;

            // Reset the buffer to the position after the box setup
            bf.resize(NextSize);

            // Position cursor at the current line
            Area.Top.offset(NextLine, 0).apply(bf);

            // Set text color
            FrontColor.setFront(bf);

            // Handle text that fits within the box width
            const int boxWidth = Area.num_cols();
            if (Msg.size() < static_cast<size_t>(boxWidth)) {
                // Append the message text
                bf.append(Msg.data(), Msg.size());

                // Reset to the default text color if needed
                if (FrontColor != Color.F) {
                    Color.F.setFront(bf);
                }

                // Fill the remaining space with spaces
                bf.append(boxWidth - Msg.size(), ' ');
            }
            // Handle text that's too long for the box
            else {
                // Truncate to fit the box width
                bf.append(Msg.data(), boxWidth);

                // Reset to the default text color if needed
                if (FrontColor != Color.F) {
                    Color.F.setFront(bf);
                }
            }

            // Update the buffer position for the next insertion
            NextSize = static_cast<int>(bf.size());

            // Move to the next line
            int Row{ ++NextLine };

            // Clear any remaining lines below
            while (Row < Area.num_rows()) {
                Area.Top.offset(Row++, 0).apply(bf);
                bf.append(boxWidth, ' ');
            }

            return NextLine;
        }

        /**
         * @brief Create a blinking effect
         *
         * Causes the entire box to blink by toggling between normal and inverted colors.
         *
         * @param NumBlinks Number of complete blink cycles
         * @param Milliseconds Duration of each blink state in milliseconds
         */
        void blink(int NumBlinks = 2, long long Milliseconds = 250) const noexcept {
            // Each blink consists of two states: inverted and normal
            NumBlinks *= 2;

            for (int i = 1; i <= NumBlinks; i++) {
                // Toggle between negative and positive display
                if (i % 2) {
                    mz::SetNegative();
                }
                else {
                    mz::ClrNegative();
                }

                // Display the current state
                print();

                // Wait between states (except after the final state)
                if (i < NumBlinks) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(Milliseconds));
                }
            }
        }

        /**
         * @brief Insert multiple lines of text
         *
         * Convenience method to insert multiple lines at once.
         *
         * @param Lines Array of text lines
         * @param Count Number of lines to insert
         * @return The line number after insertion
         */
        template <typename T>
        int insert_lines(const T* Lines, size_t Count) noexcept {
            int result = NextLine;
            for (size_t i = 0; i < Count && NextLine < Area.num_rows(); i++) {
                insert_line(Lines[i]);
                result = NextLine;
            }
            return result;
        }

        /**
         * @brief Insert a line with centered text
         *
         * @param Msg Text to display
         * @param FrontColor Text color to use
         * @return The line number after insertion
         */
        int insert_centered_line(std::wstring_view Msg, rgb FrontColor) noexcept {
            // Create a centered version of the message
            std::wstring centered;
            centered.reserve(Area.num_cols());

            const int boxWidth = Area.num_cols();
            const int msgWidth = static_cast<int>(Msg.size());

            if (msgWidth < boxWidth) {
                // Calculate padding for centering
                int leftPadding = (boxWidth - msgWidth) / 2;

                // Add left padding
                centered.append(leftPadding, ' ');

                // Add message
                centered.append(Msg);

                // Add right padding
                centered.append(boxWidth - msgWidth - leftPadding, ' ');

                // Insert the centered line
                return insert_line(centered, FrontColor);
            }
            else {
                // If the message is too long, just insert it normally
                return insert_line(Msg, FrontColor);
            }
        }

        /**
         * @brief Insert a centered line with default color
         *
         * @param Msg Text to display
         * @return The line number after insertion
         */
        int insert_centered_line(std::wstring_view Msg) noexcept {
            return insert_centered_line(Msg, Color.F);
        }
    };

} // namespace mz

#endif // MZ_CONSOLE_BOXES_H
