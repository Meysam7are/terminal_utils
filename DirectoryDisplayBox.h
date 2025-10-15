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

#ifndef MZ_DIRECTORY_DISPLAY_BOX_H
#define MZ_DIRECTORY_DISPLAY_BOX_H
#pragma once

/**
 * @file DirectoryDisplayBox.h
 * @brief Scrollable directory/file listing component for terminal UIs
 *
 * This file provides the DirectoryDisplayBox class which implements a scrollable
 * list view with two-column display for filenames or other data. It supports
 * navigation, selection, and scrolling with both vertical and horizontal scrollbars.
 *
 * @author Meysam Zare
 */

#include "ConsoleCMD.h"
#include "coord.h"
#include "cursor.h"
#include "WindowBox.h"
#include <vector>
#include <string>
#include <string_view>
#include <stdexcept>

namespace mz {

    /**
     * @class DirectoryDisplayBox
     * @brief Scrollable list display component with selection capabilities
     *
     * This class provides a terminal UI component for displaying and navigating
     * through lists of items (typically files/directories). It handles scrolling,
     * selection, and display formatting with support for long item names.
     */
    class DirectoryDisplayBox : public BasicBox {
    public:
        /**
         * @struct name_column
         * @brief Represents an item in the directory listing
         *
         * Stores the display and selection state for a single item in the list,
         * including text offsets and current scroll position.
         */
        struct name_column {
            /**
             * @brief First visible character index
             *
             * Tracks horizontal scrolling position for wide content.
             */
            int FirstIndex{ 0 };

            /**
             * @brief Start offset in the name container
             */
            int BeginOffset{ 0 };

            /**
             * @brief End offset in the name container
             */
            int EndOffset{ 0 };

            /**
             * @brief Selection state of this item
             */
            bool Selected{ false };

            /**
             * @brief Get item text length
             * @return Number of characters in this item
             */
            constexpr int size() const noexcept { return EndOffset - BeginOffset; }

            /**
             * @brief Check if item is empty
             * @return true if item has no content
             */
            constexpr bool empty() const noexcept { return BeginOffset == EndOffset; }
        };

        /**
         * @brief Container for all item texts
         *
         * Single string containing all item texts concatenated together.
         * Individual items are referenced via begin/end offsets in name_column.
         */
        std::wstring NameContainer;

        /**
         * @brief Collection of all items in the list
         *
         * Vector of name_column structures, each representing one item.
         */
        std::vector<name_column> NameColumns;

        /**
         * @brief Width of the left column in characters
         */
        int LeftColumnSize{ 14 };

        /**
         * @brief Width of the right column in characters
         *
         * Automatically calculated based on display area width.
         */
        int RightColumnSize{ 0 };

        /**
         * @brief Color scheme for focused (highlighted) item
         */
        color FocusColor;

        /**
         * @brief Color scheme for selected items
         */
        color SelectColor;

        /**
         * @brief Color scheme for items that are both selected and focused
         */
        color SelectFocusColor;

        /**
         * @brief Vertical scrollbar for navigating through items
         */
        VerticalScrollBar vScroll;

        /**
         * @brief Horizontal scrollbar for navigating wide item text
         */
        HorizontalScrollBar hScroll;

        /**
         * @brief Index of the first visible item
         */
        int TopIndex{ 0 };

        /**
         * @brief Index of the currently focused item
         */
        int FocusIndex{ 0 };

        /**
         * @brief Position offset for first column
         */
        int offsetColumn1{ 0 };

        /**
         * @brief Position offset for left column indicator character
         */
        int offsetLeftSign{ 0 };

        /**
         * @brief Position offset for second column
         */
        int offsetColumn2{ 0 };

        /**
         * @brief Position offset for right column indicator character
         */
        int offsetRightSign{ 0 };

        /**
         * @brief Length of a single text line including formatting
         */
        int TextLineLength{ 0 };

        /**
         * @brief Total number of items in the list
         */
        int NumIndexes{ 0 };

