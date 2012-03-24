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
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <algorithm>
#include <unistd.h>
#include "moth_gui.h"
#include "moth_gui_dialog.h"

static const char *const font_file =
        "/usr/share/fonts/truetype/freefont/FreeSansOblique.ttf";

const unsigned int moth_gui::load_pages = 40;
const unsigned int moth_gui::load_pages_at_start = 60;
const unsigned int moth_gui::idle_sleep_time = 50000;
const unsigned int moth_gui::moving_sleep_time = 500;
const unsigned int moth_gui::moving_ctr = 180;
const unsigned int moth_gui::move_ctr_by = 20;
const GLfloat moth_gui::page_info_ctr_val = 2.0;
const GLfloat moth_gui::page_info_fade_by = 0.1;

const char *text_info = "Please wait";
const char *text_info_tab[] = {"Mapping textures %d%%",
	                           "Mapping textures %d%% .",
	                           "Mapping textures %d%% ..",
	                           "Mapping textures %d%% ..."};

static GLfloat font_color[] = {0.0, 0.0, 1.0, 1.0};
static GLfloat line_color[] = {0.5, 0.5, 0.5};
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
    first_last_page_shitf = 0;
	sleep_time = idle_sleep_time;
    num_pages = 0;
    textures = NULL;
    textures_state = NULL;
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
	ratio = std::min((width / page_width),(height / page_height)) * 0.8;
}

void moth_gui::handle_mouse_motion(SDL_MouseMotionEvent* motion)
{
	if(button_state == SDL_PRESSED) {
		shift_x += motion->xrel;
		shift_y -= motion->yrel;
	} else {
		if(shift_state == SDL_PRESSED) {
			zoom += motion->yrel * 0.001;
			normalize_zoom();
		}
	}
}

void moth_gui::handle_mouse_button(SDL_MouseButtonEvent* button)
{
	button_state = button->state;
	if(button->button == SDL_BUTTON_WHEELDOWN)
		if(shift_state == SDL_PRESSED)
			zoom -= zoom * 0.03;
	if(button->button == SDL_BUTTON_WHEELUP)
		if(shift_state == SDL_PRESSED)
			zoom += zoom * 0.03;
	normalize_zoom();
}

void moth_gui::handle_key_up(SDL_keysym *key)
{
	switch(key->sym) {
	case SDLK_RSHIFT:
	case SDLK_LSHIFT:
		shift_state = SDL_RELEASED;
		break;
	}
}

void moth_gui::show_index()
{
    if (!has_index()) {
        std::cerr << "book has no index" << std::endl;
        return;
    }
    moth_index_gui *gui = new moth_index_gui;
    int stat = gui->show(this);
    if (stat != SUCCESS)
        std::cerr << "index gui failed" << std::endl;
    delete gui;
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
        {
            std::string nr;
            std::string info("\"Page number\"");
            moth_dialog dialog;
            moth_dialog_response resp = dialog.input(info, nr);
	        if (resp == MOTH_DIALOG_OK) {
                rm_newline(nr);
                if (nr.compare("last") == 0)
                    goto_page(book->get_pages() - 1);
                else
                    goto_page(atoi(nr.c_str()));
            }
        }
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
    int current_page = book->get_page();
    int range;
    switch (dir) {
        case move_right:
            range = ((num_pages - current_page) >= 4) ?
                         4 : num_pages - current_page;
            for(int i = 0; i < range; i++)
                if (!textures_state[current_page + i])
                    return true;
              break;
        case move_left:
           range = (current_page >= 4) ?
                         4 : current_page;
            for(int i = range; i >= 0; i--)
                if (!textures_state[current_page - i])
                    return true;
              break;
    }
    return false;
}

void moth_gui::goto_page(int number)
{
	if (page_is_moving())
		return;
	if (number < 1 || number > book->get_pages()) {
        moth_dialog dialog;
        std::string info("\"Bad page number\"");
        dialog.info(info);
        return;
    }
    if (book->get_page() == number - 1)
        return;

    number--;
    if (number & 1)
        number--;

    dir = (book->get_page() > (number)) ? move_left : move_right;
    book->set_page(number);
    if (check_textures())
        load_textures();
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
	moving_page = 0;
    page_info_ctr = page_info_ctr_val;
    font_color[3] = 1;
	std::cout << "on page " << book->get_page() << std::endl;
}

