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

#ifndef MZ_FOOTER_BOX_H
#define MZ_FOOTER_BOX_H
#pragma once

/**
 * @file FooterBox.h
 * @brief Status bar/footer implementation for terminal UI
 *
 * This file provides a specialized UI component for displaying status messages
 * at the bottom of terminal-based user interfaces. It supports text updates,
 * visual effects like blinking for alerts, and consistent styling.
 *
 * @author Meysam Zare
 */

#include "ConsoleCMD.h"
#include "coord.h"
#include "cursor.h"
#include "ConsoleBoxes.h"
#include <string>
#include <string_view>
#include <thread>
#include <chrono>

namespace mz {

    /**
     * @class FooterBox
     * @brief Single-line status bar for displaying messages
     *
     * This component provides a styled footer/status bar for terminal UIs.
     * It supports dynamic text updates, visual alerts through blinking,
     * and maintains consistent appearance with special left/right edge
     * characters.
     */
    class FooterBox : public BasicBox {
    private:
        /**
         * @brief Available space for text
         *
         * Tracks remaining character capacity in the footer.
         */
        int Capacity{ 0 };

        /**
         * @brief Buffer position after initial formatting
         *
         * Marks the buffer position after formatting but before content.
         */
        int InitSize{ 0 };

        /**
         * @brief Buffer position before end formatting
         *
         * Marks the buffer position where content ends and footer
         * end formatting begins.
         */
        int EndSize{ 0 };

        /**
         * @brief Complete the footer with ending elements
         *
         * Adds trailing spaces and the right edge character to finish the footer.
         */
        void fill_end() noexcept {
            EndSize = static_cast<int>(bf.size());

            // Fill any remaining space with spaces
            if (Capacity > 0) {
                bf.append(Capacity, ' ');
            }

            // Add right edge character
            PushBack(bf, RIGHTHALF);

            // Turn off underline and move cursor back one position
            ClrUnderline(bf);
            MoveLeft(bf);
        }

    public:
        /**
         * @brief Reset the footer to empty state
         *
         * Clears all content while maintaining the footer's visual structure.
         */
        void clear() noexcept override {
            Capacity = Area.num_cols() - 2;  // Account for left and right edge chars

            // Reset buffer to initial state (keep pre-allocated memory)
            const size_t currentCapacity = bf.capacity();
            bf.resize(InitSize);
            bf.reserve(currentCapacity);

            // Complete the footer with ending elements
            fill_end();
        }

        /**
         * @brief Add text to the footer
         *
         * Appends text to the footer, truncating if necessary to fit.
         *
         * @param text Text to append
         * @return true if text was truncated, false if it fit completely
         */
        bool append(std::wstring_view text) noexcept {
            // Resize buffer to content position
            bf.resize(EndSize);

            // Check if text needs to be truncated
            if (Capacity < text.size()) {
                // Truncate text to fit available space
                bf.append(text.substr(0, Capacity));
                Capacity = 0;
                fill_end();
                return true;  // Text was truncated
            }
            else {
                // Text fits completely
                bf.append(text);
                Capacity -= static_cast<int>(text.size());
                fill_end();
                return false;  // Text fit completely
            }
        }

        /**
         * @brief Add a special symbol to the footer
         *
         * Appends a predefined symbol/character sequence to the footer.
         *
         * @tparam N Size of the character array
         * @param Symbol Character array to append
         * @return true if there wasn't enough space, false otherwise
         */
        template <size_t N>
            requires (N > 0 && N < 6)
        bool push_back(const wchar_t(&Symbol)[N]) noexcept {
            if (Capacity > 0) {
                bf.resize(EndSize);
                PushBack(bf, Symbol);
                --Capacity;
                fill_end();
                return false;  // Symbol was added successfully
            }
            return true;  // No capacity left
        }

