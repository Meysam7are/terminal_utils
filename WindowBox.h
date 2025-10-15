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

#ifndef MZ_WINDOW_BOX_H
#define MZ_WINDOW_BOX_H
#pragma once

/**
 * @file WindowBox.h
 * @brief UI components for progress bars and scrollbars
 *
 * This file provides terminal UI components for displaying progress bars
 * and scrollbars in console applications. These components help create
 * more interactive and visual interfaces in text-based environments.
 *
 * @author Meysam Zare
 */

#include "ConsoleCMD.h"
#include "coord.h"
#include "cursor.h"
#include "ConsoleBoxes.h"
#include <format>

namespace mz {

    /**
     * @class ProgressBarControl
     * @brief Visual progress indicator for console applications
     *
     * Displays a horizontal progress bar that visually represents completion
     * percentage. The bar shows both a graphical representation and numeric
     * percentage, adapting its appearance based on the progress level.
     */
    class ProgressBarControl : public BasicBox {
    private:
        /**
         * @brief Buffer position after initial formatting
         *
         * Marks the position in the buffer after formatting but before content.
         */
        int PreMessageSize{ 0 };

        /**
         * @brief Update internal buffer with current progress
         *
         * Formats the progress bar display based on current percentage.
         * Handles different visual states based on progress level.
         */
        void draw() noexcept {
            // Create percentage text
            std::wstring percentageText{ std::format(L"{}%", Percentage) };

            // Initialize buffer if needed
            if (!PreMessageSize) {
                bf.clear();
                Area.Top.apply(bf);
                Color.apply(bf);
                PreMessageSize = static_cast<int>(bf.size());
            }

            // Reset buffer to initial state
            bf.resize(PreMessageSize);

            // Calculate dimensions and positions
            const long long barWidth = Area.num_cols();
            const long long filledWidth = static_cast<long long>(Percentage) * barWidth / 100;
            const long long textLength = Percentage < 10 ? 2 : Percentage < 100 ? 3 : 4;
            const long long leftSegmentWidth = (barWidth - textLength) / 2;
            const long long rightSegmentStart = leftSegmentWidth + textLength;

            // Draw the progress bar based on fill level
            if (filledWidth <= leftSegmentWidth) {
                // Percentage text is entirely in the unfilled area
                bf.append(filledWidth, ' ');
                Color.apply_mirror(bf);  // Switch colors for unfilled area
                bf.append(leftSegmentWidth - filledWidth, ' ');
                bf.append(percentageText);
                bf.append(barWidth - rightSegmentStart, ' ');
            }
            else if (filledWidth <= rightSegmentStart) {
                // Percentage text is partially in filled area
                bf.append(leftSegmentWidth, ' ');

                // Add characters of text that are in filled area
                for (long long i = leftSegmentWidth; i < filledWidth; i++) {
                    bf.push_back(percentageText[i - leftSegmentWidth]);
                }

                // Switch colors for unfilled area
                Color.apply_mirror(bf);

                // Add characters of text that are in unfilled area
                for (long long i = filledWidth; i < rightSegmentStart; i++) {
                    bf.push_back(percentageText[i - leftSegmentWidth]);
                }

                bf.append(rightSegmentStart - filledWidth, ' ');
            }
            else {
                // Percentage text is entirely in filled area
                bf.append(leftSegmentWidth, ' ');
                bf.append(percentageText);
                bf.append(filledWidth - rightSegmentStart, ' ');

                // Switch colors for unfilled area
                Color.apply_mirror(bf);
                bf.append(barWidth - filledWidth, ' ');
            }
        }

    public:
        /**
         * @brief Default color for progress bars
         */
        static constexpr rgb ProgressBarRGB1{ rgb::gray(50) };

        /**
         * @brief Current progress percentage (0-100)
         */
        int Percentage{ 0 };

        /**
         * @brief Default constructor
         *
         * Initializes with default colors and size.
         */
        ProgressBarControl() noexcept {
            Color.F = ProgressBarRGB1;
            Color.B = -ProgressBarRGB1;
            Area.Top = coord{ 1, 1 };
            Area.set_size(coord{ 1, 10 });
        }

