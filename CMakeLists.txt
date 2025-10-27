cmake_minimum_required(VERSION 3.16)
project(DataReel LANGUAGES C VERSION 0.1.0)

# --- Language standard ---
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)

# --- Warnings ---
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra)
endif()

# --- SDL2 & SDL2_ttf (Ubuntu-friendly using pkg-config) ---
find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2 REQUIRED IMPORTED_TARGET sdl2)
pkg_check_modules(SDL2_TTF REQUIRED IMPORTED_TARGET SDL2_ttf)

# --- UI submodule ---
add_subdirectory(src/ui)

# --- Main executable ---
add_executable(DataReel src/app/main.c)

# Link to SDL and your UI library
target_link_libraries(DataReel
    PRIVATE
        PkgConfig::SDL2
        PkgConfig::SDL2_TTF
        DataReelUI
)

# --- Post-build: copy resources (font) next to the binary ---
add_custom_command(
    TARGET DataReel POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:DataReel>/resources
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_SOURCE_DIR}/resources/Inter_28pt-Regular.ttf
            $<TARGET_FILE_DIR:DataReel>/resources/Inter_28pt-Regular.ttf
)
