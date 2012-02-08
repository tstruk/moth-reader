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
#include "moth.h"
#include "gui.h"
#include "moth_book.h"

int main(int argc, char** agrv)
{
    int error;
    moth_book *book;
    moth_gui *gui;
    gtk_init(NULL, NULL);
    try {
        gui = new moth_gui;
    }
    catch(std::exception &e)
    {
        std::cerr<< e.what() << std::endl;
        return FAIL;
    }
    try {
        gui->init_video();
    }
    catch(std::exception &e)
    {
        std::cerr<< e.what() << std::endl;
        delete gui;
        return FAIL;
    }
    std::string *file;
    if(argc > 1)
    {
        try {
            file = new std::string(agrv[1]);
        }
        catch(std::exception &e)
        {
            std::cerr<< e.what() << std::endl;
            delete gui;
            return FAIL;
        }
    }
    else
    {
        do{
            char *path = moth_gui::book_select();
            if (path)
            {
                try {
                    file = new std::string(path);
                }
                catch(std::exception &e)
                {
                    std::cerr<< e.what() << std::endl;
                    delete gui;
                    free(path);
                    return FAIL;
                }
                free(path);
            }
        }
        while(file->empty());
    }
    try {
        book = new moth_book(file);
    }
    catch(std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        delete file;
        delete gui;
        return FAIL;
    }
    try { 
        error = gui->main_loop();
    }
    catch(std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        error = FAIL;
    }
    delete book;
    delete file;
    delete gui;
    return error;
}

