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

#ifndef MZ_FRAME_BOX_H
#define MZ_FRAME_BOX_H
#pragma once

/**
 * @file FrameBox.h
 * @brief Terminal UI framed box component with title and footer
 *
 * This file provides the FrameBox class which implements a framed rectangular
 * area with a title bar and optional footer for terminal-based user interfaces.
 * It's designed to create styled, consistent UI components with borders.
 *
 * @author Meysam Zare
 */

#include "ConsoleBoxes.h"
#include "FooterBox.h"
#include <string>
#include <string_view>
#include <algorithm>

namespace mz {

    /**
     * @class FrameBox
     * @brief Bordered box with title and footer for terminal UI
     *
     * This class provides a rectangular UI component with a titled border
     * and optional footer. It's useful for creating dialog boxes, panels,
     * and other framed content areas in terminal-based user interfaces.
     */
    class FrameBox : public BasicBox {
    public:
        /**
         * @brief Buffer position after initial formatting
         *
         * Marks the buffer position after formatting but before content.
         */
        int PreMessageSize{ 0 };

        /**
         * @brief Footer component at the bottom of the frame
         *
         * Provides status text display capability at the bottom of the frame.
         */
        FooterBox Footer;

        /**
         * @brief Initialize the frame with title and position
         *
         * Creates a new framed box with the specified title and position.
         * The frame includes a border with the title displayed in the top border,
         * and a footer component at the bottom.
         *
         * @param Title Text to display in the frame title bar
         * @param BoxArea Screen area to use for the frame
         */
        void create(std::wstring_view Title, coord_box BoxArea) {
            // Ensure minimum dimensions for the box
            Area = BoxArea;
            Area.normalize(3, 3);

            // Configure footer colors based on the frame colors
            Footer.Color.F = Color.F;
            Footer.Color.B = Color.B.mix(Color.F, 30);

            // Calculate dimensions and title length
            coord Size = Area.get_size();
            int TitleLength = static_cast<int>(Title.size());

            // Ensure title fits within the frame width
            if (TitleLength >= Size.Col - 1) {
                TitleLength = Size.Col - 2;
            }

            // Pre-allocate buffer with reasonable capacity
            const size_t estimatedCapacity = static_cast<size_t>(Size.Col * Size.Row * 3);
            bf.clear();
            bf.reserve(estimatedCapacity);

            // Create the title bar with inverted colors
            Color.apply_mirror(bf);
            Area.Top.apply(bf);
            bf.push_back(L' ');  // Leading space

            // Add the title text (truncated if necessary)
            bf.append(Title.data(), std::min<size_t>(TitleLength, Title.size()));

            // Fill the rest of the title bar with spaces
            if (Size.Col > TitleLength + 1) {
                bf.append(static_cast<size_t>(Size.Col - 1 - TitleLength), ' ');
            }

            // Move to next line and reset colors
            MoveLeft(bf, Size.Col);
            MoveDown(bf);
            Color.apply(bf);

            // Draw the frame borders
            for (int i = 2; i <= Size.Row; i++) {
                // Add underline to the last row
                if (i == Size.Row) {
                    SetUnderline(bf);
                }

                // Draw left border character
                PushBack(bf, LEFTHALF);

                // Draw middle spaces
                bf.append(static_cast<size_t>(Size.Col - 2), ' ');

                // Draw right border character
                PushBack(bf, RIGHTHALF);

                // Move to next line
                MoveLeft(bf, Size.Col);
                MoveDown(bf);
            }

            // Reset text formatting
            ClrUnderline(bf);

            // Save position for content
            PreMessageSize = static_cast<int>(bf.size());

            // Initialize the footer at the bottom of the frame
            Footer.create(Area.bottom_rows(1));
        }

        /**
         * @brief Output the frame and footer to the terminal
         *
         * Renders both the frame and its footer component.
         */
        void print() const noexcept {
            BasicBox::print();
            Footer.print();
        }

        /**
         * @brief Get content area within the frame
         *
         * Returns a coordinate box representing the usable area inside the frame borders.
         *
         * @return Coordinate box for content placement
         */
        coord_box content_area() const noexcept {
            return coord_box{
                Area.Top.offset(1, 1),             // One down, one right from frame top-left
                Area.Bottom.offset(-2, -1)         // One up, one left from frame bottom-right
            };
        }

        /**
         * @brief Update the frame's title text
         *
         * @param Title New title text
         */
        void set_title(std::wstring_view Title) noexcept {
            // Calculate dimensions and title length
            coord Size = Area.get_size();
            int TitleLength = static_cast<int>(Title.size());

            // Ensure title fits within the frame width
            if (TitleLength >= Size.Col - 1) {
                TitleLength = Size.Col - 2;
            }

            // Create temporary buffer for the new title bar
            std::wstring titleBar;
            titleBar.reserve(static_cast<size_t>(Size.Col) + 64);

            // Apply inverted colors and position cursor
            Color.apply_mirror(titleBar);
            Area.Top.apply(titleBar);

            // Add title with padding
            titleBar.push_back(L' ');  // Leading space
            titleBar.append(Title.data(), std::min<size_t>(TitleLength, Title.size()));

            // Fill the rest of the title bar with spaces
            if (Size.Col > TitleLength + 1) {
                titleBar.append(static_cast<size_t>(Size.Col - 1 - TitleLength), ' ');
            }

            // Output the new title bar
            Write(titleBar);
        }

        /**
         * @brief Clear the content area of the frame
         *
         * Fills the inside of the frame with spaces, preserving the borders.
         */
        void clear_content() noexcept {
            // Get content area dimensions
            coord_box content = content_area();
            int width = content.num_cols();
            int height = content.num_rows();

            if (width <= 0 || height <= 0) return;

            // Create temporary buffer
            std::wstring clearBuf;
            clearBuf.reserve(static_cast<size_t>(width + 10));

            // Apply colors and prepare spaces
            Color.apply(clearBuf);
            std::wstring spaces(static_cast<size_t>(width), ' ');

            // Clear each line in the content area
            for (int i = 0; i < height; i++) {
                content.Top.offset(i, 0).apply(clearBuf);
                clearBuf.append(spaces);
                Write(clearBuf);
                clearBuf.clear();
            }
        }

        /**
         * @brief Set status message in the footer
         *
         * Updates the footer with a new message.
         *
         * @param message Message to display
         */
        void set_status(std::wstring_view message) noexcept {
            Footer.update_status(message);
        }

        /**
         * @brief Create a visual alert by blinking the footer
         *
         * @param color Alert color (default: red)
         * @param blinks Number of blink cycles (default: 2)
         * @param duration Duration of each blink state in milliseconds (default: 150)
         */
        void alert(rgb color = color::RED, int blinks = 2, int duration = 150) noexcept {
            Footer.blink(color, blinks, duration);
        }

        /**
         * @brief Default constructor
         *
         * Initializes with default colors for frame and footer.
         */
        FrameBox() noexcept {
            // Set default color scheme (silver text on navy background)
            Color.F = color::SILVER;
            Color.B = color::NAVY;
        }

        /**
         * @brief Constructor with specified colors
         *
         * @param frameColor Color scheme for the frame
         */
        explicit FrameBox(color frameColor) noexcept : BasicBox(frameColor) {
            // Footer will be initialized in create()
        }

        /**
         * @brief Get the area for the footer
         *
         * @return Coordinate box for the footer area
         */
        coord_box footer_area() const noexcept {
            return Area.bottom_rows(1);
        }
    };

} // namespace mz

#endif // MZ_FRAME_BOX_H
