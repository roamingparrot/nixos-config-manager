cmake_minimum_required(VERSION 3.10)
project(nixos-config-manager)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find ncurses library
find_library(NCURSES_LIBRARY NAMES ncursesw)
find_path(NCURSES_INCLUDE_DIR NAMES ncurses.h)

if(NOT NCURSES_LIBRARY OR NOT NCURSES_INCLUDE_DIR)
    message(FATAL_ERROR "ncursesw library not found")
endif()

# Include directories
include_directories(${NCURSES_INCLUDE_DIR})
include_directories(${NCURSES_INCLUDE_DIR}/ncursesw)

# Add source files
set(SOURCES
    main.cpp
    core/moduleResolver.cpp
    core/configParser.cpp
    core/configEditor.cpp
    core/rebuildManager.cpp
    core/packageSearcher.cpp
    core/fileSyntaxDetector.cpp
    core/moduleSelector.cpp
    core/packageInserter.cpp
    ui/tui.cpp
)

# Create executable
add_executable(dotman ${SOURCES})

# Link libraries
target_link_libraries(dotman ${NCURSES_LIBRARY})

# Compiler flags
target_compile_options(dotman PRIVATE -Wall -Wextra)