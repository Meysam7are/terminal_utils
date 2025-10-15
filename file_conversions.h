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

#ifndef MZ_FILE_CONVERSIONS_H
#define MZ_FILE_CONVERSIONS_H
#pragma once

/**
 * @file file_conversions.h
 * @brief Utilities for file path handling and size formatting
 *
 * This file provides utilities for working with file paths and formatting
 * file sizes in a human-readable way. It supports both binary (1024-based)
 * and decimal (1000-based) size representations.
 *
 * @author Meysam Zare
 */

#include <filesystem>
#include <string>
#include <format>
#include <string_view>
#include "TimeConversions.h"

namespace mz {
    namespace file {

        /**
         * @brief Error codes for file operations
         */
        enum class FileError : int64_t {
            None = 0,               ///< No error
            CanonicalFailed = -1,   ///< Failed to get canonical path
            TimeFailed = -2,        ///< Failed to get file time
            FileCheckFailed = -3,   ///< Failed to check if path is a file
            DirectoryCheckFailed = -4, ///< Failed to check if path is a directory
            FileSizeFailed = -5,    ///< Failed to get file size
            IsDirectory = -100,     ///< Path is a directory, not a file
        };

        /**
         * @brief Unit sizes for file size formatting
         */
        struct FileSizeConstants {
            // Binary units (powers of 1024)
            static constexpr int64_t KiB = 1024;
            static constexpr int64_t MiB = KiB * 1024;
            static constexpr int64_t GiB = MiB * 1024;
            static constexpr int64_t TiB = GiB * 1024;

            // Decimal units (powers of 1000)
            static constexpr int64_t KB = 1000;
            static constexpr int64_t MB = KB * 1000;
            static constexpr int64_t GB = MB * 1000;
            static constexpr int64_t TB = GB * 1000;
        };

        /**
         * @brief Format file size as a human-readable wide string
         *
         * Converts a file size in bytes to a human-readable format with appropriate
         * units (B, KB/KiB, MB/MiB, GB/GiB). Uses binary (1024-based) or
         * decimal (1000-based) units based on the Binary parameter.
         *
         * @param SignedLength File size in bytes (signed to handle error codes)
         * @param Binary If true, use binary units (KiB, MiB, GiB); otherwise decimal (KB, MB, GB)
         * @return Formatted wide string representing the file size with units
         */
        [[nodiscard]] inline std::wstring FileLengthWide(int64_t SignedLength, bool Binary) noexcept {
            // Ensure we're working with a non-negative length
            const int64_t Length = SignedLength >= 0 ? SignedLength : 0;

            if (Binary) {
                // Binary units (powers of 1024)
                if (Length < FileSizeConstants::KiB) {
                    return std::format(L"{:d} B", Length);
                }

                if (Length < FileSizeConstants::MiB) {
                    return std::format(L"{:d} KiB", (Length >> 10));
                }

                if (Length < FileSizeConstants::GiB) {
                    // Calculate fractional part for better precision
                    const int64_t mainPart = Length >> 20; // Divide by 1024^2
                    const int64_t fraction = (Length - (mainPart << 20)) * 10 >> 20;
                    return std::format(L"{:d}.{:d} MiB", mainPart, fraction);
                }

                // For GiB and larger
                const int64_t mainPart = Length >> 30; // Divide by 1024^3
                const int64_t fraction = (Length - (mainPart << 30)) * 100 >> 30;
                return std::format(L"{:d}.{:d} GiB", mainPart, fraction);
            }
            else {
                // Decimal units (powers of 1000)
                if (Length < FileSizeConstants::KB) {
                    return std::format(L"{:d} B", Length);
                }

                if (Length < FileSizeConstants::MB) {
                    return std::format(L"{:d} KB", (Length / FileSizeConstants::KB));
                }

                if (Length < FileSizeConstants::GB) {
                    const int64_t mainPart = Length / FileSizeConstants::MB;
                    const int64_t fraction = (Length % FileSizeConstants::MB) / (FileSizeConstants::MB / 10);
                    return std::format(L"{:d}.{:d} MB", mainPart, fraction);
                }

                // For GB and larger
                const int64_t mainPart = Length / FileSizeConstants::GB;
                const int64_t fraction = (Length % FileSizeConstants::GB) / (FileSizeConstants::GB / 100);
                return std::format(L"{:d}.{:d} GB", mainPart, fraction);
            }
        }

