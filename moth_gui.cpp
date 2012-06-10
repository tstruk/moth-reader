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

#include <glew.h>
#include <gl.h>
#include <glu.h>
#include <gdk/gdk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <algorithm>
#include <unistd.h>
#include <cstdlib>
#include "moth_gui.h"
#include "moth_gui_dialog.h"

/* #define PAGE_LAYOUT_DEBUG 1 */

static const char *const font_file =
		"/usr/share/fonts/truetype/freefont/FreeSansOblique.ttf";

const unsigned int moth_gui::load_pages = 40;
const unsigned int moth_gui::load_pages_at_start = 61;
const unsigned int moth_gui::idle_sleep_time = 100000;
const unsigned int moth_gui::moving_sleep_time = 20000;
const unsigned int moth_gui::moving_ctr = 200;
const unsigned int moth_gui::move_ctr_by = 20;
const GLfloat moth_gui::page_info_ctr_val = 2.0;
const GLfloat moth_gui::page_info_fade_by = 0.1;

const char *text_info = "Please wait";
const char *text_info_tab[] = {"Mapping textures %d%%",
							   "Mapping textures %d%% .",
							   "Mapping textures %d%% ..",
							   "Mapping textures %d%% ..."};

static GLfloat font_color[] = {1.0, 0.7, 0.3, 1.0};
static GLfloat line_color[] = {0.5, 0.5, 0.5};
static GLfloat hightlight_color[] = {1.0, 0.0, 0.0};
static GLfloat normal_color[] = {1.0, 1.0, 1.0};

void moth_gui::print_index(moth_index *ptr)
{
	while (ptr)
	{
		std::cout << " " << ptr->name.c_str() << " page " << ptr->page << std::endl;
		if (ptr->child) {
			std::cout << ">";
			print_index(ptr->child);
			std::cout << "<";
		}
		ptr = ptr->next;
	}
	std::cout << std::endl;
}

void moth_gui::free_index(moth_index *ptr)
{
	/*TODO*/
}

moth_gui::~moth_gui()
{
	glDeleteTextures(num_pages, textures);
	glDeleteTextures(1, &moth_texture);
	glDeleteTextures(1, &last_page_texture);
	delete [] textures;
	delete [] textures_state;
	delete font_renderer;
	free_index(&index);
	SDL_Quit();
}

moth_gui::moth_gui()
{
	font_renderer = new FTExtrudeFont(font_file);
	if(!font_renderer) {
		moth_dialog dialog;
		std::string info("\"Can't open font file.\"");
		dialog.info(info);
		std::cerr << "Can not open file " << font_file << std::endl;
		throw moth_bad_font();
	}
	font_renderer->FaceSize(40);
	font_renderer->Depth(2);
	font_renderer->CharMap(ft_encoding_unicode);
	index.next = NULL;
	index.child = NULL;
	running = 1;
	move_by_pages = 2;
	moving_page = 0;
	shift_x = 0.0;
	shift_y = 0.0;
	zoom = 1.0;
	moving_page_ctr = 0;
	show_search_res = 0;
	first_last_page_shift = 0;
	sleep_time = idle_sleep_time;
	num_pages = 0;
	textures = NULL;
	textures_state = NULL;
	shift_state = SDL_RELEASED;
	button_state = SDL_RELEASED;
	page_split = 0.6;
}

