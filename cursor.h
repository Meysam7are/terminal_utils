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

#ifndef MZ_CURSOR_H
#define MZ_CURSOR_H
#pragma once

/**
 * @file cursor.h
 * @brief Terminal cursor appearance and text formatting management
 *
 * This file provides utilities for controlling terminal cursor appearance,
 * text formatting attributes, and color settings. It enables fine-grained
 * control over terminal text display including shape, visibility, color,
 * and text attributes like bold and underline.
 *
 * The main class is `cursor`, which encapsulates a complete set of terminal
 * display attributes and provides methods to apply and update them.
 *
 * @author Meysam Zare
 */

#include "colors.h"
#include "coord.h"
#include "ConsoleCMD.h"
#include <cstdint>
#include <string>
#include <type_traits>
#include <stdio.h>

namespace mz {

    /**
     * @enum cursor_shape
     * @brief Defines possible cursor shapes in the terminal
     *
     * Different terminals may support different subsets of these cursor shapes.
     * Modern terminals typically support at least the block, underscore, and
     * vertical bar shapes, each in blinking and steady variants.
     */
    enum class cursor_shape : uint8_t
    {
        none = 0,    ///< Default cursor shape (terminal default)
        bblock = 1,  ///< Blinking block cursor
        sblock = 2,  ///< Steady block cursor
        bunder = 3,  ///< Blinking underscore cursor
        sunder = 4,  ///< Steady underscore cursor
        bbar = 5,    ///< Blinking vertical bar cursor
        sbar = 6,    ///< Steady vertical bar cursor
    };

    /**
     * @struct cursor_state
     * @brief Represents cursor and text attribute state
     *
     * This bit-packed structure efficiently stores cursor and text attribute
     * flags using minimal memory (single byte).
     */
    struct cursor_state {
        union {
            struct {
                cursor_shape SHAPE : 3; ///< Cursor shape (3 bits for 8 possible shapes)
                uint8_t BLINK : 1;     ///< Cursor blinking state (1=on, 0=off)
                uint8_t UNDER : 1;     ///< Text underline attribute (1=on, 0=off)
                uint8_t SHOW : 1;      ///< Cursor visibility (1=visible, 0=hidden)
                uint8_t BOLD : 1;      ///< Text bold attribute (1=on, 0=off)
                uint8_t NEG : 1;       ///< Text negative/inverted attribute (1=on, 0=off)
            };
            uint8_t cursor_state_value{ 0x00 };   ///< Combined state value (defaults to 0)
        };

        friend constexpr bool operator == (cursor_state L, cursor_state R) noexcept {
            return L.cursor_state_value == R.cursor_state_value;
        }

    };




    /**
     * @struct cursor_data
     * @brief Combined RGB color and cursor state information
     *
     * This structure efficiently packs RGB color information and cursor state
     * attributes into a single 32-bit value. It uses a union to allow access
     * to the data in different ways.
     */
    struct cursor_data {
        /**
         * @brief Union for accessing data in different forms
         *
         * Allows access to:
         * - Individual components (RGB values and state flags)
         * - Grouped state bits via cursor_state
         * - The complete 32-bit value
         */
        union {
            struct {
                uint8_t R;            ///< Red component (0-255)
                uint8_t G;            ///< Green component (0-255)
                uint8_t B;            ///< Blue component (0-255)
                cursor_shape SHAPE : 3; ///< Cursor shape
                uint8_t BLINK : 1;      ///< Cursor blink state
                uint8_t UNDER : 1;      ///< Text underline attribute
                uint8_t SHOW : 1;       ///< Cursor visibility
                uint8_t BOLD : 1;       ///< Text bold attribute
                uint8_t NEG : 1;        ///< Text negative/inverted attribute
            };
            struct {
                uint8_t : 8;          ///< Padding for alignment
                uint8_t : 8;          ///< Padding for alignment
                uint8_t : 8;          ///< Padding for alignment
                cursor_state State;   ///< Combined state flags
            };
            uint32_t value{ 0x00ffffff };  ///< Combined value (defaults to white)
        };

