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

#ifndef __MOTH_H__
#define __MOTH_H__


#include <iostream>
#include <exception>

#define FAIL -1
#define SUCCESS 0

class moth_exception : public std::exception
{
    virtual const char* what() const throw()
    {
        return "Error: Unknown moth exception";
    }
};

class moth_bad_format : public moth_exception
{
    virtual const char* what() const throw()
    {
        return "Error: Unknown file format";
    }
};

class moth_bad_gui : public moth_exception
{
    virtual const char* what() const throw()
    {
        return "Error: GUI error";
    }
};

enum moth_format_type{
    moth_format_pdf = 0,
    moth_format_mobi,
    moth_format_not_supported
};

#endif