void moth_gui::handle_resize(SDL_ResizeEvent *resize)
{
	width = resize->w;
	height = resize->h;
	float win_ratio = (float) width / (float) height;
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(90, win_ratio, 1, 1024);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(0.0, 0.0, width / 2.0f, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
	if (page_height >= page_width) {
		best_zoom = (height / page_height) * 0.8;
	}
	else {
		best_zoom = (width / (page_width * 2)) * 0.8;
	}
	zoom = best_zoom;
}

void moth_gui::handle_mouse_motion(SDL_MouseMotionEvent* motion)
{
	if (button_state == SDL_PRESSED) {
		shift_x += motion->xrel;
		shift_y -= motion->yrel;
	} else {
		if (shift_state == SDL_PRESSED) {
			zoom += motion->yrel * 0.001;
			normalize_zoom();
			stop_show_search_res();
		}
	}
}

void moth_gui::handle_mouse_button(SDL_MouseButtonEvent* button)
{
	button_state = button->state;
	if (button->button == SDL_BUTTON_WHEELDOWN)
		if (shift_state == SDL_PRESSED) {
			zoom -= zoom * 0.03;
			stop_show_search_res();
		}
	if (button->button == SDL_BUTTON_WHEELUP)
		if (shift_state == SDL_PRESSED) {
			zoom += zoom * 0.03;
			stop_show_search_res();
		}
	normalize_zoom();
}

void moth_gui::handle_key_up(SDL_keysym *key)
{
	switch(key->sym) {
	case SDLK_RSHIFT:
	case SDLK_LSHIFT:
		shift_state = SDL_RELEASED;
		break;
	default:
		break;
	}
}

void moth_gui::show_index()
{
	stop_show_search_res();
	if (!has_index()) {
		std::string info("\"This book has no index\"");
		moth_dialog dialog;
		dialog.info(info);
		return;
	}
	moth_index_gui *gui = new moth_index_gui;
	int stat = gui->show(this);
	if (stat != SUCCESS)
		std::cerr << "index gui failed" << std::endl;
	delete gui;
}

void moth_gui::handle_goto_page()
{
	std::string nr;
	std::string info("\"Go To Page Number:\"");
	moth_dialog dialog;
	moth_dialog_response resp = dialog.input(info, nr);
	if (resp == MOTH_DIALOG_OK) {
		stop_show_search_res();
		rm_newline(nr);
		if (nr.compare("last") == 0)
			goto_page(book->get_pages() - 1);
		else
			goto_page(atoi(nr.c_str()));
	}
}

void moth_gui::handle_save_copy()
{
	std::string file;
	moth_dialog dialog;
	moth_dialog_response resp = dialog.save_file(file);
	if (resp == MOTH_DIALOG_OK) {
		rm_newline(file);
		std::string url = "file://" + file;
		if (book->save_copy(url) != SUCCESS) {
			std::string err("\"Could not save file\"");
			dialog.error(err);
		}
	}
}

void moth_gui::handle_find()
{
	std::string info("\"Find:\"");
	moth_dialog dialog;
	search_string.clear();
	moth_dialog_response resp = dialog.input(info, search_string);
	if (resp == MOTH_DIALOG_OK) {
		unsigned int i;
		char found = 0;
		rm_newline(search_string);
		if (search_string.empty())
			return;
		search_results.clear();
		for(i = book->get_page(); i < book->get_pages(); i++)
		{
			book->search(search_string, i, search_results);
			if (!search_results.empty()) {
				found = 1;
				start_show_search_res();
				if (i == 0)
					i = 1;
				goto_page(i);
				break;
			}
		}
		if (!found) {
			info = "\"Text not found.\"";
			dialog.info(info);
			stop_show_search_res();
		}
	}
}

void moth_gui::handle_find_next()
{
	std::string info;
	moth_dialog dialog;
	if (!search_string.empty()) {
		char found = 0;
		unsigned int i = search_results.data()->page + 1;
		search_results.clear();
		for(; i < book->get_pages(); i++)
		{
			book->search(search_string, i, search_results);
			if (!search_results.empty()) {
				found = 1;
				start_show_search_res();
				goto_page(i+1);
				break;
			}
		}
		if (!found) {
			info = "\"Text not found.\"";
			dialog.info(info);
			stop_show_search_res();
		}
	}
}

void moth_gui::handle_find_prev()
{
	std::string info;
	moth_dialog dialog;
	if (!search_string.empty()) {
		char found = 0;
		int i = search_results.data()->page - 1;
		search_results.clear();
		for(; i >= 0; i--)
		{
			book->search(search_string, i, search_results);
			if (!search_results.empty()) {
				found = 1;
				start_show_search_res();
				goto_page(i+1);
				break;
			}
		}
		if (!found) {
			info = "\"Text not found.\"";
			dialog.info(info);
			stop_show_search_res();
		}
	}
}

void moth_gui::handle_key_down(SDL_keysym *key)
{
	switch(key->sym) {
	case SDLK_RSHIFT:
	case SDLK_LSHIFT:
		shift_state = SDL_PRESSED;
		break;
	case SDLK_ESCAPE:
		running = 0;
		break;
	case SDLK_RIGHT:
		move_page_right();
		break;
	case SDLK_LEFT:
		move_page_left();
		break;
	case SDLK_g:
		handle_goto_page();
		break;
	case SDLK_s:
		handle_save_copy();
		break;
	case SDLK_f:
		handle_find();
		break;
	case SDLK_n:
		handle_find_next();
		break;
	case SDLK_p:
		handle_find_prev();
		break;
	case SDLK_i:
		show_index();
		break;
	case SDLK_UP:
	case SDLK_DOWN:
	default:
		break;
	}
}

int moth_gui::read_book(moth_book *book)
{
	SDL_Event event;
	this->book = book;
	create_textures();
	index.name = "START";
	book->build_index(index);
	while(running) {
		while(SDL_PollEvent(&event)) {
			switch( event.type ) {
			case SDL_KEYDOWN:
				handle_key_down(&event.key.keysym);
				break;
			case SDL_KEYUP:
				handle_key_up(&event.key.keysym);
				break;
			case SDL_MOUSEMOTION:
				handle_mouse_motion(&event.motion);
				break;
			case SDL_MOUSEBUTTONDOWN:
			case SDL_MOUSEBUTTONUP:
				handle_mouse_button(&event.button);
				break;
			case SDL_VIDEORESIZE:
				handle_resize(&event.resize);
				break;
			case SDL_QUIT:
				running = 0;
				break;
			}
		}
		show_pages();
	}
	return SUCCESS;
}

bool moth_gui::check_textures()
{
	int current_page = book->get_page() & ~(0x1);
	int range;
	switch (dir) {
		case move_right:
			range = ((num_pages - current_page) >= 4) ?
						 4 : num_pages - current_page;
			for(int i = 0; i < range; i++) {
				if (!textures_state[current_page + i]) {
					return true;
				}
			}
			break;
		case move_left:
			range = (current_page >= 4) ?
						 4 : current_page;
			for(int i = range; i >= 0 && (current_page - i) < num_pages; i--) {
				if (!textures_state[current_page - i]) {
					return true;
				}
			}
			break;
	}
	return false;
}

void moth_gui::goto_page(unsigned int number)
{
	if (page_is_moving())
		return;
	if (number < 1 || number > book->get_pages()) {
		moth_dialog dialog;
		std::string info("\"Bad page number\"");
		dialog.info(info);
		return;
	}
	if (book->get_page() == (number & (int)(~0x1)))
		return;

	number = (number & ~(0x1));
	dir = (book->get_page() > (number)) ? move_left : move_right;
	int page_before = book->get_page();
	book->set_page(number);
	if (check_textures())
		load_textures();
	book->set_page(page_before);
	move_by_pages = abs(number - page_before);
	moving_page = 1;
	moving_page_ctr = moving_ctr;
	sleep_time = moving_sleep_time;
}

void moth_gui::page_moved()
{
	sleep_time = idle_sleep_time;
	if (dir == move_right) {
		book->set_page(book->get_page() + move_by_pages);
	} else {
		if (book->page_last() && book->get_page() & 1)
			book->set_page(book->get_page() - 1);
		else
			book->set_page(book->get_page() - move_by_pages);
	}
	move_by_pages = 2;
	moving_page = 0;
	page_info_ctr = page_info_ctr_val;
	font_color[3] = 1;
}

void moth_gui::move_page_left()
{
	stop_show_search_res();
	if (page_is_moving() || book->page_first())
		return;
	dir = move_left;
	if (check_textures())
		load_textures();
	moving_page = 1;
	moving_page_ctr = moving_ctr;
	sleep_time = moving_sleep_time;
}

void moth_gui::move_page_right()
{
	stop_show_search_res();
	if (page_is_moving() || book->page_last())
		return;
	dir = move_right;
	if (check_textures())
		load_textures();
	moving_page = 1;
	moving_page_ctr = moving_ctr;
	sleep_time = moving_sleep_time;
}

void moth_gui::load_textures()
{
	double w, h;
	char buff[30];
	GError *err = NULL;
	bool scale = false;
	GdkPixbuf *pixbuff_diffrent_size;
	GdkPixbuf *pixbuff_resized;
	GdkPixbuf *pixbuff = gdk_pixbuf_new(GDK_COLORSPACE_RGB,
										true, 8, page_width * 2,
										page_height * 2);
	if (!pixbuff) {
		std::cerr << "Could not allocate buffer for texture" << std::endl;
		throw moth_bad_gui();
	}
	GdkPixbuf *moth_img = gdk_pixbuf_new_from_inline(600 * 600 * 4 + 1,
                                                     (guint8*)img_data,
                                                     FALSE, &err);
	if (!moth_img) {
		std::cerr << "Could not create texture, err " << err->code << std::endl;
		std::cerr << "Err msg " << err->message << std::endl;
		g_object_unref(pixbuff);
		throw moth_bad_gui();
	}
	glBindTexture(GL_TEXTURE_2D, moth_texture);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 600, 600, 0, GL_RGBA,
				 GL_UNSIGNED_BYTE, gdk_pixbuf_get_pixels(moth_img));

	unsigned int i = (book->get_page() & ~(0x1));
	if (pages_to_load != load_pages_at_start) {
		if ((num_pages - book->get_page() - (load_pages * 2)) > 0)
			pages_to_load = load_pages;
		else
			pages_to_load = num_pages - book->get_page();

		if (num_pages > 2 && i > 2 && num_pages > i + 2) {
			i = (dir == move_right) ? i - 2 : i + 2;
		}
	}

	for(unsigned int x = 0, str_index = 0, ctr = 10; x < pages_to_load; x++) {
		/* Check if the texture for this page is already loaded */
		if (textures_state[i]) {
			i = (dir == move_right) ? i + 1 : i - 1;
			continue;
		}

		if ((i > (unsigned int)(num_pages - 1)) || (i < 0))
				break;

		/* Support only ebooks with all pages of the same size  */
		book->get_page_size(i, &w, &h);
		if (page_width != w || page_height != h) {
			/* Need to scale */
			scale = true;
			pixbuff_diffrent_size = gdk_pixbuf_new(GDK_COLORSPACE_RGB,
										true, 8, w * 2, h * 2);
			if (!pixbuff_diffrent_size) {
				std::cerr << "Could not create texture" << std::endl;
				g_object_unref(pixbuff);
				g_object_unref(moth_img);
				throw moth_bad_gui();
			}
		} else {
			scale = false;
		}

		glBindTexture(GL_TEXTURE_2D, textures[i]);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		if (scale) {

			if (SUCCESS == book->get_page(i, pixbuff_diffrent_size)) {
				pixbuff_resized = gdk_pixbuf_scale_simple(pixbuff_diffrent_size,
					                page_width * 2, page_height * 2,
									GDK_INTERP_BILINEAR);
				if (!pixbuff_resized) {
					std::cerr << "Could not resize texture" << std::endl;
					g_object_unref(pixbuff);
					g_object_unref(pixbuff_diffrent_size);
					g_object_unref(moth_img);
					throw moth_bad_gui();
				}
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, page_width * 2,
						 page_height * 2, 0, GL_RGBA,
						 GL_UNSIGNED_BYTE, gdk_pixbuf_get_pixels(pixbuff_resized));
				g_object_unref(pixbuff);
				g_object_unref(pixbuff_diffrent_size);
				textures_state[i] = 1;
			} else {
				std::cerr << "Could not get texture for page "<< i << std::endl;
				g_object_unref(pixbuff);
				g_object_unref(moth_img);
				throw moth_bad_gui();
			}
		} else {
			if (SUCCESS == book->get_page(i, pixbuff)) {
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, page_width * 2,
							 page_height * 2, 0, GL_RGBA,
							 GL_UNSIGNED_BYTE, gdk_pixbuf_get_pixels(pixbuff));
				textures_state[i] = 1;
			} else {
				std::cerr << "Could not get texture for page "<< i << std::endl;
				g_object_unref(pixbuff);
				g_object_unref(moth_img);
				throw moth_bad_gui();
			}
		}
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, moth_texture);
		glPushMatrix();
		glTranslatef(-250.0, 300.0, 0.0);
		glBegin(GL_QUADS);
		glTexCoord2d(-1.0,-1.0);
		glVertex2d(-600.0,600.0);
		glTexCoord2d(1.0,-1.0);
		glVertex2d(600.0,600.0);
		glTexCoord2d(1.0,1.0);
		glVertex2d(600.0,-600.0);
		glTexCoord2d(-1.0,1.0);
		glVertex2d(-600.0,-600.0);
		glEnd();
		glPopMatrix();
		glDisable(GL_TEXTURE_2D);
		glPushMatrix();
		glTranslatef(-160.0, -240.0, 20.0);
		if (x == ctr) {
			ctr = x + 10;
			++str_index;
			str_index = str_index % 4;
		}
		glColor3fv(font_color);
		sprintf(buff, text_info_tab[str_index],
				(int)((((double)x+1) / (double)pages_to_load) * 100));
		glDisable(GL_TEXTURE_2D);
		font_renderer->Render(buff);
		glTranslatef(100.0, -60.0, 0.0);
		font_renderer->Render(text_info);
		glEnable(GL_TEXTURE_2D);
		glPopMatrix();
		SDL_GL_SwapBuffers();
		if (dir == move_right)
			i++;
		else
			i--;
	}
	glColor3fv(normal_color);
	g_object_unref(pixbuff);
	g_object_unref(moth_img);
	pages_to_load = load_pages;
}

