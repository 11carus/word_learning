cmake_minimum_required(VERSION 3.20)

project(WordFlow
    VERSION 0.1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 6.8 REQUIRED COMPONENTS Widgets Sql Pdf)

qt_standard_project_setup(REQUIRES 6.8)

qt_add_executable(wordflow
    WIN32
    src/main.cpp
    src/mainwindow.cpp
    src/mainwindow.h
    src/database/databasemanager.cpp
    src/database/databasemanager.h
    src/review/reviewscheduler.cpp
    src/review/reviewscheduler.h
)

target_include_directories(wordflow
    PRIVATE
        src
)

target_link_libraries(wordflow
    PRIVATE
        Qt6::Widgets
        Qt6::Sql
        Qt6::Pdf
)

set_target_properties(wordflow PROPERTIES
    WIN32_EXECUTABLE TRUE
)

install(TARGETS wordflow
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
