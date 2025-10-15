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

#ifndef MZ_COLORS_H
#define MZ_COLORS_H
#pragma once

/**
 * @file colors.h
 * @brief RGB color manipulation and terminal color utilities
 *
 * This file provides classes and functions for working with RGB colors,
 * color manipulation (blending, mixing, etc.), and terminal color output.
 * It includes cross-platform terminal color support and a comprehensive
 * set of predefined colors.
 *
 * @author Meysam Zare
 */

#include "ConsoleCMD.h"
#include <cstdint>
#include <concepts>
#include <string>
#include <format>

namespace mz {

    /**
     * @class rgb
     * @brief RGB color representation and manipulation
     *
     * This class represents a 24-bit RGB color and provides methods for color
     * manipulation, blending, mixing, and conversion. It supports various
     * operations like brightening, darkening, negation, and interpolation.
     */
    struct rgb {
    private:
        /**
         * @brief Calculate average of two color components
         * @param l First component value
         * @param r Second component value
         * @return Average value (l+r)/2
         */
        static constexpr unsigned avg(unsigned l, unsigned r) noexcept {
            return (l + r) / 2;
        }

        /**
         * @brief Calculate subtraction with averaging
         * @param l First component value
         * @param r Second component value
         * @return Averaged subtraction
         */
        static constexpr unsigned sub(unsigned l, unsigned r) noexcept {
            return avg(l, r ^ 0xff);
        }

        /**
         * @brief Calculate percentage multiplication with bounds checking
         * @param l Component value
         * @param r Percentage (0-100)
         * @return Scaled value capped at 255
         */
        static constexpr unsigned mul(unsigned l, unsigned r) noexcept {
            unsigned x{ l * r };
            return x > 25500 ? 255 : (x / 100);
        }

    public:
        /**
         * @brief Union for accessing color components or combined RGB value
         *
         * Provides both individual RGB component access and combined 32-bit value
         * access to the same underlying data.
         */
        union {
            struct {
                uint8_t r;  ///< Red component (0-255)
                uint8_t g;  ///< Green component (0-255)
                uint8_t b;  ///< Blue component (0-255)
            };
            uint32_t RGB{ 0 };  ///< Combined RGB value (low 24 bits used)
        };

        /**
         * @brief Default constructor
         *
         * Initializes to black (0,0,0)
         */
        constexpr rgb() noexcept = default;

        /**
         * @brief Constructor from integer value
         *
         * Constructs an RGB color from a 24-bit integer value.
         *
         * @tparam T Integer type
         * @param X Value containing RGB data (low 24 bits used)
         */
        template <typename T>
            requires std::integral<T>
        constexpr rgb(T X) noexcept : RGB{ static_cast<uint32_t>(X) & 0xffffff } {}

        /**
         * @brief Assignment operator
         *
         * @param Rhs Source color to copy from
         * @return Reference to this object
         */
        constexpr rgb& operator = (rgb Rhs) noexcept {
            r = Rhs.r; g = Rhs.g; b = Rhs.b;
            return *this;
        }

        /**
         * @brief Constructor from individual RGB components
         *
         * @tparam TR Red component type
         * @tparam TG Green component type
         * @tparam TB Blue component type
         * @param r Red component value (0-255)
         * @param g Green component value (0-255)
         * @param b Blue component value (0-255)
         */
        template <typename TR, typename TG, typename TB>
            requires std::integral<TR>&& std::integral<TG>&& std::integral<TB>
        constexpr rgb(TR r, TG g, TB b) noexcept :
            r{ static_cast<uint8_t>(r) },
            g{ static_cast<uint8_t>(g) },
            b{ static_cast<uint8_t>(b) } {
        }

        /**
         * @brief Get combined RGB value
         * @return 24-bit RGB color value
         */
        constexpr uint32_t value() const noexcept {
            return RGB & 0x00ffffff;
        }

        /**
         * @brief Calculate weighted average of two colors
         *
         * @param L First color
         * @param R Second color
         * @param Weight Weight of first color (0-100)
         * @return Weighted average color
         */
        static constexpr rgb Avg(rgb L, rgb R, int Weight = 50) noexcept {
            // Ensure weight is in valid range
            Weight = (Weight < 0) ? 0 : (Weight > 100) ? 100 : Weight;

            return rgb{
                static_cast<uint8_t>((L.r * Weight + R.r * (100 - Weight)) / 100),
                static_cast<uint8_t>((L.g * Weight + R.g * (100 - Weight)) / 100),
                static_cast<uint8_t>((L.b * Weight + R.b * (100 - Weight)) / 100)
            };
        }

