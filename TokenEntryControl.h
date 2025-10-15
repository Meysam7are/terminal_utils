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

#ifndef MZ_TOKEN_ENTRY_CONTROL_H
#define MZ_TOKEN_ENTRY_CONTROL_H
#pragma once

/**
 * @file TokenEntryControl.h
 * @brief Token input dialog with validation
 *
 * This file provides the TokenEntryControl class which creates a dialog
 * for entering and validating tokens such as usernames, numeric values,
 * or file paths. It supports various validation rules based on the token type.
 *
 * @author Meysam Zare
 */

#include <filesystem>
#include <string>
#include <string_view>
#include <format>
#include <climits>
#include "FrameBox.h"
#include "InputControl.h"

namespace mz {

    /**
     * @class TokenEntryControl
     * @brief Token input dialog with validation
     *
     * This class provides a framed dialog for entering and validating tokens
     * like usernames, numbers, or file paths. It displays appropriate validation
     * messages based on the selected token type and validation rules.
     */
    class TokenEntryControl : public FrameBox {
    private:
        /**
         * @brief Display validation errors
         *
         * Shows validation messages with appropriate highlighting based on
         * error flags. Different token types have different validation rules.
         *
         * @param Error Bit flags for errors (0=no error)
         * @return true if any errors were found, false if input is valid
         */
        bool display_errors(int Error) noexcept {
            rgb FrontColor = color::GREEN;

            // Reset buffer to pre-message state
            bf.resize(PreMessageSize);
            Color.apply(bf);

            // Clear message box for new messages
            MsgBox.clear();

            switch (Type) {
            case TokenType::user:
            {
                // Add username validation flags
                Error |= username_error(Token.Text, Min, Max);

                // Display validation rules with red highlighting for failed rules
                MsgBox.insert_line(std::format(L"must be {}-{} characters.", Min, Max),
                    Error & 1 ? mz::color::RED : FrontColor);
                MsgBox.insert_line(L"must contain at least 1 number.",
                    Error & 2 ? mz::color::RED : FrontColor);
                MsgBox.insert_line(L"must contain at least 1 lower case.",
                    Error & 4 ? mz::color::RED : FrontColor);
                MsgBox.insert_line(L"must contain at least 1 upper case.",
                    Error & 8 ? mz::color::RED : FrontColor);
                MsgBox.insert_line(L"no space or special character allowed.",
                    Error & 16 ? mz::color::RED : FrontColor);
            } break;

            case TokenType::signed_integer:
            case TokenType::unsigned_integer:
            {
                // Parse and validate the number
                long long Res = string_to_signed(Token.Text);

                if (Res == LLONG_MIN) {
                    Error |= 1;  // Parse error
                }
                else {
                    Error |= (!!(Res < Min)) << 1;  // Too small
                    Error |= (!!(Res > Max)) << 2;  // Too large
                }

                // Display validation rules with red highlighting for failed rules
                MsgBox.insert_line(L"must be an integer.",
                    Error & 1 ? mz::color::RED : FrontColor);
                MsgBox.insert_line(std::format(L"must be >= {}", Min),
                    Error & 2 ? mz::color::RED : FrontColor);
                MsgBox.insert_line(std::format(L"must be <= {}", Max),
                    Error & 4 ? mz::color::RED : FrontColor);
            } break;

            case TokenType::bounded_length:
            {
                // Add length and character validation flags
                Error |= username_error(Token.Text, Min, Max);
                Error &= ~14;  // Clear unused flags

                // Display validation rules with red highlighting for failed rules
                MsgBox.insert_line(std::format(L"must be {}-{} characters.", Min, Max),
                    Error & 1 ? mz::color::RED : FrontColor);
                MsgBox.insert_line(L"no space or special character allowed.",
                    Error & 16 ? mz::color::RED : FrontColor);
            } break;

            default: break;
            }

            // Display messages
            MsgBox.print();

            return Error != 0;
        }

    public:
        /**
         * @enum TokenType
         * @brief Types of tokens with different validation rules
         */
        enum class TokenType {
            none = 0,             ///< No validation
            user = 1,             ///< Username validation (letters, numbers, length)
            file = 2,             ///< File path validation
            signed_integer = 3,   ///< Signed integer in range Min-Max
            unsigned_integer = 4, ///< Unsigned integer in range Min-Max
            floating_point = 5,   ///< Floating point number
            bounded_length = 6,   ///< Text with length bounds
            directory = 7,        ///< Directory path validation
        };

        /**
         * @brief Token type for validation
         */
        TokenType Type{ TokenType::user };

        /**
         * @brief Minimum value/length
         *
         * For numeric types: minimum allowed value
         * For text types: minimum allowed length
         */
        long long Min{ 4 };

        /**
         * @brief Maximum value/length
         *
         * For numeric types: maximum allowed value
         * For text types: maximum allowed length
         */
        long long Max{ 12 };

        /**
         * @brief Input control for token entry
         */
        InputControl Token;

        /**
         * @brief Message box for validation feedback
         */
        MultilineMessageBox MsgBox;

        /**
         * @brief Initialize the token entry dialog
         *
         * Creates the dialog with the specified title and label for the input field.
         *
         * @param Title Dialog title
         * @param Label Label for the input field
         */
        void create(std::wstring_view Title, std::wstring_view Label) noexcept {
            // Create the base frame
            FrameBox::create(Title, Area);

            // Clear the token input
            Token.clear();

            // Position and label the token input field
            coord Loc{ Area.Top.offset(2, 2) };
            Loc.apply(bf);
            Loc.append(bf, Label);
            Token.move_to(Loc);

            // Store buffer position for message display
            PreMessageSize = static_cast<int>(bf.size());

            // Configure the message box area with padding
            MsgBox.Area = Area.pad_rows(4, 2).pad_cols(3, 1);
            MsgBox.clear();

            // Initialize the footer
            Footer.create(Area.bottom_rows(1));
            Footer.update_status(L"");
        }