        /**
         * @brief Format file size as a human-readable string
         *
         * Narrow-string (std::string) version of FileLengthWide.
         *
         * @param SignedLength File size in bytes (signed to handle error codes)
         * @param Binary If true, use binary units (KiB, MiB, GiB); otherwise decimal (KB, MB, GB)
         * @return Formatted string representing the file size with units
         */
        [[nodiscard]] inline std::string FileLength(int64_t SignedLength, bool Binary) noexcept {
            const int64_t Length = SignedLength >= 0 ? SignedLength : 0;

            if (Binary) {
                // Binary units (powers of 1024)
                if (Length < FileSizeConstants::KiB) {
                    return std::format("{:d} B", Length);
                }

                if (Length < FileSizeConstants::MiB) {
                    return std::format("{:d} KiB", (Length >> 10));
                }

                if (Length < FileSizeConstants::GiB) {
                    const int64_t mainPart = Length >> 20;
                    const int64_t fraction = (Length - (mainPart << 20)) * 10 >> 20;
                    return std::format("{:d}.{:d} MiB", mainPart, fraction);
                }

                const int64_t mainPart = Length >> 30;
                const int64_t fraction = (Length - (mainPart << 30)) * 100 >> 30;
                return std::format("{:d}.{:d} GiB", mainPart, fraction);
            }
            else {
                // Decimal units (powers of 1000)
                if (Length < FileSizeConstants::KB) {
                    return std::format("{:d} B", Length);
                }

                if (Length < FileSizeConstants::MB) {
                    return std::format("{:d} KB", (Length / FileSizeConstants::KB));
                }

                if (Length < FileSizeConstants::GB) {
                    const int64_t mainPart = Length / FileSizeConstants::MB;
                    const int64_t fraction = (Length % FileSizeConstants::MB) / (FileSizeConstants::MB / 10);
                    return std::format("{:d}.{:d} MB", mainPart, fraction);
                }

                const int64_t mainPart = Length / FileSizeConstants::GB;
                const int64_t fraction = (Length % FileSizeConstants::GB) / (FileSizeConstants::GB / 100);
                return std::format("{:d}.{:d} GB", mainPart, fraction);
            }
        }

        /**
         * @brief Information about a file system path
         *
         * Provides basic information about a file or directory path, including
         * size, modification time, and error handling.
         */
        struct FilePath {
            int64_t Size{ 0 };                       ///< File size in bytes, or error code if negative
            int64_t Time{ 0 };                       ///< Last write time as seconds since epoch
            std::filesystem::path Path{};            ///< Path to the file or directory
            std::filesystem::file_time_type FileTime;///< File time as filesystem time point

            /**
             * @brief Construct file path info from a path view
             *
             * Collects file information including size, type, and modification time.
             * Sets appropriate error codes if operations fail.
             *
             * @param PathView Path to analyze
             */
            explicit FilePath(std::wstring_view PathView) noexcept {
                std::error_code Ec;

                // Try to get canonical path
                if (Path = std::filesystem::canonical(PathView, Ec); Ec) {
                    Size = static_cast<int64_t>(FileError::CanonicalFailed);
                    Path = PathView;
                    return;
                }

                // Try to get last write time
                if (FileTime = std::filesystem::last_write_time(Path, Ec); Ec) {
                    Size = static_cast<int64_t>(FileError::TimeFailed);
                    Path = PathView;
                    return;
                }

                // Convert file time to seconds since epoch
                Time = mz::time::getSecondsSinceEpoch(FileTime);

                // Check if this is a regular file
                bool IsFile;
                if (IsFile = std::filesystem::is_regular_file(Path, Ec); Ec) {
                    Size = static_cast<int64_t>(FileError::FileCheckFailed);
                    Path = PathView;
                    return;
                }

                if (!IsFile) {
                    // Check if this is a directory
                    bool IsFolder;
                    if (IsFolder = std::filesystem::is_directory(Path, Ec); Ec) {
                        Size = static_cast<int64_t>(FileError::DirectoryCheckFailed);
                    }
                    else if (IsFolder) {
                        Size = static_cast<int64_t>(FileError::IsDirectory);
                    }
                    else {
                        Size = 0; // Not a file or directory (could be a symlink, pipe, etc.)
                    }
                }
                else {
                    // Get file size for regular files
                    if (Size = std::filesystem::file_size(Path, Ec); Ec) {
                        Size = static_cast<int64_t>(FileError::FileSizeFailed);
                    }
                }

                // Store original path view for display
                Path = PathView;
            }

