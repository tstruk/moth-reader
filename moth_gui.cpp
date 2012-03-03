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
#include "moth_gui_file_choose.h"

static const char *const font_file = "fonts/TSCu_Times.ttf";
static const unsigned int max_pages = 700;

moth_gui::~moth_gui()
{
    glDeleteTextures(num_pages, textures);
    delete win;
    delete font_renderer;
    SDL_Quit();
}

moth_gui::moth_gui()
{
    win = new Gtk::Window;
    win->resize(1,1);
    win->show();
    font_renderer = new FTExtrudeFont(font_file);
    if(!font_renderer)
    {
        std::cerr << "Can not open file " << font_file << std::endl;
        throw moth_bad_font();
    }
    font_renderer->FaceSize(30);
    font_renderer->Depth(1);
    font_renderer->CharMap(ft_encoding_unicode);
    running = 1;
    move_by_pages = 1;
    zoom = 1.0;
    shift_x = 0.0;
    shift_y = 0.0;
    idle = 5000;
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
    ratio = std::min((width / page_width),(height / page_height));
}

void moth_gui::handle_mouse_motion(SDL_MouseMotionEvent* motion)
{
    if(button_state == SDL_PRESSED)
    {
        shift_x += motion->xrel;
        shift_y -= motion->yrel;
    }
    else
    {
        if(shift_state == SDL_PRESSED)
        {
            zoom += motion->yrel * 0.001;
            if(zoom < 0)
                zoom = 0;
            if(zoom > 10)
                zoom = 10;
        }
    }
}

void moth_gui::handle_mouse_button(SDL_MouseButtonEvent* button)
{
    button_state = button->state;
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
            book->set_page(book->get_page() + move_by_pages);
            move_by_pages = 2;
        break;
        case SDLK_LEFT:
            book->set_page(book->get_page() - move_by_pages);
            if(book->get_page() == 0)
                move_by_pages = 1;
        break;
        case SDLK_UP:
            shift_x += page_width * ratio * 0.1;
        break;
        case SDLK_DOWN:
            shift_x -= page_width * ratio * 0.1;
        break;
        case SDLK_z:
            shift_y += page_width * ratio * 0.1;
        break;
        case SDLK_x:
            shift_y -= page_width * ratio * 0.1;
        break;
        case SDLK_a:
            zoom += 0.05;
        break;
        case SDLK_s:
            zoom -= 0.05;
        break;
        default:
        break;
    }
}

int moth_gui::read_book(moth_book *book)
{
    SDL_Event event;
    this->book = book;
    create_textures();
    while(running)
    {
        while(SDL_PollEvent(&event))
        {
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
        draw_screen();
    }
    return SUCCESS;
}

void moth_gui::create_textures()
{
    double w, h;
    GError *error = NULL;
    num_pages = book->get_pages();
    book->get_page_size(0, &page_width, &page_height);
    GdkPixbuf *pixbuff = gdk_pixbuf_new(GDK_COLORSPACE_RGB,
                                        true, 8, page_width * 2, page_height * 2);

    if(!pixbuff)
    {
        std::cerr << "Could not allocate buffer for texture" << std::endl;
        throw moth_bad_gui();
    }

    ratio = std::min((width / page_width),(height / page_height));
    zoom = 0.8;
    std::cout << "Ratio = " << ratio << std::endl;

    const char *str0 = "Please wait";
    const char *str[] = {"Mapping textures %d%%",
                         "Mapping textures %d%% .",
                         "Mapping textures %d%% ..",
                         "Mapping textures %d%% ..."};
    char buff[30];
    textures = new GLuint[num_pages];

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glGenTextures(num_pages, textures);
    for(int i = 0, index = 0, ctr = 10; i < num_pages && i < max_pages; i++)
    {
        book->get_page_size(i, &w, &h);

        if(page_width != w || page_height != h)
        {
            std::cerr << "Page "<< i <<" has different size " << h << "x"
                      << w << std::endl;
            g_object_unref(pixbuff);
            throw moth_bad_pdf();
        }
        if(SUCCESS == book->get_page(i, pixbuff)){
            /* Enhance the picture */
            gdk_pixbuf_saturate_and_pixelate(pixbuff, pixbuff, 2.0, 0);
            glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
            glBindTexture(GL_TEXTURE_2D, textures[i]);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, page_width * 2,
                         page_height * 2, 0, GL_RGBA,
                         GL_UNSIGNED_BYTE, gdk_pixbuf_get_pixels(pixbuff));
        }
        else
        {
            std::cerr << "Could not get texture for page "<< i << std::endl;
            throw moth_bad_gui();
        }
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);
        glPushMatrix();
        glTranslatef(-0.9, -0.2, -10.0);
        glColorMaterial(GL_FRONT, GL_DIFFUSE);
        glTranslatef(0.0, 0.0, 20.0);
        glTranslatef(-160.0, -240.0, 0.0);
        glColor3f(0.3, 0.3, 1.0);
        if(i == ctr)
        {
            ctr = i + 10;
            index = ++index % 4;
        }
        sprintf(buff, str[index],
                        (int)((((double)(i+1)) / ((double)num_pages)) * 100));
        font_renderer->Render(buff);
        glTranslatef(70.0, -30.0, 0.0);
        font_renderer->Render(str0);
        glPopMatrix();
        SDL_GL_SwapBuffers();
    }
    glColor3f(1.0, 1.0, 1.0);
    g_object_unref(pixbuff);
}

