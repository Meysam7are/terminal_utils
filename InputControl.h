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

#ifndef MZ_INPUT_CONTROL_H
#define MZ_INPUT_CONTROL_H
#pragma once

/**
 * @file InputControl.h
 * @brief Terminal-based text input field component
 *
 * This file provides the InputControl class, which implements a text input field
 * for terminal-based user interfaces. It supports text editing with features like
 * insert/overwrite modes, cursor navigation, and password masking.
 *
 * @author Meysam Zare
 */

#include <vector>
#include <string>
#include <stdexcept>
#include "ConsoleCMD.h"
#include "coord.h"
#include "cursor.h"
#include "ConsoleBoxes.h"
#include "FooterBox.h"

namespace mz {

    /**
     * @class InputControl
     * @brief Text input field for terminal UI
     *
     * This class provides a single-line text input field with editing capabilities
     * including cursor movement, insert/overwrite modes, and password masking.
     * It manages cursor display and text scrolling for inputs longer than the
     * visible field width.
     */
    class InputControl : public BasicBox {
    private:
        /**
         * @brief Process control key input
         *
         * Handles special keys like arrows, Home/End, Delete, etc.
         *
         * @param x Key code to process
         * @return true if the key was processed, false otherwise
         */
        bool process_control_keys(int x) noexcept {
            int BoxLength = box_length();
            int EndIndex = BeginIndex + BeginOffset;

            switch (x) {
            case BACKSPACEKEY: {
                // Handle backspace key - delete character to the left of cursor
                if (BeginIndex + BeginOffset > 0) {
                    // Shift remaining text left
                    for (size_t i = BeginIndex + BeginOffset; i < Text.size(); i++) {
                        Text[i - 1] = Text[i];
                    }
                    Text.pop_back();

                    // Move cursor left
                    --BeginOffset;
                    if (BeginOffset < 0) {
                        BeginOffset = 0;
                        --BeginIndex;
                    }
                    print_and_display_cursor();
                }
                return true;
            }

            case HOMEKEY: {
                // Handle home key - move cursor to start of text
                if (BeginIndex + BeginOffset > BoxLength) {
                    // Text scrolled - reset position
                    BeginIndex = 0;
                    BeginOffset = 0;
                    print_and_display_cursor();
                }
                else if (BeginIndex + BeginOffset > 0) {
                    // Move cursor left to start
                    mz::MoveLeft(BeginOffset);
                    if (BeginOffset == BoxLength) {
                        write_shape();
                    }
                    BeginIndex = 0;
                    BeginOffset = 0;
                }
                return true;
            }

            case LEFTKEY: {
                // Handle left arrow key - move cursor left
                if (BeginOffset == BoxLength) {
                    // At right edge of box - move left
                    --BeginOffset;
                    write_left_and_shape();
                }
                else if (BeginOffset > 0) {
                    // Within box - move left
                    --BeginOffset;
                    write_left();
                }
                else if (BeginIndex > 0) {
                    // At left edge - scroll text
                    --BeginIndex;
                    print_and_display_cursor();
                }
                return true;
            }

            case RIGHTKEY: {
                // Handle right arrow key - move cursor right
                if (BeginIndex + BeginOffset < static_cast<int>(Text.size())) {
                    ++BeginOffset;
                    if (BeginOffset < BoxLength) {
                        // Within box - move right
                        mz::MoveRight();
                    }
                    else if (BeginOffset == BoxLength) {
                        // At right edge - show bar cursor
                        write_right_bbar();
                    }
                    else {
                        // Beyond right edge - scroll text
                        ++BeginIndex;
                        BeginOffset = BoxLength;
                        print_and_display_cursor();
                    }
                }
                return true;
            }

            case ENDKEY: {
                // Handle end key - move cursor to end of text
                int textSize = static_cast<int>(Text.size());

                if (BeginIndex + BoxLength > textSize) {
                    // End is visible - move to it
                    if (BeginIndex + BeginOffset < textSize) {
                        mz::MoveRight(textSize - BeginIndex - BeginOffset);
                        BeginOffset = textSize - BeginIndex;
                    }
                }
                else if (BeginIndex + BoxLength < textSize) {
                    // End is beyond visible area - scroll to show it
                    BeginIndex = textSize - BoxLength;
                    BeginOffset = BoxLength;
                    print_and_display_cursor();
                }
                else if (BeginIndex + BoxLength == textSize) {
                    // End is at edge - move to it
                    if (BeginOffset < BoxLength) {
                        write_right_bbar(BoxLength - BeginOffset);
                        BeginOffset = textSize - BeginIndex;
                    }
                }
                return true;
            }

            case INSERTKEY: {
                // Toggle insert mode
                InsertOn = !InsertOn;
                if (BeginOffset < BoxLength) {
                    write_shape();
                }
                return true;
            }

            case DELETEKEY: {
                // Handle delete key - delete character at cursor
                if (EndIndex < Text.size()) {
                    // Shift remaining text left
                    for (size_t i = EndIndex; i < Text.size() - 1; i++) {
                        Text[i] = Text[i + 1];
                    }
                    Text.pop_back();
                    print_and_display_cursor();
                }
                return true;
            }

            default:
                break;
            }
            return false;
        }

