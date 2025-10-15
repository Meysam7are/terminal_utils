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

#ifndef MZ_BUTTON_BOX_H
#define MZ_BUTTON_BOX_H
#pragma once

/**
 * @file ButtonBox.h
 * @brief Vertical button menu component for terminal UIs
 *
 * This file provides the ButtonBox class which implements a vertical stack
 * of selectable buttons for terminal-based user interfaces. It supports keyboard
 * navigation, focus highlighting, and selection.
 *
 * @author Meysam Zare
 */

#include "FrameBox.h"
#include <string>
#include <string_view>
#include <algorithm>
#include <thread>

namespace mz {

    /**
     * @class ButtonBox
     * @brief Vertical button menu component
     *
     * This class provides a vertical stack of selectable buttons with keyboard
     * navigation. It supports focus highlighting, different text alignment options,
     * and can be embedded in larger UI layouts.
     */
    class ButtonBox : public BasicBox {
    private:
        /**
         * @brief Temporary buffer for formatting operations
         *
         * Thread-local buffer to avoid repeated allocations during rendering.
         */
        inline static thread_local std::wstring Temp{};

        /**
         * @brief ANSI escape sequences for cursor and text formatting
         *
         * Preamble contains sequences to hide cursor and reset text formatting.
         */
        static constexpr wchar_t const Preamble[24]{ L"\x1b[?25l\x1b[22m\x1b[24m\x1b[27m" };

        /**
         * @brief Set a button to passive (unfocused) state
         *
         * Updates the color formatting for a button to show it as unfocused.
         *
         * @param Index Button index to update
         */
        void set_passive(long long Index) noexcept {
            Temp.clear();
            Color.apply(Temp);
            bf.replace(LineBlockSize * Index, Temp.size(), Temp);
        }

        /**
         * @brief Set a button to active (focused) state
         *
         * Updates the color formatting for a button to show it as focused.
         *
         * @param Index Button index to update
         */
        void set_active(long long Index) noexcept {
            Temp.clear();
            FocusColor.apply(Temp);
            bf.replace(LineBlockSize * Index, Temp.size(), Temp);
        }

        /**
         * @brief Offset to coordinate position in buffer
         *
         * Position in the buffer where coordinate ANSI sequences start.
         */
        int LineBlockCoordOffset{ 0 };

        /**
         * @brief Offset to text position in buffer
         *
         * Position in the buffer where button text begins.
         */
        int LineBlockTextOffset{ 0 };

        /**
         * @brief Total size of a button line in buffer
         *
         * Number of characters in the buffer for a complete button.
         */
        int LineBlockSize{ 0 };

        /**
         * @brief Set button text with alignment
         *
         * Updates the text content of a button with specified alignment.
         *
         * @param wv Button text
         * @param Index Button index to update
         * @param Alignment Text alignment (-1=left, 0=center, 1=right)
         * @return true if error occurred, false otherwise
         */
        bool set_button_text(std::wstring_view wv, int Index, int Alignment = 0) noexcept {
            // Check index bounds
            int NumButtons = Area.num_rows();
            int ButtonWidth = Area.num_cols();
            if (Index < 0 || Index >= NumButtons) { return true; }

            // Calculate padding based on alignment
            int ViewLength = static_cast<int>(wv.size());
            int LeftPadding = 0;
            int RightPadding = 0;

            if (ViewLength >= ButtonWidth) {
                // Text is too long, truncate
                ViewLength = ButtonWidth;
            }
            else if (Alignment < 0) {
                // Left alignment
                RightPadding = ButtonWidth - ViewLength;
            }
            else if (Alignment > 0) {
                // Right alignment
                LeftPadding = ButtonWidth - ViewLength;
            }
            else {
                // Center alignment
                LeftPadding = (ButtonWidth - ViewLength) / 2;
                RightPadding = ButtonWidth - ViewLength - LeftPadding;
            }

            // Prepare the formatted button text
            std::wstring Line;
            Line.reserve(static_cast<size_t>(ButtonWidth));

            if (LeftPadding > 0) {
                Line.append(static_cast<size_t>(LeftPadding), ' ');
            }

            Line.append(wv.substr(0, static_cast<size_t>(ViewLength)));

            if (RightPadding > 0) {
                Line.append(static_cast<size_t>(RightPadding), ' ');
            }

            // Update the button text in the buffer
            bf.replace(Index * LineBlockSize + LineBlockTextOffset, Line.size(), Line);

            return false;
        }

    public:
        /**
         * @brief Index of the currently focused button
         *
         * -1 indicates no button is focused.
         */
        int FocusIndex{ -1 };

        /**
         * @brief Color scheme for the focused button
         *
         * Defines how the active/focused button appears.
         */
        color FocusColor;