        /**
         * @brief Calculate weighted average of two colors (256-scale)
         *
         * Similar to Avg but uses 256-scale weights for more precise blending
         *
         * @param L First color
         * @param R Second color
         * @param Weight Weight of first color (0-256)
         * @return Weighted average color
         */
        static constexpr rgb Avg256(rgb L, rgb R, int Weight = 128) noexcept {
            // Ensure weight is in valid range
            Weight = (Weight < 0) ? 0 : (Weight > 256) ? 256 : Weight;

            return rgb{
                static_cast<uint8_t>((L.r * Weight + R.r * (256 - Weight)) / 256),
                static_cast<uint8_t>((L.g * Weight + R.g * (256 - Weight)) / 256),
                static_cast<uint8_t>((L.b * Weight + R.b * (256 - Weight)) / 256)
            };
        }

        /**
         * @brief Mix this color with another color
         *
         * @param Rhs Color to mix with
         * @param RhsPercentage Percentage of Rhs in the result (0-100)
         * @return Mixed color
         */
        constexpr rgb mix(rgb Rhs, int RhsPercentage) const noexcept {
            return Avg(Rhs, *this, RhsPercentage);
        }

        /**
         * @brief Create darker or lighter version of this color
         *
         * If BlackPercentage is positive, the color is darkened by mixing with black.
         * If BlackPercentage is negative, the color is lightened by mixing with white.
         *
         * @param BlackPercentage Amount of darkening/lightening (positive darkens, negative lightens)
         * @return Modified color
         */
        constexpr rgb darken(int BlackPercentage) const noexcept {
            return BlackPercentage > 0
                ? Avg(rgb{}, *this, BlackPercentage)  // Mix with black
                : Avg(rgb{ 255,255,255 }, *this, -BlackPercentage);  // Mix with white
        }

        /**
         * @brief Create lighter or darker version of this color
         *
         * If WhitePercentage is positive, the color is lightened by mixing with white.
         * If WhitePercentage is negative, the color is darkened by mixing with black.
         *
         * @param WhitePercentage Amount of lightening/darkening (positive lightens, negative darkens)
         * @return Modified color
         */
        constexpr rgb brighten(int WhitePercentage) const noexcept {
            return WhitePercentage > 0
                ? Avg(rgb{ 255,255,255 }, *this, WhitePercentage)  // Mix with white
                : Avg(rgb{}, *this, -WhitePercentage);  // Mix with black
        }

        /**
         * @brief Mix this color with another color (256-scale)
         *
         * @param Rhs Color to mix with
         * @param RhsShare256 Amount of Rhs in the result (0-256)
         * @return Mixed color
         */
        constexpr rgb mix256(rgb Rhs, int RhsShare256) const noexcept {
            return Avg256(Rhs, *this, RhsShare256);
        }

        /**
         * @brief Create darker or lighter version of this color (256-scale)
         *
         * @param BlackShare256 Amount of darkening (0-256)
         * @return Modified color
         */
        constexpr rgb darken256(int BlackShare256) const noexcept {
            return Avg256(rgb{}, *this, BlackShare256);
        }

        /**
         * @brief Create lighter version of this color (256-scale)
         *
         * @param WhiteShare256 Amount of lightening (0-256)
         * @return Modified color
         */
        constexpr rgb brighten256(int WhiteShare256) const noexcept {
            return Avg256(rgb{ 255,255,255 }, *this, WhiteShare256);
        }

        /**
         * @brief Create a gray color with specified intensity
         *
         * @tparam T Integer type
         * @param X Gray intensity (0=black, 255=white)
         * @return Gray color
         */
        template <typename T>
            requires std::integral<T>
        static constexpr rgb gray(T X) noexcept {
            return rgb{ static_cast<uint8_t>(X),
                       static_cast<uint8_t>(X),
                       static_cast<uint8_t>(X) };
        }

        /**
         * @brief Create negative/inverse of this color
         *
         * @return Color with inverted RGB components
         */
        constexpr rgb neg() const noexcept {
            return rgb{ static_cast<uint32_t>(RGB ^ 0xffffff) };
        }

