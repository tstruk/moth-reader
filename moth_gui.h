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

#include <gtkmm.h>
#include <SDL.h>
#include "moth.h"

class moth_gui {
    Gtk::Window win;
	SDL_Surface *screen;
    int bpp;
    int flags;
    int width;
    int height;
    int running;
    int handle_keyboard_event(SDL_Event *event);
    int handle_mouse_event(SDL_Event *event);
    int handle_event(SDL_Event *event, int *quit);
    void init_opengl();
    void handle_key(SDL_keysym* key);
    void process_events();
    void draw_screen();

    public:
    void init_video();
    void book_select(std::string&);
    int main_loop();
    moth_gui();
    virtual ~moth_gui();
};

#endif


