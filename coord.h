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

#ifndef MZ_COORD_H
#define MZ_COORD_H
#pragma once

/**
 * @file coord.h
 * @brief Terminal coordinate handling and box manipulation utilities
 *
 * This file provides classes for handling terminal coordinates, screen positions,
 * and rectangular areas (boxes). It includes methods for cursor movement, layout
 * calculations, and terminal screen management.
 *
 * The two main classes are:
 * - coord: Represents a single terminal position (row, column)
 * - coord_box: Represents a rectangular area defined by two coordinates
 *
 * @author Meysam Zare
 */


#include "colors.h"
#include <type_traits>
#include <algorithm>
#include <string>
#include <format>

namespace mz {

    /**
     * @class coord
     * @brief Represents a terminal coordinate (row, column)
     *
     * This class represents a position in a terminal, with methods for
     * coordinate manipulation, cursor movement, and text output. The
     * coordinate system is 0-based, where (0,0) is the top-left corner.
     */
    struct coord {
        /**
         * @brief Row position (vertical coordinate)
         *
         * The row position in the terminal, starting from 0 at the top.
         */
        short Row{ 0 };

        /**
         * @brief Column position (horizontal coordinate)
         *
         * The column position in the terminal, starting from 0 at the left.
         */
        short Col{ 0 };

        /**
         * @brief Default constructor
         *
         * Initializes a coordinate at position (0,0).
         */
        constexpr coord() noexcept = default;

        /**
         * @brief Constructor with row and column values
         *
         * @tparam TR Row value type
         * @tparam TC Column value type
         * @param Row Row position
         * @param Col Column position
         */
        template <typename TR, typename TC>
        constexpr coord(TR Row, TC Col) noexcept :
            Row{ static_cast<short>(Row) },
            Col{ static_cast<short>(Col) } {
            static_assert(std::is_integral<TR>::value, "Row must be an integer type");
            static_assert(std::is_integral<TC>::value, "Column must be an integer type");
        }

        /**
         * @brief Add another coordinate to this one
         *
         * Adds row and column values from another coordinate.
         *
         * @param rhs Coordinate to add
         * @return Reference to this coordinate after addition
         */
        constexpr coord& operator += (coord rhs) noexcept {
            Row += rhs.Row;
            Col += rhs.Col;
            return *this;
        }

        /**
         * @brief Subtract another coordinate from this one
         *
         * Subtracts row and column values from another coordinate.
         *
         * @param rhs Coordinate to subtract
         * @return Reference to this coordinate after subtraction
         */
        constexpr coord& operator -= (coord rhs) noexcept {
            Row -= rhs.Row;
            Col -= rhs.Col;
            return *this;
        }

        /**
         * @brief Create a new coordinate offset from this one
         *
         * Creates a new coordinate by adding specified row and column offsets.
         *
         * @param NumRows Number of rows to offset
         * @param NumCols Number of columns to offset
         * @return New coordinate with the offset applied
         */
        constexpr coord offset(int NumRows, int NumCols) const noexcept {
            return coord{ Row + NumRows, Col + NumCols };
        }

        /**
         * @brief Less than comparison operator
         *
         * Compares coordinates lexicographically, with row having higher precedence.
         *
         * @param L Left coordinate
         * @param R Right coordinate
         * @return true if L is less than R
         */
        friend constexpr bool operator < (coord L, coord R) noexcept {
            return L.Row < R.Row || ((L.Row == R.Row) && (L.Col < R.Col));
        }

        /**
         * @brief Less than or equal comparison operator
         *
         * Compares coordinates lexicographically, with row having higher precedence.
         *
         * @param L Left coordinate
         * @param R Right coordinate
         * @return true if L is less than or equal to R
         */
        friend constexpr bool operator <= (coord L, coord R) noexcept {
            return L.Row < R.Row || ((L.Row == R.Row) && (L.Col <= R.Col));
        }

        /**
         * @brief Equality comparison operator
         *
         * @param L Left coordinate
         * @param R Right coordinate
         * @return true if coordinates are equal
         */
        friend constexpr bool operator == (coord L, coord R) noexcept {
            return L.Row == R.Row && L.Col == R.Col;
        }

        /**
         * @brief Addition operator
         *
         * @param L Left coordinate
         * @param R Right coordinate
         * @return Sum of coordinates
         */
        friend constexpr coord operator + (coord L, coord R) noexcept {
            return coord{ static_cast<short>(L.Row + R.Row), static_cast<short>(L.Col + R.Col) };
        }

