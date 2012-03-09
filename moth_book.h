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

#ifndef __MOTH_BOOK__
#define __MOTH_BOOK__

#include<string>
#include "moth_reader.h"

class moth_book
{
	unsigned int current_page;
	moth_reader *reader;
	std::string file_name;
	moth_format_type get_type();
	moth_format_type type;
	moth_book(moth_book&);
	moth_book& operator=(moth_book&);
public:
	moth_book(const std::string&);
	virtual ~moth_book();
	void set_page(unsigned int const page);
	unsigned int get_page(void) {
		return current_page;
	}
	int get_page(int, GdkPixbuf *&pixbuff);
	int get_pages();
	int get_page_size(int, double*, double*);
	bool page_first() {
		if (current_page == 0)
			return true;
		return false;
	}
	bool page_last() {
		if (current_page == reader->get_pages())
			return true;
		return false;
	}
};
#endif
