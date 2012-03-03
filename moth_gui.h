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
#include <gl.h>
#include <FTGL/ftgl.h>

#include "moth.h"
#include "moth_book.h"

class moth_gui {
    Gtk::Window *win;
	SDL_Surface *screen;
    moth_book *book;
    FTFont *font_renderer;
    int bpp;
    int flags;
    double width;
    double height;
    double page_width;
    double page_height;
    double ratio;
    double zoom;
    double shift_x;
    double shift_y;
    int running;
    int num_pages;
    int move_by_pages;
    int moveing_page;
    int idle;
    uint8_t shift_state;
    uint8_t button_state;
    GLuint *textures;

    void init_opengl();
    void handle_key_down(SDL_keysym*);
    void handle_key_up(SDL_keysym*);
    void handle_resize(SDL_ResizeEvent*);
    void handle_mouse_motion(SDL_MouseMotionEvent*);
    void handle_mouse_button(SDL_MouseButtonEvent*);
    void process_events();
    void draw_screen();
    void create_textures();
    moth_gui(moth_gui&);
    moth_gui& operator=(moth_gui&);
    public:
        void init_video();
        void book_select(std::string&);
        int read_book(moth_book*);
        moth_gui();
        virtual ~moth_gui();
};

#endif


