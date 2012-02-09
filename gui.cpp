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
}
#include "gui.h"
#include "moth.h"

moth_gui::~moth_gui()
{
    SDL_Quit();
}

moth_gui::moth_gui()
{
    if(SUCCESS != init_video())
    {
        std::cerr<< "Init Video failed" << std::endl;
    }
}

int moth_gui::main_loop()
{
    
    return SUCCESS;
}

int moth_gui::init_video()
{
    /* initialize SDL */
    int error = SDL_Init( SDL_INIT_VIDEO );
    if ( error != SUCCESS ) {
        std::cerr<< "Video initialization failed: " <<
                     SDL_GetError( ) << std::endl;
        return error;
    }
    const SDL_VideoInfo *info = SDL_GetVideoInfo();
    flags = SDL_DOUBLEBUF | SDL_HWSURFACE | SDL_OPENGL;
    bpp = 16;
    SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 5);
    SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 5);
    SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 5);
    SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 16);
    SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1);
    this->screen = SDL_SetVideoMode(info->current_w, info->current_h, bpp, flags);
    if(NULL == this->screen) {
        std::cerr<< "Set Video Mode failed: " <<
                     SDL_GetError( ) << std::endl;
        return FAIL;
    }
    return SUCCESS;
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
        filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER (file_dialog));
    }
    gtk_widget_destroy (file_dialog);
    if(response == GTK_RESPONSE_CANCEL)
    {
        throw moth_bad_cancel();
    }
    return filename;
}

