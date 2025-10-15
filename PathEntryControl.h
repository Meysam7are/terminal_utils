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

#ifndef MZ_PATH_ENTRY_CONTROL_H
#define MZ_PATH_ENTRY_CONTROL_H
#pragma once

/**
 * @file PathEntryControl.h
 * @brief File or directory path input dialog component
 *
 * This file provides the PathEntryControl class for terminal-based UI,
 * which creates a dialog for entering and validating file or directory paths.
 * It integrates input field, validation, and feedback messages in a styled frame.
 *
 * @author Meysam Zare
 */

#include "FrameBox.h"
#include "FileInfo.h"
#include "InputControl.h"
#include <string_view>

namespace mz {

    /**
     * @class PathEntryControl
     * @brief Dialog for file or directory path input and validation
     *
     * This class provides a complete UI component for entering and validating
     * file or directory paths. It includes an input field, validation logic,
     * and feedback messages in a styled frame.
     */
    class PathEntryControl : public mz::FrameBox {
    public:
        /**
         * @brief File information about the entered path
         *
         * Stores metadata and status about the path entered by the user.
         * Available after validation for accessing path properties.
         */
        mz::FileInfo PathInfo;

        /**
         * @brief Input field for path entry
         *
         * The text input component where users enter the file or directory path.
         */
        mz::InputControl Name;

        /**
         * @brief Message display area
         *
         * Displays validation feedback and instructions to the user.
         */
        mz::MultilineMessageBox MsgBox;

        /**
         * @brief Path type selector
         *
         * Determines whether the control accepts files or directories.
         * true = file selection, false = directory selection
         */
        bool ChooseFile;

        /**
         * @brief Constructor with path type selection
         *
         * Initializes the control with default styling and dimensions.
         *
         * @param ChooseFile true for file selection, false for directory selection
         */
        explicit PathEntryControl(bool ChooseFile) noexcept : ChooseFile{ ChooseFile } {
            // Set color scheme - aqua on dark background
            Color.F = mz::color::AQUA.brighten(80);
            Color.B = mz::color::AQUA.darken(90);
            MsgBox.Color = Color;

            // Configure footer with slightly blended colors
            Footer.Color = Color.blend(20);

            // Configure the input field size
            Name.set_size(40, 255);

            // Set overall dialog dimensions
            Area.set_size(mz::coord{ 8, 50 });
        }

        /**
         * @brief Initialize the dialog with title and position
         *
         * Creates the dialog UI components with the specified title and
         * centers it within the parent area.
         *
         * @param Title Text to display in the dialog title bar
         * @param Parent Screen area in which to center the dialog
         */
        void create(std::wstring_view Title, mz::coord_box Parent) {
            // Center the dialog in the parent area
            Area = Parent.place_center(Area.get_size());

            // Create the basic frame
            FrameBox::create(Title, Area);

            // Position and create the path input field
            coord Loc{ Area.Top.offset(2, 2) };
            Loc.apply(bf);
            Loc.append(bf, L"path: ");
            Name.move_to(Loc);
            PreMessageSize = static_cast<int>(bf.size());

            // Configure the message box area
            MsgBox.Area = Area.top_left_child(mz::coord{ 2, 45 }).shift(mz::coord{ 4, 2 });
            MsgBox.clear();

            // Set initial footer message
            Footer.update_status(L"Press ESC to cancel");
        }