        /**
         * @brief Subtraction operator
         *
         * @param L Left coordinate
         * @param R Right coordinate
         * @return Difference of coordinates
         */
        friend constexpr coord operator - (coord L, coord R) noexcept {
            return coord{ static_cast<short>(L.Row - R.Row), static_cast<short>(L.Col - R.Col) };
        }

        /**
         * @brief Inequality comparison operator
         *
         * @param L Left coordinate
         * @param R Right coordinate
         * @return true if coordinates are not equal
         */
        friend constexpr bool operator != (coord L, coord R) noexcept {
            return L.Row != R.Row || L.Col != R.Col;
        }

        /**
         * @brief Greater than comparison operator
         *
         * @param L Left coordinate
         * @param R Right coordinate
         * @return true if L is greater than R
         */
        friend constexpr bool operator > (coord L, coord R) noexcept {
            return R < L;
        }

        /**
         * @brief Greater than or equal comparison operator
         *
         * @param L Left coordinate
         * @param R Right coordinate
         * @return true if L is greater than or equal to R
         */
        friend constexpr bool operator >= (coord L, coord R) noexcept {
            return R <= L;
        }

        //=========================================================================
        // CURSOR MOVEMENT METHODS - SINGLE STEP
        //=========================================================================

        /**
         * @brief Move cursor up one position
         *
         * Updates the coordinate and appends cursor movement command to buffer.
         *
         * @param Buff Buffer to append command to
         */
        inline void move_up(std::wstring& Buff) noexcept {
            --Row;
            MoveUp(Buff);
        }

        /**
         * @brief Move cursor down one position
         *
         * Updates the coordinate and appends cursor movement command to buffer.
         *
         * @param Buff Buffer to append command to
         */
        inline void move_down(std::wstring& Buff) noexcept {
            ++Row;
            MoveDown(Buff);
        }

        /**
         * @brief Move cursor left one position
         *
         * Updates the coordinate and appends cursor movement command to buffer.
         *
         * @param Buff Buffer to append command to
         */
        inline void move_left(std::wstring& Buff) noexcept {
            --Col;
            MoveLeft(Buff);
        }

        /**
         * @brief Move cursor right one position
         *
         * Updates the coordinate and appends cursor movement command to buffer.
         *
         * @param Buff Buffer to append command to
         */
        inline void move_right(std::wstring& Buff) noexcept {
            ++Col;
            MoveRight(Buff);
        }

        //=========================================================================
        // CURSOR MOVEMENT METHODS - MULTIPLE STEPS
        //=========================================================================

        /**
         * @brief Move cursor up multiple positions
         *
         * Updates the coordinate and appends cursor movement command to buffer.
         *
         * @param Buff Buffer to append command to
         * @param X Number of positions to move
         */
        inline void move_up(std::wstring& Buff, int X) noexcept {
            if (X > 0) {
                Row -= static_cast<short>(X);
                MoveUp(Buff, X);
            }
        }

        /**
         * @brief Move cursor down multiple positions
         *
         * Updates the coordinate and appends cursor movement command to buffer.
         *
         * @param Buff Buffer to append command to
         * @param X Number of positions to move
         */
        inline void move_down(std::wstring& Buff, int X) noexcept {
            if (X > 0) {
                Row += static_cast<short>(X);
                MoveDown(Buff, X);
            }
        }

        /**
         * @brief Move cursor left multiple positions
         *
         * Updates the coordinate and appends cursor movement command to buffer.
         *
         * @param Buff Buffer to append command to
         * @param X Number of positions to move
         */
        inline void move_left(std::wstring& Buff, int X) noexcept {
            if (X > 0) {
                Col -= static_cast<short>(X);
                MoveLeft(Buff, X);
            }
        }

        /**
         * @brief Move cursor right multiple positions
         *
         * Updates the coordinate and appends cursor movement command to buffer.
         *
         * @param Buff Buffer to append command to
         * @param X Number of positions to move
         */
        inline void move_right(std::wstring& Buff, int X) noexcept {
            if (X > 0) {
                Col += static_cast<short>(X);
                MoveRight(Buff, X);
            }
        }

        //=========================================================================
        // CURSOR MOVEMENT METHODS - DIRECTIONAL
        //=========================================================================