        /**
         * @brief ANSI sequence for normal text
         */
        std::wstring CommInit;

        /**
         * @brief ANSI sequence for focused text
         */
        std::wstring CommFocus;

        /**
         * @brief ANSI sequence for selected text
         */
        std::wstring CommSelect;

        /**
         * @brief ANSI sequence for text that is both selected and focused
         */
        std::wstring CommBoth;

        /**
         * @brief ANSI sequence for returning to the start of a new line
         */
        std::wstring CommReturn;

        /**
         * @brief Temporary buffer for rendering operations
         */
        std::wstring TempBuffer;

        /**
         * @brief Default constructor
         */
        DirectoryDisplayBox() noexcept {}

        /**
         * @brief Constructor with position and size
         *
         * @param displayArea Screen area for this component
         */
        explicit DirectoryDisplayBox(coord_box displayArea) noexcept {
            Area = displayArea;
        }

        /**
         * @brief Initialize component state
         *
         * Sets up colors, buffers, and scrollbars for the component.
         * Must be called after setting Area and before create().
         */
        void initialize() {
            // Calculate right column width based on display area
            RightColumnSize = Area.num_cols() - LeftColumnSize - 3;

            // Set up color schemes
            Color = ListColors;
            FocusColor = ListFocusColors;
            SelectColor = ListSelectColors;
            SelectFocusColor = ListSelectFocusColors;

            // Prepare display command sequences
            CommInit.clear();
            CommFocus.clear();
            CommSelect.clear();
            CommBoth.clear();

            // Apply colors to command sequences
            Color.apply(CommInit);
            FocusColor.apply(CommFocus);
            SelectColor.apply(CommSelect);
            SelectFocusColor.apply(CommBoth);

            // Ensure all command sequences have same length for easy replacement
            size_t ColorLength = CommInit.size();
            ColorLength = max(ColorLength, CommBoth.size());
            ColorLength = max(ColorLength, CommFocus.size());
            ColorLength = max(ColorLength, CommSelect.size());

            // Pad command sequences to consistent length
            if (CommInit.size() < ColorLength) { CommInit.append(ColorLength - CommInit.size(), 0); }
            if (CommBoth.size() < ColorLength) { CommBoth.append(ColorLength - CommBoth.size(), 0); }
            if (CommFocus.size() < ColorLength) { CommFocus.append(ColorLength - CommFocus.size(), 0); }
            if (CommSelect.size() < ColorLength) { CommSelect.append(ColorLength - CommSelect.size(), 0); }

            // Prepare temporary position buffer
            TempBuffer.clear();
            SetPos(TempBuffer, 1, 1);

            // Prepare line return sequence
            CommReturn.clear();
            MoveLeft(CommReturn, LeftColumnSize + RightColumnSize + 2);
            MoveDown(CommReturn);

            // Ensure move sequences have consistent length
            size_t MoveLength = max(TempBuffer.size(), CommReturn.size());
            if (TempBuffer.size() < MoveLength) { TempBuffer.append(MoveLength - TempBuffer.size(), 0); }
            if (CommReturn.size() < MoveLength) { CommReturn.append(MoveLength - CommReturn.size(), 0); }

            // Set up horizontal scrollbar
            hScroll.TopLeft = Area.bottom_left();
            hScroll.BackRGB = Color.B;
            hScroll.ScrollColors = Color.blend(20);
            hScroll.BarLength = RightColumnSize - 1;
            hScroll.PreLength = LeftColumnSize + 1;
            hScroll.PostLength = 1;

            // Set up vertical scrollbar
            vScroll.PostLength = 0;
            vScroll.PreLength = 0;
            vScroll.BackRGB = Color.B;
            vScroll.ScrollColors = Color.blend(20);
            vScroll.BarLength = Area.num_rows() - 1;
            vScroll.TopLeft = Area.top_right();

            // Clear data containers
            NameContainer.clear();
            bf.clear();

            // Create template for item display line
            bf.append(CommInit);
            bf.append(LeftColumnSize, ' ');
            offsetLeftSign = static_cast<int>(bf.size());
            PushBack(bf, LLQUOTE);
            offsetColumn2 = static_cast<int>(bf.size());
            bf.append(RightColumnSize, ' ');
            offsetRightSign = static_cast<int>(bf.size());
            PushBack(bf, RRQUOTE);
            bf.append(CommReturn);
            TextLineLength = static_cast<int>(bf.size());
            bf.clear();
        }

