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

#include <string>
#include "moth.h"
#include "moth_reader.h"
#include "moth_reader_pdf.h"


moth_reader_pdf::moth_reader_pdf(const char *const file)
{
    std::string url = get_url(file);
    doc = poppler_document_new_from_file (url.c_str(), NULL, NULL);
    if(NULL == doc)
    {
        throw moth_exception();
    }
    int pages = poppler_document_get_n_pages(doc);
    std::cout << "Oppened doc: there are " << pages << " pages in it" << std::endl;
}

int moth_reader_pdf::get_pages()
{
    return 0;
}

int moth_reader_pdf::get_page(int)
{
    g_object_unref(doc);
    return 0;
}
