#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <stdbool.h>


#include "../ui/menubar.h"
#include "../ui/theme.h"


#define WINDOW_TITLE "DataReel — Starter"
#define WINDOW_W 1000
#define WINDOW_H 640


static void dr_fill(SDL_Renderer* r, int x,int y,int w,int h, SDL_Color c){
SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
SDL_SetRenderDrawColor(r, c.r,c.g,c.b,c.a);
SDL_Rect rc={x,y,w,h};
SDL_RenderFillRect(r,&rc);
}


static void dr_text(SDL_Renderer* r, TTF_Font* font, const char* s, int x, int y, SDL_Color color){
SDL_Surface* surf = TTF_RenderUTF8_Blended(font, s, color);
if(!surf) return;
SDL_Texture* tex = SDL_CreateTextureFromSurface(r, surf);
int w=0,h=0; SDL_QueryTexture(tex,NULL,NULL,&w,&h);
SDL_FreeSurface(surf);
SDL_Rect dst={x,y,w,h};
SDL_RenderCopy(r, tex, NULL, &dst);
SDL_DestroyTexture(tex);
}


int main(int argc, char** argv){
(void)argc; (void)argv;
if(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER)!=0){ fprintf(stderr,"SDL: %s\n", SDL_GetError()); return 1; }
if(TTF_Init()!=0){ fprintf(stderr,"TTF: %s\n", TTF_GetError()); SDL_Quit(); return 1; }


SDL_Window* win = SDL_CreateWindow(WINDOW_TITLE, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
WINDOW_W, WINDOW_H, SDL_WINDOW_SHOWN|SDL_WINDOW_RESIZABLE);
if(!win){ fprintf(stderr,"Window: %s\n", SDL_GetError()); goto quit_sdl; }


SDL_Renderer* ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED|SDL_RENDERER_PRESENTVSYNC);
if(!ren){ fprintf(stderr,"Renderer: %s\n", SDL_GetError()); goto destroy_win; }


TTF_Font* font = TTF_OpenFont("resources/Inter_28pt-Regular.ttf", 16);
if(!font){ fprintf(stderr,"Font: %s\n", TTF_GetError()); goto destroy_ren; }


dr_menubar_t menubar; dr_menubar_init(&menubar);


bool running=true; int ww=WINDOW_W, wh=WINDOW_H;
while(running){
SDL_Event e;
while(SDL_PollEvent(&e)){
if(e.type==SDL_QUIT) running=false;
else if(e.type==SDL_WINDOWEVENT && e.window.event==SDL_WINDOWEVENT_SIZE_CHANGED){ ww=e.window.data1; wh=e.window.data2; }
else dr_menubar_handle_event(&menubar, font, &e);
}


// Background
SDL_SetRenderDrawColor(ren, DR_BG.r,DR_BG.g,DR_BG.b,DR_BG.a);
SDL_RenderClear(ren);


// Top menu bar
int top_h = dr_menubar_draw(&menubar, ren, font, ww);


// Content
dr_fill(ren, 0, top_h, ww, wh-top_h, DR_BG);
char line[160];
snprintf(line, sizeof(line), "%s — coming soon", dr_tab_label(menubar.active));
dr_text(ren, font, line, 24, top_h + 24, DR_TEXT);


SDL_RenderPresent(ren);
}


TTF_CloseFont(font);
destroy_ren:
SDL_DestroyRenderer(ren);
destroy_win:
SDL_DestroyWindow(win);
quit_sdl:
TTF_Quit(); SDL_Quit();
return 0;
}