        /**
         * @brief Format and prepare display buffer
         *
         * Fills the buffer with the visible portion of text (or masking character)
         *
         * @param DisplayColor Colors to use for display
         * @param DisplayCharacter If non-zero, use this character instead of actual text (for password fields)
         */
        void format_to(color DisplayColor, wchar_t DisplayCharacter) noexcept {
            size_t BoxLength = Area.num_cols();
            size_t RemLength = Text.size() - BeginIndex;

            // Prepare the buffer
            bf.clear();
            mz::SetHide(bf);
            DisplayColor.apply(bf);
            Area.Top.apply(bf);

            // Limit text to box width
            if (RemLength > BoxLength) {
                RemLength = BoxLength;
            }

            // Add text or masking characters
            if (DisplayCharacter) {
                bf.append(RemLength, DisplayCharacter);
            }
            else {
                bf.append(std::wstring_view(Text).substr(BeginIndex, RemLength));
            }

            // Fill the rest with spaces
            bf.append(BoxLength - RemLength, ' ');
        }

        /**
         * @brief Display buffer and position cursor
         */
        void print_and_display_cursor() noexcept {
            print(Color, wc);
            display_cursor();
        }

        /**
         * @brief Insert or overwrite a character
         *
         * Adds a new character at the cursor position, using either insert
         * or overwrite mode as appropriate.
         *
         * @param x Character to insert
         * @return true if the character was inserted, false otherwise
         */
        bool insert(int x) {
            // Validate state
            if (BeginIndex < 0 ||
                BeginOffset < 0 ||
                BeginOffset > Area.num_cols() ||
                BeginIndex + BeginOffset > static_cast<int>(Text.size()) ||
                static_cast<int>(Text.size()) > MaxLength) {
                throw std::logic_error("Invalid input control state");
            }

            // Process special keys first
            if (process_control_keys(x)) {
                return true;
            }

            // Get current state
            int OffsetIndex = BeginIndex + BeginOffset;
            int BoxLength = box_length();
            int Processed = 0;
            int textSize = static_cast<int>(Text.size());

            // Ignore non-displayable characters
            if (!IsDisplayCharacter(x)) {
                return false;
            }

            // Handle input based on text position and insert mode
            if (Text.size() < static_cast<size_t>(MaxLength)) {
                if (OffsetIndex == static_cast<int>(Text.size())) {
                    // Append to end of text
                    Text.push_back(static_cast<wchar_t>(x));

                    if (BeginOffset < BoxLength) {
                        // Cursor within box - display character and advance
                        if (wc) {
                            mz::Write(wc);
                        }
                        else {
                            mz::Write(Text.back());
                        }
                        ++BeginOffset;

                        if (BeginOffset == BoxLength) {
                            write_bbar_shape();
                        }
                        return true;
                    }
                    else {
                        // Cursor at box edge - scroll text
                        ++BeginIndex;
                        BeginOffset = BoxLength; // redundant
                        print_and_display_cursor();
                        return true;
                    }
                }
                else if (InsertOn) {
                    // Overwrite existing character
                    Text[OffsetIndex] = static_cast<wchar_t>(x);
                    Processed = 1;
                }
                else {
                    // Insert new character
                    Text.push_back(Text.back());
                    for (size_t i = Text.size() - 1; i > static_cast<size_t>(OffsetIndex); i--) {
                        Text[i] = Text[i - 1];
                    }
                    Text[OffsetIndex] = static_cast<wchar_t>(x);
                    Processed = 1;
                }

                // Advance cursor
                ++BeginOffset;
                if (BeginOffset >= BoxLength) {
                    Processed = 1;
                    BeginIndex += BeginOffset - BoxLength;
                    BeginOffset = BoxLength;
                }
            }
            else if (InsertOn && OffsetIndex < textSize) {
                // Text at maximum length but in overwrite mode
                Text[OffsetIndex] = static_cast<wchar_t>(x);

                if (BeginOffset < BoxLength) {
                    ++BeginOffset;
                }
                else if (BeginIndex + BeginOffset < MaxLength) {
                    ++BeginIndex;
                }

                print_and_display_cursor();
                return true;
            }

            // Update display if needed
            if (Processed > 0) {
                print_and_display_cursor();
            }

            return Processed >= 0;
        }

