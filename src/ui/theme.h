#pragma once
#include <SDL2/SDL.h>


// Dimensions
#define DR_TOPBAR_H 48
#define DR_TAB_PADX 16
#define DR_TAB_SPACING 8


// Colors (dark theme)
static inline SDL_Color dr_rgba(Uint8 r, Uint8 g, Uint8 b, Uint8 a){ SDL_Color c={r,g,b,a}; return c; }
static const SDL_Color DR_BG = {18,18,20,255};
static const SDL_Color DR_TOPBAR_BG = {28,30,34,255};
static const SDL_Color DR_TAB_BG = {38,40,45,255};
static const SDL_Color DR_TAB_BG_ACTIVE = {56,60,66,255};
static const SDL_Color DR_TEXT = {230,230,235,255};