        /**
         * @brief Negation operator (alias for neg())
         * @return Negated color
         */
        constexpr rgb operator - () const noexcept {
            return neg();
        }

        /**
         * @brief Equality comparison operator
         *
         * @param L First color
         * @param R Second color
         * @return true if colors are equal, false otherwise
         */
        friend constexpr bool operator == (rgb L, rgb R) noexcept {
            return L.r == R.r && L.g == R.g && L.b == R.b;
        }

        /**
         * @brief Create negative of a color
         *
         * @param RGB Color to negate
         * @return Negated color
         */
        friend constexpr rgb negative(rgb RGB) noexcept {
            return rgb{ static_cast<uint32_t>(RGB.RGB ^ 0xffffff) };
        }

        /**
         * @brief Addition operator (component-wise average)
         *
         * @param L First color
         * @param R Second color
         * @return Average of the two colors
         */
        friend constexpr rgb operator + (rgb L, rgb R) noexcept {
            return rgb{ avg(L.r, R.r), avg(L.g, R.g), avg(L.b, R.b) };
        }

        /**
         * @brief Subtraction operator (component-wise subtraction)
         *
         * @param L First color
         * @param R Second color
         * @return Subtracted color
         */
        friend constexpr rgb operator - (rgb L, rgb R) noexcept {
            return rgb{ sub(L.r, R.r), sub(L.g, R.g), sub(L.b, R.b) };
        }

        /**
         * @brief Multiplication operator (scale color)
         *
         * @param L Color to scale
         * @param x Scaling factor (percentage)
         * @return Scaled color
         */
        friend constexpr rgb operator * (rgb L, int x) noexcept {
            L = x >= 0 ? L : -L;
            x = x >= 0 ? x : -x;
            return rgb{ mul(L.r, x), mul(L.g, x), mul(L.b, x) };
        }

        /**
         * @brief Set background color in terminal string
         *
         * Appends ANSI sequence for setting background color to the string
         *
         * @param Buf String buffer to append to
         */
        void setBack(std::wstring& Buf) const noexcept {
            SetBackColor(Buf, r, g, b);
        }

        /**
         * @brief Set foreground color in terminal string
         *
         * Appends ANSI sequence for setting foreground color to the string
         *
         * @param Buf String buffer to append to
         */
        void setFront(std::wstring& Buf) const noexcept {
            SetFrontColor(Buf, r, g, b);
        }

        /**
         * @brief Set background color directly to terminal
         */
        void setBack() const noexcept {
            mz::Write(std::format(L"\x1b[48;2;{:0>3d};{:0>3d};{:0>3d}m", r, g, b));
        }

        /**
         * @brief Set foreground color directly to terminal
         */
        void setFront() const noexcept {
            mz::Write(std::format(L"\x1b[38;2;{:0>3d};{:0>3d};{:0>3d}m", r, g, b));
        }
    };

    /**
     * @class color
     * @brief Color pair for terminal foreground and background
     *
     * This class represents a foreground and background color pair for terminal output.
     * It provides methods for manipulating and applying color combinations.
     */
    struct color {
        //=========================================================================
        // STANDARD COLOR CONSTANTS
        //=========================================================================

        // Basic colors
        static constexpr rgb BLACK{ 0, 0, 0 };
        static constexpr rgb GRAY{ 128, 128, 128 };
        static constexpr rgb SILVER{ 192, 192, 192 };
        static constexpr rgb WHITE{ 255, 255, 255 };

        // Red colors
        static constexpr rgb MAROON{ 128, 0, 0 };
        static constexpr rgb DARKRED{ 139, 0, 0 };
        static constexpr rgb BROWN{ 165, 42, 42 };
        static constexpr rgb ORANGE{ 255, 165, 0 };
        static constexpr rgb GOLD{ 255, 215, 0 };
        static constexpr rgb RED{ 255, 0, 0 };

        // Green colors
        static constexpr rgb LIME{ 0, 255, 0 };
        static constexpr rgb GREEN{ 0, 128, 0 };
        static constexpr rgb OLIVE{ 128, 128, 0 };
        static constexpr rgb SEAGREEN{ 46, 139, 87 };
        static constexpr rgb DARKGREEN{ 0, 100, 0 };