        /**
         * @brief Clear the button box and prepare for new content
         *
         * Resets the component to an empty state with no buttons.
         */
        void clear() noexcept {
            // Get button width
            int ButtonWidth = Area.num_cols();

            // Reset area height to zero (no buttons)
            Area.set_rows(0);

            // Clear and prepare the buffer
            bf.clear();

            // Apply base color and save position
            Color.apply(bf);
            LineBlockCoordOffset = static_cast<int>(bf.size());

            // Add position placeholder
            Area.Top.apply(bf);

            // Add formatting control sequences
            bf.append(Preamble, 23);
            LineBlockTextOffset = static_cast<int>(bf.size());

            // Calculate the total size of each button block
            LineBlockSize = LineBlockTextOffset + ButtonWidth;

            // Reset the buffer to empty
            bf.clear();
        }

        /**
         * @brief Add a new button to the box
         *
         * Appends a new button with the specified text and alignment.
         *
         * @param wv Button text
         * @param Alignment Text alignment (-1=left, 0=center, 1=right)
         */
        void append(std::wstring_view wv, int Alignment = 0) noexcept {
            // Get button dimensions
            int ButtonWidth = Area.num_cols();
            int NumButtons = Area.num_rows();
            int Index = NumButtons++;

            // Increase area height for new button
            Area.set_rows(NumButtons);

            // Apply color and position
            Color.apply(bf);
            Area.Top.offset(Index, 0).apply(bf);

            // Add formatting sequences
            bf.append(Preamble, 23);

            // Reserve space for button text
            bf.resize(bf.size() + static_cast<size_t>(ButtonWidth));

            // Set the button text with specified alignment
            set_button_text(wv, Index, Alignment);
        }

        /**
         * @brief Move focus down one button
         *
         * Moves the focus highlight to the next button below.
         *
         * @return true if focus was moved, false if at bottom
         */
        bool move_down() noexcept {
            if (FocusIndex >= 0 && FocusIndex + 1 < Area.num_rows()) {
                // Update button states
                set_passive(FocusIndex);
                set_active(FocusIndex + 1);

                // Update display
                mz::Write(bf.substr(
                    static_cast<size_t>(LineBlockSize * FocusIndex),
                    static_cast<size_t>(LineBlockSize * 2)
                ));

                // Update focus index
                ++FocusIndex;
                return true;
            }
            return false;
        }

        /**
         * @brief Move focus to bottom button
         *
         * Moves the focus highlight directly to the last button.
         *
         * @return true if focus was moved, false if already at bottom
         */
        bool page_down() noexcept {
            int LastButton = Area.num_rows() - 1;
            if (FocusIndex >= 0 && FocusIndex < LastButton) {
                // Update button states
                set_passive(FocusIndex);
                set_active(LastButton);

                // Update display
                mz::Write(bf.substr(
                    static_cast<size_t>(LineBlockSize * FocusIndex),
                    static_cast<size_t>(LineBlockSize * (LastButton - FocusIndex + 1))
                ));

                // Update focus index
                FocusIndex = LastButton;
                return true;
            }
            return false;
        }

        /**
         * @brief Move focus up one button
         *
         * Moves the focus highlight to the next button above.
         *
         * @return true if focus was moved, false if at top
         */
        bool move_up() noexcept {
            if (FocusIndex > 0 && FocusIndex < Area.num_rows()) {
                // Update focus index
                --FocusIndex;

                // Update button states
                set_active(FocusIndex);
                set_passive(FocusIndex + 1);

                // Update display
                mz::Write(bf.substr(
                    static_cast<size_t>(LineBlockSize * FocusIndex),
                    static_cast<size_t>(LineBlockSize * 2)
                ));

                return true;
            }
            return false;
        }

        /**
         * @brief Move focus to top button
         *
         * Moves the focus highlight directly to the first button.
         *
         * @return true if focus was moved, false if already at top
         */
        bool page_up() noexcept {
            if (FocusIndex > 0 && FocusIndex < Area.num_rows()) {
                // Update button states
                set_active(0);
                set_passive(FocusIndex);

                // Update display
                mz::Write(bf.substr(
                    0,
                    static_cast<size_t>(LineBlockSize * (FocusIndex + 1))
                ));

                // Update focus index
                FocusIndex = 0;
                return true;
            }
            return false;
        }

        /**
         * @brief Set focus to a specific button
         *
         * Updates the focus highlight to the specified button index.
         *
         * @param Index Button index to focus
         */
        void set_focus(int Index) noexcept {
            // Set focus index
            FocusIndex = Index;

            // Update all buttons' states
            for (int i = 0; i < Area.num_rows(); i++) {
                if (i == FocusIndex) {
                    set_active(i);
                }
                else {
                    set_passive(i);
                }
            }

            // Render the updated display
            print();
        }