        /**
         * @brief Create the display with empty items
         *
         * Initializes the display with appropriate number of empty rows.
         * Call after initialize() to prepare the component for use.
         */
        void create() noexcept {
            // Reset view position
            TopIndex = 0;
            FocusIndex = 0;

            // Calculate visible items count
            int NumLines = Area.num_rows() - 1;

            // Fill remaining lines with empty items
            for (int i = NumIndexes; i < NumLines; i++) {
                auto& col = NameColumns.emplace_back(name_column{});
                col.FirstIndex = LeftColumnSize;
                col.BeginOffset = static_cast<int>(NameContainer.size());
                col.EndOffset = col.BeginOffset;

                // Add empty line to display buffer
                bf.append(CommInit);
                bf.append(LeftColumnSize + RightColumnSize + 2, ' ');
                bf.append(CommReturn);
            }

            // Highlight first item if any items exist
            if (NumIndexes > 0) {
                bf.replace(0, CommFocus.size(), CommFocus);
            }
        }

        /**
         * @brief Add a new item to the list
         *
         * Adds an item with the specified text and formats it appropriately.
         *
         * @param sv Text for the new item
         * @throws std::bad_alloc If memory allocation fails
         */
        void add_item(std::wstring_view sv) {
            // Check for valid state
            if (CommReturn.empty()) {
                throw std::bad_alloc();
            }

            // Calculate column sizes
            int ColumnSizes = LeftColumnSize + RightColumnSize;

            // Create new item
            ++NumIndexes;
            auto& col = NameColumns.emplace_back(name_column{});
            col.FirstIndex = LeftColumnSize;
            col.BeginOffset = static_cast<int>(NameContainer.size());
            col.Selected = false;

            // Format display of the item
            bf.append(CommInit);
            bf.append(sv.substr(0, LeftColumnSize));
            bf.push_back(' ');

            // Handle text that fits entirely in the visible area
            if (sv.size() <= static_cast<size_t>(ColumnSizes)) {
                bf.append(sv.substr(LeftColumnSize, sv.size() - LeftColumnSize));
                if (sv.size() < static_cast<size_t>(ColumnSizes)) {
                    bf.append(static_cast<size_t>(ColumnSizes) - sv.size(), ' ');
                }
                bf.push_back(' ');
            }
            // Handle text that requires horizontal scrolling
            else {
                bf.append(sv.substr(LeftColumnSize, RightColumnSize));
                NameContainer.append(sv.substr(LeftColumnSize, sv.size() - LeftColumnSize));
                PushBack(bf, RRQUOTE);
            }

            // Complete the line
            bf.append(CommReturn);
            col.EndOffset = static_cast<int>(NameContainer.size());
        }

        /**
         * @brief Render the entire display
         *
         * Draws all visible items and scrollbars.
         */
        void draw_all2() noexcept {
            TempBuffer.clear();

            // Draw scrollbars if items exist
            if (FocusIndex >= 0 && FocusIndex < NumIndexes) {
                name_column Item = NameColumns[FocusIndex];
                vScroll.draw(TempBuffer, FocusIndex, NumIndexes);
                hScroll.draw(TempBuffer, Item.FirstIndex - LeftColumnSize, Item.size());
            }
            else {
                vScroll.draw(TempBuffer, 0, 0);
                hScroll.draw(TempBuffer, 0, 0);
            }

            // Position cursor and render
            Area.Top.apply(TempBuffer);
            mz::Write(TempBuffer);
            mz::Write(std::wstring_view(bf).substr(
                TextLineLength * TopIndex,
                TextLineLength * (Area.num_rows() - 1))
            );
        }