        /**
         * @brief Move cursor vertically by a signed amount
         *
         * Moves up for negative values, down for positive values.
         *
         * @param Buff Buffer to append command to
         * @param X Number of rows to move (negative=up, positive=down)
         */
        inline void move_row_by(std::wstring& Buff, int X) noexcept {
            if (X < 0) {
                move_up(Buff, -X);
            }
            else if (X > 0) {
                move_down(Buff, X);
            }
        }

        /**
         * @brief Move cursor horizontally by a signed amount
         *
         * Moves left for negative values, right for positive values.
         *
         * @param Buff Buffer to append command to
         * @param X Number of columns to move (negative=left, positive=right)
         */
        inline void move_col_by(std::wstring& Buff, int X) noexcept {
            if (X < 0) {
                move_left(Buff, -X);
            }
            else if (X > 0) {
                move_right(Buff, X);
            }
        }

        /**
         * @brief Move cursor by a coordinate offset
         *
         * Moves by the row and column values specified in the coordinate.
         *
         * @param Buff Buffer to append command to
         * @param C Coordinate offset to move by
         */
        inline void move_by(std::wstring& Buff, coord C) noexcept {
            move_row_by(Buff, C.Row);
            move_col_by(Buff, C.Col);
        }

        //=========================================================================
        // CURSOR MOVEMENT METHODS - ABSOLUTE POSITIONING
        //=========================================================================

        /**
         * @brief Move cursor to a specific row
         *
         * @param Buff Buffer to append command to
         * @param X Target row
         */
        inline void move_row_to(std::wstring& Buff, int X) noexcept {
            move_row_by(Buff, X - Row);
        }

        /**
         * @brief Move cursor to a specific column
         *
         * @param Buff Buffer to append command to
         * @param X Target column
         */
        inline void move_col_to(std::wstring& Buff, int X) noexcept {
            move_col_by(Buff, X - Col);
        }

        /**
         * @brief Update cursor position relative to current position
         *
         * @param Buff Buffer to append command to
         * @param C Target coordinate
         */
        inline void update_to(std::wstring& Buff, coord C) noexcept {
            move_by(Buff, C - *this);
        }

        /**
         * @brief Move cursor to absolute position
         *
         * @param Buff Buffer to append command to
         * @param C Target coordinate
         */
        inline void move_to(std::wstring& Buff, coord C) noexcept {
            Row = C.Row;
            Col = C.Col;
            apply(Buff);
        }

        /**
         * @brief Ensure coordinate has non-negative values
         *
         * Clamps row and column values to be non-negative.
         */
        constexpr void normalize() noexcept {
            Row = Row > 0 ? Row : 0;
            Col = Col > 0 ? Col : 0;
        }

        //=========================================================================
        // TEXT OUTPUT METHODS
        //=========================================================================

        /**
         * @brief Append a character array and move cursor right
         *
         * @tparam N Size of the character array
         * @param Buff Buffer to append to
         * @param L Character array to append
         */
        template <size_t N>
            requires (N > 0 && N < 6)
        inline void push_back(std::wstring& Buff, const wchar_t(&L)[N]) noexcept {
            Col += 1;
            Buff.append(L, N);
        }

        /**
         * @brief Append nulls and move cursor right
         *
         * @tparam N Size of the character array
         * @param Buff Buffer to append to
         * @param L Character array (unused)
         */
        template <size_t N>
            requires (N > 0 && N < 6)
        inline void push_back_null(std::wstring& Buff, const wchar_t(&L)[N]) noexcept {
            Col += 1;
            Buff.append(N, 0);
        }

        /**
         * @brief Append a wide character and move cursor right
         *
         * @param Buff Buffer to append to
         * @param wc Wide character to append
         */
        inline void push_back(std::wstring& Buff, int wc) noexcept {
            ++Col;
            Buff.push_back(static_cast<wchar_t>(wc));
        }

        /**
         * @brief Append a null-terminated wide string literal
         *
         * @tparam N Size of the string literal including null terminator
         * @param Buff Buffer to append to
         * @param L String literal to append
         */
        template <size_t N>
            requires (N > 0)
        inline void append(std::wstring& Buff, const wchar_t(&L)[N]) noexcept {
            Col += static_cast<short>(N - 1);
            Buff.append(L, N - 1);
        }

        /**
         * @brief Append a wide string
         *
         * @param Buff Buffer to append to
         * @param Msg String to append
         */
        inline void append(std::wstring& Buff, const std::wstring& Msg) noexcept {
            Buff.append(Msg);
            Col += static_cast<short>(Msg.size());
        }

