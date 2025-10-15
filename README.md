# Terminal Utils Library
MIT License | C++14 | Cross-Platform
## Overview
Terminal Utils is a comprehensive C++ library for building sophisticated terminal-based user interfaces with cross-platform compatibility. It provides a robust set of tools for terminal manipulation, text formatting, layout management, and interactive UI components.
## Key Features
•	Cross-Platform Terminal Control: Works consistently across Windows, macOS, and Linux
•	ANSI Escape Sequence Support: Rich text formatting with colors and styles
•	Advanced Layout System: Box model for precise UI component positioning
•	Interactive UI Components: Buttons, input fields, directory browsers, and more
•	UTF-8 Support: Full international character handling
•	Event-Driven Architecture: Clean keyboard input handling with events
•	Responsive Design: Terminal resizing support with automatic layout adjustment
## Requirements
•	C++14 compatible compiler
•	No external dependencies for core functionality
## Installation
### Using CMake
```
git clone https://github.com/meysam-zm/terminal_utils.git
cd terminal_utils
mkdir build && cd build
cmake ..
cmake --build .
cmake --install .
```
### As a Subproject
Add the repository as a git submodule:
```
git submodule add https://github.com/meysam-zm/terminal_utils.git extern/terminal_utils
```
Then in your CMakeLists.txt:
```
add_subdirectory(extern/terminal_utils)
target_link_libraries(your_target_name PRIVATE mz::terminal_utils)
```
## Basic Usage
### Terminal Setup and Management
```
#include <mz/terminal/TerminalManager.h>
#include <iostream>

int main() {
    // Initialize terminal with cross-platform support
    mz::TerminalManager terminal;
    terminal.setup(30, 100); // 30 rows, 100 columns
    
    // Write colored text
    std::wstring buffer;
    mz::color::GREEN.setFront(buffer);
    mz::color::BLACK.setBack(buffer);
    
    // Position cursor
    mz::coord pos{5, 10}; // row 5, column 10
    pos.apply(buffer);
    
    buffer.append(L"Hello, Terminal Utils!");
    mz::Write(buffer);
    
    return 0;
}
```
### Creating a Simple UI
```
#include <mz/terminal/ConsoleWindow.h>
#include <mz/terminal/ButtonBox.h>
#include <mz/terminal/InputControl.h>

int main() {
    // Create main window
    mz::ConsoleWindow window(30, 100);
    
    // Create UI components
    auto inputField = std::make_shared<mz::InputControl>(20);
    auto submitButton = std::make_shared<mz::ButtonBox>(L"Submit");
    
    // Position components
    window.addControl(inputField, {5, 10});
    window.addControl(submitButton, {8, 10});
    
    // Set button action
    submitButton->setAction([&inputField]() {
        // Process input when button is pressed
        std::wstring input = inputField->getText();
        // ... handle input ...
        return true;
    });
    
    // Run the UI
    window.run();
    
    return 0;
}
```
## Core Components
### TerminalManager
Cross-platform terminal configuration and control:
```
mz::TerminalManager terminal;
terminal.set_console_size(30, 100);
terminal.set_font(L"Cascadia Code", 16); // Windows only
terminal.clear_all();
```
### Coordinate System
Precise positioning in the terminal:
```
// Create a coordinate position (row, column)
mz::coord position{10, 20};

// Create a rectangular box area
mz::coord_box box{{5, 5}, {15, 50}}; // top-left to bottom-right

// Apply position to output buffer
std::wstring buffer;
position.apply(buffer);
buffer.append(L"Text at position");
```
### Colors and Styling
Rich text formatting:
```
std::wstring buffer;

// Set foreground and background colors
mz::color::RED.setFront(buffer);
mz::color::WHITE.setBack(buffer);

// Style attributes
mz::cursor cursor;
cursor.set_bold();
cursor.set_underline();
cursor.apply(buffer);

buffer.append(L"Styled text");
mz::Write(buffer);
```
### UI Components
Pre-built interactive components:
•	ButtonBox: Clickable button control
•	InputControl: Text input field
•	WindowBox: Container with border and title
•	DirectoryDisplayBox: File/directory browser
•	LoginControl: Username/password entry
•	FrameBox: Decorative frame around content
•	FooterBox: Status line or footer control
## Advanced Features
### Layout Management
Terminal Utils provides sophisticated layout tools:
```
// Create a main window box
mz::coord_box mainArea{{0, 0}, {29, 99}};

// Create a three-panel layout
auto leftPanel = mainArea.left_columns(30);
auto rightTopPanel = mainArea.right_columns(69).top_rows(15);
auto rightBottomPanel = mainArea.right_columns(69).bottom_rows(14);

// Add padding for content areas
auto leftContent = leftPanel.pad_rows(1, 1).pad_cols(1, 1);
```
### Event Handling
The library supports event-driven programming:
```
mz::ConsoleWindow window(30, 100);

window.addKeyHandler(mz::KeyCode::F1, [&window]() {
    window.showHelp();
    return true;
});

window.addKeyHandler(mz::KeyCode::ESC, [&window]() {
    window.requestExit();
    return true;
});
```
## Cross-Platform Considerations
Terminal Utils automatically handles platform differences:
•	Windows: Uses Win32 Console API with ANSI escape sequences
•	macOS/Linux: Uses termios and POSIX API
•	All Platforms: Consistent API with automatic feature detection
## Performance Considerations
•	Minimal memory allocations with buffer reuse
•	Efficient screen updates that only redraw changed areas
•	Optimized text processing for large output volumes
•	Thread-safe design for multi-threaded applications
## License
This library is distributed under the MIT License. See the LICENSE file for details.
## Credits
Developed by Meysam Zare (c) 2021-2025