        /**
         * @brief Get the RGB portion of the value
         * @return 24-bit RGB value
         */
        constexpr uint32_t rgb_value() const noexcept {
            return value & 0x00ffffff;
        }

        /**
         * @brief Get the RGB color as an rgb object
         * @return rgb object representing the color
         */
        constexpr mz::rgb rgb() const noexcept {
            return mz::rgb{ R, G, B };
        }

        /**
         * @brief Alias for rgb()
         * @return rgb object representing the color
         */
        constexpr mz::rgb get() const noexcept {
            return mz::rgb{ R, G, B };
        }

        /**
         * @brief Set the RGB color
         * @param RGB New color value
         */
        constexpr void set(mz::rgb RGB) noexcept {
            R = RGB.r; G = RGB.g; B = RGB.b;
        }

        /**
         * @brief Update the RGB color and check if it changed
         *
         * @param RGB New color value
         * @return true if the color was changed, false if it was the same
         */
        constexpr bool update(mz::rgb RGB) noexcept {
            uint32_t RGB_VAL{ rgb_value() };
            set(RGB);
            return RGB_VAL != RGB.value();
        }

        /**
         * @brief Equality comparison operator
         * @param L First cursor_data to compare
         * @param R Second cursor_data to compare
         * @return true if data is identical
         */
        friend constexpr bool operator == (cursor_data L, cursor_data R) noexcept {
            return L.value == R.value;
        }

        /**
         * @brief Inequality comparison operator
         * @param L First cursor_data to compare
         * @param R Second cursor_data to compare
         * @return true if data is different
         */
        friend constexpr bool operator != (cursor_data L, cursor_data R) noexcept {
            return L.value != R.value;
        }

        /**
         * @brief Default constructor
         * Initializes to white with no attributes
         */
        constexpr cursor_data() noexcept = default;

        /**
         * @brief Constructor from integer value
         * @param Value 32-bit value containing RGB and state data
         */
        template <typename T>
        constexpr cursor_data(T Value) noexcept : value{ static_cast<uint32_t>(Value) } {
            static_assert(std::is_integral<T>::value, "Value must be an integral type");
        }

        /**
         * @brief Constructor from RGB color
         * @param RGB Color to use
         */
        constexpr cursor_data(mz::rgb RGB) noexcept : value{ RGB.RGB & 0x00ffffff } {};

        /**
         * @brief Constructor from cursor state
         * @param State Cursor state to use
         */
        constexpr cursor_data(cursor_state State) noexcept : State{ State } {};

        /**
         * @brief Constructor from RGB color and cursor state
         * @param RGB Color to use
         * @param State Cursor state to use
         */
        constexpr cursor_data(mz::rgb RGB, cursor_state State) noexcept : cursor_data{ RGB } {
            this->State = State;
        };
    };

    /**
     * @class cursor
     * @brief Complete terminal cursor and text attribute controller
     *
     * This class manages all aspects of terminal cursor appearance and text formatting,
     * including colors, cursor shape, visibility, and text attributes. It provides
     * methods to efficiently apply and update these settings.
     */
    class cursor
    {
    private:
        /**
         * @brief Foreground (text) color and attributes
         */
        cursor_data F{ 0x00ffffff }; // White text

        /**
         * @brief Background color
         */
        cursor_data B{ 0x00000000 }; // Black background

        /**
         * @brief Equality comparison operator
         * @param L First cursor to compare
         * @param R Second cursor to compare
         * @return true if cursors have identical settings
         */
        friend constexpr bool operator == (cursor L, cursor R) noexcept {
            return L.F == R.F && L.B == R.B;
        }

    public:
        /**
         * @brief Default constructor
         *
         * Creates a cursor with white text on black background,
         * default shape, and no text attributes.
         */
        constexpr cursor() noexcept = default;