void moth_gui::move_page_left()
{
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
	GError *error = NULL;
	char buff[30];
    int pages_to_load;
	num_pages = book->get_pages();
	GdkPixbuf *pixbuff = gdk_pixbuf_new(GDK_COLORSPACE_RGB,
	                                    true, 8, page_width * 2,
                                        page_height * 2);
	if (!pixbuff) {
		std::cerr << "Could not allocate buffer for texture" << std::endl;
		throw moth_bad_gui();
	}
    if((num_pages - book->get_page() - (load_pages * 2)) > 0)
        pages_to_load = load_pages;
    else
        pages_to_load = num_pages - book->get_page();

	for(int x = 0, i = book->get_page(),
                   index = 0, ctr = 10; x < pages_to_load; x++) {

        if (dir == move_right) {
            if (((book->get_page()) + x) == num_pages - 1)
                break;
        }
        else {
            if (((book->get_page()) - x) == 0)
                break;
        }

        if(textures_state[i])
            continue;
		std::cout << "loading textures for page " << i << " x " << x << std::endl;
		book->get_page_size(i, &w, &h);
		if (page_width != w || page_height != h) {
			std::cerr << "Page "<< i <<" has different size " << h << "x"
			          << w << std::endl;
			std::cerr << "Currently documents with " <<
                         "different page sizes are not supported" << std::endl;
			g_object_unref(pixbuff);
			throw moth_bad_pdf();
		}
	    glEnable(GL_TEXTURE_2D);
		if (SUCCESS == book->get_page(i, pixbuff)) {
			glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			/* Enhance the picture */
			gdk_pixbuf_saturate_and_pixelate(pixbuff, pixbuff, 2.0, 0);
			glBindTexture(GL_TEXTURE_2D, textures[i]);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, page_width * 2,
			             page_height * 2, 0, GL_RGBA,
			             GL_UNSIGNED_BYTE, gdk_pixbuf_get_pixels(pixbuff));
            textures_state[i] = 1;
		} else {
			std::cerr << "Could not get texture for page "<< i << std::endl;
			throw moth_bad_gui();
		}
        if (dir == move_right && i + 1 == num_pages && num_pages & 0x1) {
		    std::cout << "and one more " << std::endl;
			glBindTexture(GL_TEXTURE_2D, textures[i + 1]);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, page_width * 2,
			             page_height * 2, 0, GL_RGBA,
			             GL_UNSIGNED_BYTE, gdk_pixbuf_get_pixels(pixbuff));
            textures_state[i + 1] = 1;
        }
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	    glDisable(GL_TEXTURE_2D);
		glPushMatrix();
		glTranslatef(-160.0, -240.0, 20.0);
		glColor3fv(font_color);
		if (x == ctr) {
			ctr = x + 10;
			index = ++index % 4;
		}
		sprintf(buff, text_info_tab[index],
		        (int)((((double)x+1) / (double)pages_to_load) * 100));
		font_renderer->Render(buff);
		glTranslatef(100.0, -60.0, 0.0);
		font_renderer->Render(text_info);
		glPopMatrix();
		SDL_GL_SwapBuffers();
        if (dir == move_right)
            i++;
        else
            i--;
	}
	glColor3fv(normal_color);
	g_object_unref(pixbuff);
}