        /**
         * @brief Append a wide string view
         *
         * @param Buff Buffer to append to
         * @param Msg String view to append
         */
        inline void append(std::wstring& Buff, std::wstring_view Msg) noexcept {
            Buff.append(Msg);
            Col += static_cast<short>(Msg.size());
        }

        /**
         * @brief Append part of a wide string
         *
         * @param Buff Buffer to append to
         * @param pcstr Pointer to string data
         * @param Length Number of characters to append
         */
        inline void append(std::wstring& Buff, const wchar_t* pcstr, size_t Length) {
            Buff.append(pcstr, Length);
            Col += static_cast<short>(Length);
        }

        /**
         * @brief Append multiple copies of a character
         *
         * @param bf Buffer to append to
         * @param Count Number of copies to append
         * @param c Character to append
         */
        inline void append(std::wstring& bf, size_t Count, wchar_t c) noexcept {
            Col += static_cast<short>(Count);
            bf.append(Count, c);
        }

        /**
         * @brief Conditionally append multiple copies of a character
         *
         * @tparam T Integer type
         * @param bf Buffer to append to
         * @param Count Number of copies to append
         * @param c Character to append
         */
        template <typename T>
            requires std::is_integral<T>::value
        inline void append_if(std::wstring& bf, T Count, wchar_t c) noexcept {
            if (Count > 0) {
                Col += static_cast<short>(Count);
                bf.append(static_cast<size_t>(Count), c);
            }
        }

        /**
         * @brief Apply this coordinate position to a buffer
         *
         * Appends cursor positioning command to the buffer.
         *
         * @param Buff Buffer to append command to
         */
        inline void apply(std::wstring& Buff) const noexcept {
            SetPos(Buff, Row + 1, Col + 1);  // Terminal positions are 1-based
        }

        /**
         * @brief Position cursor and reset tracking position
         *
         * Moves cursor to a specific location and resets this coordinate to (0,0).
         *
         * @param Buff Buffer to append command to
         * @param Loc Target location
         */
        inline void place(std::wstring& Buff, coord Loc) noexcept {
            SetPos(Buff, Loc.Row + 1, Loc.Col + 1);  // Terminal positions are 1-based
            Row = Col = 0;
        }

        /**
         * @brief Convert coordinate to string representation
         *
         * @return String representation (e.g., "(2,3)")
         */
        std::wstring string() const noexcept {
            return std::format(L"({},{})", Row, Col);
        }

    private:
        /**
         * @brief Move up multiple positions (internal implementation)
         *
         * @param Buff Buffer to append command to
         * @param Count Number of positions to move
         */
        inline void MoveUp_(std::wstring& Buff, int Count) noexcept {
            Row -= static_cast<short>(Count);
            MoveUp(Buff, Count);
        }

        /**
         * @brief Move down multiple positions (internal implementation)
         *
         * @param Buff Buffer to append command to
         * @param Count Number of positions to move
         */
        inline void MoveDown_(std::wstring& Buff, int Count) noexcept {
            Row += static_cast<short>(Count);
            MoveDown(Buff, Count);
        }

        /**
         * @brief Move left multiple positions (internal implementation)
         *
         * @param Buff Buffer to append command to
         * @param Count Number of positions to move
         */
        inline void MoveLeft_(std::wstring& Buff, int Count) noexcept {
            Col -= static_cast<short>(Count);
            MoveLeft(Buff, Count);
        }

        /**
         * @brief Move right multiple positions (internal implementation)
         *
         * @param Buff Buffer to append command to
         * @param Count Number of positions to move
         */
        inline void MoveRight_(std::wstring& Buff, int Count) noexcept {
            Col += static_cast<short>(Count);
            MoveRight(Buff, Count);
        }

        /**
         * @brief Get the coordinate with minimum values from two coordinates
         *
         * @param Lhs First coordinate
         * @param Rhs Second coordinate
         * @return Coordinate with minimum row and column values
         */
        friend constexpr coord Min(coord Lhs, coord Rhs) noexcept {
			short R = Lhs.Row < Rhs.Row ? Lhs.Row : Rhs.Row;
			short C = Lhs.Col < Rhs.Col ? Lhs.Col : Rhs.Col;
            return coord{ R, C };
        }