        /**
         * @brief Constructor with foreground color
         *
         * Creates a cursor with the specified foreground color and
         * auto-calculated contrasting background.
         *
         * @param F Foreground color
         */
        constexpr cursor(rgb F) noexcept : F{ F }, B{ -F } {};

        /**
         * @brief Constructor with foreground and background colors
         *
         * @param F Foreground color
         * @param B Background color
         */
        constexpr cursor(rgb F, rgb B) noexcept : F{ F }, B{ B } {};

        /**
         * @brief Constructor from color pair
         *
         * @param Colors Color pair with foreground and background
         */
        constexpr cursor(color Colors) noexcept {
            F.set(Colors.F);
            B.set(Colors.B);
        }

        /**
         * @brief Constructor from foreground value
         *
         * @param FrontValue 32-bit value containing RGB and state data
         */
        constexpr cursor(uint32_t FrontValue) noexcept : F{ FrontValue } {};

        //=========================================================================
        // ATTRIBUTE GETTERS
        //=========================================================================

        /**
         * @brief Get the background color
         * @return Background color as rgb
         */
        constexpr rgb get_back_rgb() const noexcept {
            return B.rgb();
        }

        /**
         * @brief Get the foreground color
         * @return Foreground color as rgb
         */
        constexpr rgb get_front_rgb() const noexcept {
            return F.rgb();
        }

        /**
         * @brief Get both foreground and background colors
         * @return color object with both colors
         */
        constexpr color get_colors() const noexcept {
            return color{ F.get(), B.get() };
        }

        /**
         * @brief Get the cursor shape
         * @return Current cursor shape
         */
        constexpr cursor_shape get_shape() const noexcept {
            return F.SHAPE;
        }

        /**
         * @brief Get cursor blink state
         * @return true if blinking is enabled
         */
        constexpr bool get_blink() const noexcept {
            return F.BLINK;
        }

        /**
         * @brief Get text bold state
         * @return true if bold is enabled
         */
        constexpr bool get_bold() const noexcept {
            return F.BOLD;
        }

        /**
         * @brief Get cursor visibility
         * @return true if cursor is visible
         */
        constexpr bool get_show() const noexcept {
            return F.SHOW;
        }

        /**
         * @brief Get text negative/inverted state
         * @return true if text is displayed in negative mode
         */
        constexpr bool get_negative() const noexcept {
            return F.NEG;
        }

        /**
         * @brief Get text underline state
         * @return true if underline is enabled
         */
        constexpr bool get_under() const noexcept {
            return F.UNDER;
        }

        //=========================================================================
        // ATTRIBUTE SETTERS (IN-MEMORY ONLY)
        //=========================================================================

        /**
         * @brief Set the background color
         * @param BackRGB New background color
         */
        constexpr void set_back_rgb(rgb BackRGB) noexcept {
            B.set(BackRGB);
        }

        /**
         * @brief Set the foreground color
         * @param FrontRGB New foreground color
         */
        constexpr void set_front_rgb(rgb FrontRGB) noexcept {
            F.set(FrontRGB);
        }

        /**
         * @brief Set both foreground and background colors
         * @param Colors New color pair
         */
        constexpr void set_colors(color Colors) noexcept {
            F.set(Colors.F);
            B.set(Colors.B);
        }

        /**
         * @brief Set the cursor shape
         * @param Shape New cursor shape
         */
        constexpr void set_shape(cursor_shape Shape) noexcept {
            F.SHAPE = Shape;
        }

        /**
         * @brief Reset cursor shape to default
         */
        constexpr void reset_shape() noexcept {
            F.SHAPE = cursor_shape::none;
        }

        /**
         * @brief Enable cursor blinking
         */
        constexpr void set_blink() noexcept {
            F.BLINK = 1;
        }

