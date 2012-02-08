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

#include <cstring>
#include "moth.h"
#include "moth_book.h"
#include "moth_reader_pdf.h"

static const char *const formats[] = {".pdf",
                                      ".mobi"};

moth_format_type moth_book::get_type()
{
    moth_format_type type = moth_format_pdf;
    for(int i = 0; i < moth_format_not_supported; i++)
    {
        type = static_cast <moth_format_type>(i);
        if(std::string::npos != file_name.find(formats[i],
                                    file_name.length() - strlen(formats[i])))
        {
            return type;
        }
    }
    return moth_format_not_supported;
    
}

moth_book::moth_book(std::string *path)
{
    file_name = path->c_str();
    type = get_type();
    if(!(type < moth_format_not_supported))
    {
        throw moth_bad_format();
    }
    if(type == moth_format_pdf)
    {
        reader = new moth_reader_pdf(file_name.c_str());
    }
    if(type == moth_format_mobi)
    {
        /* Not yet supported */
        throw moth_bad_format();
    }
}

moth_book::~moth_book()
{
    delete reader;
}
