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

#define MOTH_VER_BIG 0
#define MOTH_VER_SMALL 2
#define MOTH_VER_STRING "0.2"

#define FAIL -1
#define SUCCESS 0

#define PIPE "/tmp/.moth_pipe"

class moth_exception : public std::exception
{
	virtual const char* what() const throw() {
		return "Error: Unknown moth exception";
	}
};

class moth_bad_file : public moth_exception
{
	virtual const char* what() const throw() {
		return "Error: Cannot open file";
	}
};

class moth_bad_format : public moth_exception
{
	virtual const char* what() const throw() {
		return "Error: Unknown file format";
	}
};

class moth_bad_pdf : public moth_exception
{
	virtual const char* what() const throw() {
		return "Error: Not supported pdf format";
	}
};

class moth_bad_cancel : public moth_exception
{
	virtual const char* what() const throw() {
		return "Error: Operation canceled";
	}
};

class moth_bad_gui : public moth_exception
{
	virtual const char* what() const throw() {
		return "Error: GUI error";
	}
};

class moth_bad_ogl : public moth_exception
{
	virtual const char* what() const throw() {
		return "Error: openGL error";
	}
};

class moth_bad_font : public moth_exception
{
	virtual const char* what() const throw() {
		return "Error: Font error";
	}
};

enum moth_format_type {
	moth_format_pdf,
	moth_format_mobi,
	moth_format_not_supported
};

enum moth_search_dir {
	moth_search_forward,
	moth_search_backward
};

class moth
{
	private:
	std::string file;
	int argc;
	char **argv;
	void help();
	moth(const moth&);
	moth& operator=(const moth&);
	public:
	moth(int, char**);
	~moth();
	int run();
};
#endif

