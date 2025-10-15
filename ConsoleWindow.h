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

#ifndef MZ_CONSOLE_WINDOW_H
#define MZ_CONSOLE_WINDOW_H
#pragma once

/**
 * @file ConsoleWindow.h
 * @brief Full-screen terminal window with graphical capabilities
 *
 * This file provides the ConsoleWindow class, which combines a graphical/bitmap
 * display area with a message box for text output. It can render console graphics
 * and provides functionality for updating specific regions of the display.
 *
 * @author Meysam Zare
 */

#include "ConsoleBoxes.h"
#include "FooterBox.h"
#include <string>
#include <string_view>
#include <stdexcept>

namespace mz {

    /**
     * @class ConsoleWindow
     * @brief Full terminal window with graphics and messaging capabilities
     *
     * This class provides a complete terminal UI window that can display
     * bitmap graphics and text messages. It manages the layout of the window
     * and provides methods to draw and update both the entire window and
     * specific regions.
     */
    class ConsoleWindow : public mz::BasicBox {
    public:
        /**
         * @brief Message display area for text output
         *
         * This sub-component handles text message display in the window.
         */
        MultilineMessageBox MsgBox{};

        /**
         * @brief Initialize the window with a bitmap and boundaries
         *
         * Sets up the window to display the given bitmap within the specified area.
         * Configures the layout including the message display area and prepares
         * the graphical content.
         *
         * @param Pic Console bitmap to display
         * @param Boundary Screen area to use for the window
         * @throws std::bad_alloc If memory allocation fails during initialization
         */
        void initialize(mz::console_picture const& Pic, mz::coord_box Boundary) {
            // Configure message box position at bottom of window
            Window = Boundary;
            MsgBox.Area = Window.bottom_rows(5, 1, 1).shift(-1, 0);
            Area = Window.top_rows(Window.num_rows() - 6);

            // Calculate dimensions for the bitmap
            int LogoWidth = Pic.Width;
            int LogoHeight = Pic.Height;
            auto WindowSize = Area.get_size();

            // Ensure bitmap fits within the window
            if (LogoHeight > WindowSize.Row) {
                LogoHeight = WindowSize.Row;
            }

            if (LogoWidth > WindowSize.Col) {
                LogoWidth = WindowSize.Col;
            }

            // Calculate padding for centering
            int WidthPadding = WindowSize.Col - LogoWidth;
            int LeftPad = WidthPadding / 2;
            int RightPad = WidthPadding - LeftPad;

            int HeightPadding = WindowSize.Row - LogoHeight;
            int TopPad = HeightPadding / 2;
            int BottomPad = HeightPadding - TopPad;

            // Calculate line length for the picture
            PictureLineLength = WindowSize.Col * 2 + 2;

            // Pre-allocate the picture buffer
            Picture.clear();
            Picture.reserve((PictureLineLength * WindowSize.Row) + 10);

            // Add top padding
            for (int i = 0; i < TopPad; i++) {
                for (int j = 0; j < WindowSize.Col; j++) {
                    mz::PushBack(Picture, mz::BLOCK00);
                }
                mz::PushBack(Picture, mz::ENDLINE2);

                // Verify line length is correct
                if (i == 0 && PictureLineLength != Picture.size()) {
                    throw std::bad_alloc();
                }
            }

            // Add bitmap rows with padding
            for (int i = 0; i < LogoHeight; i++) {
                // Left padding
                for (int j = 0; j < LeftPad; j++) {
                    mz::PushBack(Picture, mz::BLOCK00);
                }

                // Bitmap content
                for (int j = 0; j < LogoWidth; j++) {
                    if (Pic.Pixels[j + i * Pic.Width]) {
                        mz::PushBack(Picture, mz::BLOCK75);
                    }
                    else {
                        mz::PushBack(Picture, mz::BLOCK00);
                    }
                }

                // Right padding
                for (int j = 0; j < RightPad; j++) {
                    mz::PushBack(Picture, mz::BLOCK00);
                }

                mz::PushBack(Picture, mz::ENDLINE2);
            }

            // Add bottom padding (including space for message box)
            BottomPad += MsgBox.Area.num_rows() + 1;
            for (int i = 1; i <= BottomPad; i++) {
                for (int j = 0; j < WindowSize.Col; j++) {
                    mz::PushBack(Picture, mz::BLOCK00);
                }
                mz::PushBack(Picture, mz::ENDLINE2);
            }

            // Remove trailing newline characters if needed
            if (WindowSize.Row > 0) {
                Picture.pop_back();
                Picture.pop_back();
            }
        }

