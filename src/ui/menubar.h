#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdbool.h>


#ifdef __cplusplus
extern "C" {
#endif


typedef enum {
DR_TAB_QUEUE = 0,
DR_TAB_HISTORY,
DR_TAB_PLUGINS,
DR_TAB_SETTINGS,
DR_TAB_COUNT
} dr_tab_t;


// Owns no memory; uses renderer/font provided by the app
typedef struct {
dr_tab_t active;
} dr_menubar_t;


// Initialize menu bar state (active tab, etc.)
void dr_menubar_init(dr_menubar_t* mb);


// Handle events (mouse clicks to switch tabs). Return true if it consumed the event.
bool dr_menubar_handle_event(dr_menubar_t* mb, TTF_Font* font, const SDL_Event* e);


// Draw the top bar and tabs. Returns the rendered height (top bar height).
int dr_menubar_draw(dr_menubar_t* mb, SDL_Renderer* r, TTF_Font* font, int win_w);


// Get human label for the active tab (for placeholder content, etc.)
const char* dr_tab_label(dr_tab_t t);


#ifdef __cplusplus
}
#endif