void moth_gui::create_textures()
{
	double w, h;
	GError *error = NULL;
	char buff[30];
    int pages_to_alloc;
	num_pages = book->get_pages();
    if (num_pages & 0x1)
        pages_to_alloc = num_pages + 1;
    else
        pages_to_alloc = num_pages;

	std::cout << "pages to alloc " << pages_to_alloc << std::endl;
	book->get_page_size(0, &page_width, &page_height);
	ratio = std::min((width / page_width),(height / page_height)) * 0.8;
	GdkPixbuf *pixbuff = gdk_pixbuf_new(GDK_COLORSPACE_RGB,
	                                    true, 8, page_width * 2,
                                        page_height * 2);
	if (!pixbuff) {
		std::cerr << "Could not allocate buffer for texture" << std::endl;
		throw moth_bad_gui();
	}

	textures = new GLuint[pages_to_alloc];
	textures_state = new uint8_t[pages_to_alloc];
    memset(textures_state, '\0', pages_to_alloc);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glGenTextures(num_pages, textures);
	for(int i = 0, index = 0, ctr = 10; i < num_pages
                         && i < load_pages_at_start; i++) {

		book->get_page_size(i, &w, &h);
		if (page_width != w || page_height != h) {
			std::cerr << "Page "<< i <<" has different size " << h << "x"
			          << w << std::endl;
			std::cerr << "Currently documents with " <<
                         "different page sizes are not supported" << std::endl;
			g_object_unref(pixbuff);
			throw moth_bad_pdf();
		}
		if (SUCCESS == book->get_page(i, pixbuff)) {
			glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			/* Enhance the picture */
			gdk_pixbuf_saturate_and_pixelate(pixbuff, pixbuff, 2.0, 0);
			glBindTexture(GL_TEXTURE_2D, textures[i]);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, page_width * 2,
			             page_height * 2, 0, GL_RGBA,
			             GL_UNSIGNED_BYTE, gdk_pixbuf_get_pixels(pixbuff));
            textures_state[i] = 1;
		} else {
			std::cerr << "Could not get texture for page " << i << std::endl;
			throw moth_bad_gui();
		}
		std::cout << "creating textures for page " << i << std::endl;
        if (i + 1 == num_pages && num_pages & 0x1) {
		    std::cout << "and one more " << std::endl;
			glBindTexture(GL_TEXTURE_2D, textures[i + 1]);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, page_width * 2,
			             page_height * 2, 0, GL_RGBA,
			             GL_UNSIGNED_BYTE, gdk_pixbuf_get_pixels(pixbuff));
            textures_state[i + 1] = 1;
        }
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glPushMatrix();
		glTranslatef(-160.0, -240.0, 20.0);
		glColor3fv(font_color);
		if (i == ctr) {
			ctr = i + 10;
			index = ++index % 4;
		}
		sprintf(buff, text_info_tab[index],
		        (int)((((double)(i+1)) /
                        ((double)std::min((int)num_pages,
                            (int)load_pages_at_start))) * 100));
		font_renderer->Render(buff);
		glTranslatef(100.0, -60.0, 0.0);
		font_renderer->Render(text_info);
		glPopMatrix();
		SDL_GL_SwapBuffers();
	}
	glColor3fv(normal_color);
	g_object_unref(pixbuff);
}

