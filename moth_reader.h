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

#ifndef __MOTH_READER__
#define __MOTH_READER__

#include <string>
extern "C" {
#include <gdk-pixbuf/gdk-pixbuf.h>
}
class moth_reader {
    protected:
        int num_pages;
        void get_url(const std::string &file, std::string &url);
    public:
        moth_reader();
        virtual int get_pages() = 0;
        virtual int get_page(int, GdkPixbuf*) = 0;
        virtual int get_page_size(int, double*, double*) = 0;
};

#endif
