# Installed Codex Skills

Installation date: 2026-07-13 (Asia/Hong_Kong)

This record covers Codex skills only. No application source code was created or modified.

## Qt Skills

Source: `TheQtCompanyRnD/agent-skills` at commit `71d6c10da78b9a764468ae11c86ab3bc4ca4921f`

| Skill | Scope | Status | Installed path |
|---|---|---|---|
| qt-cmake-project | Project | Installed and verified | `D:\word_learning\.agents\skills\qt-cmake-project` |
| qt-qml | Project | Installed and verified | `D:\word_learning\.agents\skills\qt-qml` |
| qt-ui-design | Project | Installed and verified | `D:\word_learning\.agents\skills\qt-ui-design` |
| qt-cpp-review | Project | Installed and verified | `D:\word_learning\.agents\skills\qt-cpp-review` |
| qt-qml-review | Project | Installed and verified | `D:\word_learning\.agents\skills\qt-qml-review` |
| qt-qml-test | Project | Installed and verified | `D:\word_learning\.agents\skills\qt-qml-test` |
| qt-qml-test-run | Project | Installed and verified | `D:\word_learning\.agents\skills\qt-qml-test-run` |

All Qt skill directories contain `SKILL.md`; each declared `name` matches its directory name. Their frontmatter includes `description`, `license`, and `compatibility`, and declares Qt 6.x support.

## C++ Skills

Source: `mohitmishra786/low-level-dev-skills` at commit `bdc58472fa9f309ed1b3f7d985a0d8e9bd8f4608`

| Skill | Scope | Status | Installed path |
|---|---|---|---|
| cmake | Project | Installed and verified | `D:\word_learning\.agents\skills\cmake` |
| ninja | Project | Installed and verified; Ninja 1.13.2 is installed and on the user PATH | `D:\word_learning\.agents\skills\ninja` |
| gcc | Project | Installed and verified | `D:\word_learning\.agents\skills\gcc` |
| gdb | Project | Installed and verified | `D:\word_learning\.agents\skills\gdb` |
| static-analysis | Project | Installed and verified | `D:\word_learning\.agents\skills\static-analysis` |
| sanitizers | Project | Installed and verified | `D:\word_learning\.agents\skills\sanitizers` |
| conan-vcpkg | Project | Installed and verified | `D:\word_learning\.agents\skills\conan-vcpkg` |

All C++ skill directories contain `SKILL.md`, and each declared `name` matches its directory name. These skills include `name` and `description`, but their frontmatter does not declare `license` or `compatibility`. Compatibility with this project is based on their stated C/C++, GCC, GDB, CMake, and sanitizer use cases; they do not explicitly declare Qt 6 or C++20 compatibility.

## Planning Skill

Source: `OthmanAdi/planning-with-files` at commit `e8ce66f962adde12894a50f7f784809cd3aae655`

| Skill | Scope | Status | Installed path |
|---|---|---|---|
| planning-with-files-zh | Global | Installed and verified | `C:\Users\86152\.codex\skills\planning-with-files-zh` |

The skill contains `SKILL.md`, and its declared `name` matches its directory. Its frontmatter contains `name` and `description` but does not declare `license` or `compatibility`.

The planning skill defines automatic hooks and bundled PowerShell, shell, and Python scripts. The reviewed scripts create or update `task_plan.md`, `findings.md`, and `progress.md` in the active project and may read local Codex session logs for session recovery. No network upload, credential access, destructive user-file deletion, or system-configuration modification was found.

## Environment

- Operating system: Windows, x64
- Codex desktop package: 26.707.3748.0
- Project root: `D:\word_learning`
- Node: 24.12.0
- npm: 11.6.2
- npx: 11.6.2
- CMake: 4.3.1
- Qt: 6.10.3 MinGW 64-bit, including QML/Qt Quick and Qt PDF
- Compiler: Qt MinGW-w64 GCC/G++ 13.1.0 (preferred); legacy GCC/G++ 8.1.0 retained later on PATH
- Debugger: Qt GDB 11.2 (preferred); legacy GDB 8.1 retained
- Ninja executable: 1.13.2
- Qt Creator: 20.0.0
- MSVC: not detected
- Clang: not detected

## Skipped Skills

| Skill or group | Reason |
|---|---|
| msvc-cl | MSVC was not detected |
| clang | Clang was not detected |
| Qt Figma skills | Not required for this project |
| All other repository skills | Exact selection was requested; `--all` was not used |

No requested skill was skipped as a duplicate. No English `planning-with-files` installation was found, so only the Simplified Chinese variant was installed.

## Failed Skills

None. All three installation operations exited successfully.

## Verification and Security Review

- Repositories were cloned to a system temporary directory for read-only review before installation.
- Root README files, target `SKILL.md` frontmatter, and bundled executable-text scripts were inspected.
- No binary executables were present in the selected skill directories.
- Installed directories were compared file-by-file with the reviewed source directories. After normalizing Windows versus Unix line endings, all 15 skills had zero missing, extra, or mismatched files.
- Documentation examples mention official web documentation, package-manager downloads, Linux `sudo`, and Conan upload commands. They are examples and were not executed during installation.
- No skill script was executed during this task.
- No API key, SSH key, token, private file, or environment variable was uploaded.

## Recommended Priority

1. `planning-with-files-zh` for multi-stage project planning and progress records.
2. `qt-cmake-project` for the Qt 6/CMake project structure.
3. `qt-qml` and `qt-ui-design` for the application interface.
4. `cmake`, `gcc`, and `gdb` for the detected MinGW toolchain.
5. `qt-qml-test`, `qt-qml-test-run`, `static-analysis`, and `sanitizers` during verification.
6. `qt-cpp-review` and `qt-qml-review` before merging changes to `main`.
7. `conan-vcpkg` when adding PDF or other third-party libraries.

## Activation

Restart Codex before relying on the newly installed skills and refreshed user PATH. The project-scoped skills should then be discovered from `D:\word_learning\.agents\skills`, and the planning skill from the global Codex skills directory.
