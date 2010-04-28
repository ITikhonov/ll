#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <SDL/SDL.h>
#include "font.h"

SDL_Surface *surface;
SDL_Surface *fontsf;

void sdl_poll() {
    SDL_Event event;
    SDL_Flip(surface);
    usleep(50000);
    while(SDL_PollEvent(&event)) {
	    switch(event.type) {
		case SDL_KEYDOWN:
			SDL_Quit();
#ifndef TEST
			down();
#endif
			exit(0);
		default:
		    break;
	    }
    }

    return;
}

void sdl_init()
{
    int videoFlags;

    if(SDL_Init(SDL_INIT_VIDEO)<0) { return; }

    videoFlags=0;//SDL_OPENGL;
    videoFlags|=SDL_GL_DOUBLEBUFFER;
    videoFlags|=SDL_RESIZABLE;
    videoFlags|=SDL_HWSURFACE;
    videoFlags|=SDL_HWACCEL;

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER,1);
    surface=SDL_SetVideoMode(800,600,24,videoFlags);

    if(!surface) { return; }


    SDL_Color b={0,0,0},w={255,255,255};
    fontsf=SDL_CreateRGBSurfaceFrom(font,fontw,8,8,fontw,0,0,0,0);
    SDL_SetColors(fontsf,&b,0,1);
    SDL_SetColors(fontsf,&w,255,1);
}

void sdl_clear() {
    SDL_FillRect(surface,0,0xffffff);
}

int sdl_char(int c, int x, int y) {
    int w=fontm[c];
    SDL_Rect s={c*8,0,w,8};
    SDL_Rect d={x,y,w,8};
    SDL_BlitSurface(fontsf,&s,surface,&d);
    return w;
}

void sdl_color(uint64_t c) {
    SDL_Color b={(c>>16)&0xff,(c>>8)&0xff,c&0xff};
    SDL_SetColors(fontsf,&b,0,1);
}

#ifdef TEST

int main() {
	sdl_init();
	sdl_clear();
	sdl_char(10,100,100);
	for(;;) { sdl_poll(); }
}

#endif
