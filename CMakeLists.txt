cmake_minimum_required(VERSION 3.15)
project(datareel LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Find required packages
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK4 REQUIRED gtk4)
pkg_check_modules(JSON_GLIB REQUIRED json-glib-1.0)

# Platform-specific settings
if(APPLE)
    set(CMAKE_MACOSX_RPATH 1)
    set(CMAKE_INSTALL_RPATH "@executable_path/../lib")
endif()

# Source files
set(SOURCES
    src/main.c
    src/ui/main_window.c
    src/ui/download_options.c
    src/ui/download_item_widget.c
    src/ui/settings_panel.c
    src/core/ytdlp_manager.c
    src/core/download_engine.c
    src/core/metadata_fetcher.c
    src/core/process_manager.c
    src/utils/config.c
    src/utils/string_utils.c
)

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${GTK4_INCLUDE_DIRS}
    ${JSON_GLIB_INCLUDE_DIRS}
)

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Link libraries
target_link_libraries(${PROJECT_NAME}
    ${GTK4_LIBRARIES}
    ${JSON_GLIB_LIBRARIES}
)

target_link_directories(${PROJECT_NAME} PRIVATE
    ${GTK4_LIBRARY_DIRS}
    ${JSON_GLIB_LIBRARY_DIRS}
)

target_compile_options(${PROJECT_NAME} PRIVATE
    ${GTK4_CFLAGS_OTHER}
    ${JSON_GLIB_CFLAGS_OTHER}
    -Wall -Wextra
)

# Install target
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
)