        /**
         * @brief Disable cursor blinking
         */
        constexpr void reset_blink() noexcept {
            F.BLINK = 0;
        }

        /**
         * @brief Enable text bold attribute
         */
        constexpr void set_bold() noexcept {
            F.BOLD = 1;
        }

        /**
         * @brief Disable text bold attribute
         */
        constexpr void reset_bold() noexcept {
            F.BOLD = 0;
        }

        /**
         * @brief Make cursor visible
         */
        constexpr void set_show() noexcept {
            F.SHOW = 1;
        }

        /**
         * @brief Hide cursor
         */
        constexpr void set_hide() noexcept {
            F.SHOW = 0;
        }

        /**
         * @brief Enable negative/inverted text display
         */
        constexpr void set_negative() noexcept {
            F.NEG = 1;
        }

        /**
         * @brief Disable negative/inverted text display
         */
        constexpr void reset_negative() noexcept {
            F.NEG = 0;
        }

        /**
         * @brief Enable text underline attribute
         */
        constexpr void set_under() noexcept {
            F.UNDER = 1;
        }

        /**
         * @brief Disable text underline attribute
         */
        constexpr void reset_under() noexcept {
            F.UNDER = 0;
        }

        /**
         * @brief Reset all settings to defaults
         *
         * Sets white text on black background with no attributes.
         */
        constexpr void reset() noexcept {
            F.value = 0x00ffffff;  // White text
            B.value = 0;           // Black background
        }

        /**
         * @brief Swap foreground and background colors
         *
         * Exchanges the RGB components between foreground and background.
         */
        inline void mirror_colors() noexcept
        {
            uint8_t tempR = F.R, tempG = F.G, tempB = F.B;
            F.R = B.R; F.G = B.G; F.B = B.B;
            B.R = tempR; B.G = tempG; B.B = tempB;
        }

        //=========================================================================
        // ATTRIBUTE APPLICATION METHODS (OUTPUT TO BUFFER)
        //=========================================================================

        /**
         * @brief Apply cursor visibility setting to buffer
         * @param Buff Buffer to append command to
         */
        inline void apply_show(std::wstring& Buff) const noexcept {
            if (F.SHOW) {
                SetShow(Buff);
            }
            else {
                SetHide(Buff);
            }
        }

        /**
         * @brief Apply cursor blink setting to buffer
         * @param Buff Buffer to append command to
         */
        inline void apply_blink(std::wstring& Buff) const noexcept {
            if (F.BLINK) {
                SetBlink(Buff);
            }
            else {
                ClrBlink(Buff);
            }
        }

        /**
         * @brief Apply cursor shape setting to buffer
         * @param Buff Buffer to append command to
         */
        inline void apply_shape(std::wstring& Buff) const noexcept {
            SetShape(Buff, static_cast<int>(F.SHAPE));
        }

        /**
         * @brief Apply all cursor attributes to buffer
         * @param Buff Buffer to append command to
         */
        inline void apply_cursor(std::wstring& Buff) const noexcept {
            apply_show(Buff);
            apply_shape(Buff);
            apply_blink(Buff);
        }

        /**
         * @brief Apply bold attribute to buffer
         * @param Buff Buffer to append command to
         */
        inline void apply_bold(std::wstring& Buff) const noexcept {
            if (F.BOLD) {
                SetBold(Buff);
            }
            else {
                ClrBold(Buff);
            }
        }

        /**
         * @brief Apply negative/inverted attribute to buffer
         * @param Buff Buffer to append command to
         */
        inline void apply_negative(std::wstring& Buff) const noexcept {
            if (F.NEG) {
                SetNegative(Buff);
            }
            else {
                ClrNegative(Buff);
            }
        }

        /**
         * @brief Apply underline attribute to buffer
         * @param Buff Buffer to append command to
         */
        inline void apply_underline(std::wstring& Buff) const noexcept {
            if (F.UNDER) {
                SetUnderline(Buff);
            }
            else {
                ClrUnderline(Buff);
            }
        }