        /**
         * @brief Toggle selection state of focused item
         *
         * @return New selection state (true=selected, false=unselected)
         */
        bool swap_select() noexcept {
            // Toggle selection state
            NameColumns[FocusIndex].Selected = !NameColumns[FocusIndex].Selected;

            // Update display formatting based on new state
            if (NameColumns[FocusIndex].Selected) {
                bf.replace(TextLineLength * FocusIndex, CommBoth.size(), CommBoth);
            }
            else {
                bf.replace(TextLineLength * FocusIndex, CommFocus.size(), CommFocus);
            }

            // Render the updated item
            TempBuffer.clear();
            Area.Top.offset(FocusIndex - TopIndex, 0).apply(TempBuffer);
            mz::Write(TempBuffer);
            mz::Write(std::wstring_view(bf).substr(TextLineLength * FocusIndex, TextLineLength));

            return NameColumns[FocusIndex].Selected;
        }

        /**
         * @brief Move to previous page of items
         *
         * @return New focused item index, or -1 if empty
         */
        int page_up() noexcept {
            if (!NumIndexes) { return -1; }

            // Calculate display parameters
            int NumPrintIndex = min(Area.num_rows() - 1, NumIndexes);

            // If focus is below top item, move focus to top
            if (FocusIndex > TopIndex) {
                // Update scrollbar based on new position
                TempBuffer.clear();
                if (NameColumns[FocusIndex].size() || NameColumns[TopIndex].size()) {
                    hScroll.draw(TempBuffer, NameColumns[TopIndex].FirstIndex - LeftColumnSize, NameColumns[TopIndex].size());
                }

                // Unmark current focus
                if (NameColumns[FocusIndex].Selected) {
                    bf.replace(FocusIndex * TextLineLength, CommSelect.size(), CommSelect);
                }
                else {
                    bf.replace(FocusIndex * TextLineLength, CommInit.size(), CommInit);
                }

                // Render the updated line
                Area.Top.offset(FocusIndex - TopIndex, 0).apply(TempBuffer);
                mz::Write(TempBuffer);
                mz::Write(std::wstring_view(bf).substr(TextLineLength * FocusIndex, TextLineLength));

                // Move focus to top visible item
                FocusIndex = TopIndex;

                // Mark new focus
                if (NameColumns[FocusIndex].Selected) {
                    bf.replace(FocusIndex * TextLineLength, CommBoth.size(), CommBoth);
                }
                else {
                    bf.replace(FocusIndex * TextLineLength, CommFocus.size(), CommFocus);
                }

                // Render the updated focus line
                TempBuffer.clear();
                Area.Top.apply(TempBuffer);
                mz::Write(TempBuffer);
                mz::Write(std::wstring_view(bf).substr(TextLineLength * FocusIndex, TextLineLength));
            }
            // If top item is not the first item, scroll up a page
            else if (TopIndex > 0) {
                // Unmark current focus
                if (NameColumns[FocusIndex].Selected) {
                    bf.replace(FocusIndex * TextLineLength, CommSelect.size(), CommSelect);
                }
                else {
                    bf.replace(FocusIndex * TextLineLength, CommInit.size(), CommInit);
                }

                // Calculate new top position (scroll up a page)
                TopIndex -= Area.num_rows() - 1;
                TopIndex = max(0, TopIndex);
                FocusIndex = TopIndex;

                // Mark new focus
                if (NameColumns[FocusIndex].Selected) {
                    bf.replace(FocusIndex * TextLineLength, CommBoth.size(), CommBoth);
                }
                else {
                    bf.replace(FocusIndex * TextLineLength, CommFocus.size(), CommFocus);
                }

                // Update scrollbars and render
                TempBuffer.clear();
                vScroll.draw(TempBuffer, TopIndex, NumIndexes);
                hScroll.draw(TempBuffer, NameColumns[FocusIndex].FirstIndex - LeftColumnSize, NameColumns[FocusIndex].size());
                Area.Top.apply(TempBuffer);
                mz::Write(TempBuffer);
                mz::Write(std::wstring_view(bf).substr(TextLineLength * TopIndex, TextLineLength * NumPrintIndex));
            }
            // Already at top, just wait a bit to show feedback
            else {
                std::this_thread::sleep_for(std::chrono::milliseconds{ 100 });
            }

            return FocusIndex;
        }

