# Beach-Themed C++ IDE

A modern C++ IDE with AI-powered code completion and a beautiful beach-inspired theme.

## Directory Structure

```
.
├── src/            # Source files
│   ├── main.cpp
│   ├── mainwindow.cpp
│   ├── completionwidget.cpp
│   └── highlighter.cpp
├── include/        # Header files
│   ├── mainwindow.h
│   ├── completionwidget.h
│   └── highlighter.h
├── resources/      # UI and resource files
│   ├── mainwindow.ui
│   └── resources.qrc
├── tests/          # Test files
│   └── test.cpp
├── build/          # Build output (not in git)
│   ├── obj/       # Object files
│   ├── moc/       # Qt meta-object files
│   ├── rcc/       # Qt resource files
│   └── ui/        # Qt UI files
└── CppIDE.pro     # Qt project file
```

## Building

1. Make sure you have Qt5 and a C++17 compatible compiler installed
2. Create a `.env` file with your OpenAI API key
3. Run:
   ```bash
   qmake
   make
   ```
4. The executable will be in the `build/` directory

## Features

- Modern C++17 codebase
- AI-powered code completion using OpenAI's GPT-4
- Beautiful beach-themed syntax highlighting
- Qt5-based modern UI