void moth_gui::create_textures()
{
	GError *err = NULL;
	GdkPixbuf *img = gdk_pixbuf_new_from_inline(420 * 300 * 4 + 1,
                                                (guint8*)last_page_img,
                                                FALSE, &err);
	if (!img) {
		std::cerr << "Could not create texture, err " << err->code << std::endl;
		std::cerr << "Err msg " << err->message << std::endl;
		throw moth_bad_gui();
	}

	num_pages = book->get_pages();
	book->get_page_size(0, &page_width, &page_height);
	textures = new GLuint[num_pages];
	textures_state = new uint8_t[num_pages];
	memset(textures_state, '\0', num_pages);
	memset(textures, '\0', num_pages * sizeof(GLuint));
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glGenTextures(1, &moth_texture);
	glGenTextures(1, &last_page_texture);
	glGenTextures(num_pages, textures);

	glBindTexture(GL_TEXTURE_2D, last_page_texture);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 300, 420, 0, GL_RGBA,
				 GL_UNSIGNED_BYTE, gdk_pixbuf_get_pixels(img));
	g_object_unref(img);
	dir = move_right;
	if (page_height >= page_width) {
		best_zoom = (height / page_height) * 0.8;
	}
	else {
		best_zoom = (width / (page_width * 2)) * 0.8;
	}
	zoom = best_zoom;
	pages_to_load = load_pages_at_start;
	load_textures();
	return;
}