        /**
         * @brief Move to next page of items
         *
         * @return New focused item index, or -1 if empty
         */
        int page_down() noexcept {
            if (!NumIndexes) { return -1; }

            // Calculate display parameters
            int NumPrintIndex = min(Area.num_rows() - 1, NumIndexes);
            int BottomIndex = TopIndex + Area.num_rows() - 2;
            BottomIndex = min(BottomIndex, NumIndexes - 1);

            // If focus is above bottom item, move focus to bottom
            if (FocusIndex < BottomIndex) {
                // Update scrollbar based on new position
                TempBuffer.clear();
                if (NameColumns[FocusIndex].size() || NameColumns[BottomIndex].size()) {
                    hScroll.draw(TempBuffer, NameColumns[BottomIndex].FirstIndex - LeftColumnSize, NameColumns[BottomIndex].size());
                }

                // Unmark current focus
                if (NameColumns[FocusIndex].Selected) {
                    bf.replace(FocusIndex * TextLineLength, CommSelect.size(), CommSelect);
                }
                else {
                    bf.replace(FocusIndex * TextLineLength, CommInit.size(), CommInit);
                }

                // Render the updated line
                Area.Top.offset(FocusIndex - TopIndex, 0).apply(TempBuffer);
                mz::Write(TempBuffer);
                mz::Write(std::wstring_view(bf).substr(TextLineLength * FocusIndex, TextLineLength));

                // Move focus to bottom visible item
                FocusIndex = BottomIndex;

                // Mark new focus
                if (NameColumns[FocusIndex].Selected) {
                    bf.replace(FocusIndex * TextLineLength, CommBoth.size(), CommBoth);
                }
                else {
                    bf.replace(FocusIndex * TextLineLength, CommFocus.size(), CommFocus);
                }

                // Render the updated focus line
                TempBuffer.clear();
                Area.Top.offset(BottomIndex - TopIndex, 0).apply(TempBuffer);
                mz::Write(TempBuffer);
                mz::Write(std::wstring_view(bf).substr(TextLineLength * FocusIndex, TextLineLength));
            }
            // If bottom item is not the last item, scroll down a page
            else if (BottomIndex < NumIndexes - 1) {
                // Unmark current focus
                if (NameColumns[FocusIndex].Selected) {
                    bf.replace(FocusIndex * TextLineLength, CommSelect.size(), CommSelect);
                }
                else {
                    bf.replace(FocusIndex * TextLineLength, CommInit.size(), CommInit);
                }

                // Calculate new bottom position (scroll down a page)
                BottomIndex += Area.num_rows() - 2;
                BottomIndex = min(BottomIndex, NumIndexes - 1);
                FocusIndex = BottomIndex;

                // Mark new focus
                if (NameColumns[FocusIndex].Selected) {
                    bf.replace(FocusIndex * TextLineLength, CommBoth.size(), CommBoth);
                }
                else {
                    bf.replace(FocusIndex * TextLineLength, CommFocus.size(), CommFocus);
                }

                // Calculate new top position
                TopIndex = BottomIndex - (Area.num_rows() - 2);
                TopIndex = max(0, TopIndex);

                // Calculate number of items to render
                NumPrintIndex = BottomIndex - TopIndex + 1;

                // Update scrollbars and render
                TempBuffer.clear();
                vScroll.draw(TempBuffer, TopIndex, NumIndexes);
                hScroll.draw(TempBuffer, NameColumns[FocusIndex].FirstIndex - LeftColumnSize, NameColumns[FocusIndex].size());
                Area.Top.apply(TempBuffer);
                mz::Write(TempBuffer);
                mz::Write(std::wstring_view(bf).substr(TextLineLength * TopIndex, TextLineLength * NumPrintIndex));
            }
            // Already at bottom, just wait a bit to show feedback
            else {
                std::this_thread::sleep_for(std::chrono::milliseconds{ 100 });
            }

            return FocusIndex;
        }