        /**
         * @brief Apply all text attributes to buffer
         * @param Buff Buffer to append command to
         */
        inline void apply_text(std::wstring& Buff) const noexcept {
            apply_bold(Buff);
            apply_negative(Buff);
            apply_underline(Buff);
        }

        /**
         * @brief Apply foreground and background colors to buffer
         * @param Buff Buffer to append command to
         */
        inline void apply_color(std::wstring& Buff) const noexcept {
            F.get().setFront(Buff);
            B.get().setBack(Buff);
        }

        /**
         * @brief Apply colors with foreground and background swapped
         * @param Buff Buffer to append command to
         */
        inline void apply_mirror(std::wstring& Buff) noexcept {
            mirror_colors();
            apply_color(Buff);
        }

        /**
         * @brief Apply all cursor settings, text attributes, and colors
         * @param Buff Buffer to append command to
         */
        void apply(std::wstring& Buff) const noexcept {
            apply_cursor(Buff);
            apply_text(Buff);
            apply_color(Buff);
        }

        //=========================================================================
        // COLOR MANIPULATION METHODS
        //=========================================================================

        /**
         * @brief Blend foreground and background colors
         *
         * Creates a smoother transition between foreground and background.
         *
         * @param Percentage Blend amount (0-100)
         */
        void blend_colors(int Percentage) noexcept {
            set_colors(get_colors().blend(Percentage));
        }

        /**
         * @brief Set colors based on a base color and contrast
         *
         * Creates a foreground/background pair from a single base color,
         * with foreground being a lighter version and background darker.
         *
         * @param RGB Base color
         * @param ContrastPercentage Amount of contrast between foreground and background
         */
        void set_color_contrast(mz::rgb RGB, int ContrastPercentage) noexcept {
            set_front_rgb(RGB.brighten(ContrastPercentage));
            set_back_rgb(RGB.darken(ContrastPercentage));
        }

        //=========================================================================
        // COMBINED SET AND APPLY METHODS
        //=========================================================================

        /**
         * @brief Make cursor visible and apply to buffer
         * @param Buff Buffer to append command to
         */
        inline void set_show(std::wstring& Buff) noexcept {
            F.SHOW = 1;
            SetShow(Buff);
        }

        /**
         * @brief Hide cursor and apply to buffer
         * @param Buff Buffer to append command to
         */
        inline void set_hide(std::wstring& Buff) noexcept {
            F.SHOW = 0;
            SetHide(Buff);
        }

        /**
         * @brief Enable cursor blinking and apply to buffer
         * @param Buff Buffer to append command to
         */
        inline void set_blink(std::wstring& Buff) noexcept {
            F.BLINK = 1;
            SetBlink(Buff);
        }

        /**
         * @brief Disable cursor blinking and apply to buffer
         * @param Buff Buffer to append command to
         */
        inline void set_unblink(std::wstring& Buff) noexcept {
            F.BLINK = 0;
            ClrBlink(Buff);
        }

        /**
         * @brief Reset cursor shape to default and apply to buffer
         * @param Buff Buffer to append command to
         */
        inline void set_shape(std::wstring& Buff) noexcept {
            F.SHAPE = cursor_shape::none;
            ResetShape(Buff);
        }

        /**
         * @brief Set cursor shape and apply to buffer
         * @param Buff Buffer to append command to
         * @param Shape New cursor shape
         */
        inline void set_shape(std::wstring& Buff, cursor_shape Shape) noexcept {
            F.SHAPE = Shape;
            SetShape(Buff, static_cast<int>(Shape));
        }

        /**
         * @brief Enable bold text and apply to buffer
         * @param Buff Buffer to append command to
         */
        inline void set_bold(std::wstring& Buff) noexcept {
            F.BOLD = 1;
            SetBold(Buff);
        }

