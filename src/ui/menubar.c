#include "menubar.h"
#include "theme.h"
#include <stdio.h>


static const char* DR_TAB_LABELS[DR_TAB_COUNT] = {
"Queue","History","Plugins","Settings"
};


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


static void dr_text_size(TTF_Font* font, const char* s, int* out_w, int* out_h){
int w=0,h=0; TTF_SizeUTF8(font, s, &w, &h);
if(out_w) *out_w=w;
if(out_h) *out_h=h;
}


void dr_menubar_init(dr_menubar_t* mb){
mb->active = DR_TAB_QUEUE;
}


bool dr_menubar_handle_event(dr_menubar_t* mb, TTF_Font* font, const SDL_Event* e){
if(e->type == SDL_MOUSEBUTTONDOWN && e->button.button == SDL_BUTTON_LEFT){
const int my = e->button.y;
if(my < DR_TOPBAR_H){
int x=12; const int pad=DR_TAB_PADX; const int spacing=DR_TAB_SPACING;
for(int i=0;i<DR_TAB_COUNT;i++){
int tw,th; dr_text_size(font, DR_TAB_LABELS[i], &tw, &th);
int w = tw + pad*2; SDL_Rect rr={x, (DR_TOPBAR_H - (th+8))/2, w, th+8};
if(e->button.x>=rr.x && e->button.x<rr.x+rr.w && my>=rr.y && my<rr.y+rr.h){
mb->active = (dr_tab_t)i;
return true;
}
x += w + spacing;
}
}
}
return false;
}


int dr_menubar_draw(dr_menubar_t* mb, SDL_Renderer* r, TTF_Font* font, int win_w){
// Top bar background
dr_fill(r, 0,0, win_w, DR_TOPBAR_H, DR_TOPBAR_BG);


int x=12; const int pad=DR_TAB_PADX; const int spacing=DR_TAB_SPACING;
for(int i=0;i<DR_TAB_COUNT;i++){
int tw,th; dr_text_size(font, DR_TAB_LABELS[i], &tw, &th);
int w = tw + pad*2; SDL_Rect rr={x, (DR_TOPBAR_H - (th+8))/2, w, th+8};
SDL_Color bg = ((int)i == (int)mb->active) ? DR_TAB_BG_ACTIVE : DR_TAB_BG;
dr_fill(r, rr.x, rr.y, rr.w, rr.h, bg);
dr_text(r, font, DR_TAB_LABELS[i], rr.x + pad, rr.y + 4, DR_TEXT);
x += w + spacing;
}
return DR_TOPBAR_H;
}


const char* dr_tab_label(dr_tab_t t){
if(t>=0 && t<DR_TAB_COUNT) return DR_TAB_LABELS[t];
return "?";
}