        /**
         * @brief Default constructor
         *
         * Initializes the control with default colors and dimensions.
         */
        TokenEntryControl() noexcept {
            // Set color scheme - dark blue background with bright aqua text
            Color.B = color::BLUE.darken(90);
            Color.F = color::AQUA.brighten(85);

            // Set same colors for the message box
            MsgBox.Color = Color;

            // Set a slightly lighter color for the footer
            Footer.Color = Color.blend(20);

            // Set initial dimensions
            Area.Top = { 1, 1 };
            Area.set_size({ 11, 44 });

            // Configure token input field size
            Token.set_size(16, 16);
        }

        /**
         * @brief Enter interactive token entry mode
         *
         * Displays the dialog and allows the user to enter a token.
         * Validates the input according to the selected token type.
         *
         * @return 0=canceled, 1=valid token accepted
         */
        int get() noexcept {
            // Show initial validation rules
            display_errors(-1);

            // Initial display
            mz::Write(bf);
            Footer.print();
            MsgBox.print();
            Token.print();

            while (true) {
                // Get token input
                if (Token.get(Color, 0)) {
                    // Validate input
                    if (!display_errors(0)) {
                        // Valid input
                        return 1;
                    }
                }
                else {
                    // User canceled
                    Token.Text.clear();
                    Token.print(Color, 0);
                    mz::Write(bf);
                    return 0;
                }
            }
        }

        /**
         * @brief Enter interactive token entry mode with password masking
         *
         * Similar to get() but with character masking for sensitive input.
         *
         * @param HiddenCharacter Character to display instead of actual text (0 for none)
         * @return 0=canceled, 1=valid token accepted
         */
        int get_hidden(wchar_t HiddenCharacter = 0) noexcept {
            // Show initial validation rules
            display_errors(-1);

            // Initial display
            mz::Write(bf);
            Footer.print();
            MsgBox.print();

            // Set masking character
            Token.wc = HiddenCharacter;
            Token.print();

            while (true) {
                // Get masked token input
                if (Token.get_hidden(Footer, Color, 0)) {
                    // Validate input
                    if (!display_errors(0)) {
                        // Valid input
                        Token.print(Color, 0);
                        return 1;
                    }
                }
                else {
                    // User canceled
                    Token.Text.clear();
                    Token.print(Color, 0);
                    mz::Write(bf);
                    return 0;
                }
            }
        }

        /**
         * @brief Run test dialog
         *
         * Creates and displays a test token entry dialog in the specified area.
         *
         * @param Parent Screen area for the dialog
         */
        static void Test(coord_box Parent) noexcept {
            TokenEntryControl tec;
            tec.Area = Parent.place_center(tec.Area.get_size());
            tec.create(L"Select Username", L"username: ");
            tec.get();
        }

        /**
         * @brief Set token validation type
         *
         * Changes the validation type and optionally updates Min/Max bounds.
         *
         * @param newType Token type for validation
         * @param minValue Minimum value/length (optional)
         * @param maxValue Maximum value/length (optional)
         */
        void set_token_type(TokenType newType, long long minValue = 0, long long maxValue = 0) noexcept {
            Type = newType;

            // Update bounds if provided
            if (minValue != 0 || maxValue != 0) {
                Min = minValue;
                Max = maxValue;
            }
            else {
                // Set sensible defaults based on type
                switch (newType) {
                case TokenType::user:
                    Min = 4;
                    Max = 12;
                    break;
                case TokenType::bounded_length:
                    Min = 1;
                    Max = 32;
                    break;
                case TokenType::signed_integer:
                    Min = LLONG_MIN;
                    Max = LLONG_MAX;
                    break;
                case TokenType::unsigned_integer:
                    Min = 0;
                    Max = LLONG_MAX;
                    break;
                default:
                    Min = 0;
                    Max = 255;
                    break;
                }
            }
        }

        /**
         * @brief Get the entered token as string
         *
         * @return Current token text from the input field
         */
        std::wstring_view get_token() const noexcept {
            return Token.Text;
        }

        /**
         * @brief Get the entered token as a signed integer
         *
         * @param defaultValue Value to return if conversion fails
         * @return Parsed integer or defaultValue if parsing fails
         */
        long long get_token_as_integer(long long defaultValue = 0) const noexcept {
            try {
                return std::stoll(Token.Text);
            }
            catch (...) {
                return defaultValue;
            }
        }

        /**
         * @brief Set initial token text
         *
         * Pre-fills the token input field with the specified text.
         *
         * @param initialToken Token to display initially
         */
        void set_initial_token(const std::wstring& initialToken) noexcept {
            Token.Text = initialToken;
        }

        /**
         * @brief Set validation message colors
         *
         * Changes colors used for validation messages.
         *
         * @param validColor Color for satisfied rules
         * @param errorColor Color for violated rules
         */
        void set_validation_colors(rgb validColor, rgb errorColor) noexcept {
            MsgBox.Color.F = validColor;
            // Error color is passed directly to insert_line
        }
    };

} // namespace mz

#endif // MZ_TOKEN_ENTRY_CONTROL_H
