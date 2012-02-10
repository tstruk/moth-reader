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

#include <iostream>
extern "C" {
#include <gtk/gtk.h>
#include "SDL_syswm.h"
}
#include "moth.h"
#include "moth_gui.h"

moth_gui::~moth_gui()
{
    SDL_Quit();
}

moth_gui::moth_gui()
{
    init_video();
}

int moth_gui::main_loop()
{
    return SUCCESS;
}

void moth_gui::init_opengl()
{
    float ratio = (float) width / (float) height;
    /* Our shading model--Gouraud (smooth). */
    glShadeModel( GL_SMOOTH );

    /* Culling. */
    glCullFace( GL_BACK );
    glFrontFace( GL_CCW );
    glEnable( GL_CULL_FACE );

    /* Set the clear color. */
    glClearColor( 0, 0, 0, 0 );

    /* Setup our viewport. */
    glViewport( 0, 0, width, height );

    /*
     * Change to the projection matrix and set
     * our viewing volume.
     */
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity( );
    /*
     * EXERCISE:
     * Replace this with a call to glFrustum.
     */
    gluPerspective( 60.0, ratio, 1.0, 1024.0 );
}

void moth_gui::init_video()
{
    /* initialize SDL */
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

char* moth_gui::book_select()
{
    char *filename = NULL;
    int response;
    GtkWidget *file_dialog = gtk_file_chooser_dialog_new ("Open a File",
                      NULL,
                      GTK_FILE_CHOOSER_ACTION_OPEN,
                      GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                      GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
                      NULL);
    response = gtk_dialog_run (GTK_DIALOG (file_dialog));
    if (response == GTK_RESPONSE_ACCEPT)
    {
        filename =
            gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(file_dialog));
    }
    else
    {
        gtk_widget_destroy(file_dialog);
        throw moth_bad_cancel();
    }
    gtk_widget_destroy(file_dialog);
    return filename;
}