        /**
         * @brief Move focus to previous item
         *
         * @return New focused item index, or -1 if empty
         */
        int move_up() noexcept {
            if (!NumIndexes) { return -1; }

            // Default number of lines to render
            int NumPrintIndex = 2;

            // Move up if not at first item
            if (FocusIndex > 0) {
                // Unmark current focus
                if (NameColumns[FocusIndex].Selected) {
                    bf.replace(FocusIndex * TextLineLength, CommSelect.size(), CommSelect);
                }
                else {
                    bf.replace(FocusIndex * TextLineLength, CommInit.size(), CommInit);
                }

                // Move focus up one item
                --FocusIndex;

                // Mark new focus
                if (NameColumns[FocusIndex].Selected) {
                    bf.replace(FocusIndex * TextLineLength, CommBoth.size(), CommBoth);
                }
                else {
                    bf.replace(FocusIndex * TextLineLength, CommFocus.size(), CommFocus);
                }

                // Prepare render buffer
                TempBuffer.clear();

                // If focus moved above visible area, scroll up
                if (TopIndex > FocusIndex) {
                    TopIndex = FocusIndex;
                    vScroll.draw(TempBuffer, TopIndex, NumIndexes);
                    NumPrintIndex = Area.num_rows() - 1;
                }

                // Update horizontal scrollbar if needed
                if (NameColumns[FocusIndex + 1].size() || NameColumns[FocusIndex].size()) {
                    hScroll.draw(TempBuffer, NameColumns[FocusIndex].FirstIndex - LeftColumnSize, NameColumns[FocusIndex].size());
                }

                // Render the updated display
                Area.Top.offset(FocusIndex - TopIndex, 0).apply(TempBuffer);
                mz::Write(TempBuffer);
                mz::Write(std::wstring_view(bf).substr(TextLineLength * FocusIndex, TextLineLength * NumPrintIndex));
            }
            // Already at top, just wait a bit to show feedback
            else {
                std::this_thread::sleep_for(std::chrono::milliseconds{ 100 });
            }

            return FocusIndex;
        }

        /**
         * @brief Move focus to next item
         *
         * @return New focused item index, or -1 if empty
         */
        int move_down() noexcept {
            // Default number of lines to render
            int NumPrintIndex = 2;
            int TopPrintIndex = FocusIndex;

            // Move down if not at last item
            if (FocusIndex + 1 < NumIndexes) {
                // Unmark current focus
                if (NameColumns[FocusIndex].Selected) {
                    bf.replace(FocusIndex * TextLineLength, CommSelect.size(), CommSelect);
                }
                else {
                    bf.replace(FocusIndex * TextLineLength, CommInit.size(), CommInit);
                }

                // Move focus down one item
                ++FocusIndex;

                // Mark new focus
                if (NameColumns[FocusIndex].Selected) {
                    bf.replace(FocusIndex * TextLineLength, CommBoth.size(), CommBoth);
                }
                else {
                    bf.replace(FocusIndex * TextLineLength, CommFocus.size(), CommFocus);
                }

                // Prepare render buffer
                TempBuffer.clear();

                // If focus moved below visible area, scroll down
                if (FocusIndex - TopIndex >= Area.num_rows() - 1) {
                    TopPrintIndex = ++TopIndex;
                    NumPrintIndex = Area.num_rows() - 1;
                    vScroll.draw(TempBuffer, TopIndex, NumIndexes);
                }

                // Update horizontal scrollbar if needed
                if (NameColumns[FocusIndex - 1].size() || NameColumns[FocusIndex].size()) {
                    hScroll.draw(TempBuffer, NameColumns[FocusIndex].FirstIndex - LeftColumnSize, NameColumns[FocusIndex].size());
                }

                // Render the updated display
                Area.Top.offset(TopPrintIndex - TopIndex, 0).apply(TempBuffer);
                mz::Write(TempBuffer);
                mz::Write(std::wstring_view(bf).substr(TextLineLength * TopPrintIndex, TextLineLength * NumPrintIndex));
            }
            // Already at bottom, just wait a bit to show feedback
            else {
                std::this_thread::sleep_for(std::chrono::milliseconds{ 100 });
            }

            return FocusIndex;
        }