        /**
         * @brief Interactive button selection loop
         *
         * Enters an input loop allowing the user to navigate and select a button.
         *
         * @param Index Initial button to focus
         * @return Selected button index, or -1 if canceled
         */
        int get(int Index = 0) noexcept {
            // Set initial focus
            set_focus(Index);

            // Input loop
            while (true) {
                switch (wgetch()) {
                case PAGEUPKEY: page_up(); break;
                case PAGEDOWNKEY: page_down(); break;
                case UPKEY: move_up(); break;
                case DOWNKEY: move_down(); break;
                case RETURNKEY: return FocusIndex;
                case ESCAPEKEY: return -1;
                }
            }
        }

        /**
         * @brief Move the button box to a new position
         *
         * Relocates the entire button box to a different screen position.
         *
         * @param NewTopLeft New top-left position
         */
        void move_to(coord NewTopLeft) noexcept {
            // Get number of buttons
            int NumButtons = Area.num_rows();

            // Update area position
            Area.move_top_to(NewTopLeft);

            // Update each button's position in the buffer
            std::wstring Text;
            for (int i = 0; i < NumButtons; i++) {
                Text.clear();
                Area.Top.offset(i, 0).apply(Text);
                bf.replace(
                    LineBlockSize * i + LineBlockCoordOffset,
                    Text.size(),
                    Text
                );
            }

            // Render the updated display
            mz::Write(bf);
        }

        /**
         * @brief Default constructor
         *
         * Initializes with default color scheme and control keys.
         */
        ButtonBox() noexcept {
            // Set color scheme
            Color = color{ mz::color::BLACK, mz::color::LAVENDER };

            // Set focus color (will be blended in constructor)
            FocusColor = Color;

            // Create passive color with reduced contrast
            Color = Color.blend(30);

            // Set up control keys
            Controls.UpDown = 1;
            Controls.Escape = 1;
            Controls.Return = 1;
            Controls.PageUpDown = 1;
        }

        /**
         * @brief Constructor with custom colors
         *
         * @param normalColor Color for unfocused buttons
         * @param focusColor Color for focused button
         */
        ButtonBox(color normalColor, color focusColor) noexcept :
            BasicBox(normalColor), FocusColor(focusColor) {
            // Set up control keys
            Controls.UpDown = 1;
            Controls.Escape = 1;
            Controls.Return = 1;
            Controls.PageUpDown = 1;
        }

        /**
         * @brief Set button text for an existing button
         *
         * Updates the text of a button at the specified index.
         *
         * @param index Button index to update
         * @param text New button text
         * @param alignment Text alignment (-1=left, 0=center, 1=right)
         * @return true if successful, false if index out of range
         */
        bool update_button(int index, std::wstring_view text, int alignment = 0) noexcept {
            if (index < 0 || index >= Area.num_rows()) {
                return false;
            }

            return !set_button_text(text, index, alignment);
        }

        /**
         * @brief Get number of buttons
         *
         * @return Number of buttons in the box
         */
        int button_count() const noexcept {
            return Area.num_rows();
        }

        /**
         * @brief Set button colors
         *
         * Changes the color scheme for normal and focused buttons.
         *
         * @param normalColor Color for unfocused buttons
         * @param focusColor Color for focused button
         */
        void set_colors(color normalColor, color focusColor) noexcept {
            Color = normalColor;
            FocusColor = focusColor;

            // Update display if any button is focused
            if (FocusIndex >= 0 && FocusIndex < Area.num_rows()) {
                set_focus(FocusIndex);
            }
        }

        /**
         * @brief Run test of the button box
         *
         * Creates a sample button box for testing.
         */
        static void test() {
            mz::ButtonBox c;

            // Set position and width
            c.Area.Top = mz::coord{ 2, 2 };
            c.Area.set_cols(13);
            c.FocusIndex = 0;

            // Initialize and add buttons
            c.clear();
            c.append(L"ONE");
            c.append(L"TWO");
            c.append(L"THREE");
            c.append(L"FOUR");
            c.append(L"FIVE");

            // Display the button box
            mz::Write(c.bf);

            // Test interaction loop
            while (true) {
                switch (mz::wgetch()) {
                case mz::UPKEY: c.move_up(); break;
                case mz::DOWNKEY: c.move_down(); break;
                case mz::RIGHTKEY: c.move_to(c.Area.Top.offset(2, 20)); break;
                default: return;
                }
            }
        }
    };

} // namespace mz

#endif // MZ_BUTTON_BOX_H