        // Blue colors
        static constexpr rgb BLUE{ 0, 0, 255 };
        static constexpr rgb NAVY{ 0, 0, 128 };
        static constexpr rgb DARKBLUE{ 0, 0, 139 };

        // Other colors
        static constexpr rgb YELLOW{ 255, 255, 0 };
        static constexpr rgb MAGENTA{ 255, 0, 255 };
        static constexpr rgb CYAN{ 0, 255, 255 };
        static constexpr rgb AQUA{ 0, 255, 255 };

        // Special colors
        static constexpr rgb KHAKI{ 240, 230, 140 };
        static constexpr rgb LAVENDER{ 230, 230, 250 };
        static constexpr rgb LIGHTCYAN{ 224, 255, 255 };
        static constexpr rgb BRIGHTGREEN{ 22, 198, 12 };
        static constexpr rgb BRIGHTYELLOW{ 249, 241, 165 };
        static constexpr rgb SIENNA{ 160, 82, 45 };

        /**
         * @brief Foreground color
         */
        rgb F{ color::WHITE };

        /**
         * @brief Background color
         */
        rgb B{ color::BLACK };

        /**
         * @brief Default constructor
         *
         * Initializes with white foreground and black background
         */
        constexpr color() noexcept = default;

        /**
         * @brief Constructor with specified foreground and background
         *
         * @param F Foreground color
         * @param B Background color
         */
        constexpr color(rgb F, rgb B) noexcept : F{ F }, B{ B } {}

        /**
         * @brief Constructor with automatic contrast generation
         *
         * Creates a color pair based on a single color and a contrast level.
         * The foreground is darkened and the background is lightened
         * (or vice versa for negative contrast).
         *
         * @param C Base color
         * @param Contrast Contrast level (-100 to 100)
         */
        constexpr color(rgb C, int Contrast) noexcept :
            F{ C.darken(Contrast) },
            B{ C.brighten(Contrast) } {
        }

        /**
         * @brief Create a color with swapped foreground and background
         * @return Color with F and B swapped
         */
        constexpr color mirror() const noexcept {
            return color{ B, F };
        }

        /**
         * @brief Apply colors to a string buffer
         *
         * Appends ANSI sequences to set foreground and background colors
         *
         * @param Buffer String buffer to append to
         */
        void apply(std::wstring& Buffer) const noexcept {
            F.setFront(Buffer);
            B.setBack(Buffer);
        }

        /**
         * @brief Apply swapped colors to a string buffer
         *
         * Appends ANSI sequences with foreground and background swapped
         *
         * @param Buffer String buffer to append to
         */
        void apply_mirror(std::wstring& Buffer) const noexcept {
            F.setBack(Buffer);
            B.setFront(Buffer);
        }

        /**
         * @brief Apply negative colors to a string buffer
         *
         * Appends ANSI sequences with both colors negated
         *
         * @param Buffer String buffer to append to
         */
        void apply_negative(std::wstring& Buffer) const noexcept {
            (-F).setFront(Buffer);
            (-B).setBack(Buffer);
        }

        /**
         * @brief Apply colors directly to terminal output
         */
        void apply() const noexcept {
            F.setFront();
            B.setBack();
        }

        /**
         * @brief Apply swapped colors directly to terminal output
         */
        void apply_mirror() const noexcept {
            F.setBack();
            B.setFront();
        }

        /**
         * @brief Apply negative colors directly to terminal output
         */
        void apply_negative() const noexcept {
            (-F).setFront();
            (-B).setBack();
        }

        /**
         * @brief Create a blended color pair
         *
         * Creates a color pair where foreground and background
         * are blended toward each other.
         *
         * @param Level Blend level (0-100)
         * @return Blended color pair
         */
        constexpr color blend(int Level) const noexcept {
            return color{ rgb::Avg(B, F, Level), rgb::Avg(F, B, Level) };
        }

        /**
         * @brief Mix foreground and background
         *
         * @param FrontPercentage Percentage of foreground in mix
         * @return Mixed color
         */
        constexpr rgb mix(int FrontPercentage) const noexcept {
            return rgb::Avg(F, B, FrontPercentage);
        }

        /**
         * @brief Create negated color pair
         *
         * @return Color pair with both colors negated
         */
        constexpr color operator - () const noexcept {
            return color{ -F, -B };
        }

