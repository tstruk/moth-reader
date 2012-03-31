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

#include <SDL.h>
#include <gl.h>
#include <FTGL/ftgl.h>

#include "moth.h"
#include "moth_book.h"
#include "moth_index.h"

typedef enum {
	move_right,
	move_left,
} move_direction;

class moth_gui
{
	SDL_Surface *screen;
	moth_book *book;
	moth_index index;
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
	uint16_t move_by_pages;
	uint16_t sleep_time;
	uint16_t num_pages;
	uint8_t shift_state;
	uint8_t button_state;
	uint32_t first_last_page_shitf;
	move_direction dir;
	uint32_t moving_page_ctr;
	GLuint *textures;
	uint8_t *textures_state;
	uint32_t running     : 1;
	uint32_t moving_page : 1;
	GLfloat page_info_ctr;

	static const unsigned int load_pages;
	static const unsigned int load_pages_at_start;
	static const unsigned int idle_sleep_time;
	static const unsigned int moving_sleep_time;
	static const unsigned int moving_ctr;
	static const unsigned int move_ctr_by;
	static const GLfloat page_info_ctr_val;
	static const GLfloat page_info_fade_by;

	void init_opengl();
	void handle_key_down(SDL_keysym*);
	void handle_key_up(SDL_keysym*);
	void handle_resize(SDL_ResizeEvent*);
	void handle_mouse_motion(SDL_MouseMotionEvent*);
	void handle_mouse_button(SDL_MouseButtonEvent*);
	void process_events();
	void show_pages();
	void create_textures();
	bool check_textures();
	void load_textures();
    void goto_page(unsigned int number);
	void move_page_left();
	void move_page_right();
	void page_moved();
    void print_index(moth_index *ptr);
    void free_index(moth_index *ptr);
    void show_index();
    bool has_index()
    {
        return (NULL != index.next);
    }

    double get_z_shift()
    {
        if (zoom > 1)
            return (page_width / 4) * (1/zoom);
        else
            return (page_width / 4) * zoom;
    }

	void normalize_zoom() {
		if (zoom < 0.3)
			zoom = 0.3;
		if (zoom > 3)
			zoom = 3;
	}

    void rm_newline(std::string& str)
    {
        unsigned int i = str.find('\n');
        if (i != std::string::npos)
            str.erase(i);
    }

    bool page_is_moving() {
        return moving_page;
    }
	moth_gui(moth_gui&);
	moth_gui& operator=(moth_gui&);
public:
	void init_video();
	void book_select(std::string&);
	int read_book(moth_book*);
	moth_gui();
	virtual ~moth_gui();
    friend class moth_index_gui;
};

#endif