        /**
         * @brief Current insert/overwrite mode
         *
         * true = overwrite mode, false = insert mode
         */
        bool InsertOn{ false };

        /**
         * @brief Index of first visible character
         *
         * Used for horizontal scrolling when text is longer than the box.
         */
        int BeginIndex{ 0 };

        /**
         * @brief Cursor position within visible area
         *
         * Position relative to the start of the visible text.
         */
        int BeginOffset{ 0 };

        /**
         * @brief Maximum text length
         *
         * Limits the total number of characters that can be entered.
         */
        int MaxLength{ 0 };

        // Cursor shape helpers
        inline void write_bunder_shape() const noexcept { Write(L"\x1b[3 q"); }
        inline void write_bblock_shape() const noexcept { Write(L"\x1b[1 q"); }
        inline void write_bbar_shape() const noexcept { Write(L"\x1b[5 q"); }
        inline void write_right_bbar() const noexcept { Write(L"\x1b[C\x1b[5 q"); }
        inline void write_right_bbar(int Count) const noexcept { Write(std::format(L"\x1b[{}C\x1b[5 q", Count)); }
        inline void write_left() const noexcept { MoveLeft(); }

        /**
         * @brief Move cursor left and update shape
         */
        inline void write_left_and_shape() const noexcept {
            if (InsertOn) {
                Write(L"\x1b[D\x1b[1 q");
            }
            else {
                Write(L"\x1b[D\x1b[3 q");
            }
        }

        /**
         * @brief Set cursor shape based on insert mode
         */
        inline void write_shape() const noexcept {
            if (InsertOn) {
                write_bblock_shape();
            }
            else {
                write_bunder_shape();
            }
        }

        /**
         * @brief Calculate effective box length
         *
         * Returns the smaller of the physical box length and MaxLength.
         *
         * @return Effective box length
         */
        inline int box_length() const noexcept {
            int BoxLength = Area.num_cols();
            return BoxLength <= MaxLength ? BoxLength : MaxLength;
        }

    public:
        /**
         * @brief Text content
         *
         * Current text in the input field.
         */
        std::wstring Text{};

        /**
         * @brief Character to display instead of actual text
         *
         * When non-zero, this character is displayed instead of the actual text.
         * Used for password fields.
         */
        wchar_t wc{ 0 };

        /**
         * @brief Reset the input control
         *
         * Clears text and resets cursor position.
         */
        void clear() noexcept {
            Text.clear();
            BeginIndex = 0;
            BeginOffset = 0;
        }

        /**
         * @brief Set the size of the input field
         *
         * @param BoxLength Visual width of the input field
         * @param MaxTextLength Maximum number of characters allowed (default: 255)
         */
        void set_size(int BoxLength, int MaxTextLength = 255) noexcept {
            Area.set_size(1, BoxLength);
            MaxLength = MaxTextLength;
        }

        /**
         * @brief Move the input field to a new position
         *
         * @param Top New top-left position
         */
        void move_to(coord Top) noexcept {
            Area.move_top_to(Top);
        }

        /**
         * @brief Enter interactive input mode
         *
         * Allows the user to edit text until Enter or Escape is pressed.
         *
         * @param Final Color to use when input is complete
         * @param DisplayCharacter Character to display instead of text (0 for normal display)
         * @return true if input was confirmed with Enter, false if canceled with Escape
         */
        bool get(color Final, wchar_t DisplayCharacter = 0) {
            print_and_display_cursor();

            while (true) {
                int x = wgetch();
                switch (x) {
                case RETURNKEY:
                    print(Final, DisplayCharacter);
                    return true;
                case ESCAPEKEY:
                    clear();
                    Text.clear();
                    print(Final, DisplayCharacter);
                    return false;
                default:
                    insert(x);
                    break;
                }
            }
        }