static GLfloat page_one[3][3][3] = {{0},{0},{0}};
static GLfloat page_two[3][3][3] = {{0},{0},{0}};
static GLfloat page_moving[3][3][3] = {{0},{0},{0}};
static GLfloat texpts[2][2][2] = {{{0.0, 0.0}, {1.0, 0.0}},
                                  {{0.0, 1.0}, {1.0, 1.0}}};
static const int evaluators = 200;

void moth_gui::draw_screen()
{
    GLfloat z_shift;
    /* Assume modelview */
    /* Clear the color and depth buffers. */
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    /* Move down the z-axis. */
    glPushMatrix();

    if (book->page_first() || book->page_last())
    {
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
    }
    else
    {
        z_shift = 200;
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

        glBindTexture(GL_TEXTURE_2D, textures[book->get_page()]);
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

        glBindTexture(GL_TEXTURE_2D, textures[book->get_page() + 1]);
        glMapGrid2f(evaluators, 0.0, 1.0, evaluators, 0.0, 1.0);
        glEvalMesh2(GL_FILL, 0, evaluators, 0, evaluators);

    }
    glPopMatrix();
    SDL_GL_SwapBuffers();
    usleep(idle);
}

void moth_gui::init_opengl()
{
    GLenum err = glewInit();
    if(GLEW_OK != err)
    {
        std::cerr<< "Failed to initialize GL Extension Wrangler library"
                 << std::endl;
        throw moth_bad_ogl();
    }
    /* For texture sizes different than 2^m + 2b need opengl 2.0 or higher*/
    if(!GLEW_VERSION_2_0)
    {
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
#if 0
/* TODO: This doesn't work */
{
char SDL_windowhack[32];
Glib::RefPtr<Gdk::Screen> gscreen = win->get_screen();
Glib::RefPtr<Gdk::Window> gwin = gscreen->get_root_window();
sprintf(SDL_windowhack,"SDL_WINDOWID=%ld", GDK_WINDOW_XID((gwin->gobj())));
std::cout << "!!! window id = " << SDL_windowhack << std::endl;
putenv(SDL_windowhack);
}
#endif

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
    height = 1080;
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
};

void moth_gui::book_select(std::string &file)
{
    moth_gui_file_ch *fc = NULL;
    try {
        fc = new moth_gui_file_ch;
        fc->choose_file(file, *win);
    }
    catch(std::exception &e)
    {
        std::cerr << e.what() << std::endl;
    }
    delete fc;
    if(file.empty())
        throw moth_bad_cancel();

}

