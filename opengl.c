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
    while(SDL_PollEvent(&event)) {
	    switch(event.type) {
		case SDL_KEYDOWN:
			SDL_Quit();
		default:
		    break;
	    }
    }

    SDL_Rect r={8,0,8,8};

    SDL_FillRect(surface,0,0xffffff);
    SDL_BlitSurface(fontsf,&r,surface,0);
    SDL_Flip(surface);
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
    printf("%p %p\n",surface,fontsf);
}

#ifdef TEST

int main() {
	sdl_init();
	for(;;) { sdl_poll(); }
}

#endif
