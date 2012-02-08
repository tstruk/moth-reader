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
extern "C"{
#include <gtk/gtk.h>
}
#include "gui.h"
#include "moth_book.h"

int main(int argc, char** agrv)
{
    int error = ERROR;
    moth_book *book;
    moth_gui *gui = new moth_gui;
    if(NULL == gui)
    {
        std::cerr<< "Failed to allocate memory for gui" << std::endl;
        delete gui;
        return ERROR;
    }

    std::cout << "argc = " << argc << std::endl;

    gtk_init(NULL, NULL);
    error = gui->init_video();
    if(SUCCESS != error)
    {
        std::cerr<< "Failed to initialize gui" << std::endl;
        delete gui;
        return ERROR;
    }
    std::string *file;
    if(argc > 1)
    {
        file = new std::string(agrv[1]);
    }
    else
    {
        do{
            char *path = moth_gui::book_select();
            if (path)
            {
                file = new std::string(path);
                free(path);
            }
        }
        while(file->empty());
    }
    
    std::cout << "Open a file: " << file->c_str() << std::endl;
    book = new moth_book(file);
    if(NULL != book)
    {
        
    }
    
    error = gui->main_loop();
    delete book;
    delete file;
    delete gui;
    return error;
}