        /**
         * @brief Enable underline text and apply to buffer
         * @param Buff Buffer to append command to
         */
        inline void set_under(std::wstring& Buff) noexcept {
            F.UNDER = 1;
            SetUnderline(Buff);
        }

        /**
         * @brief Enable negative/inverted text and apply to buffer
         * @param Buff Buffer to append command to
         */
        inline void set_negative(std::wstring& Buff) noexcept {
            F.NEG = 1;
            SetNegative(Buff);
        }

        /**
         * @brief Disable bold text and apply to buffer
         * @param Buff Buffer to append command to
         */
        inline void set_unbold(std::wstring& Buff) noexcept {
            F.BOLD = 0;
            ClrBold(Buff);
        }

        /**
         * @brief Disable underline text and apply to buffer
         * @param Buff Buffer to append command to
         */
        inline void set_nounder(std::wstring& Buff) noexcept {
            F.UNDER = 0;
            ClrUnderline(Buff);
        }

        /**
         * @brief Disable negative/inverted text and apply to buffer
         * @param Buff Buffer to append command to
         */
        inline void set_positive(std::wstring& Buff) noexcept {
            F.NEG = 0;
            ClrNegative(Buff);
        }

        /**
         * @brief Set text inversion based on boolean and apply to buffer
         * @param Buff Buffer to append command to
         * @param Negative Whether to enable negative mode
         * @return The Negative parameter (for chaining)
         */
        inline bool set_inversion(std::wstring& Buff, bool Negative) noexcept {
            if (Negative) {
                set_negative(Buff);
            }
            else {
                set_positive(Buff);
            }
            return Negative;
        }

        /**
         * @brief Set background color and apply to buffer
         * @param Buff Buffer to append command to
         * @param RGB New background color
         */
        inline void set_back_rgb(std::wstring& Buff, rgb RGB) noexcept {
            B.set(RGB);
            SetBackColor(Buff, B.R, B.G, B.B);
        }

        /**
         * @brief Set foreground color and apply to buffer
         * @param Buff Buffer to append command to
         * @param RGB New foreground color
         */
        inline void set_front_rgb(std::wstring& Buff, rgb RGB) noexcept {
            F.set(RGB);
            SetFrontColor(Buff, F.R, F.G, F.B);
        }

        /**
         * @brief Set both colors and apply to buffer
         * @param Buff Buffer to append command to
         * @param COLOR New color pair
         */
        inline void set_colors(std::wstring& Buff, mz::color COLOR) noexcept {
            set_back_rgb(Buff, COLOR.B);
            set_front_rgb(Buff, COLOR.F);
        }

        /**
         * @brief Set swapped colors and apply to buffer
         * @param Buff Buffer to append command to
         * @param COLOR New color pair (will be swapped)
         */
        inline void set_mirror_colors(std::wstring& Buff, mz::color COLOR) noexcept {
            set_back_rgb(Buff, COLOR.F);
            set_front_rgb(Buff, COLOR.B);
        }

        /**
         * @brief Set negated colors and apply to buffer
         * @param Buff Buffer to append command to
         * @param COLOR New color pair (will be negated)
         */
        inline void set_negative_colors(std::wstring& Buff, mz::color COLOR) noexcept {
            set_back_rgb(Buff, -COLOR.B);
            set_front_rgb(Buff, -COLOR.F);
        }

        /**
         * @brief Copy all settings from another cursor and apply to buffer
         * @param Buff Buffer to append command to
         * @param Rhs Source cursor to copy from
         */
        inline void set_to(std::wstring& Buff, cursor Rhs) noexcept {
            F = Rhs.F;
            B = Rhs.B;
            apply(Buff);
        }

        //=========================================================================
        // CONDITIONAL UPDATE METHODS
        //=========================================================================

        /**
         * @brief Make cursor visible if currently hidden
         * @param Buff Buffer to append command to
         */
        inline void update_show(std::wstring& Buff) noexcept {
            if (!F.SHOW) {
                set_show(Buff);
            }
        }