        /**
         * @brief Draw the entire window
         *
         * Renders the complete window including the bitmap and message area.
         */
        void draw() {
            // Prepare the buffer
            bf.clear();
            mz::SetHide(bf);
            Color.apply(bf);
            Area.Top.apply(bf);

            // Output to terminal
            mz::Write(bf);
            mz::Write(Picture);
        }

        /**
         * @brief Draw a specific region of the window
         *
         * Renders only a portion of the window, which is more efficient
         * for updates to small areas.
         *
         * @param Box Region to redraw
         */
        void draw(mz::coord_box Box) {
            // Ensure the box is within window boundaries
            Box.Top.Row = std::max(Box.Top.Row, Window.Top.Row);
            Box.Top.Col = std::max(Box.Top.Col, Window.Top.Col);
            Box.Bottom.Row = std::min(Box.Bottom.Row, Window.Bottom.Row - 1);
            Box.Bottom.Col = std::min(Box.Bottom.Col, Window.Bottom.Col);

            // Prepare and output the positioning commands
            bf.clear();
            mz::SetHide(bf);
            Color.apply(bf);
            Box.Top.apply(bf);
            mz::Write(bf);

            // Draw each row of the specified region
            bf.clear();
            mz::MoveLeft(bf, Box.num_cols());
            mz::MoveDown(bf);
            for (int i = Box.Top.Row; i < Box.Bottom.Row; i++) {
                // Extract the relevant portion of the picture for this row
                auto wv = std::wstring_view(Picture).substr(
                    PictureLineLength * i + Box.Top.Col * 2 - 2,
                    Box.num_cols() * 2
                );

                // Write the row and move to the next line
                mz::Write(wv);
                mz::Write(bf);
            }

            // Handle the last row
            bf.clear();
            mz::SetHide(bf);
            mz::MoveLeft(bf);
            mz::color::BLACK.setBack(bf);
            mz::color::WHITE.setFront(bf);
            mz::Write(std::wstring_view(Picture).substr(
                PictureLineLength * Box.Bottom.Row + Box.Top.Col * 2 - 2,
                Box.num_cols() * 2
            ));
            mz::Write(bf);
        }

        /**
         * @brief Set a note or caption (placeholder)
         *
         * This method is a placeholder for setting a note or caption
         * in the window.
         *
         * @param note Text to display (currently unused)
         */
        void set_note(std::wstring_view note) noexcept {
            // Placeholder for future implementation
        }

        /**
         * @brief Get the footer area of the window
         *
         * Returns the coordinate box for the bottom row of the window,
         * which can be used for a status bar or footer.
         *
         * @return Coordinate box for the footer area
         */
        mz::coord_box footer() const noexcept {
            return Window.bottom_rows(1);
        }

        /**
         * @brief Clear the message area
         *
         * Redraws the message area, effectively clearing its contents.
         */
        void clear_message() noexcept {
            draw(MsgBox.Area);
        }

        /**
         * @brief Default constructor with predefined colors
         *
         * Initializes the window with black text on dark gray background
         * and a message box with white text on dark khaki.
         */
        ConsoleWindow() noexcept {
            // Set main window colors
            Color.F = mz::rgb(0, 0, 0);
            Color.B = mz::rgb(20, 20, 20);

            // Set message box colors
            MsgBox.Color.F = color::WHITE;
            MsgBox.Color.B = color::KHAKI.darken(70);
        }

        /**
         * @brief Constructor with specified window area
         *
         * @param Window Coordinate box defining the window boundaries
         */
        ConsoleWindow(mz::coord_box Window) noexcept : Window{ Window } {
            // Set main window colors
            Color.F = mz::rgb(0, 0, 0);
            Color.B = mz::rgb(20, 20, 20);

            // Set message box colors
            MsgBox.Color.F = color::WHITE;
            MsgBox.Color.B = color::KHAKI.darken(70);
        }

    private:
        /**
         * @brief Window boundaries
         *
         * Defines the rectangular area of the entire window.
         */
        mz::coord_box Window;

        /**
         * @brief Bitmap content as a formatted string
         *
         * Contains the pre-formatted bitmap with appropriate padding
         * and formatting characters.
         */
        std::wstring Picture;

        /**
         * @brief Length of each picture line including newline characters
         *
         * Used for calculating positions within the picture buffer.
         */
        int PictureLineLength{ 0 };
    };

} // namespace mz

#endif // MZ_CONSOLE_WINDOW_H