        /**
         * @brief Initialize the progress bar
         *
         * Creates a progress bar at the specified position with the given width.
         *
         * @param Top Top-left position for the bar
         * @param NumBars Width of the progress bar in characters
         */
        void create(mz::coord Top, int NumBars) noexcept {
            // Enforce minimum and maximum width
            NumBars = NumBars < 4 ? 4 : NumBars;
            NumBars = NumBars > 100 ? 100 : NumBars;

            // Set position and size
            Area.Top = Top;
            Area.set_size(1, NumBars);

            // Initialize buffer
            bf.clear();
            bf.reserve(NumBars * 2 + 100);  // Pre-allocate buffer
            Color.apply(bf);
            Area.Top.apply(bf);
            SetHide(bf);
            ClrBold(bf);
            ClrNegative(bf);
            PreMessageSize = static_cast<int>(bf.size());

            // Start with 0% progress
            Percentage = 0;
            draw();
        }

        /**
         * @brief Update progress bar with new percentage
         *
         * Sets the progress to the specified percentage and redraws if changed.
         *
         * @param ProgressPercentage New progress value (0-100)
         */
        void update(int ProgressPercentage) noexcept {
            // Only redraw if percentage changed or at 0%
            if (ProgressPercentage != Percentage || !Percentage) {
                draw(ProgressPercentage);
            }
        }

        /**
         * @brief Set progress and redraw
         *
         * Sets the progress to the specified percentage and forces redraw.
         *
         * @param ProgressPercentage New progress value (0-100)
         */
        void draw(int ProgressPercentage) noexcept {
            // Constrain percentage to valid range
            ProgressPercentage = std::clamp(ProgressPercentage, 0, 100);
            Percentage = ProgressPercentage;

            // Update display
            draw();
            print();
        }

        /**
         * @brief Run a test animation
         *
         * Creates a progress bar and animates it from 0% to 100%.
         *
         * @param Window Screen area for the test
         */
        static void Test(coord_box Window) noexcept {
            ProgressBarControl pbc;
            pbc.Color = color(mz::color::AQUA, 80);
            pbc.create(mz::coord{ 2, 2 }, 40);

            for (int i = 0; i <= 100; i++) {
                pbc.draw(i);
                Sleep(100);
            }
        }

        /**
         * @brief Set progress bar colors
         *
         * Changes the filled and unfilled area colors.
         *
         * @param filledColor Color for the filled portion
         * @param unfilledColor Color for the unfilled portion
         */
        void set_colors(rgb filledColor, rgb unfilledColor) noexcept {
            Color.F = filledColor;
            Color.B = unfilledColor;
        }

        /**
         * @brief Set progress bar style
         *
         * Changes the appearance style of the progress bar.
         *
         * @param contrastLevel Contrast level between filled and unfilled areas (0-100)
         */
        void set_style(int contrastLevel) noexcept {
            Color = color(ProgressBarRGB1, contrastLevel);
        }
    };

    /**
     * @class BasicScrollBar
     * @brief Base class for scrollbar components
     *
     * Provides common properties and functionality for horizontal
     * and vertical scrollbars.
     */
    class BasicScrollBar {
    public:
        /**
         * @brief Total length of the scrollbar in characters
         */
        int BarLength{ 0 };

        /**
         * @brief Space before the scrollbar
         */
        int PreLength{ 0 };

        /**
         * @brief Space after the scrollbar
         */
        int PostLength{ 0 };

        /**
         * @brief Background color for the scrollbar area
         */
        rgb BackRGB;

        /**
         * @brief Top-left position of the scrollbar
         */
        coord TopLeft;

        /**
         * @brief Colors for the scrollbar elements
         *
         * F=thumb color, B=track color
         */
        color ScrollColors;

        /**
         * @brief Default constructor
         */
        BasicScrollBar() noexcept = default;

        /**
         * @brief Constructor with position and size
         *
         * @param position Top-left position
         * @param length Length of the scrollbar
         */
        BasicScrollBar(coord position, int length) noexcept
            : BarLength(length), TopLeft(position) {
            // Set default colors
            BackRGB = color::BLACK;
            ScrollColors.F = color::WHITE;
            ScrollColors.B = color::GRAY;
        }

        /**
         * @brief Set scrollbar colors
         *
         * @param background Background color
         * @param thumbColor Scrollbar thumb color
         * @param trackColor Scrollbar track color
         */
        void set_colors(rgb background, rgb thumbColor, rgb trackColor) noexcept {
            BackRGB = background;
            ScrollColors.F = thumbColor;
            ScrollColors.B = trackColor;
        }
    };

    /**
     * @class HorizontalScrollBar
     * @brief Horizontal scrollbar for navigating content
     *
     * Displays a horizontal scrollbar that represents the current
     * viewport position within a larger content area.
     */
    class HorizontalScrollBar : public BasicScrollBar {
    public:
        /**
         * @brief Default constructor
         */
        HorizontalScrollBar() noexcept = default;

