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

moth_gui::~moth_gui()
{
    glDeleteTextures(num_pages, textures);
    delete win;
    SDL_Quit();
}

moth_gui::moth_gui()
{
    win = new Gtk::Window;
    win->resize(1,1);
    win->show();
    running = 1;
}

void moth_gui::handle_resize(SDL_ResizeEvent *resize)
{

}

void moth_gui::handle_key(SDL_keysym *key)
{
    switch(key->sym) {
    case SDLK_ESCAPE:
        running = 0;
        break;
    default:
        break;
    }
}
void moth_gui::handle_mouse_motion(SDL_MouseMotionEvent* motion)
{

}

void moth_gui::handle_mouse_button(SDL_MouseButtonEvent* button)
{

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
    double w, h;
    num_pages = book->get_pages();
    book->get_page_size(0, &w, &h);
    GdkPixbuf *pixbuff = gdk_pixbuf_new(GDK_COLORSPACE_RGB,
                                        true, 8, w, h);

    textures = new GLuint[num_pages];
    glGenTextures(num_pages, textures);
    for(int i = 0; i < num_pages; i++)
    {
        if(SUCCESS == book->get_page(i, pixbuff)){
            glBindTexture(GL_TEXTURE_RECTANGLE_ARB, textures[i]);
            glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, GL_RGBA, w,
                         h, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                         gdk_pixbuf_get_pixels(pixbuff));
        }
        else
        {
            throw moth_bad_gui();
        }
        std::cout << "Mapped texture " << i << " name " << textures[i] <<std::endl;
    }
}

void moth_gui::draw_screen()
{
    /* Clear the color and depth buffers. */
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    /* We don't want to modify the projection matrix. */
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    /* Move down the z-axis. */
    glTranslatef(0.0, 0.0, -2.0);
    glEnable(GL_TEXTURE_RECTANGLE_ARB);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, textures[0]);
    glBegin(GL_QUADS);
    glTexCoord2d(0.0,0.0); glVertex3d(-2.0,-1.0, 0.0);
    glTexCoord2d(0.0,1.0); glVertex3d(-2.0,1.0, 0.0);
    glTexCoord2d(1.0,1.0); glVertex3d(0.0,1.0, 0.0);
    glTexCoord2d(1.0,0.0); glVertex3d(0.0,-1.0, 0.0);
    glEnd();
    glDisable(GL_TEXTURE_RECTANGLE_ARB);
    SDL_GL_SwapBuffers();
}

void moth_gui::init_opengl()
{
    float ratio = (float) width / (float) height;
    GLfloat mat_diffuse[] = { 0.7, 0.7, 0.7, 1.0 };
    GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 1.0 };
    GLfloat mat_shininess[] = { 100.0 };
    if(!(glewGetExtension("GL_ARB_texture_rectangle") ||
         glewGetExtension("GL_NV_texture_rectangle")))
    {
        std::cerr << "No support for texture_rectangle" << std::endl;
        throw moth_bad_ogl();
    }
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glShadeModel(GL_SMOOTH);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_AUTO_NORMAL);
    glEnable(GL_NORMALIZE);
	glEnable(GL_TEXTURE_RECTANGLE_ARB);
	glEnable(GL_DEPTH_TEST);
    glOrtho(-1.50, 1.50, -1.50, 1.50, -1.50, 1.50);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    glEnable(GL_CULL_FACE);
    glViewport( 0, 0, width, height );
    /*
     * Change to the projection matrix and set
     * viewing volume.
     */
    gluPerspective( 60.0, ratio, 1.0, 1024.0 );
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
    width = 800;
    height = 600;
    flags = SDL_DOUBLEBUF | SDL_HWSURFACE | SDL_OPENGL;
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

