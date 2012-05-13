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
#include <vector>
extern "C" {
#include <gdk-pixbuf/gdk-pixbuf.h>
}

#include "moth_index.h"

struct moth_highlight
{
	double x1;
	double y1;
	double x2;
	double y2;
};

class moth_reader
{
protected:
	unsigned int num_pages;
	void get_url(const std::string &file, std::string &url);
public:
	moth_reader();
	virtual unsigned int get_pages() = 0;
	virtual int get_page(int, GdkPixbuf*&) = 0;
	virtual int get_page_size(int, double*, double*) = 0;
	virtual int build_index(moth_index&) = 0;
	virtual int save_copy(std::string&) = 0;
	virtual int search(std::string&, int, std::vector <moth_highlight>&) = 0;
};
#endif