        /**
         * @brief Constructor with position and size
         *
         * @param position Top-left position
         * @param length Length of the scrollbar
         */
        HorizontalScrollBar(coord position, int length) noexcept
            : BasicScrollBar(position, length) {
        }

        /**
         * @brief Draw the scrollbar
         *
         * Renders the horizontal scrollbar to represent the current
         * scroll position within a collection of items.
         *
         * @param bf Buffer to append drawing commands to
         * @param FirstItem Index of first visible item
         * @param NumItems Total number of items
         */
        void draw(std::wstring& bf, int FirstItem, int NumItems) const noexcept {
            // Set cursor position to start of scrollbar
            coord Loc{ TopLeft };

            // Ensure bar length is valid
            int NumBars = BarLength;
            NumBars = NumBars > 0 ? NumBars : 0;

            // Setup display
            SetHide(bf);
            ClrUnderline(bf);
            BackRGB.setBack(bf);

            // Position cursor
            Loc.apply(bf);

            // Add pre-scrollbar space if needed
            if (PreLength > 0) {
                Loc.append(bf, PreLength, ' ');
            }

            // If all items fit in view, show inactive scrollbar
            if (NumItems <= BarLength) {
                ScrollColors.B.setFront(bf);
                Loc.push_back(bf, LHEAD);  // Left arrow

                // Draw inactive scrollbar of full width
                for (int i = 0; i < BarLength; i++) {
                    Loc.push_back(bf, FSQUARE);
                }

                Loc.push_back(bf, RHEAD);  // Right arrow
            }
            else {
                // Calculate thumb position and size
                int thumbStart = 0;
                int thumbEnd = (FirstItem + BarLength) * BarLength / NumItems;

                // Ensure thumb is visible and sized appropriately
                if (thumbEnd >= BarLength) {
                    thumbEnd = BarLength - (FirstItem + BarLength < NumItems ? 1 : 0);
                }

                if (thumbEnd > 0) {
                    // Calculate thumb start based on its size and position
                    thumbStart = thumbEnd - BarLength * BarLength / NumItems;
                    thumbStart -= (thumbStart == thumbEnd ? 1 : 0);
                    thumbStart = thumbStart > 0 ? thumbStart : 0;
                }
                else {
                    // Minimum thumb size
                    thumbStart = 0;
                    thumbEnd = 1;
                }

                // Adjust for special case at beginning
                if (thumbStart == 0 && FirstItem > 0) {
                    ++thumbStart;
                    ++thumbEnd;
                }

                // Set up cursor control sequence
                cursor Seq;

                // Left arrow color based on scroll position
                if (thumbStart > 0) {
                    Seq.set_front_rgb(bf, ScrollColors.F);  // Active
                }
                else {
                    Seq.set_front_rgb(bf, ScrollColors.B);  // Inactive
                }

                // Draw left arrow
                Loc.push_back(bf, LHEAD);

                // Draw track before thumb
                if (thumbStart > 0) {
                    Seq.update_front_rgb(bf, ScrollColors.B);
                    for (int i = 0; i < thumbStart; i++) {
                        Loc.push_back(bf, FSQUARE);
                    }
                }

                // Draw thumb
                if (thumbStart < thumbEnd) {
                    Seq.update_front_rgb(bf, ScrollColors.F);
                    for (int i = thumbStart; i < thumbEnd; i++) {
                        Loc.push_back(bf, FSQUARE);
                    }
                }

                // Draw track after thumb
                if (thumbEnd < NumBars) {
                    Seq.update_front_rgb(bf, ScrollColors.B);
                    for (int i = thumbEnd; i < NumBars; i++) {
                        Loc.push_back(bf, FSQUARE);
                    }
                }

                // Right arrow color based on scroll position
                if (thumbEnd < NumBars) {
                    Seq.update_front_rgb(bf, ScrollColors.F);  // Active
                }
                else {
                    Seq.update_front_rgb(bf, ScrollColors.B);  // Inactive
                }

                // Draw right arrow
                Loc.push_back(bf, RHEAD);
            }

            // Add post-scrollbar space if needed
            if (PostLength > 0) {
                Loc.append(bf, PostLength, ' ');
            }
        }
    };