        /**
         * @brief Hide cursor if currently visible
         * @param Buff Buffer to append command to
         */
        inline void update_hide(std::wstring& Buff) noexcept {
            if (F.SHOW) {
                set_hide(Buff);
            }
        }

        /**
         * @brief Enable blinking if currently disabled
         * @param Buff Buffer to append command to
         */
        inline void update_blink(std::wstring& Buff) noexcept {
            if (!F.BLINK) {
                set_blink(Buff);
            }
        }

        /**
         * @brief Disable blinking if currently enabled
         * @param Buff Buffer to append command to
         */
        inline void update_unblink(std::wstring& Buff) noexcept {
            if (F.BLINK) {
                set_unblink(Buff);
            }
        }

        /**
         * @brief Reset shape if not already default
         * @param Buff Buffer to append command to
         */
        inline void update_shape(std::wstring& Buff) noexcept {
            if (F.SHAPE != cursor_shape::none) {
                set_shape(Buff);
            }
        }

        /**
         * @brief Update shape if different from target
         * @param Buff Buffer to append command to
         * @param Shape Target cursor shape
         */
        inline void update_shape(std::wstring& Buff, cursor_shape Shape) noexcept {
            if (F.SHAPE != Shape) {
                set_shape(Buff, Shape);
            }
        }

        /**
         * @brief Enable bold if currently disabled
         * @param Buff Buffer to append command to
         */
        inline void update_bold(std::wstring& Buff) noexcept {
            if (!F.BOLD) {
                set_bold(Buff);
            }
        }

        /**
         * @brief Enable underline if currently disabled
         * @param Buff Buffer to append command to
         */
        inline void update_under(std::wstring& Buff) noexcept {
            if (!F.UNDER) {
                set_under(Buff);
            }
        }

        /**
         * @brief Enable negative mode if currently disabled
         * @param Buff Buffer to append command to
         */
        inline void update_negative(std::wstring& Buff) noexcept {
            if (!F.NEG) {
                set_negative(Buff);
            }
        }

        /**
         * @brief Disable bold if currently enabled
         * @param Buff Buffer to append command to
         */
        inline void update_unbold(std::wstring& Buff) noexcept {
            if (F.BOLD) {
                set_unbold(Buff);
            }
        }

        /**
         * @brief Disable underline if currently enabled
         * @param Buff Buffer to append command to
         */
        inline void update_nounder(std::wstring& Buff) noexcept {
            if (F.UNDER) {
                set_nounder(Buff);
            }
        }

        /**
         * @brief Disable negative mode if currently enabled
         * @param Buff Buffer to append command to
         */
        inline void update_positive(std::wstring& Buff) noexcept {
            if (F.NEG) {
                set_positive(Buff);
            }
        }

        /**
         * @brief Update inversion state if different from target
         * @param Buff Buffer to append command to
         * @param Negative Target negative mode state
         * @return The Negative parameter (for chaining)
         */
        inline bool update_inversion(std::wstring& Buff, bool Negative) noexcept {
            if (Negative) {
                update_negative(Buff);
            }
            else {
                update_positive(Buff);
            }
            return Negative;
        }

        /**
         * @brief Update background color if different from target
         * @param Buff Buffer to append command to
         * @param RGB Target background color
         */
        inline void update_back_rgb(std::wstring& Buff, rgb RGB) noexcept {
            if (B.update(RGB)) {
                SetBackColor(Buff, B.R, B.G, B.B);
            }
        }

        /**
         * @brief Update foreground color if different from target
         * @param Buff Buffer to append command to
         * @param RGB Target foreground color
         */
        inline void update_front_rgb(std::wstring& Buff, rgb RGB) noexcept {
            if (F.update(RGB)) {
                SetFrontColor(Buff, F.R, F.G, F.B);
            }
        }

