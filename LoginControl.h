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

#ifndef MZ_LOGIN_CONTROL_H
#define MZ_LOGIN_CONTROL_H
#pragma once

/**
 * @file LoginControl.h
 * @brief Username and password input dialog component
 *
 * This file provides the LoginControl class which implements a framed dialog
 * for collecting username and password credentials with proper styling,
 * masking for passwords, and validation feedback.
 *
 * @author Meysam Zare
 */

#include "FrameBox.h"
#include "InputControl.h"
#include <string>
#include <string_view>

namespace mz {

    /**
     * @class LoginControl
     * @brief Username and password input dialog
     *
     * This class provides a styled login dialog with username and password
     * fields, support for password masking, and validation feedback.
     * It's designed to provide a consistent and secure credential entry UI
     * for terminal applications.
     */
    class LoginControl : public FrameBox {
    public:
        /**
         * @brief Username input field
         *
         * Field for entering the username with optional character masking.
         */
        InputControl Name;

        /**
         * @brief Password input field
         *
         * Field for entering the password with character masking.
         */
        InputControl Pass;

        /**
         * @brief Message display area
         *
         * Area for showing validation messages and instructions.
         */
        MultilineMessageBox MsgBox;

        /**
         * @brief Initialize the login dialog
         *
         * Creates the login dialog UI with title, input fields, and message area.
         *
         * @param Title Dialog title text
         */
        void create(std::wstring_view Title) noexcept {
            // Create the frame with specified title
            FrameBox::create(Title, Area);

            // Position and label the username field
            coord Loc{ Area.Top.offset(2, 3) };
            Loc.apply(bf);
            Loc.append(bf, L"username: ");
            Name.move_to(Loc);

            // Position and label the password field
            Loc = Area.Top.offset(4, 3);
            Loc.apply(bf);
            Loc.append(bf, L"password: ");
            Pass.move_to(Loc);

            // Save buffer position for content
            PreMessageSize = static_cast<int>(bf.size());

            // Configure message box area
            MsgBox.Area = Area.pad_rows(6, 1).pad_cols(2, 2);
            MsgBox.clear();

            // Initialize footer with instructions
            Footer.create(Area.bottom_rows(1));
            Footer.update_status(L"SPACE: Hide Characters");
        }

        /**
         * @brief Default constructor
         *
         * Initializes with default colors, sizes, and settings.
         */
        LoginControl() noexcept {
            // Set color scheme
            Color = FrameColors1;
            MsgBox.Color = Color;

            // Configure footer colors
            Footer.set_front(Color.F);
            Footer.set_back(Color.B.mix(Color.F, 30));

            // Set default size and position
            Area.Top = { 1, 1 };
            Area.set_size({ 11, 40 });

            // Configure input fields
            Name.set_size(16, 16);
            Pass.set_size(16, 16);

            // Set masking characters (none for username, * for password)
            Name.wc = 0;
            Pass.wc = '*';
        }

        /**
         * @brief Constructor with custom position and size
         *
         * @param position Top-left position of the dialog
         * @param size Size of the dialog (rows, columns)
         */
        LoginControl(coord position, coord size) noexcept : LoginControl() {
            Area.Top = position;
            Area.set_size(size);
        }

        /**
         * @brief Interactive login dialog
         *
         * Displays the dialog and collects username and password input.
         *
         * @param HiddenCharacter Character to use for username masking (0 for no masking)
         * @return true if login was entered, false if canceled
         */
        bool get(wchar_t HiddenCharacter = 0) {
            // Display the dialog components
            mz::Write(bf);
            Footer.print();
            Name.print(HiddenCharacter);
            Pass.print(Pass.wc);

            // Get username (with optional masking)
            if (!Name.get_hidden(Footer, Color, HiddenCharacter)) {
                // User canceled username entry
                Pass.print(Color, '*');
                return false;
            }

            // Get password (with masking)
            if (!Pass.get_hidden(Footer, Color, '*')) {
                // User canceled password entry
                Name.Text.clear();
                Name.print(Color, '*');
                return false;
            }

            // Both fields were completed
            return true;
        }

        /**
         * @brief Display login failure message and retry option
         *
         * Shows an error message and gives the option to retry or cancel.
         *
         * @param HiddenCharacter Character to use for username masking on retry
         * @return 1 if login was re-entered, 0 if canceled
         */
        int retry_message(wchar_t HiddenCharacter = 0) noexcept {
            // Clear input fields
            Name.clear();
            Name.print();
            Pass.clear();
            Pass.print();

            // Show error message
            MsgBox.clear();
            MsgBox.insert_line(L"Incorrect username and/or password.", mz::color::RED);
            MsgBox.insert_line(L"Press ESC to cancel logging in.", mz::color::RED);
            MsgBox.insert_line(L"Or any other key to retry.", mz::color::RED);
            MsgBox.print();

            // Wait for user decision
            int wc = wgetch();

            // Clear message area
            MsgBox.clear();
            MsgBox.print();

            // Handle user choice
            if (wc == ESCAPEKEY) {
                return 0;  // User canceled
            }
            else {
                return get(HiddenCharacter) ? 1 : 0;  // Try again
            }
        }

        /**
         * @brief Run a test of the login dialog
         *
         * Creates and displays a sample login dialog in the specified area.
         *
         * @param Parent Screen area in which to center the dialog
         */
        static void Test(coord_box Parent) noexcept {
            // Create and position login dialog
            LoginControl cbox;
            cbox.Area = Parent.place_center(cbox.Area.get_size());

            // Initialize and run login test
            cbox.create(L"Login");
            cbox.get('*');
            cbox.retry_message('*');
        }

        /**
         * @brief Get entered username
         *
         * @return Current username text
         */
        std::wstring_view get_username() const noexcept {
            return Name.Text;
        }

        /**
         * @brief Get entered password
         *
         * @return Current password text
         */
        std::wstring_view get_password() const noexcept {
            return Pass.Text;
        }

        /**
         * @brief Set initial username
         *
         * Pre-fills the username field with the specified text.
         *
         * @param username Username to display
         */
        void set_username(const std::wstring& username) noexcept {
            Name.Text = username;
        }

        /**
         * @brief Set custom display message
         *
         * Updates the message area with custom text.
         *
         * @param message Text to display
         * @param messageColor Color for the message (default: RED)
         */
        void set_message(std::wstring_view message, rgb messageColor = color::RED) noexcept {
            MsgBox.clear();
            MsgBox.insert_line(message, messageColor);
            MsgBox.print();
        }

        /**
         * @brief Set username masking mode
         *
         * Controls whether username input is masked with a character.
         *
         * @param maskChar Character to use (0 for no masking)
         */
        void set_username_masking(wchar_t maskChar) noexcept {
            Name.wc = maskChar;
        }

        /**
         * @brief Set password masking character
         *
         * Changes the character used to mask password input.
         *
         * @param maskChar Character to use (default: '*')
         */
        void set_password_masking(wchar_t maskChar = '*') noexcept {
            Pass.wc = maskChar;
        }

        /**
         * @brief Clear entered credentials
         *
         * Resets both username and password fields to empty.
         */
        void clear_credentials() noexcept {
            Name.clear();
            Pass.clear();
            Name.print();
            Pass.print();
        }
    };

} // namespace mz

#endif // MZ_LOGIN_CONTROL_H

