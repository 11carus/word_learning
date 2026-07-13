# WordFlow

WordFlow is a lightweight offline vocabulary learning desktop app for a C++ course project.

## Tech Stack

- C++20
- Qt 6 Widgets
- Qt SQL with SQLite
- CMake
- Ninja

## Current Status

The project currently contains the first runnable skeleton:

- Qt Widgets main window
- Home, vocabulary, review, and statistics tabs
- SQLite database initialization
- `words` and `review_logs` tables
- Simple review scheduler for Again, Hard, Good, and Easy ratings

## Build

The local development environment uses Qt 6.10.3 with MinGW 13.1.

```powershell
qt-cmake --preset debug
cmake --build --preset debug
```

The executable is generated at:

```text
build/debug/wordflow.exe
```

## Roadmap

1. Vocabulary CRUD
2. Daily review workflow
3. Review logs and statistics
4. Testing and course report materials
5. Optional PDF reading and word extraction extension