        /**
         * @brief Get the coordinate with maximum values from two coordinates
         *
         * @param Lhs First coordinate
         * @param Rhs Second coordinate
         * @return Coordinate with maximum row and column values
         */
        friend constexpr coord Max(coord Lhs, coord Rhs) noexcept {
			short R = Lhs.Row > Rhs.Row ? Lhs.Row : Rhs.Row;
			short C = Lhs.Col > Rhs.Col ? Lhs.Col : Rhs.Col;
            return coord{ R, C };
        }
    };

    /**
     * @class coord_box
     * @brief Represents a rectangular area in terminal coordinates
     *
     * This class defines a rectangular region using top-left and bottom-right
     * coordinates. It provides methods for calculating child boxes, intersections,
     * and various layout operations.
     */
    struct coord_box
    {
        /**
         * @brief Top-left corner coordinate
         */
        coord Top{ 1, 1 };

        /**
         * @brief Bottom-right corner coordinate
         */
        coord Bottom{ 1, 1 };

        /**
         * @brief Set the top-left corner position
         * @param TopPosition New top-left position
         */
        constexpr void set_top(coord TopPosition) noexcept {
            Top = TopPosition;
        }

        /**
         * @brief Set the box size while keeping the same top position
         * @param BoxSize Size of the box (rows, columns)
         */
        constexpr void set_size(coord BoxSize) noexcept {
            Bottom = Top + BoxSize - coord{ 1, 1 };
        }

        /**
         * @brief Set the box size while keeping the same top position
         * @param NumRows Number of rows
         * @param NumCols Number of columns
         */
        constexpr void set_size(int NumRows, int NumCols) noexcept {
            Bottom = Top + coord{ NumRows, NumCols } - coord{ 1, 1 };
        }

        /**
         * @brief Set the number of rows while keeping the same top position and columns
         * @param NumRows Number of rows
         */
        constexpr void set_rows(int NumRows) noexcept {
            Bottom.Row = Top.Row + NumRows - 1;
        }

        /**
         * @brief Set the number of columns while keeping the same top position and rows
         * @param NumCols Number of columns
         */
        constexpr void set_cols(int NumCols) noexcept {
            Bottom.Col = Top.Col + NumCols - 1;
        }

        /**
         * @brief Get the top-left corner position
         * @return Top-left coordinate
         */
        constexpr coord get_top() const noexcept {
            return Top;
        }

        /**
         * @brief Get the box size (rows, columns)
         * @return Size as a coordinate
         */
        constexpr coord get_size() const noexcept {
            return Bottom - Top + coord{ 1, 1 };
        }

        /**
         * @brief Get the bottom-right corner position
         * @return Bottom-right coordinate
         */
        constexpr coord get_bottom() const noexcept {
            return Bottom;
        }

        /**
         * @brief Get the number of rows in the box
         * @return Number of rows
         */
        constexpr int num_rows() const noexcept {
            return Bottom.Row - Top.Row + 1;
        }

        /**
         * @brief Get the number of columns in the box
         * @return Number of columns
         */
        constexpr int num_cols() const noexcept {
            return Bottom.Col - Top.Col + 1;
        }

        /**
         * @brief Check if this box is completely separated from another
         *
         * @param Rhs Box to check against
         * @return true if boxes don't intersect
         */
        constexpr bool disjoint(coord_box Rhs) const noexcept {
            return Top.Row > Rhs.Bottom.Row     // This box is below the other
                || Top.Col > Rhs.Bottom.Col     // This box is to the right of the other
                || Rhs.Top.Row > Bottom.Row     // Other box is below this one
                || Rhs.Top.Col > Bottom.Col;    // Other box is to the right of this one
        }

        /**
         * @brief Check if this box completely contains another
         *
         * @param Rhs Box to check against
         * @return true if this box contains the other
         */
        constexpr bool contains(coord_box Rhs) const noexcept {
            return Top.Row <= Rhs.Top.Row && Top.Col <= Rhs.Top.Col &&
                Bottom.Row >= Rhs.Bottom.Row && Bottom.Col >= Rhs.Bottom.Col;
        }

        /**
         * @brief Check if boxes partially intersect
         *
         * Boxes partially intersect if they have some overlap but
         * neither fully contains the other.
         *
         * @param Rhs Box to check against
         * @return true if boxes partially intersect
         */
        constexpr bool partially_intersects(coord_box Rhs) const noexcept {
            return !disjoint(Rhs) && !contains(Rhs) && !Rhs.contains(*this);
        }

        /**
         * @brief Calculate position to place a child box centered at the top
         *
         * @param ChildSize Size of the child box
         * @return Top-left position for the centered child
         */
        constexpr coord place_center_top(coord ChildSize) const noexcept {
            coord SizeDifference{ get_size() - ChildSize };
            return coord{ Top.Row, Top.Col + SizeDifference.Col / 2 };
        }