void moth_gui::show_pages()
{
	static const int evaluators = 200;
	static GLfloat page_one[3][3][3] = {{0},{0},{0}};
	static GLfloat page_two[3][3][3] = {{0},{0},{0}};
	static GLfloat page_moving[3][3][3] = {{0},{0},{0}};
	static GLfloat texpts[2][2][2] = {{{0.0, 0.0}, {1.0, 0.0}},
	                                  {{0.0, 1.0}, {1.0, 1.0}}};
    static char buf[25] = {0};
    static const char *page_info = "Page %d out of %d";
	GLfloat z_shift;
	glColor3fv(normal_color);
	/* Assume modelview */
	/* Clear the color and depth buffers. */
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (book->page_first() || book->page_last()) {
		if (!page_is_moving()) {
			z_shift = 0;
			page_one[0][0][0] = (-page_width * ratio * zoom) + shift_x;
			page_one[0][0][1] = (page_height * ratio * zoom) + shift_y;
			page_one[0][0][2] = 0;
			page_one[0][1][0] = ((page_width * ratio * zoom) + shift_x) / 4;
			page_one[0][1][1] = (page_height * ratio * zoom) + shift_y;
			page_one[0][1][2] = z_shift;
			page_one[0][2][0] = (page_width * ratio * zoom) + shift_x;
			page_one[0][2][1] = (page_height * ratio * zoom) + shift_y;
			page_one[0][2][2] = 0;

			page_one[1][0][0] = (-page_width * ratio * zoom) + shift_x;
			page_one[1][0][1] = ((page_height * ratio * zoom) + shift_y) / 2;
			page_one[1][0][2] = 0;
			page_one[1][1][0] = ((page_width * ratio * zoom) + shift_x) / 4;
			page_one[1][1][1] = ((page_height * ratio * zoom) + shift_y) / 2;
			page_one[1][1][2] = z_shift;
			page_one[1][2][0] = (page_width * ratio * zoom) + shift_x;
			page_one[1][2][1] = ((page_height * ratio * zoom) + shift_y) / 2;
			page_one[1][2][2] = 0;

			page_one[2][0][0] = (-page_width * ratio * zoom) + shift_x;
			page_one[2][0][1] = (-page_height * ratio * zoom) + shift_y;
			page_one[2][0][2] = 0;
			page_one[2][1][0] = ((page_width * ratio * zoom) + shift_x) / 4;
			page_one[2][1][1] = (-page_height * ratio * zoom) + shift_y;
			page_one[2][1][2] = z_shift;
			page_one[2][2][0] = (page_width * ratio * zoom) + shift_x;
			page_one[2][2][1] = (-page_height * ratio * zoom) + shift_y;
			page_one[2][2][2] = 0;

			glMap2f(GL_MAP2_VERTEX_3, 0, 1, 3, 3,
			        0, 1, 9, 3, &page_one[0][0][0]);
			glMap2f(GL_MAP2_TEXTURE_COORD_2, 0, 1, 2, 2,
			        0, 1, 4, 2, &texpts[0][0][0]);

			glBindTexture(GL_TEXTURE_2D, textures[book->get_page()]);
			glMapGrid2f(evaluators, 0.0, 1.0, evaluators, 0.0, 1.0);
			glEvalMesh2(GL_FILL, 0, evaluators, 0, evaluators);
		} else {
            /* moving page first or last */
			int angle;
            int x3;
			GLfloat x1, x2;
            /* fisrt shift the page to side */
            if (!(first_last_page_shitf >= page_width / 2)) {
                first_last_page_shitf += 50;
            }
	        if (book->page_first()) {
				x1 = 0;
				x2 = (page_width * 1.5 *  ratio * zoom);
                x3 = first_last_page_shitf;
				angle = -(moving_ctr - moving_page_ctr);
			} else {
				x1 = (-page_width * 1.5 * ratio * zoom);
				x2 = 0;
                x3 = -first_last_page_shitf;
				angle = moving_ctr - moving_page_ctr;
			}
            z_shift = get_z_shift();
			page_one[0][0][0] = (-page_width * ratio * zoom) + shift_x + x3;
			page_one[0][0][1] = (page_height * ratio * zoom) + shift_y;
			page_one[0][0][2] = 0;
			page_one[0][1][0] = (((page_width * ratio * zoom) + shift_x) / 4) + x3;
			page_one[0][1][1] = (page_height * ratio * zoom) + shift_y;
			page_one[0][1][2] = z_shift;
			page_one[0][2][0] = (page_width * ratio * zoom) + shift_x + x3;
			page_one[0][2][1] = (page_height * ratio * zoom) + shift_y;
			page_one[0][2][2] = 0;

			page_one[1][0][0] = (-page_width * ratio * zoom) + shift_x + x3;
			page_one[1][0][1] = ((page_height * ratio * zoom) + shift_y) / 2;
			page_one[1][0][2] = 0;
			page_one[1][1][0] = (((page_width * ratio * zoom) + shift_x) / 4) + x3;
			page_one[1][1][1] = ((page_height * ratio * zoom) + shift_y) / 2;
			page_one[1][1][2] = z_shift;
			page_one[1][2][0] = (page_width * ratio * zoom) + shift_x + x3;
			page_one[1][2][1] = ((page_height * ratio * zoom) + shift_y) / 2;
			page_one[1][2][2] = 0;

			page_one[2][0][0] = (-page_width * ratio * zoom) + shift_x + x3;
			page_one[2][0][1] = (-page_height * ratio * zoom) + shift_y;
			page_one[2][0][2] = 0;
			page_one[2][1][0] = ((page_width * ratio * zoom) + shift_x) / 4;
			page_one[2][1][1] = (-page_height * ratio * zoom) + shift_y;
			page_one[2][1][2] = z_shift;
			page_one[2][2][0] = (page_width * ratio * zoom) + shift_x + x3;
			page_one[2][2][1] = (-page_height * ratio * zoom) + shift_y;
			page_one[2][2][2] = 0;

			glMap2f(GL_MAP2_VERTEX_3, 0, 1, 3, 3,
			        0, 1, 9, 3, &page_one[0][0][0]);
			glMap2f(GL_MAP2_TEXTURE_COORD_2, 0, 1, 2, 2,
			        0, 1, 4, 2, &texpts[0][0][0]);

                /* TODO pick next texture depending on dir */
			glBindTexture(GL_TEXTURE_2D, textures[book->get_page()]);
			glMapGrid2f(evaluators, 0.0, 1.0, evaluators, 0.0, 1.0);
			glEvalMesh2(GL_FILL, 0, evaluators, 0, evaluators);

            if (first_last_page_shitf >= page_width / 2)
            {
                /* if shifted enough then start flipping page */
			    glPushMatrix();
                glTranslatef(shift_x, 0, 0);
		        glRotatef(angle, 0.0, 1.0, 0.0);
			    page_moving[0][0][0] = x1;
			    page_moving[0][0][1] = (page_height * ratio * zoom) + shift_y;
			    page_moving[0][0][2] = 0;
			    page_moving[0][1][0] = x1 / 2;
			    page_moving[0][1][1] = (page_height * ratio * zoom) + shift_y;
			    page_moving[0][1][2] = z_shift;
			    page_moving[0][2][0] = x2;
			    page_moving[0][2][1] = (page_height * ratio * zoom) + shift_y;
			    page_moving[0][2][2] = 0;

			    page_moving[1][0][0] = x1;
			    page_moving[1][0][1] = ((page_height * ratio * zoom) + shift_y) / 2;
			    page_moving[1][0][2] = 0;
			    page_moving[1][1][0] = x1 / 2;
			    page_moving[1][1][1] = ((page_height * ratio * zoom) + shift_y) / 2;
			    page_moving[1][1][2] = z_shift;
			    page_moving[1][2][0] = x2;
			    page_moving[1][2][1] = ((page_height * ratio * zoom) + shift_y) / 2;
			    page_moving[1][2][2] = 0;

			    page_moving[2][0][0] = x1;
			    page_moving[2][0][1] = (-page_height * ratio * zoom) + shift_y;
			    page_moving[2][0][2] = 0;
			    page_moving[2][1][0] = x1 / 2;
			    page_moving[2][1][1] = (-page_height * ratio * zoom) + shift_y;
			    page_moving[2][1][2] = z_shift;
			    page_moving[2][2][0] = x2;
			    page_moving[2][2][1] = (-page_height * ratio * zoom) + shift_y;
			    page_moving[2][2][2] = 0;

			    glMap2f(GL_MAP2_VERTEX_3, 0, 1, 3, 3,
			        0, 1, 9, 3, &page_moving[0][0][0]);
			    glMap2f(GL_MAP2_TEXTURE_COORD_2, 0, 1, 2, 2,
			        0, 1, 4, 2, &texpts[0][0][0]);

			    glBindTexture(GL_TEXTURE_2D, textures[book->get_page()]);
			    glMapGrid2f(evaluators, 0.0, 1.0, evaluators, 0.0, 1.0);
			    glEvalMesh2(GL_FILL, 0, evaluators, 0, evaluators);
			    glPopMatrix();
			    moving_page_ctr -= move_ctr_by;
            }
			if (!moving_page_ctr) {
				page_moved();
                first_last_page_shitf = 0;
			}
		}
	} else {
        /* pages other than last and first */
        z_shift = get_z_shift();
		page_one[0][0][0] = (-page_width * 2 * ratio * zoom) + shift_x;
		page_one[0][0][1] = (page_height * ratio * zoom) + shift_y;
		page_one[0][0][2] = 0;
		page_one[0][1][0] = ((-page_width * 2 * ratio * zoom) + shift_x) / 4;
		page_one[0][1][1] = (page_height * ratio * zoom) + shift_y;
		page_one[0][1][2] = z_shift;
		page_one[0][2][0] = shift_x;
		page_one[0][2][1] = (page_height * ratio * zoom) + shift_y;
		page_one[0][2][2] = 0;

		page_one[1][0][0] = (-page_width * 2 * ratio * zoom) + shift_x;
		page_one[1][0][1] = ((page_height * ratio * zoom) + shift_y) / 2;
		page_one[1][0][2] = 0;
		page_one[1][1][0] = ((-page_width * 2 * ratio * zoom) + shift_x) / 4;
		page_one[1][1][1] = ((page_height * ratio * zoom) + shift_y) / 2;
		page_one[1][1][2] = z_shift;
		page_one[1][2][0] = shift_x;
		page_one[1][2][1] = ((page_height * ratio * zoom) + shift_y) / 2;
		page_one[1][2][2] = 0;

		page_one[2][0][0] = (-page_width * 2 * ratio * zoom) + shift_x;
		page_one[2][0][1] = (-page_height * ratio * zoom) + shift_y;
		page_one[2][0][2] = 0;
		page_one[2][1][0] = ((-page_width * 2 * ratio * zoom) + shift_x) / 4;
		page_one[2][1][1] = (-page_height * ratio * zoom) + shift_y;
		page_one[2][1][2] = z_shift;
		page_one[2][2][0] = shift_x;
		page_one[2][2][1] = (-page_height * ratio * zoom) + shift_y;
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
		glEvalMesh2(GL_FILL, 0, evaluators, 0, evaluators);

		page_two[0][0][0] = shift_x;
		page_two[0][0][1] = (page_height * ratio * zoom) + shift_y;
		page_two[0][0][2] = 0;
		page_two[0][1][0] = ((page_width * ratio * zoom) + shift_x) / 4;
		page_two[0][1][1] = (page_height * ratio * zoom) + shift_y;
		page_two[0][1][2] = z_shift;
		page_two[0][2][0] = (page_width * 2 * ratio * zoom) + shift_x;
		page_two[0][2][1] = (page_height * ratio * zoom) + shift_y;
		page_two[0][2][2] = 0;

		page_two[1][0][0] = shift_x;
		page_two[1][0][1] = ((page_height * ratio * zoom) + shift_y) / 2;
		page_two[1][0][2] = 0;
		page_two[1][1][0] = ((page_width * ratio * zoom) + shift_x) / 4;
		page_two[1][1][1] = ((page_height * ratio * zoom) + shift_y) / 2;
		page_two[1][1][2] = z_shift;
		page_two[1][2][0] = (page_width * 2 * ratio * zoom) + shift_x;
		page_two[1][2][1] = ((page_height * ratio * zoom) + shift_y) / 2;
		page_two[1][2][2] = 0;

		page_two[2][0][0] = shift_x;
		page_two[2][0][1] = (-page_height * ratio * zoom) + shift_y;
		page_two[2][0][2] = 0;
		page_two[2][1][0] = ((page_width * ratio * zoom) + shift_x) / 4;
		page_two[2][1][1] = (-page_height * ratio * zoom) + shift_y;
		page_two[2][1][2] = z_shift;
		page_two[2][2][0] = (page_width * 2 * ratio * zoom) + shift_x;
		page_two[2][2][1] = (-page_height * ratio * zoom) + shift_y;
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
		glEvalMesh2(GL_FILL, 0, evaluators, 0, evaluators);

		if (!page_is_moving()) {
			glPushMatrix();
			glDisable(GL_TEXTURE_2D);
			glColor3fv(line_color);
			glTranslatef(0.0, 0.0, 1.0);
			glBegin(GL_LINES);
			glVertex2d(0 + shift_x, (page_height * ratio * zoom) + shift_y);
			glVertex2d(0 + shift_x, (-page_height * ratio * zoom) + shift_y);
			glEnd();
			glColor3fv(normal_color);
			glEnable(GL_TEXTURE_2D);
			glPopMatrix();
		} else {
            /* moving pages inside the book */
			int angle;
			GLfloat x1, x2;
			if(dir == move_right) {
				x1 = 0;
				x2 = (page_width * 1.5 *  ratio * zoom);
				angle = -(moving_ctr - moving_page_ctr);
			} else {
				x1 = (-page_width * 1.5 * ratio * zoom);
				x2 = 0;
				angle = moving_ctr - moving_page_ctr;
			}
		    z_shift = get_z_shift();
			glPushMatrix();
			glTranslatef(shift_x, 0, 0);
			glRotatef(angle, 0.0, 1.0, 0.0);
			page_moving[0][0][0] = x1;
			page_moving[0][0][1] = (page_height * ratio * zoom) + shift_y;
			page_moving[0][0][2] = 0;
			page_moving[0][1][0] = x1 / 2;
			page_moving[0][1][1] = (page_height * ratio * zoom) + shift_y;
			page_moving[0][1][2] = z_shift;
			page_moving[0][2][0] = x2;
			page_moving[0][2][1] = (page_height * ratio * zoom) + shift_y;
			page_moving[0][2][2] = 0;

			page_moving[1][0][0] = x1;
			page_moving[1][0][1] = ((page_height * ratio * zoom) + shift_y) / 2;
			page_moving[1][0][2] = 0;
			page_moving[1][1][0] = x1 / 2;
			page_moving[1][1][1] = ((page_height * ratio * zoom) + shift_y) / 2;
			page_moving[1][1][2] = z_shift;
			page_moving[1][2][0] = x2;
			page_moving[1][2][1] = ((page_height * ratio * zoom) + shift_y) / 2;
			page_moving[1][2][2] = 0;

			page_moving[2][0][0] = x1;
			page_moving[2][0][1] = (-page_height * ratio * zoom) + shift_y;
			page_moving[2][0][2] = 0;
			page_moving[2][1][0] = x1 / 2;
			page_moving[2][1][1] = (-page_height * ratio * zoom) + shift_y;
			page_moving[2][1][2] = z_shift;
			page_moving[2][2][0] = x2;
			page_moving[2][2][1] = (-page_height * ratio * zoom) + shift_y;
			page_moving[2][2][2] = 0;

			glMap2f(GL_MAP2_VERTEX_3, 0, 1, 3, 3,
			        0, 1, 9, 3, &page_moving[0][0][0]);
			glMap2f(GL_MAP2_TEXTURE_COORD_2, 0, 1, 2, 2,
			        0, 1, 4, 2, &texpts[0][0][0]);

                /* TODO pick next texture depending on dir */
			glBindTexture(GL_TEXTURE_2D, textures[book->get_page()]);
			glMapGrid2f(evaluators, 0.0, 1.0, evaluators, 0.0, 1.0);
			glEvalMesh2(GL_FILL, 0, evaluators, 0, evaluators);
			glPopMatrix();
			moving_page_ctr -= move_ctr_by;
			if (!moving_page_ctr) {
				page_moved();
			}
		}
	}
    if (!page_is_moving() && page_info_ctr > 0) {
        sprintf(buf, page_info, (book->get_page() == 0) ? 1 : book->get_page(),
                                                            book->get_pages());
        if (page_info_ctr < 1)
            font_color[3] = page_info_ctr;

	    font_renderer->FaceSize(30);
        glDisable(GL_TEXTURE_2D);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glPushMatrix();
        glTranslatef(/*-width + 300*/-120, height - 230, 100.0);
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
	int error = SDL_Init(SDL_INIT_VIDEO);
	if ( error != SUCCESS ) {
		std::cerr<< "Video initialization failed: " <<
		         SDL_GetError( ) << std::endl;
		throw moth_bad_gui();
	}
	const SDL_VideoInfo *info = SDL_GetVideoInfo();
	if (!info) {
		std::cerr<< "Get Video info failed: " <<
		         SDL_GetError( ) << std::endl;
		throw moth_bad_gui();
	}
	bpp = info->vfmt->BitsPerPixel;
	width = info->current_w;
	height = info->current_h;
	width = 1920;
	height =1080;
	flags = SDL_DOUBLEBUF | SDL_HWSURFACE | SDL_OPENGL | SDL_RESIZABLE;
	SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 5);
	SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 5);
	SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 5);
	SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 16);
	SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1);
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

