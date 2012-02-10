/*************************************************************************
 * Moth ebook reader
 * Copyright (C) Tadeusz Struk 2012 <tstruk@gmail.com>
 *
 * This is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * It is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * <http://www.gnu.org/licenses/>
 *
 *************************************************************************/
#ifndef __MOTH_GUI__
#define __MOTH_GUI__

extern "C" {
    #include <SDL.h>
    #include <SDL_opengl.h>
    #include <gl.h>
    #include <glu.h>
}
#include "moth.h"

class moth_gui {
	SDL_Surface *screen;
    int bpp;
    int flags;
    int width;
    int height;
    int handle_keyboard_event(SDL_Event *event);
    int handle_mouse_event(SDL_Event *event);
    int handle_event(SDL_Event *event, int *quit);
    voind init_opengl();
    public:
    void init_video();
    static char* book_select();
    int main_loop();
    moth_gui();
    virtual ~moth_gui();
};

#endif