        /**
         * @brief Create a centered child box
         *
         * @param ChildSize Size of the child box
         * @return Centered child box
         */
        constexpr coord_box place_center(coord ChildSize) const noexcept {
            coord CenterTop = Top;
            coord Size = get_size();

            // Calculate centering offsets
            CenterTop.Row += (Size.Row - ChildSize.Row) / 2;
            CenterTop.Col += (Size.Col - ChildSize.Col) / 2;

            return coord_box{ CenterTop, CenterTop + ChildSize - coord{1, 1} };
        }

        /**
         * @brief Create a child box with specified offset from top-left
         *
         * @param ChildSize Size of the child box
         * @param Offset Offset from the top-left corner
         * @return Offset child box
         */
        constexpr coord_box place_offset(coord ChildSize, coord Offset) const noexcept {
            coord ChildTop{ Top + Offset };
            coord ChildBottom{ ChildTop + ChildSize - coord{1, 1} };
            return coord_box{ ChildTop, ChildBottom };
        }

        /**
         * @brief Normalize box to ensure valid dimensions
         *
         * Ensures the box has non-negative coordinates and meets
         * minimum size requirements.
         *
         * @param MinNumRows Minimum number of rows
         * @param MinNumCols Minimum number of columns
         */
        constexpr void normalize(int MinNumRows = 1, int MinNumCols = 1) {
            // Ensure non-negative coordinates
            Top.normalize();
            Bottom.normalize();

            // Apply minimum size constraints
            MinNumRows = MinNumRows > 0 ? MinNumRows : 0;
            MinNumCols = MinNumCols > 0 ? MinNumCols : 0;

            // Adjust bottom coordinate if needed
			short R = Top.Row + MinNumRows - 1;
			short C = Top.Col + MinNumCols - 1;
			Bottom.Row = R > Bottom.Row ? R : Bottom.Row;
			Bottom.Col = C > Bottom.Col ? C : Bottom.Col;
        }

        //=========================================================================
        // POSITION CALCULATION METHODS
        //=========================================================================

        /**
         * @brief Get the center point of the box
         * @return Center coordinate
         */
        constexpr coord center() const noexcept {
            return coord{ (Top.Row + Bottom.Row) / 2, (Top.Col + Bottom.Col) / 2 };
        }

        /**
         * @brief Get a point at the top edge centered horizontally
         * @param RowOffset Vertical offset from the top
         * @return Top-center coordinate
         */
        constexpr coord center_top(int RowOffset = 0) const noexcept {
            return coord{ Top.Row + RowOffset, (Top.Col + Bottom.Col) / 2 };
        }

        /**
         * @brief Get a point at the left edge centered vertically
         * @param ColumnOffset Horizontal offset from the left
         * @return Left-center coordinate
         */
        constexpr coord center_left(int ColumnOffset = 0) const noexcept {
            return coord{ (Top.Row + Bottom.Row) / 2, Top.Col + ColumnOffset };
        }

        /**
         * @brief Get a point at the right edge centered vertically
         * @param ColumnOffset Horizontal offset from the right
         * @return Right-center coordinate
         */
        constexpr coord center_right(int ColumnOffset = 0) const noexcept {
            return coord{ (Top.Row + Bottom.Row) / 2, Bottom.Col + ColumnOffset };
        }

        /**
         * @brief Get a point at the bottom edge centered horizontally
         * @param RowOffset Vertical offset from the bottom
         * @return Bottom-center coordinate
         */
        constexpr coord center_bottom(int RowOffset = 0) const noexcept {
            return coord{ Bottom.Row + RowOffset, (Top.Col + Bottom.Col) / 2 };
        }

        /**
         * @brief Get the top-right corner
         * @return Top-right coordinate
         */
        constexpr coord top_right() const noexcept {
            return coord{ Top.Row, Bottom.Col };
        }

        /**
         * @brief Get the bottom-left corner
         * @return Bottom-left coordinate
         */
        constexpr coord bottom_left() const noexcept {
            return coord{ Bottom.Row, Top.Col };
        }

        /**
         * @brief Move the box to a new top position
         *
         * Preserves the box size while changing its position.
         *
         * @param NewTop New top-left position
         */
        constexpr void move_top_to(coord NewTop) noexcept {
            coord Displacement = NewTop - Top;
            Bottom += Displacement;
            Top = NewTop;
        }

