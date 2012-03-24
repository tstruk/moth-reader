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
#ifndef __MOTH_INDEX__
#define __MOTH_INDEX__

#include <string>

class moth_index {
    public:
    std::string name;
    int page;
    moth_index* next;
    moth_index* child;
    moth_index() throw() : page(0), next(NULL), child(NULL)
    {
        name = "";
    }
    ~moth_index()
    {
        delete next;
        delete child;
    }
};

const static uint16_t line_len = 64;

class moth_gui;

class moth_index_gui {
	FILE *stream;
    int pipe;
	char line[line_len];
    public:
    int show(moth_gui*) throw();
    void print_index(moth_index *index) throw();
    moth_index_gui() throw()
    {
        pipe = -1;
        stream = NULL;
    }

    ~moth_index_gui()
    {
        if (stream)
	        pclose(stream);
        stream = NULL;
    }
};
#endif
