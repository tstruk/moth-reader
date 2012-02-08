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

extern "C" {
#include <poppler.h>
}

class moth_reader_pdf : public moth_reader {
    PopplerDocument *doc;
    public:
    moth_reader_pdf(const char * const);
    virtual int get_pages();
    virtual int get_page(int);


};

#endif