        /**
         * @brief Update both colors if different from targets
         * @param Buff Buffer to append command to
         * @param COLOR Target color pair
         */
        inline void update_colors(std::wstring& Buff, mz::color COLOR) noexcept {
            update_back_rgb(Buff, COLOR.B);
            update_front_rgb(Buff, COLOR.F);
        }

        /**
         * @brief Update with swapped colors if different from targets
         * @param Buff Buffer to append command to
         * @param COLOR Target color pair (will be swapped)
         */
        inline void update_mirror(std::wstring& Buff, mz::color COLOR) noexcept {
            update_back_rgb(Buff, COLOR.F);
            update_front_rgb(Buff, COLOR.B);
        }

        /**
         * @brief Update with negated colors if different from targets
         * @param Buff Buffer to append command to
         * @param COLOR Target color pair (will be negated)
         */
        inline void update_negative(std::wstring& Buff, mz::color COLOR) noexcept {
            update_back_rgb(Buff, -COLOR.B);
            update_front_rgb(Buff, -COLOR.F);
        }

        /**
         * @brief Update color settings if different from targets
         * @param Buff Buffer to append command to
         * @param Color Target color pair
         */
        inline void update_color(std::wstring& Buff, color Color) noexcept
        {
            update_back_rgb(Buff, Color.B);
            update_front_rgb(Buff, Color.F);
        }

        /**
         * @brief Update all cursor settings if different from target
         *
         * This method efficiently updates only the attributes that differ
         * between the current cursor state and the target state.
         *
         * @param Buff Buffer to append command to
         * @param Rhs Target cursor state
         */
        inline void update_to(std::wstring& Buff, cursor Rhs) noexcept
        {
            // Update colors first (most common change)
            update_back_rgb(Buff, Rhs.B.rgb());
            update_front_rgb(Buff, Rhs.F.rgb());

            // If foreground state is different, check each attribute
            if (F.State != Rhs.F.State) {
                if (F.SHAPE != Rhs.F.SHAPE) {
                    F.SHAPE = Rhs.F.SHAPE;
                    apply_shape(Buff);
                }
                if (F.SHOW != Rhs.F.SHOW) {
                    F.SHOW = Rhs.F.SHOW;
                    apply_show(Buff);
                }
                if (F.BLINK != Rhs.F.BLINK) {
                    F.BLINK = Rhs.F.BLINK;
                    apply_blink(Buff);
                }
                if (F.NEG != Rhs.F.NEG) {
                    F.NEG = Rhs.F.NEG;
                    apply_negative(Buff);
                }
                if (F.BOLD != Rhs.F.BOLD) {
                    F.BOLD = Rhs.F.BOLD;
                    apply_bold(Buff);
                }
                if (F.UNDER != Rhs.F.UNDER) {
                    F.UNDER = Rhs.F.UNDER;
                    apply_underline(Buff);
                }
            }
        }

        //=========================================================================
        // DEBUGGING METHODS
        //=========================================================================

        /**
         * @brief Print cursor state to stdout for debugging
         *
         * Outputs both text attributes and color values.
         */
        void report() const noexcept
        {
            // Print text attributes
            printf("[%d,%d,%d:%d,%d] ",
                static_cast<int>(F.BOLD),
                static_cast<int>(F.UNDER),
                static_cast<int>(F.NEG),
                static_cast<int>(F.SHOW),
                static_cast<int>(F.SHAPE));

            // Print color values
            printf("[%x,%x,%x:%x,%x,%x] ",
                F.R, F.G, F.B, B.R, B.G, B.B);
        }

        /**
         * @brief Print cursor state at specific screen position
         *
         * @param Row Screen row for output
         * @param Col Screen column for output
         */
        void report(int Row, int Col) const noexcept
        {
            // Save current position, move to target, print, and restore
            SavePos();
            MoveTo(Row, Col);
            report();
            LoadPos();
        }
    };

} // namespace mz

#endif // MZ_CURSOR_H