        /**
         * @brief Scroll item text left
         *
         * For items wider than the display area, this shifts the visible
         * portion of text to the left.
         */
        void move_left() noexcept {
            if (!NumIndexes) return;

            // Get current item
            auto& item = NameColumns[FocusIndex];

            // Calculate item dimensions
            int ItemSize = item.EndOffset - item.BeginOffset + LeftColumnSize;

            // Only scroll if not already at leftmost position
            if (item.FirstIndex > LeftColumnSize) {
                // Move left one character
                --item.FirstIndex;

                // Update left indicator if needed
                if (item.FirstIndex == LeftColumnSize) {
                    bf[TextLineLength * FocusIndex + offsetLeftSign] = ' ';
                }

                // Update the visible text
                auto sv = std::wstring_view(NameContainer).substr(item.BeginOffset, item.EndOffset);
                bf.replace(TextLineLength * FocusIndex + offsetColumn2,
                    RightColumnSize,
                    sv.substr(item.FirstIndex - LeftColumnSize, RightColumnSize));

                // Ensure right indicator is visible
                bf[TextLineLength * FocusIndex + offsetRightSign] = RRQUOTE[0];

                // Update scrollbar and render
                TempBuffer.clear();
                hScroll.draw(TempBuffer, item.FirstIndex - LeftColumnSize, ItemSize - LeftColumnSize);
                Area.Top.offset(FocusIndex - TopIndex, 0).apply(TempBuffer);
                mz::Write(TempBuffer);
                mz::Write(std::wstring_view(bf).substr(TextLineLength * FocusIndex, TextLineLength));
            }
        }

        /**
         * @brief Scroll item text right
         *
         * For items wider than the display area, this shifts the visible
         * portion of text to the right.
         */
        void move_right() noexcept {
            if (!NumIndexes) return;

            // Get current item
            auto& item = NameColumns[FocusIndex];

            // Calculate item dimensions
            int ItemSize = item.EndOffset - item.BeginOffset + LeftColumnSize;
            int MaxFirstIndex = ItemSize - RightColumnSize;

            // Only scroll if not already at rightmost position
            if (item.FirstIndex < MaxFirstIndex) {
                // Move right one character
                ++item.FirstIndex;

                // Ensure left indicator is visible
                bf[TextLineLength * FocusIndex + offsetLeftSign] = LLQUOTE[0];

                // Update the visible text
                auto sv = std::wstring_view(NameContainer).substr(item.BeginOffset, item.EndOffset);
                bf.replace(TextLineLength * FocusIndex + offsetColumn2,
                    RightColumnSize,
                    sv.substr(item.FirstIndex - LeftColumnSize, RightColumnSize));

                // Update right indicator if needed
                if (item.FirstIndex == MaxFirstIndex) {
                    bf[TextLineLength * FocusIndex + offsetRightSign] = ' ';
                }

                // Update scrollbar and render
                TempBuffer.clear();
                hScroll.draw(TempBuffer, item.FirstIndex - LeftColumnSize, ItemSize - LeftColumnSize);
                Area.Top.offset(FocusIndex - TopIndex, 0).apply(TempBuffer);
                mz::Write(TempBuffer);
                mz::Write(std::wstring_view(bf).substr(TextLineLength * FocusIndex, TextLineLength));
            }
        }