        /**
         * @brief Initialize the footer
         *
         * Creates a new footer in the specified screen area.
         *
         * @param Place Screen area to use (will use just the bottom row)
         */
        void create(coord_box Place) noexcept {
            // Ensure minimum width
            if (Place.num_cols() < 2) {
                Place.set_cols(2);
            }

            // Use only the bottom row of the provided area
            Area = Place.bottom_rows(1);

            // Initialize state
            Capacity = 0;

            // Pre-allocate buffer with reasonable capacity
            const size_t requiredCapacity =
                100 + static_cast<size_t>(Area.num_cols() * 2);
            bf.clear();
            bf.reserve(requiredCapacity);

            // Apply colors and position cursor
            Color.apply(bf);
            Area.Top.apply(bf);

            // Set text formatting
            ClrBold(bf);
            ClrNegative(bf);
            SetUnderline(bf);
            SetHide(bf);

            // Add left edge character
            PushBack(bf, LEFTHALF);

            // Store initial buffer size
            InitSize = static_cast<int>(bf.size());

            // Complete footer setup
            clear();
        }

        /**
         * @brief Replace footer text with new message
         *
         * Clears current content and displays a new message.
         *
         * @param Msg New message to display
         */
        void update_status(std::wstring_view Msg) noexcept {
            clear();
            append(Msg);
        }

        /**
         * @brief Create visual alert by blinking the footer
         *
         * Alternates between normal and alert colors to draw attention.
         *
         * @param BlinkBack Background color for the alert state
         * @param NumBlinks Number of complete blink cycles
         * @param NumMilliSeconds Duration of each blink state in milliseconds
         */
        void blink(rgb BlinkBack = color::RED, int NumBlinks = 2,
            int NumMilliSeconds = 150) noexcept {
            // Prepare temporary buffer and alternative color
            std::wstring Temp;
            color TempColor{ Color };
            TempColor.B = BlinkBack;

            // Calculate total states (each blink has two states)
            const int totalStates = NumBlinks * 2;

            // Execute the blink sequence
            for (int i = 0; i < totalStates; i++) {
                Temp.clear();

                // Alternate between normal and alert colors
                if (i % 2) {
                    Color.apply(Temp);
                }
                else {
                    TempColor.apply(Temp);
                }

                // Update color in buffer
                bf.replace(0, Temp.size(), Temp);

                // Display the current state
                print();

                // Wait before next state
                std::this_thread::sleep_for(std::chrono::milliseconds(NumMilliSeconds));
            }
        }

        /**
         * @brief Center text in the footer
         *
         * Clears the footer and displays text centered horizontally.
         *
         * @param text Text to display
         */
        void centered_text(std::wstring_view text) noexcept {
            clear();

            const int maxLength = Area.num_cols() - 2;  // Account for edge chars
            const int textLength = static_cast<int>(text.length());

            if (textLength < maxLength) {
                // Calculate padding for centering
                const int padding = (maxLength - textLength) / 2;

                // Add left padding
                if (padding > 0) {
                    bf.resize(EndSize);
                    bf.append(padding, ' ');
                    Capacity -= padding;
                }

                // Add text
                append(text);
            }
            else {
                // Text too long, just truncate
                append(text);
            }
        }

        /**
         * @brief Right-align text in the footer
         *
         * Clears the footer and displays text aligned to the right.
         *
         * @param text Text to display
         */
        void right_aligned_text(std::wstring_view text) noexcept {
            clear();

            const int maxLength = Area.num_cols() - 2;  // Account for edge chars
            const int textLength = static_cast<int>(text.length());

            if (textLength < maxLength) {
                // Calculate padding for right alignment
                const int padding = maxLength - textLength;

                // Add left padding
                if (padding > 0) {
                    bf.resize(EndSize);
                    bf.append(padding, ' ');
                    Capacity -= padding;
                }

                // Add text
                append(text);
            }
            else {
                // Text too long, just truncate
                append(text);
            }
        }

        /**
         * @brief Constructor with default settings
         */
        FooterBox() noexcept {
            // Set default colors (silver text on dark gray background)
            Color = color{ color::SILVER, color::GRAY.darken(50) };
        }
    };

} // namespace mz

#endif // MZ_FOOTER_BOX_H