        /**
         * @brief Validate path and display appropriate messages
         *
         * Checks if the entered path exists and is of the correct type
         * (file or directory). Displays validation feedback messages.
         *
         * @return Error code: 0=valid, 1=invalid path, 2=wrong type, 3=not file/dir
         */
        int display_message() noexcept {
            int ErrorCode = 0;

            // Create a FileInfo object from the entered path
            PathInfo = mz::FileInfo{ Name.Text };
            MsgBox.clear();

            // Check if path exists
            if (PathInfo.m_size < 0 || !PathInfo.exists()) {
                MsgBox.insert_line(L"Invalid path.", mz::color::RED);
                ErrorCode = 1;
            }
            // Path is a file but we wanted a directory
            else if (PathInfo.isFile()) {
                if (!ChooseFile) {
                    MsgBox.insert_line(L"The specified path is a file.", mz::color::RED);
                    ErrorCode = 2;
                }
            }
            // Path is a directory but we wanted a file
            else if (PathInfo.isDirectory()) {
                if (ChooseFile) {
                    MsgBox.insert_line(L"The specified path is a directory.", mz::color::RED);
                    ErrorCode = 2;
                }
            }
            // Path is neither a file nor a directory
            else {
                if (ChooseFile) {
                    MsgBox.insert_line(L"The specified path is not a file.", mz::color::RED);
                    ErrorCode = 3;
                }
                else {
                    MsgBox.insert_line(L"The specified path is not a directory.", mz::color::RED);
                    ErrorCode = 3;
                }
            }

            // Show additional message if there's an error
            if (ErrorCode) {
                MsgBox.insert_line(L"Press ESC to cancel or any key to continue.", mz::color::RED);
            }
            else {
                Name.print(Color, 0);
            }

            // Display messages and prepare for next input
            MsgBox.print();
            MsgBox.clear();

            return ErrorCode;
        }

        /**
         * @brief Enter interactive path entry mode
         *
         * Displays the dialog and allows the user to enter and validate a path.
         * The path is validated against the selected type (file/directory).
         *
         * @return 0=success (valid path selected), 100=canceled
         */
        int get() noexcept {
            // Initial display
            mz::Write(bf);
            Footer.print();
            Name.print();

            while (true) {
                // Get path input
                if (Name.get(Color, 0)) {
                    // Validate input
                    int ErrorCode = display_message();

                    if (ErrorCode) {
                        // Show error and wait for user decision
                        Name.print(Color, 0);
                        int wc = mz::wgetch();
                        MsgBox.clear();
                        MsgBox.print();

                        if (wc == mz::ESCAPEKEY) {
                            return 100;  // User canceled after error
                        }
                        else {
                            continue;  // User wants to try again
                        }
                    }

                    // Path is valid, ask for confirmation
                    MsgBox.clear();
                    MsgBox.insert_line(L"Press Enter to continue...", mz::color::YELLOW);
                    MsgBox.insert_line(L"or Press Escape to cancel.", mz::color::YELLOW);
                    MsgBox.print();
                    Footer.update_status(L"ESC:Cancel | ENTER:Accept");
                    Footer.print();

                    // Wait for final decision
                    while (true) {
                        int wc = mz::wgetch();
                        switch (wc) {
                        case mz::RETURNKEY:
                            return 0;  // User confirmed
                        case mz::ESCAPEKEY:
                            PathInfo = mz::FileInfo{};  // Reset path info
                            return 100;  // User canceled at confirmation
                        }
                    }
                }
                else {
                    // User canceled during initial input
                    PathInfo = mz::FileInfo{};
                    Name.Text.clear();
                    MsgBox.clear();
                    MsgBox.print();
                    Name.print(Color, 0);
                    return 100;
                }
            }
        }

        /**
         * @brief Set initial path text
         *
         * Pre-fills the path input field with the specified text.
         *
         * @param initialPath Path to display initially
         */
        void set_initial_path(const std::wstring& initialPath) noexcept {
            Name.set_text(initialPath);
        }

        /**
         * @brief Get the entered path as string
         *
         * @return Current path text from the input field
         */
        std::wstring_view get_path() const noexcept {
            return Name.Text;
        }

        /**
         * @brief Change path selection mode
         *
         * @param fileMode true to select files, false to select directories
         */
        void set_file_mode(bool fileMode) noexcept {
            ChooseFile = fileMode;
        }
    };

} // namespace mz

#endif // MZ_PATH_ENTRY_CONTROL_H

