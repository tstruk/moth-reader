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

#ifndef __MOTH_READER_PDF__
#define __MOTH_READER_PDF__

#include <string>

extern "C" {
#include <poppler.h>
}
#include "moth_reader.h"

class moth_reader_pdf : public moth_reader
{
	PopplerDocument *doc;
	PopplerPage **pages;
    int walk_index(moth_index &index, PopplerIndexIter *iter);
	moth_reader_pdf(moth_reader_pdf&);
	moth_reader_pdf& operator =(moth_reader_pdf&);
public:
	moth_reader_pdf(const std::string&);
	virtual ~moth_reader_pdf();
	virtual unsigned int get_pages();
	virtual int get_page(int, GdkPixbuf*&);
	virtual int get_page_size(int page, double*, double*);
    virtual int build_index(moth_index&);
};
#endif

