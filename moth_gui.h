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

typedef enum {
	move_right,
	move_left,
} move_direction;

class moth_gui
{
	Gtk::Window *win;
	SDL_Surface *screen;
	moth_book *book;
	FTFont *font_renderer;
	uint32_t bpp;
	uint32_t flags;
	double width;
	double height;
	double page_width;
	double page_height;
	double ratio;
	double zoom;
	double shift_x;
	double shift_y;
	uint32_t move_by_pages;
	uint32_t sleep_time;
	uint16_t num_pages;
	uint8_t shift_state;
	uint8_t button_state;
	uint32_t first_last_page_shitf;
	move_direction dir;
	uint32_t moving_page_ctr;
	GLuint *textures;
	uint32_t running     : 1;
	uint32_t moving_page : 1;

	static const unsigned int max_pages;
	static const unsigned int idle_sleep_time;
	static const unsigned int moving_sleep_time;
	static const unsigned int moving_ctr;
	static const unsigned int move_ctr_by;

	void init_opengl();
	void handle_key_down(SDL_keysym*);
	void handle_key_up(SDL_keysym*);
	void handle_resize(SDL_ResizeEvent*);
	void handle_mouse_motion(SDL_MouseMotionEvent*);
	void handle_mouse_button(SDL_MouseButtonEvent*);
	void process_events();
	void draw_screen();
	void create_textures();

	void normalize_zoom() {
		if(zoom < 0.3)
			zoom = 0.3;
		if(zoom > 3)
			zoom = 3;
	}

	void move_page_left() {
		if (moving_page || book->page_first())
			return;
		moving_page = 1;
		dir = move_left;
		moving_page_ctr = moving_ctr;
		sleep_time = moving_sleep_time;
	}

	void move_page_right() {
		if (moving_page || book->page_last())
			return;
		moving_page = 1;
		dir = move_right;
		moving_page_ctr = moving_ctr;
		sleep_time = moving_sleep_time;
	}

	void page_moved() {
		moving_page = 0;
		sleep_time = idle_sleep_time;
		if (dir == move_right) {
			book->set_page(book->get_page() + move_by_pages);
			move_by_pages = 2;
		} else {
			book->set_page(book->get_page() - move_by_pages);
			if(book->get_page() == 0)
				move_by_pages = 1;
		}
	}

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