        /**
         * @brief Filter list to show only selected items
         *
         * Removes unselected items and reindexes the list.
         */
        void squeeze() noexcept {
            // Create new list with only selected items
            int WriteIndex = 0;
            for (int ReadIndex = 0; ReadIndex < NumIndexes; ReadIndex++) {
                if (NameColumns[ReadIndex].Selected) {
                    // Update focus position if it was on this item
                    if (FocusIndex == ReadIndex) {
                        FocusIndex = WriteIndex;
                        bf.replace(TextLineLength * ReadIndex, CommSelect.size(), CommSelect);
                    }

                    // Move item to new position if needed
                    if (ReadIndex != WriteIndex) {
                        auto sv = std::wstring_view(bf).substr(TextLineLength * ReadIndex, TextLineLength);
                        bf.replace(TextLineLength * WriteIndex, TextLineLength, sv);
                    }

                    ++WriteIndex;
                }
            }

            // Update item count
            NumIndexes = WriteIndex;

            // Resize buffer and fill with empty lines if needed
            bf.resize(TextLineLength * NumIndexes);
            for (int i = NumIndexes; i < Area.num_rows() - 1; i++) {
                bf.append(CommInit);
                bf.append(LeftColumnSize + RightColumnSize + 2, ' ');
                bf.append(CommReturn);
            }

            // Reset view position
            TopIndex = 0;
            FocusIndex = 0;
        }

        /**
         * @brief Run a test of the directory display
         *
         * Creates a test display with sample data.
         *
         * @param Window Screen area for the test
         */
        static void Test(coord_box Window) noexcept {
            DirectoryDisplayBox sb;
            sb.Area = Window.center_box(10, 50);

            // Add test items
            sb.add_item(L"Line12345678  0123456789012345678901234567890123456789");
            for (int i = 0; i < 31; i++) {
                sb.add_item(std::format(L"Line12345678  {:<2} HELLO {:->{}} HI", i, '-', i));
            }

            // Initialize and display
            sb.initialize();
            sb.create();
            sb.draw_all2();

            // Event loop for testing
            while (true) {
                int wc = mz::wgetch();
                switch (wc) {
                    // Add key handling for interactive testing if needed
                case -1:
                default: break;
                }
            }
        }

        /**
         * @brief Get currently focused item text
         *
         * @return Text of the focused item or empty view if none
         */
        std::wstring_view get_focused_item() const noexcept {
            if (FocusIndex >= 0 && FocusIndex < NumIndexes) {
                const auto& item = NameColumns[FocusIndex];
                if (item.BeginOffset < item.EndOffset) {
                    return std::wstring_view(NameContainer).substr(
                        item.BeginOffset,
                        item.EndOffset - item.BeginOffset
                    );
                }
            }
            return std::wstring_view();
        }

        /**
         * @brief Get index of focused item
         *
         * @return Current focus index, or -1 if list is empty
         */
        int get_focused_index() const noexcept {
            return NumIndexes > 0 ? FocusIndex : -1;
        }

        /**
         * @brief Clear all items from the list
         */
        void clear_items() noexcept {
            NameContainer.clear();
            NameColumns.clear();
            NumIndexes = 0;
            TopIndex = 0;
            FocusIndex = 0;
            bf.clear();
            initialize();
            create();
        }

        /**
         * @brief Get number of selected items
         *
         * @return Count of selected items
         */
        int get_selection_count() const noexcept {
            int count = 0;
            for (int i = 0; i < NumIndexes; i++) {
                if (NameColumns[i].Selected) {
                    count++;
                }
            }
            return count;
        }

        /**
         * @brief Get vector of selected item indices
         *
         * @return Vector containing indices of all selected items
         */
        std::vector<int> get_selected_indices() const {
            std::vector<int> selected;
            selected.reserve(get_selection_count());

            for (int i = 0; i < NumIndexes; i++) {
                if (NameColumns[i].Selected) {
                    selected.push_back(i);
                }
            }

            return selected;
        }
    };

} // namespace mz

#endif // MZ_DIRECTORY_DISPLAY_BOX_H