        /**
         * @brief Enter interactive password input mode
         *
         * Similar to get() but with a toggle for showing/hiding characters
         * and status display in the footer.
         *
         * @param Footer Footer box for status messages
         * @param Final Color to use when input is complete
         * @param DisplayCharacter Initial masking character (0 for no masking)
         * @return true if input was confirmed with Enter, false if canceled with Escape
         */
        bool get_hidden(FooterBox& Footer, color Final, wchar_t DisplayCharacter = '*') {
            // Set initial masking state
            wc = DisplayCharacter;

            // Update footer with hint
            if (wc) {
                Footer.update_status(L"SPACE: Show Characters");
            }
            else {
                Footer.update_status(L"SPACE: Hide Characters");
            }
            Footer.print();

            print_and_display_cursor();

            while (true) {
                int x = wgetch();

                switch (x) {
                case RETURNKEY:
                    print(Final, DisplayCharacter);
                    return true;
                case ESCAPEKEY:
                    Text.clear();
                    print(Final, DisplayCharacter);
                    return false;
                case SPACEKEY:
                    // Toggle between showing and hiding characters
                    if (wc) {
                        wc = 0;
                        Footer.update_status(L"SPACE: Hide Characters");
                    }
                    else {
                        wc = '*';
                        Footer.update_status(L"SPACE: Show Characters");
                    }
                    Footer.print();
                    print_and_display_cursor();
                    break;
                default:
                    // Process regular input
                    if (!insert(x)) {
                        Footer.blink();
                        print_and_display_cursor();
                    }
                    break;
                }
            }
        }

        /**
         * @brief Display the input field
         *
         * Renders the input field without entering interactive mode.
         */
        void print() const noexcept {
            BasicBox::print();
        }

        /**
         * @brief Display the input field with specific formatting
         *
         * @param DisplayColor Color to use
         * @param DisplayCharacter Character to display instead of text (0 for normal display)
         */
        void print(color DisplayColor, wchar_t DisplayCharacter = 0) {
            format_to(DisplayColor, DisplayCharacter);
            mz::Write(bf);
        }

        /**
         * @brief Show the cursor at the current position
         *
         * Positions the terminal cursor at the current input position
         * and sets the appropriate cursor shape.
         */
        void display_cursor() noexcept {
            int BoxLength = box_length();
            auto Loc = Area.Top.offset(0, BeginOffset);

            mz::SetPos(Loc.Row + 1, Loc.Col + 1);  // +1 for 1-based terminal coordinates

            if (BeginOffset >= BoxLength) {
                write_bbar_shape();
            }
            else if (InsertOn) {
                write_bblock_shape();
            }
            else {
                write_bunder_shape();
            }

            mz::SetShow();
        }

        /**
         * @brief Default constructor
         *
         * Initializes with standard input colors and control key mapping.
         */
        InputControl() noexcept {
            // Set up control key handling
            Controls.Delete = 1;
            Controls.Insert = 1;
            Controls.Escape = 1;
            Controls.Return = 1;
            Controls.HomeEnd = 1;
            Controls.LeftRight = 1;

            // Set default colors
            Color = InputColors;

            // Prepare display
            format_to(Color, 0);
        }

        /**
         * @brief Constructor with specified colors
         *
         * @param InputColor Color scheme to use
         */
        explicit InputControl(color InputColor) noexcept : InputControl() {
            Color = InputColor;
            format_to(Color, 0);
        }

        /**
         * @brief Get the current text value
         *
         * @return Current text content
         */
        std::wstring_view value() const noexcept {
            return Text;
        }

        /**
         * @brief Set the text content
         *
         * @param text New text content
         */
        void set_text(const std::wstring& text) noexcept {
            Text = text;
            if (static_cast<int>(Text.size()) > MaxLength) {
                Text.resize(MaxLength);
            }
            BeginIndex = 0;
            BeginOffset = 0;
        }

        /**
         * @brief Set the input mode
         *
         * @param insert_mode true for overwrite mode, false for insert mode
         */
        void set_insert_mode(bool insert_mode) noexcept {
            InsertOn = insert_mode;
        }

        /**
         * @brief Get the current insert mode
         *
         * @return true if in overwrite mode, false if in insert mode
         */
        bool get_insert_mode() const noexcept {
            return InsertOn;
        }
    };

} // namespace mz

#endif // MZ_INPUT_CONTROL_H

