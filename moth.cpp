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
#include <gtkmm.h>
#include "moth.h"
#include "moth_gui.h"
#include "moth_book.h"


moth::moth(int argc, char** argv): gtk_kit(argc, argv)
{
    this->argc = argc;
    this->argv = argv;
}

moth::~moth()
{
}
void moth::help()
{
    std::cout<< "Use: " << argv[0] <<
                " [path to ebook]" << std::endl;
}

int moth::run()
{
    moth_book *book;
    moth_gui *gui;

    /* Create a GUI object */
    try {
        gui = new moth_gui;
    }
    catch(std::exception &e)
    {
        std::cerr<< e.what() << std::endl;
        return FAIL;
    }

    /* Get the path to an ebook */
    if(argc < 2) {
        try{
            gui->book_select(file);
        }
        catch(std::exception &e)
        {
            std::cerr<< e.what() << std::endl;
            delete gui;
            return FAIL;
        }
    }
    else if (argc == 2){
        file = argv[1];
    }
    else
    {
        help();
        delete gui;
        return FAIL;
    }

    /* Create the ebook */
    try {
        book = new moth_book(file);
    }
    catch(std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        delete gui;
        return FAIL;
    }

    /* Initialize GUI*/
    try {
        gui->init_video();
    }
    catch(std::exception &e)
    {
        std::cerr<< e.what() << std::endl;
        delete gui;
        delete book;
        return FAIL;
    }

    int error;
    try {
        error = gui->read_book(book);
    }
    catch(std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        delete gui;
        delete book;
        return FAIL;
    }
    delete gui;
    delete book;
    return error;
}

int main(int argc, char** argv)
{
    int error;
    moth *m;
    try{
        m = new moth(argc, argv);
    }
    catch(std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return FAIL;
    }
    error = m->run();
    delete m;
    return error;
}