    /**
     * @class VerticalScrollBar
     * @brief Vertical scrollbar for navigating content
     *
     * Displays a vertical scrollbar that represents the current
     * viewport position within a larger content area.
     */
    class VerticalScrollBar : public BasicScrollBar {
    public:
        /**
         * @brief Default constructor
         */
        VerticalScrollBar() noexcept = default;

        /**
         * @brief Constructor with position and size
         *
         * @param position Top-left position
         * @param length Length of the scrollbar
         */
        VerticalScrollBar(coord position, int length) noexcept
            : BasicScrollBar(position, length) {
        }

        /**
         * @brief Draw the scrollbar
         *
         * Renders the vertical scrollbar to represent the current
         * scroll position within a collection of items.
         *
         * @param bf Buffer to append drawing commands to
         * @param FirstRow Index of first visible row
         * @param NumItems Total number of items
         */
        void draw(std::wstring& bf, int FirstRow, int NumItems) const noexcept {
            // Set up cursor control sequence
            cursor Seq;

            // Setup display
            SetHide(bf);
            ClrUnderline(bf);
            BackRGB.setBack(bf);

            // Position cursor
            coord Loc{ TopLeft };
            Loc.apply(bf);

            // Calculate usable bar length (excluding arrows)
            int NumBars = BarLength - 2;

            // Track start and end positions
            int thumbStart = 0;
            int thumbEnd = 0;

            // If all items fit in view, show inactive scrollbar
            if (NumItems < BarLength) {
                thumbStart = 0;
                thumbEnd = NumBars;

                // Draw inactive up arrow
                Seq.set_front_rgb(bf, ScrollColors.B);
                Loc.push_back(bf, TRIUP);

                // Draw inactive scrollbar body
                Seq.set_back_rgb(bf, ScrollColors.B);
                for (int i = 0; i < NumBars; i++) {
                    Loc.move_left(bf);
                    Loc.move_down(bf);
                    Loc.push_back(bf, ' ');
                }

                // Restore background color
                Seq.update_back_rgb(bf, BackRGB);
            }
            else {
                // Calculate thumb position and size
                thumbEnd = (FirstRow + BarLength) * NumBars / NumItems;

                // Ensure thumb is visible and sized appropriately
                if (thumbEnd >= NumBars) {
                    thumbEnd = NumBars - (FirstRow + BarLength < NumBars ? 1 : 0);
                }

                if (thumbEnd > 0) {
                    // Calculate thumb start based on its size and position
                    thumbStart = thumbEnd - BarLength * NumBars / NumItems;
                    thumbStart -= (thumbStart == thumbEnd ? 1 : 0);
                    thumbStart = thumbStart > 0 ? thumbStart : 0;
                }
                else {
                    // Minimum thumb size
                    thumbStart = 0;
                    thumbEnd = 1;
                }

                // Adjust for special case at beginning
                if (thumbStart == 0 && FirstRow > 0) {
                    ++thumbStart;
                    ++thumbEnd;
                }

                // Up arrow color based on scroll position
                if (thumbStart > 0) {
                    Seq.set_front_rgb(ScrollColors.F);  // Active
                }
                else {
                    Seq.set_front_rgb(ScrollColors.B);  // Inactive
                }

                // Draw up arrow
                Seq.apply(bf);
                Loc.push_back(bf, TRIUP);

                // Set color for track
                Seq.update_front_rgb(bf, ScrollColors.F);
                Seq.update_back_rgb(bf, ScrollColors.B);

                // Draw track before thumb
                for (int i = 0; i < thumbStart; i++) {
                    Loc.move_left(bf);
                    Loc.move_down(bf);
                    Loc.push_back(bf, ' ');
                }

                // Draw thumb (inverted colors)
                SetNegative(bf);
                for (int i = thumbStart; i < thumbEnd; i++) {
                    Loc.move_left(bf);
                    Loc.move_down(bf);
                    Loc.push_back(bf, ' ');
                }
                ClrNegative(bf);

                // Draw track after thumb
                for (int i = thumbEnd; i < NumBars; i++) {
                    Loc.move_left(bf);
                    Loc.move_down(bf);
                    Loc.push_back(bf, ' ');
                }

                // Restore background color
                Seq.update_back_rgb(bf, BackRGB);

                // Down arrow color based on scroll position
                if (thumbEnd == NumBars) {
                    Seq.update_front_rgb(bf, ScrollColors.B);  // Inactive
                }
            }

            // Draw down arrow
            Loc.move_left(bf);
            Loc.move_down(bf);
            Loc.push_back(bf, TRIDOWN);
        }
    };

} // namespace mz

#endif // MZ_WINDOW_BOX_H