        /**
         * @brief Create a new box shifted by a displacement
         *
         * @param Displacement Amount to shift the box
         * @return Shifted box
         */
        constexpr coord_box shift(coord Displacement) const noexcept {
            return { Top + Displacement, Bottom + Displacement };
        }

        /**
         * @brief Create a new box shifted by row and column amounts
         *
         * @param NumRows Number of rows to shift
         * @param NumCols Number of columns to shift
         * @return Shifted box
         */
        constexpr coord_box shift(int NumRows, int NumCols) const noexcept {
            return shift(coord{ NumRows, NumCols });
        }

        /**
         * @brief Calculate the intersection of this box with another
         *
         * @param Rhs Box to intersect with
         * @return Box representing the intersection
         */
        constexpr coord_box intersect(coord_box Rhs) const noexcept {
            // Create a new box to avoid modifying Rhs
            short TR = Top.Row > Rhs.Top.Row ? Top.Row : Rhs.Top.Row;
            short TC = Top.Col > Rhs.Top.Col ? Top.Col : Rhs.Top.Col;
			short BR = Bottom.Row < Rhs.Bottom.Row ? Bottom.Row : Rhs.Bottom.Row;
			short BC = Bottom.Col < Rhs.Bottom.Col ? Bottom.Col : Rhs.Bottom.Col;
            return coord_box{
                coord { TR, TC},
				coord { BR, BC }
            };
        }

        //=========================================================================
        // CHILD BOX CREATION METHODS
        //=========================================================================

        /**
         * @brief Create a child box in the top-left corner
         *
         * @param Size Size of the child box
         * @return Top-left child box
         */
        constexpr coord_box top_left_child(coord Size) const noexcept {
            Size = Min(Size, get_size());
            return coord_box{ Top, Top + Size - coord{1, 1} };
        }

        /**
         * @brief Create a child box in the bottom-right corner
         *
         * @param Size Size of the child box
         * @return Bottom-right child box
         */
        constexpr coord_box bottom_right_child(coord Size) const noexcept {
            Size = Min(Size, get_size());
            return coord_box{ Bottom - Size + coord{1, 1}, Bottom };
        }

        /**
         * @brief Create a child box in the top-right corner
         *
         * @param Size Size of the child box
         * @return Top-right child box
         */
        constexpr coord_box top_right_child(coord Size) const noexcept {
            Size = Min(Size, get_size());
            return coord_box{
                coord{ Top.Row, Bottom.Col - Size.Col + 1 },
                coord{ Top.Row + Size.Row - 1, Bottom.Col } };
        }

        /**
         * @brief Create a child box in the bottom-left corner
         *
         * @param Size Size of the child box
         * @return Bottom-left child box
         */
        constexpr coord_box bottom_left_child(coord Size) const noexcept {
            Size = Min(Size, get_size());
            return coord_box{
                coord{ Bottom.Row - Size.Row + 1, Top.Col },
                coord{ Bottom.Row, Top.Col + Size.Col - 1 } };
        }

        //=========================================================================
        // SECTION EXTRACTION METHODS
        //=========================================================================

        /**
         * @brief Extract the top rows of the box
         *
         * @param NumRows Number of rows to include
         * @param LeftPad Number of columns to pad from left
         * @param RightPad Number of columns to pad from right
         * @return Box representing the top rows
         */
        constexpr coord_box top_rows(int NumRows, int LeftPad = 0, int RightPad = 0) const noexcept {
            return intersect(coord_box{
                coord{Top.Row, Top.Col + LeftPad},
                coord{Top.Row + NumRows - 1, Bottom.Col - RightPad} });
        }

        /**
         * @brief Extract the bottom rows of the box
         *
         * @param NumRows Number of rows to include
         * @param LeftPad Number of columns to pad from left
         * @param RightPad Number of columns to pad from right
         * @return Box representing the bottom rows
         */
        constexpr coord_box bottom_rows(int NumRows, int LeftPad = 0, int RightPad = 0) const noexcept {
            return intersect(coord_box{
                coord{Bottom.Row - NumRows + 1, Top.Col + LeftPad},
                coord{Bottom.Row, Bottom.Col - RightPad} });
        }