        /**
         * @brief Equality comparison operator
         *
         * @param L First color pair
         * @param R Second color pair
         * @return true if both pairs have equal F and B, false otherwise
         */
        friend constexpr bool operator == (color L, color R) noexcept {
            return L.F == R.F && L.B == R.B;
        }

        /**
         * @brief Create weighted average of two color pairs
         *
         * @param Rhs Second color pair
         * @param Weight Weight of this color pair (0-100)
         * @return Averaged color pair
         */
        constexpr color avg(color Rhs, int Weight = 50) noexcept {
            return color{
                rgb::Avg(F, Rhs.F, Weight),
                rgb::Avg(B, Rhs.B, Weight)
            };
        }
    };

    //=========================================================================
    // TERMINAL COLOR FUNCTIONS
    //=========================================================================

    /**
     * @brief Set background color in terminal
     *
     * @param RGB Color to set as background
     */
    inline void SetBackColor(rgb RGB) noexcept {
        mz::Write(std::format(L"\x1b[48;2;{:0>3d};{:0>3d};{:0>3d}m", RGB.r, RGB.g, RGB.b));
    }

    /**
     * @brief Set foreground color in terminal
     *
     * @param RGB Color to set as foreground
     */
    inline void SetFrontColor(rgb RGB) noexcept {
        mz::Write(std::format(L"\x1b[38;2;{:0>3d};{:0>3d};{:0>3d}m", RGB.r, RGB.g, RGB.b));
    }

    /**
     * @brief Set both foreground and background colors in terminal
     *
     * @param C Color pair to apply
     */
    inline void SetColors(color C) noexcept {
        mz::Write(std::format(L"\x1b[38;2;{};{};{}m\x1b[48;2;{};{};{}m",
            C.F.r, C.F.g, C.F.b, C.B.r, C.B.g, C.B.b));
    }

    /**
     * @brief Reset terminal colors to defaults
     */
    inline void ResetColors() noexcept {
        mz::Write(L"\x1b[0m");
    }

    //=========================================================================
    // PREDEFINED UI COLOR SCHEMES
    //=========================================================================

    // Frame colors
    static constexpr rgb FrameFrontColor{ 0xda, 0xda, 0xd0 };
    static constexpr rgb FrameBackColor{ 0x1e, 0x1e, 0x1e };

    // Scroll bar colors
    static constexpr rgb ScrollFrontColor{ 0x4d, 0x4d, 0x4d };
    static constexpr rgb ScrollBackColor{ 0x1f, 0x1f, 0x1f };

    // List view colors
    static constexpr rgb ListBackColor{ 0x20, 0x20, 0x20 };
    static constexpr rgb ListFrontColor{ 0xff, 0xff, 0xff };
    static constexpr rgb ListFocusColor{ 0x4d, 0x4d, 0x4d };
    static constexpr rgb ListSelectColor{ 0x77, 0x77, 0x77 };

    // List color pairs
    static constexpr color ListColors{ rgb{0xff, 0xff, 0xff}, rgb{0x20, 0x20, 0x20} };
    static constexpr color ListFocusColors{ rgb{0xff, 0xff, 0xff}, rgb{0x4d, 0x4d, 0x4d} };
    static constexpr color ListSelectColors{ rgb{0xff, 0xff, 0xff}, rgb{0x77, 0x77, 0x77} };
    static constexpr color ListSelectFocusColors{ rgb{0xff, 0xff, 0xff}, rgb{0x87, 0x87, 0x87} };

    // Button color pairs
    static constexpr color ButtonActiveColors{ color::BLACK, color::LAVENDER };
    static constexpr color ButtonPassiveColors{ ButtonActiveColors.blend(30) };

    // Input field colors
    static constexpr color InputColors{ rgb::Avg(color::BLUE, color::WHITE, 30), color::WHITE * 20 };

    // Frame color schemes
    static constexpr color FrameColors3{ rgb::Avg(color::WHITE, color::AQUA, 80), rgb::Avg(color::BLACK, color::AQUA, 90) };
    static constexpr color FrameColors2{ rgb::Avg(color::WHITE, color::AQUA, 90), rgb::Avg(color::BLACK, color::AQUA, 90) };
    static constexpr color FrameColors1{ rgb::Avg(color::WHITE, color::BLUE, 90), rgb::Avg(color::BLACK, color::BLUE, 85) };

} // namespace mz

#endif // MZ_COLORS_H

