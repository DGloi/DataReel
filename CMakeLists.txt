cmake_minimum_required(VERSION 3.15)
project(DataReel VERSION 1.0.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Find required packages
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK4 REQUIRED gtk4)

# GLib compile resources
find_program(GLIB_COMPILE_RESOURCES NAMES glib-compile-resources REQUIRED)

# Generate resource C file
set(GRESOURCE_XML ${CMAKE_SOURCE_DIR}/resources/app.gresource.xml)
set(GRESOURCE_C ${CMAKE_BINARY_DIR}/resources.c)

add_custom_command(
    OUTPUT ${GRESOURCE_C}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/resources
    COMMAND ${GLIB_COMPILE_RESOURCES}
    ARGS
        --target=${GRESOURCE_C}
        --generate-source
        ${GRESOURCE_XML}
    DEPENDS ${GRESOURCE_XML}
)

# Platform-specific settings
if(APPLE)
    set(CMAKE_MACOSX_RPATH 1)
    set(CMAKE_INSTALL_RPATH "@executable_path/../lib")
endif()

# Source files
set(SOURCES
    src/main.c
    src/ui/main_window.c
    src/ui/download_row.c
    src/core/downloader.c
    src/core/process_manager.c
    src/utils/config.c
)

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${GTK4_INCLUDE_DIRS}
)

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Link libraries
target_link_libraries(${PROJECT_NAME}
    ${GTK4_LIBRARIES}
)

target_link_directories(${PROJECT_NAME} PRIVATE
    ${GTK4_LIBRARY_DIRS}
)

target_compile_options(${PROJECT_NAME} PRIVATE
    ${GTK4_CFLAGS_OTHER}
    -Wall -Wextra
)

# Install target
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
)