        /**
         * @brief Extract the left columns of the box
         *
         * @param NumColumns Number of columns to include
         * @param TopPad Number of rows to pad from top
         * @param BottomPad Number of rows to pad from bottom
         * @return Box representing the left columns
         */
        constexpr coord_box left_columns(int NumColumns, int TopPad = 0, int BottomPad = 0) const noexcept {
            return intersect(coord_box{
                coord{Top.Row + TopPad, Top.Col},
                coord{Bottom.Row - BottomPad, Top.Col + NumColumns - 1} });
        }

        /**
         * @brief Extract the right columns of the box
         *
         * @param NumColumns Number of columns to include
         * @param TopPad Number of rows to pad from top
         * @param BottomPad Number of rows to pad from bottom
         * @return Box representing the right columns
         */
        constexpr coord_box right_columns(int NumColumns, int TopPad = 0, int BottomPad = 0) const noexcept {
            return intersect(coord_box{
                coord{Top.Row + TopPad, Bottom.Col - NumColumns + 1},
                coord{Bottom.Row - BottomPad, Bottom.Col} });
        }

        /**
         * @brief Create a box with padding on top and bottom
         *
         * @param TopPad Number of rows to pad from top
         * @param BottomPad Number of rows to pad from bottom
         * @return Padded box
         */
        constexpr coord_box pad_rows(int TopPad, int BottomPad) const noexcept {
            return intersect(coord_box{
                coord{Top.Row + TopPad, Top.Col},
                coord{Bottom.Row - BottomPad, Bottom.Col}
                });
        }

        /**
         * @brief Create a box with padding on left and right
         *
         * @param LeftPad Number of columns to pad from left
         * @param RightPad Number of columns to pad from right
         * @return Padded box
         */
        constexpr coord_box pad_cols(int LeftPad, int RightPad) const noexcept {
            return intersect(coord_box{
                coord{Top.Row, Top.Col + LeftPad},
                coord{Bottom.Row, Bottom.Col - RightPad}
                });
        }

        /**
         * @brief Create a box with vertical padding
         *
         * @param TopOffset Number of rows to offset from top
         * @param BottomOffset Number of rows to offset from bottom
         * @return Vertically centered box
         */
        constexpr coord_box center_rows(int TopOffset, int BottomOffset) const noexcept {
            return coord_box{ Top + coord{TopOffset, 0}, Bottom - coord{BottomOffset, 0} };
        }

        /**
         * @brief Create a box with horizontal padding
         *
         * @param LeftOffset Number of columns to offset from left
         * @param RightOffset Number of columns to offset from right
         * @return Horizontally centered box
         */
        constexpr coord_box center_columns(int LeftOffset, int RightOffset) const noexcept {
            return coord_box{ Top + coord{0, LeftOffset}, Bottom - coord{0, RightOffset} };
        }

        /**
         * @brief Create a centered box with specific dimensions
         *
         * @param NumRows Number of rows for the centered box
         * @param NumColumns Number of columns for the centered box
         * @return Centered box
         */
        constexpr coord_box center_box(int NumRows, int NumColumns) const noexcept {
            coord t;
            t.Row = Top.Row + (Bottom.Row - Top.Row - NumRows + 1) / 2;
            t.Col = Top.Col + (Bottom.Col - Top.Col - NumColumns + 1) / 2;
            return coord_box{ t, t + coord{NumRows - 1, NumColumns - 1} };
        }

        /**
         * @brief Convert box to string representation
         * @return String representation (e.g., "[(1,1)-(5,10)]")
         */
        std::wstring string() const noexcept {
            return std::format(L"[({},{})-({},{})]", Top.Row, Top.Col, Bottom.Row, Bottom.Col);
        }

        /**
         * @brief Clear the area defined by this box
         *
         * Fills the box with spaces, effectively clearing it.
         *
         * @param bf Buffer to append commands to
         */
        void clear(std::wstring& bf) const noexcept {
            int NumRows{ num_rows() };
            int NumCols{ num_cols() };

            // Only proceed if box has valid dimensions
            if (NumRows <= 0 || NumCols <= 0) return;

            // Position cursor at the top-left corner
            Top.apply(bf);

            // Write spaces for each row
            for (int i = 0; i < NumRows - 1; i++) {
                bf.append(static_cast<size_t>(NumCols), ' ');
                MoveLeft(bf, NumCols);
                MoveDown(bf);
            }

            // Write spaces for the last row
            bf.append(static_cast<size_t>(NumCols), ' ');

            // Move cursor back to the end of the last row
            MoveLeft(bf, 1);
        }
    };

} // namespace mz

#endif // MZ_COORD_H