            /**
             * @brief Construct file path info from a filesystem path
             *
             * Overload for std::filesystem::path input.
             *
             * @param FilesystemPath Path to analyze
             */
            explicit FilePath(const std::filesystem::path& FilesystemPath) noexcept
                : FilePath(FilesystemPath.wstring()) {
            }

            /**
             * @brief Get file size and name as formatted string
             *
             * Returns a formatted string containing the file size (or status)
             * and the filename with extension.
             *
             * @param MinSizeLength Minimum width for the size field
             * @return Formatted string with size and filename
             */
            [[nodiscard]] std::wstring size_and_name_string(size_t MinSizeLength = 10) const noexcept {
                if (Size >= 0) {
                    // Format regular file with size
                    return std::format(L"{:>{}}  {}{}",
                        FileLengthWide(Size, true), MinSizeLength,
                        Path.stem().wstring(), Path.extension().wstring());
                }
                else if (Size == static_cast<int64_t>(FileError::IsDirectory)) {
                    // Format directory
                    return std::format(L"{:>{}}  {}{}",
                        L"Directory", MinSizeLength,
                        Path.stem().wstring(), Path.extension().wstring());
                }
                else {
                    // Format with error code
                    return std::format(L"{:>{}}  {}{}",
                        std::format(L"Error {}", static_cast<int>(Size)), MinSizeLength,
                        Path.stem().wstring(), Path.extension().wstring());
                }
            }

            /**
             * @brief Get formatted file size with binary units
             *
             * @return File size as a formatted string
             */
            [[nodiscard]] std::wstring formatted_size() const noexcept {
                return FileLengthWide(Size, true);
            }

            /**
             * @brief Get formatted file time
             *
             * @return File modification time as a formatted string
             */
            [[nodiscard]] std::wstring formatted_time() const noexcept {
                try {
                    auto tp = std::chrono::time_point<std::chrono::system_clock, std::chrono::seconds>{
                        std::chrono::seconds{Time}
                    };
                    return std::format(L"{:%Y-%m-%d %H:%M:%S}", tp);
                }
                catch (const std::exception&) {
                    return L"Invalid time";
                }
            }

            /**
             * @brief Check if the path is a directory
             *
             * @return true if this is a directory
             */
            [[nodiscard]] constexpr bool is_directory() const noexcept {
                return Size == static_cast<int64_t>(FileError::IsDirectory);
            }

            /**
             * @brief Check if the path is a regular file
             *
             * @return true if this is a regular file
             */
            [[nodiscard]] constexpr bool is_file() const noexcept {
                return Size >= 0;
            }

            /**
             * @brief Check if there was an error accessing this path
             *
             * @return true if there was an error (excluding directory detection)
             */
            [[nodiscard]] constexpr bool has_error() const noexcept {
                return Size < 0 && Size != static_cast<int64_t>(FileError::IsDirectory);
            }

            /**
             * @brief Get error message for this path
             *
             * @return Error message or empty string if no error
             */
            [[nodiscard]] std::wstring error_message() const noexcept {
                if (Size >= 0) return L"";

                switch (static_cast<FileError>(Size)) {
                case FileError::CanonicalFailed:
                    return L"Failed to get canonical path";
                case FileError::TimeFailed:
                    return L"Failed to get file time";
                case FileError::FileCheckFailed:
                    return L"Failed to check if path is a file";
                case FileError::DirectoryCheckFailed:
                    return L"Failed to check if path is a directory";
                case FileError::FileSizeFailed:
                    return L"Failed to get file size";
                case FileError::IsDirectory:
                    return L"";  // Not an error
                default:
                    return L"Unknown error";
                }
            }
        };

    } // namespace file
} // namespace mz

#endif // MZ_FILE_CONVERSIONS_H
