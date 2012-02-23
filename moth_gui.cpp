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
    font_renderer->Depth(3);
    font_renderer->CharMap(ft_encoding_unicode);
    running = 1;
}

static int pn = 0;

void moth_gui::handle_resize(SDL_ResizeEvent *resize)
{
    width = resize->w;
    height = resize->h;
    float ratio = (float) width / (float) height;
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(90, ratio, 1, 1024);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(0.0, 0.0, width / 2.0f, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
    std::cout << "resized to " << width << "x" << height << std::endl;
}

void moth_gui::handle_mouse_motion(SDL_MouseMotionEvent* motion)
{

}

void moth_gui::handle_mouse_button(SDL_MouseButtonEvent* button)
{

}

void moth_gui::handle_key(SDL_keysym *key)
{
    switch(key->sym) {
    case SDLK_ESCAPE:
        running = 0;
        break;
    case SDLK_UP:
        pn++;
        break;
    case SDLK_DOWN:
        pn--;
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
                    handle_key(&event.key.keysym);
                    break;
                case SDL_MOUSEMOTION:
                    handle_mouse_motion(&event.motion);
                    break;
                case SDL_MOUSEBUTTONDOWN:
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
        usleep(300);
    }
    return SUCCESS;
}

void moth_gui::create_textures()
{
    double w, h, wp, hp;
    num_pages = book->get_pages();
    book->get_page_size(0, &w, &h);
    GdkPixbuf *pixbuff = gdk_pixbuf_new(GDK_COLORSPACE_RGB,
                                        true, 8, w, h);
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
        book->get_page_size(i, &wp, &hp);

        if(w != wp ||  h != hp)
        {
            std::cerr << "Page "<< i <<" has different size " << hp << "x"
                      << wp << std::endl;
            throw moth_bad_pdf();
        }
        if(SUCCESS == book->get_page(i, pixbuff)){
            glBindTexture(GL_TEXTURE_2D, textures[i]);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
                            GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,
                            GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
                            GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                            GL_NEAREST);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA,
                         GL_UNSIGNED_BYTE, gdk_pixbuf_get_pixels(pixbuff));
        }
        else
        {
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
    g_object_unref (pixbuff);
}

void moth_gui::draw_screen()
{
    /* Clear the color and depth buffers. */
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    /* We don't want to modify the projection matrix. */
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    /* Move down the z-axis. */
    glPushMatrix();
    glTranslatef(0.0, 0.0, -5.0);
    glEnable(GL_TEXTURE_2D);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    glBindTexture(GL_TEXTURE_2D, textures[pn]);
    glBegin(GL_QUADS);

    glTexCoord2d(1.0,0.0); glVertex2d(-4.0, -4.0);
    glTexCoord2d(0.0,0.0); glVertex2d(-4.0, 4.0);
    glTexCoord2d(0.0,1.0); glVertex2d(4.0, 4.0);
    glTexCoord2d(1.0,1.0); glVertex2d(4.0, -4.0);

    glEnd();
    glDisable(GL_TEXTURE_2D);
    glPushMatrix();
    SDL_GL_SwapBuffers();
}

void moth_gui::init_opengl()
{
    float ratio = (float) width / (float) height;
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
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(90, ratio, 1, 1024);
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
    //width = 800;
    //height = 600;
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