void moth_gui::show_pages()
{
	static const int evaluators = 50;
	static GLfloat page_one[3][3][3] = {{{0}},{{0}},{{0}}};
	static GLfloat page_two[3][3][3] = {{{0}},{{0}},{{0}}};
	static GLfloat page_moving[3][3][3] = {{{0}},{{0}},{{0}}};
	static GLfloat texpts[2][2][2] = {{{0.0, 0.0}, {1.0, 0.0}},
									  {{0.0, 1.0}, {1.0, 1.0}}};
	static char buf[32] = {0};
	static const char *page_info = "Page %d out of %d";
	static const char *pages_info = "Page %d and %d out of %d";
	GLfloat z_shift;
	glColor3fv(normal_color);
	/* Assume modelview */
	/* Clear the color and depth buffers. */
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_TEXTURE_2D);

	if (book->page_first() || book->page_last()) {
		if (!page_is_moving()) {
			z_shift = 0;
			page_one[0][0][0] = (-page_width * zoom) + shift_x;
			page_one[0][0][1] = (page_height * zoom) + shift_y;
			page_one[0][0][2] = 0;
			page_one[0][1][0] = shift_x;
			page_one[0][1][1] = (page_height * zoom) + shift_y;
			page_one[0][1][2] = z_shift;
			page_one[0][2][0] = (page_width * zoom) + shift_x;
			page_one[0][2][1] = (page_height * zoom) + shift_y;
			page_one[0][2][2] = 0;

			page_one[1][0][0] = (-page_width * zoom) + shift_x;
			page_one[1][0][1] = shift_y;
			page_one[1][0][2] = 0;
			page_one[1][1][0] = shift_x;
			page_one[1][1][1] = shift_y;
			page_one[1][1][2] = z_shift;
			page_one[1][2][0] = (page_width * zoom) + shift_x;
			page_one[1][2][1] = shift_y;
			page_one[1][2][2] = 0;

			page_one[2][0][0] = (-page_width * zoom) + shift_x;
			page_one[2][0][1] = (-page_height * zoom) + shift_y;
			page_one[2][0][2] = 0;
			page_one[2][1][0] = shift_x;
			page_one[2][1][1] = (-page_height * zoom) + shift_y;
			page_one[2][1][2] = z_shift;
			page_one[2][2][0] = (page_width * zoom) + shift_x;
			page_one[2][2][1] = (-page_height * zoom) + shift_y;
			page_one[2][2][2] = 0;

			glMap2f(GL_MAP2_VERTEX_3, 0, 1, 3, 3,
					0, 1, 9, 3, &page_one[0][0][0]);
			glMap2f(GL_MAP2_TEXTURE_COORD_2, 0, 1, 2, 2,
					0, 1, 4, 2, &texpts[0][0][0]);
			if (book->page_first()) {
				glBindTexture(GL_TEXTURE_2D, textures[book->get_page()]);
			}
			else {
				if (num_pages & 1)
					glBindTexture(GL_TEXTURE_2D, last_page_texture);
				else
					glBindTexture(GL_TEXTURE_2D, textures[book->get_page()-1]);
			}
			glMapGrid2f(evaluators, 0.0, 1.0, evaluators, 0.0, 1.0);
			#ifdef PAGE_LAYOUT_DEBUG
			glEvalMesh2(GL_LINE, 0, evaluators, 0, evaluators);
			#else
			glEvalMesh2(GL_FILL, 0, evaluators, 0, evaluators);
			#endif
			if (show_search_res) {
				/* Highlight all search results */
				std::vector<moth_highlight>::iterator itr;
				glColor3fv(hightlight_color);
				glDisable(GL_TEXTURE_2D);
				glPushMatrix();
				glTranslatef(-page_width, -page_height, z_shift + 1);
				itr = search_results.begin();
				glLineWidth(1.5);
				for(; itr != search_results.end(); ++itr)
				{
					glBegin(GL_LINE_LOOP);
					glVertex2d(((*itr).x1 * zoom) + shift_x, ((*itr).y1 * zoom) + shift_y);
					glVertex2d(((*itr).x2 * zoom) + shift_x, ((*itr).y1 * zoom) + shift_y);
					glVertex2d(((*itr).x2 * zoom) + shift_x, ((*itr).y2 * zoom) + shift_y);
					glVertex2d(((*itr).x1 * zoom) + shift_x, ((*itr).y2 * zoom) + shift_y);
					glEnd();
				}
				glLineWidth(1.0);
				glEnable(GL_TEXTURE_2D);
				glPopMatrix();
			}
			glColor3fv(normal_color);
		} else {
			/* moving page first or last */
			int angle;
			int x3;
			GLfloat x1, x2;
			/* first shift the page to the side */
			if (!(first_last_page_shift >= page_width / 2)) {
				first_last_page_shift += 50;
			}
			if (book->page_first()) {
				x1 = 0;
				x2 = (page_width * 1.5 * zoom);
				x3 = first_last_page_shift;
				angle = -(moving_ctr - moving_page_ctr);
			} else {
				x1 = (-page_width * 1.5 * zoom);
				x2 = 0;
				x3 = -first_last_page_shift;
				angle = moving_ctr - moving_page_ctr;
			}
			z_shift = get_z_shift();
			page_one[0][0][0] = (-page_width * zoom) + shift_x + x3;
			page_one[0][0][1] = (page_height * zoom) + shift_y;
			page_one[0][0][2] = 0;
			page_one[0][1][0] = shift_x + x3;
			page_one[0][1][1] = (page_height * zoom) + shift_y;
			page_one[0][1][2] = z_shift;
			page_one[0][2][0] = (page_width * zoom) + shift_x + x3;
			page_one[0][2][1] = (page_height * zoom) + shift_y;
			page_one[0][2][2] = 0;

			page_one[1][0][0] = (-page_width * zoom) + shift_x + x3;
			page_one[1][0][1] = shift_y;
			page_one[1][0][2] = 0;
			page_one[1][1][0] = shift_x + x3;
			page_one[1][1][1] = shift_y;
			page_one[1][1][2] = z_shift;
			page_one[1][2][0] = (page_width * zoom) + shift_x + x3;
			page_one[1][2][1] = shift_y;
			page_one[1][2][2] = 0;

			page_one[2][0][0] = (-page_width * zoom) + shift_x + x3;
			page_one[2][0][1] = (-page_height * zoom) + shift_y;
			page_one[2][0][2] = 0;
			page_one[2][1][0] = shift_x + x3;
			page_one[2][1][1] = (-page_height * zoom) + shift_y;
			page_one[2][1][2] = z_shift;
			page_one[2][2][0] = (page_width * zoom) + shift_x + x3;
			page_one[2][2][1] = (-page_height * zoom) + shift_y;
			page_one[2][2][2] = 0;

			glMap2f(GL_MAP2_VERTEX_3, 0, 1, 3, 3,
					0, 1, 9, 3, &page_one[0][0][0]);
			glMap2f(GL_MAP2_TEXTURE_COORD_2, 0, 1, 2, 2,
					0, 1, 4, 2, &texpts[0][0][0]);

			/* TODO pick next texture depending on dir */
			glBindTexture(GL_TEXTURE_2D, textures[book->get_page()]);
			glMapGrid2f(evaluators, 0.0, 1.0, evaluators, 0.0, 1.0);
			#ifdef PAGE_LAYOUT_DEBUG
			glEvalMesh2(GL_LINE, 0, evaluators, 0, evaluators);
			#else
			glEvalMesh2(GL_FILL, 0, evaluators, 0, evaluators);
			#endif

			if (first_last_page_shift >= page_width * 0.5)
			{
				/* if shifted enough then start flipping page */
				glPushMatrix();
				glTranslatef(shift_x, 0, 0);
				glRotatef(angle, 0.0, 1.0, 0.0);
				page_moving[0][0][0] = x1;
				page_moving[0][0][1] = (page_height * zoom) + shift_y;
				page_moving[0][0][2] = 0;
				page_moving[0][1][0] = x1 * 0.5;
				page_moving[0][1][1] = (page_height * zoom) + shift_y;
				page_moving[0][1][2] = z_shift;
				page_moving[0][2][0] = x2;
				page_moving[0][2][1] = (page_height * zoom) + shift_y;
				page_moving[0][2][2] = 0;

				page_moving[1][0][0] = x1;
				page_moving[1][0][1] = shift_y;
				page_moving[1][0][2] = 0;
				page_moving[1][1][0] = x1 * 0.5;
				page_moving[1][1][1] = shift_y;
				page_moving[1][1][2] = z_shift;
				page_moving[1][2][0] = x2;
				page_moving[1][2][1] = shift_y;
				page_moving[1][2][2] = 0;

				page_moving[2][0][0] = x1;
				page_moving[2][0][1] = (-page_height * zoom) + shift_y;
				page_moving[2][0][2] = 0;
				page_moving[2][1][0] = x1 * 0.5;
				page_moving[2][1][1] = (-page_height * zoom) + shift_y;
				page_moving[2][1][2] = z_shift;
				page_moving[2][2][0] = x2;
				page_moving[2][2][1] = (-page_height * zoom) + shift_y;
				page_moving[2][2][2] = 0;

				glMap2f(GL_MAP2_VERTEX_3, 0, 1, 3, 3,
					0, 1, 9, 3, &page_moving[0][0][0]);
				glMap2f(GL_MAP2_TEXTURE_COORD_2, 0, 1, 2, 2,
					0, 1, 4, 2, &texpts[0][0][0]);

				glBindTexture(GL_TEXTURE_2D, textures[book->get_page()]);
				glMapGrid2f(evaluators, 0.0, 1.0, evaluators, 0.0, 1.0);
				#ifdef PAGE_LAYOUT_DEBUG
				glEvalMesh2(GL_LINE, 0, evaluators, 0, evaluators);
				#else
				glEvalMesh2(GL_FILL, 0, evaluators, 0, evaluators);
				#endif
				glPopMatrix();
				moving_page_ctr -= move_ctr_by;
			}
			if (!moving_page_ctr) {
				page_moved();
				first_last_page_shift = 0;
			}
		}
	} else {
		/* pages other than last and first */
		z_shift = get_z_shift();
		page_one[0][0][0] = (-page_width * 2 * zoom) + shift_x;
		page_one[0][0][1] = (page_height * zoom) + shift_y;
		page_one[0][0][2] = 0;
		page_one[0][1][0] = (-page_width * page_split * zoom) + shift_x;
		page_one[0][1][1] = (page_height * zoom) + shift_y;
		page_one[0][1][2] = z_shift;
		page_one[0][2][0] = shift_x;
		page_one[0][2][1] = (page_height * zoom) + shift_y;
		page_one[0][2][2] = 0;

		page_one[1][0][0] = (-page_width * 2 * zoom) + shift_x;
		page_one[1][0][1] = shift_y;
		page_one[1][0][2] = 0;
		page_one[1][1][0] = (-page_width * page_split * zoom) + shift_x;
		page_one[1][1][1] = shift_y;
		page_one[1][1][2] = z_shift;
		page_one[1][2][0] = shift_x;
		page_one[1][2][1] = shift_y;
		page_one[1][2][2] = 0;

		page_one[2][0][0] = (-page_width * 2 * zoom) + shift_x;
		page_one[2][0][1] = (-page_height * zoom) + shift_y;
		page_one[2][0][2] = 0;
		page_one[2][1][0] = (-page_width * page_split * zoom) + shift_x;
		page_one[2][1][1] = (-page_height * zoom) + shift_y;
		page_one[2][1][2] = z_shift;
		page_one[2][2][0] = shift_x;
		page_one[2][2][1] = (-page_height * zoom) + shift_y;
		page_one[2][2][2] = 0;

		glMap2f(GL_MAP2_VERTEX_3, 0, 1, 3, 3,
				0, 1, 9, 3, &page_one[0][0][0]);
		glMap2f(GL_MAP2_TEXTURE_COORD_2, 0, 1, 2, 2,
				0, 1, 4, 2, &texpts[0][0][0]);

		if (page_is_moving() && dir == move_left &&
						(book->get_page()) >= 3) {
				glBindTexture(GL_TEXTURE_2D, textures[book->get_page()-3]);
		}
		else {
			glBindTexture(GL_TEXTURE_2D, textures[book->get_page()-1]);
		}
		glMapGrid2f(evaluators, 0.0, 1.0, evaluators, 0.0, 1.0);
		#ifdef PAGE_LAYOUT_DEBUG
		glEvalMesh2(GL_LINE, 0, evaluators, 0, evaluators);
		#else
		glEvalMesh2(GL_FILL, 0, evaluators, 0, evaluators);
		#endif

		page_two[0][0][0] = shift_x;
		page_two[0][0][1] = (page_height * zoom) + shift_y;
		page_two[0][0][2] = 0;
		page_two[0][1][0] = (page_width * page_split * zoom) + shift_x;
		page_two[0][1][1] = (page_height * zoom) + shift_y;
		page_two[0][1][2] = z_shift;
		page_two[0][2][0] = (page_width * 2 * zoom) + shift_x;
		page_two[0][2][1] = (page_height * zoom) + shift_y;
		page_two[0][2][2] = 0;

		page_two[1][0][0] = shift_x;
		page_two[1][0][1] = shift_y;
		page_two[1][0][2] = 0;
		page_two[1][1][0] = (page_width * page_split * zoom) + shift_x;
		page_two[1][1][1] = shift_y;
		page_two[1][1][2] = z_shift;
		page_two[1][2][0] = (page_width * 2 * zoom) + shift_x;
		page_two[1][2][1] = shift_y;
		page_two[1][2][2] = 0;

		page_two[2][0][0] = shift_x;
		page_two[2][0][1] = (-page_height * zoom) + shift_y;
		page_two[2][0][2] = 0;
		page_two[2][1][0] = (page_width * page_split * zoom) + shift_x;
		page_two[2][1][1] = (-page_height * zoom) + shift_y;
		page_two[2][1][2] = z_shift;
		page_two[2][2][0] = (page_width * 2 * zoom) + shift_x;
		page_two[2][2][1] = (-page_height * zoom) + shift_y;
		page_two[2][2][2] = 0;

		glMap2f(GL_MAP2_VERTEX_3, 0, 1, 3, 3,
				0, 1, 9, 3, &page_two[0][0][0]);
		glMap2f(GL_MAP2_TEXTURE_COORD_2, 0, 1, 2, 2,
				0, 1, 4, 2, &texpts[0][0][0]);

		if (page_is_moving() && dir == move_right && moving_page_ctr > 10 &&
						(book->get_pages() - book->get_page()) >= 2) {
			glBindTexture(GL_TEXTURE_2D, textures[book->get_page() + 2]);
		}
		else {
			glBindTexture(GL_TEXTURE_2D, textures[book->get_page()]);
		}

		glMapGrid2f(evaluators, 0.0, 1.0, evaluators, 0.0, 1.0);
		#ifdef PAGE_LAYOUT_DEBUG
		glEvalMesh2(GL_LINE, 0, evaluators, 0, evaluators);
		#else
		glEvalMesh2(GL_FILL, 0, evaluators, 0, evaluators);
		#endif

		if (!page_is_moving()) {
			glPushMatrix();
			glDisable(GL_TEXTURE_2D);
			glColor3fv(line_color);
			glTranslatef(0.0, 0.0, 1.0);
			glBegin(GL_LINES);
			glVertex2d(0 + shift_x, (page_height * zoom) + shift_y);
			glVertex2d(0 + shift_x, (-page_height * zoom) + shift_y);
			glEnd();
			if (show_search_res) {
				/* Highlight all search results */
				std::vector<moth_highlight>::iterator itr;
				glColor3fv(hightlight_color);
				glPushMatrix();
				glTranslatef(0.0, -page_height, z_shift + 1);
				itr = search_results.begin();
				if ((*itr).page & 1)
					glTranslatef(-page_width * 2, 0.0, 0.0);

				glLineWidth(1.5);
				for(; itr != search_results.end(); ++itr)
				{
					glBegin(GL_LINE_LOOP);
					glVertex2d(((*itr).x1 * zoom) + shift_x, ((*itr).y1 * zoom) + shift_y);
					glVertex2d(((*itr).x2 * zoom) + shift_x, ((*itr).y1 * zoom) + shift_y);
					glVertex2d(((*itr).x2 * zoom) + shift_x, ((*itr).y2 * zoom) + shift_y);
					glVertex2d(((*itr).x1 * zoom) + shift_x, ((*itr).y2 * zoom) + shift_y);
					glEnd();
				}
				glLineWidth(1.0);
				glPopMatrix();
			}
			glColor3fv(normal_color);
			glEnable(GL_TEXTURE_2D);
			glPopMatrix();
		} else {
			/* moving pages inside the book */
			int angle;
			GLfloat x1, x2;
			if (dir == move_right) {
				x1 = 0;
				x2 = (page_width * 1.5 * zoom);
				angle = -(moving_ctr - moving_page_ctr);
			} else {
				x1 = (-page_width * 1.5 * zoom);
				x2 = 0;
				angle = moving_ctr - moving_page_ctr;
			}
			z_shift = get_z_shift();
			glPushMatrix();
			glTranslatef(shift_x, 0, 0);
			glRotatef(angle, 0.0, 1.0, 0.0);
			page_moving[0][0][0] = x1;
			page_moving[0][0][1] = (page_height * zoom) + shift_y;
			page_moving[0][0][2] = 0;
			page_moving[0][1][0] = x1 * 0.5;
			page_moving[0][1][1] = (page_height * zoom) + shift_y;
			page_moving[0][1][2] = z_shift;
			page_moving[0][2][0] = x2;
			page_moving[0][2][1] = (page_height * zoom) + shift_y;
			page_moving[0][2][2] = 0;

			page_moving[1][0][0] = x1;
			page_moving[1][0][1] = ((page_height * zoom) + shift_y) * 0.5;
			page_moving[1][0][2] = 0;
			page_moving[1][1][0] = x1 * 0.5;
			page_moving[1][1][1] = ((page_height * zoom) + shift_y) * 0.5;
			page_moving[1][1][2] = z_shift;
			page_moving[1][2][0] = x2;
			page_moving[1][2][1] = ((page_height * zoom) + shift_y) * 0.5;
			page_moving[1][2][2] = 0;

			page_moving[2][0][0] = x1;
			page_moving[2][0][1] = (-page_height * zoom) + shift_y;
			page_moving[2][0][2] = 0;
			page_moving[2][1][0] = x1 * 0.5;
			page_moving[2][1][1] = (-page_height * zoom) + shift_y;
			page_moving[2][1][2] = z_shift;
			page_moving[2][2][0] = x2;
			page_moving[2][2][1] = (-page_height * zoom) + shift_y;
			page_moving[2][2][2] = 0;

			glMap2f(GL_MAP2_VERTEX_3, 0, 1, 3, 3,
					0, 1, 9, 3, &page_moving[0][0][0]);
			glMap2f(GL_MAP2_TEXTURE_COORD_2, 0, 1, 2, 2,
					0, 1, 4, 2, &texpts[0][0][0]);

			/* TODO pick next texture depending on dir */
			glBindTexture(GL_TEXTURE_2D, textures[book->get_page()]);
			glMapGrid2f(evaluators, 0.0, 1.0, evaluators, 0.0, 1.0);
			#ifdef PAGE_LAYOUT_DEBUG
			glEvalMesh2(GL_LINE, 0, evaluators, 0, evaluators);
			#else
			glEvalMesh2(GL_FILL, 0, evaluators, 0, evaluators);
			#endif
			glPopMatrix();
			moving_page_ctr -= move_ctr_by;
			if (!moving_page_ctr) {
				page_moved();
			}
		}
	}
	if (!page_is_moving() && page_info_ctr > 0) {
		int p = book->get_pages();
		if (book->page_first())
			sprintf(buf, page_info, 1, p);
		else if (book->page_last())
			sprintf(buf, page_info, p, p);
		else
			sprintf(buf, pages_info, book->get_page(), book->get_page() + 1, p);
		/* Show page info */
		if (page_info_ctr < 1)
			font_color[3] = page_info_ctr;

		font_renderer->FaceSize(30);
		glDisable(GL_TEXTURE_2D);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glPushMatrix();
		glTranslatef(-120, height - 230, 100.0);
		glColor4fv(font_color);
		font_renderer->Render(buf);
		glPopMatrix();
		glColor3fv(normal_color);
		glDisable(GL_BLEND);
		glEnable(GL_TEXTURE_2D);
		page_info_ctr -= page_info_fade_by;
		font_renderer->FaceSize(40);
	}
	SDL_GL_SwapBuffers();
	usleep(sleep_time);
}

