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

#include "moth_gui.h"
#include "moth_gui_file_choose.h"
#include <iostream>
#include <SDL_syswm.h>
#include <SDL_opengl.h>
#include <gl.h>
#include <glu.h>
#include <gdk/gdkx.h>

moth_gui::~moth_gui()
{
    SDL_Quit();
    delete win;
}

moth_gui::moth_gui()
{
    win = new Gtk::Window;
    win->resize(1,1);
    win->show();
    running = 1;
}

void moth_gui::handle_key(SDL_keysym* key)
{

    switch(key->sym) {
    case SDLK_ESCAPE:
        running = 0;
        break;
    default:
        break;
    }
}

int moth_gui::main_loop()
{
    SDL_Event event;
    while(running)
    {
        draw_screen();
        while(SDL_PollEvent(&event))
        {
            switch( event.type ) {
                case SDL_KEYDOWN:
                    handle_key(&event.key.keysym);
                break;
            }
        }
        usleep(300);
    }
    return SUCCESS;
}

void moth_gui::draw_screen()
{
    /* Our angle of rotation. */
    static float angle = 0.0f;

    /*
     * EXERCISE:
     * Replace this awful mess with vertex
     * arrays and a call to glDrawElements.
     *
     * EXERCISE:
     * After completing the above, change
     * it to use compiled vertex arrays.
     *
     * EXERCISE:
     * Verify my windings are correct here ;).
     */
    static GLfloat v0[] = { -1.0f, -1.0f,  1.0f };
    static GLfloat v1[] = {  1.0f, -1.0f,  1.0f };
    static GLfloat v2[] = {  1.0f,  1.0f,  1.0f };
    static GLfloat v3[] = { -1.0f,  1.0f,  1.0f };
    static GLfloat v4[] = { -1.0f, -1.0f, -1.0f };
    static GLfloat v5[] = {  1.0f, -1.0f, -1.0f };
    static GLfloat v6[] = {  1.0f,  1.0f, -1.0f };
    static GLfloat v7[] = { -1.0f,  1.0f, -1.0f };
    static GLubyte red[]    = { 255,   0,   0, 255 };
    static GLubyte green[]  = {   0, 255,   0, 255 };
    static GLubyte blue[]   = {   0,   0, 255, 255 };
    static GLubyte white[]  = { 255, 255, 255, 255 };
    static GLubyte yellow[] = {   0, 255, 255, 255 };
    static GLubyte black[]  = {   0,   0,   0, 255 };
    static GLubyte orange[] = { 255, 255,   0, 255 };
    static GLubyte purple[] = { 255,   0, 255,   0 };

    /* Clear the color and depth buffers. */
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    /* We don't want to modify the projection matrix. */
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity( );

    /* Move down the z-axis. */
    glTranslatef( 0.0, 0.0, -5.0 );

    /* Rotate. */
    glRotatef( angle, 0.0, 1.0, 0.0 );

    if(1) {

        angle += 0.01;
        if( angle > 360.0f ) {
            angle = 0.0f;
        }

    }

    glDisable(GL_LIGHTING);
    /* Send our triangle data to the pipeline. */
    glBegin( GL_TRIANGLES );

    glColor4ubv( red );
    glVertex3fv( v0 );
    glColor4ubv( green );
    glVertex3fv( v1 );
    glColor4ubv( blue );
    glVertex3fv( v2 );

    glColor4ubv( red );
    glVertex3fv( v0 );
    glColor4ubv( blue );
    glVertex3fv( v2 );
    glColor4ubv( white );
    glVertex3fv( v3 );

    glColor4ubv( green );
    glVertex3fv( v1 );
    glColor4ubv( black );
    glVertex3fv( v5 );
    glColor4ubv( orange );
    glVertex3fv( v6 );

    glColor4ubv( green );
    glVertex3fv( v1 );
    glColor4ubv( orange );
    glVertex3fv( v6 );
    glColor4ubv( blue );
    glVertex3fv( v2 );

    glColor4ubv( black );
    glVertex3fv( v5 );
    glColor4ubv( yellow );
    glVertex3fv( v4 );
    glColor4ubv( purple );
    glVertex3fv( v7 );

    glColor4ubv( black );
    glVertex3fv( v5 );
    glColor4ubv( purple );
    glVertex3fv( v7 );
    glColor4ubv( orange );
    glVertex3fv( v6 );

    glColor4ubv( yellow );
    glVertex3fv( v4 );
    glColor4ubv( red );
    glVertex3fv( v0 );
    glColor4ubv( white );
    glVertex3fv( v3 );

    glColor4ubv( yellow );
    glVertex3fv( v4 );
    glColor4ubv( white );
    glVertex3fv( v3 );
    glColor4ubv( purple );
    glVertex3fv( v7 );

    glColor4ubv( white );
    glVertex3fv( v3 );
    glColor4ubv( blue );
    glVertex3fv( v2 );
    glColor4ubv( orange );
    glVertex3fv( v6 );

    glColor4ubv( white );
    glVertex3fv( v3 );
    glColor4ubv( orange );
    glVertex3fv( v6 );
    glColor4ubv( purple );
    glVertex3fv( v7 );

    glColor4ubv( green );
    glVertex3fv( v1 );
    glColor4ubv( red );
    glVertex3fv( v0 );
    glColor4ubv( yellow );
    glVertex3fv( v4 );

    glColor4ubv( green );
    glVertex3fv( v1 );
    glColor4ubv( yellow );
    glVertex3fv( v4 );
    glColor4ubv( black );
    glVertex3fv( v5 );

    glEnd( );

    /*
     * EXERCISE:
     * Draw text telling the user that 'Spc'
     * pauses the rotation and 'Esc' quits.
     * Do it using vetors and textured quads.
     */

    /*
     * Swap the buffers. This this tells the driver to
     * render the next frame from the contents of the
     * back-buffer, and to set all rendering operations
     * to occur on what was the front-buffer.
     *
     * Double buffering prevents nasty visual tearing
     * from the application drawing on areas of the
     * screen that are being updated at the same time.
     */
    glEnable(GL_LIGHTING);
    SDL_GL_SwapBuffers( );
}

void moth_gui::init_opengl()
{
    float ratio = (float) width / (float) height;
    GLfloat mat_diffuse[] = { 0.7, 0.7, 0.7, 1.0 };
    GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 1.0 };
    GLfloat mat_shininess[] = { 100.0 };
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_AUTO_NORMAL);
    glEnable(GL_NORMALIZE);
    glOrtho(-1.50, 1.50, -1.50, 1.50, -1.50, 1.50);
    glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                     GL_LINEAR_MIPMAP_NEAREST );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
	glEnable(GL_TEXTURE_2D);
	glEnable( GL_DEPTH_TEST );
    glShadeModel( GL_SMOOTH );
    glCullFace( GL_BACK );
    glFrontFace( GL_CCW );
    glEnable( GL_CULL_FACE );
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
    char *filename;
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

