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


moth::moth(int argc, char** argv)
{
    this->argc = argc;
    this->argv = argv;
    Gtk::Main kit(argc, argv);
}

moth::~moth()
{
}
void moth::help(int argc, char** agrv)
{
    std::cout<< "Use: " << agrv[0] <<
                "[path to ebook]" << std::endl;
}

int moth::run()
{
    moth_book *book;
    moth_gui *gui;
    std::string file;
    try {
        gui = new moth_gui;
    }
    catch(std::exception &e)
    {
        std::cerr<< e.what() << std::endl;
        return FAIL;
    }
    gui->book_select(file);
    try {
        gui->init_video();
    }
    catch(std::exception &e)
    {
        std::cerr<< e.what() << std::endl;
        delete gui;
        return FAIL;
    }
    try {
        book = new moth_book(file);
    }
    catch(std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        delete gui;
        return FAIL;
    }
    int error;
    try {
        error = gui->main_loop();
    }
    catch(std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return FAIL;
    }
    delete book;
    delete gui;
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
    return error;
}