void moth_gui::init_opengl()
{
	GLenum err = glewInit();
	if(GLEW_OK != err) {
		std::cerr<< "Failed to initialize GL Extension Wrangler library"
				 << std::endl;
		throw moth_bad_ogl();
	}
	/* For texture sizes different than 2^m + 2b need opengl 2.0 or higher*/
	if(!GLEW_VERSION_2_0) {
		std::cerr<< "OpenGL 2.0 or greater is required" << std::endl;
		throw moth_bad_ogl();
	}

	glEnable(GL_TEXTURE_2D);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_MAP2_TEXTURE_COORD_2);
	glEnable(GL_MAP2_VERTEX_3);
	glShadeModel(GL_FLAT);
	float win_ratio = (float) width / (float) height;
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(90, win_ratio, 1, 1024);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(0.0, 0.0, width / 2.0f, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
}

void moth_gui::init_video()
{
	GdkScreen *gdk_screen = gdk_screen_get_default();
	int error = SDL_Init(SDL_INIT_VIDEO);
	if (error != SUCCESS || NULL == gdk_screen) {
		std::cerr<< "Video initialization failed: " <<
				 SDL_GetError( ) << std::endl;
		throw moth_bad_gui();
	}
	int monitor = gdk_screen_get_primary_monitor(gdk_screen);
	GdkRectangle res;
	gdk_screen_get_monitor_geometry(gdk_screen, monitor, &res);
	const SDL_VideoInfo *info = SDL_GetVideoInfo();
	if (!info) {
		std::cerr<< "Get Video info failed: " <<
				 SDL_GetError( ) << std::endl;
		throw moth_bad_gui();
	}
	bpp = info->vfmt->BitsPerPixel;
	width = res.width;
	height = res.height;

	flags = SDL_DOUBLEBUF | SDL_HWSURFACE | SDL_OPENGL | SDL_RESIZABLE;
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 5);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	screen = SDL_SetVideoMode(width, height, bpp, flags);
	if(NULL == screen) {
		std::cerr<< "Set Video Mode failed: " <<
				 SDL_GetError( ) << std::endl;
		throw moth_bad_gui();
	}
	SDL_WM_SetCaption("moth - " MOTH_VER_STRING, NULL);
	init_opengl();
}

void moth_gui::book_select(std::string &file)
{
	moth_dialog file_dialog;
	std::string type("\"Ebooks | *.pdf *.mobi\"");
	moth_dialog_response resp = file_dialog.choose_file(type, file);
	if(resp != MOTH_DIALOG_OK || file.empty())
		throw moth_bad_cancel();
	rm_newline(file);
}

