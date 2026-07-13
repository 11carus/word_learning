# Development Environment

Installation date: 2026-07-13 (Asia/Hong_Kong)

## Installed Components

| Component | Version | Location | Installation method |
|---|---|---|---|
| Qt | 6.10.3, MinGW 64-bit | `D:\Qt\6.10.3\mingw_64` | `aqtinstall` from the official Qt download repository |
| Qt QML / Qt Quick | 6.10.3 | Qt installation above | Qt base package |
| Qt PDF / Qt PDF Quick | 6.10.3 | Qt installation above | Qt `qtpdf` module |
| MinGW-w64 GCC/G++ | 13.1.0 | `D:\Qt\Tools\mingw1310_64` | Qt tool package |
| GDB | 11.2 | `D:\Qt\Tools\mingw1310_64` | Qt tool package |
| Qt Creator | 20.0.0 | `D:\Qt\Tools\QtCreator` | Qt tool package; telemetry plugin not installed |
| Ninja | 1.13.2 | User WinGet package directory | WinGet package `Ninja-build.Ninja` |
| CMake | 4.3.1 | Existing system installation | Pre-existing |
| aqtinstall | 3.3.0 | `%LOCALAPPDATA%\codex-tools\aqt-venv` | Isolated Python virtual environment |

## User PATH Priority

The following entries were placed at the beginning of the user PATH:

1. `D:\Qt\6.10.3\mingw_64\bin`
2. `D:\Qt\Tools\mingw1310_64\bin`
3. `D:\Qt\Tools\QtCreator\bin`
4. The WinGet installation directory for Ninja 1.13.2

The previous MinGW 8.1 installation remains available later on PATH. New terminals should resolve the Qt-compatible MinGW 13.1 toolchain first.

## Verification

A temporary CMake probe was configured with C++20 and the Ninja generator. It successfully found and linked:

- `Qt6::Core`
- `Qt6::Qml`
- `Qt6::Quick`
- `Qt6::Pdf`
- `Qt6::PdfQuick`

The probe compiled with GNU C++ 13.1.0, linked successfully, and exited with code 0.

CMake reported that optional Vulkan headers were not installed. This does not block the project because Qt Quick on Windows uses Direct3D by default.

## Recommended CMake Settings

Use these values when configuring the project from a terminal or IDE:

```text
CMAKE_PREFIX_PATH=D:/Qt/6.10.3/mingw_64
CMAKE_CXX_COMPILER=D:/Qt/Tools/mingw1310_64/bin/g++.exe
Generator=Ninja
C++ standard=20
```

Open a new terminal or restart Codex before relying on the updated user PATH.
