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
#ifndef __MOTH_FONTS__
#define __MOTH_FONTS__

#include <FTGL/ftgl.h>
#include "moth.h"

class moth_fonts
{
	std::string ttf_file;
	FTFont *font;
	void find_system_fonts();
	public:
		void set_size(unsigned int);
		void set_depth(unsigned int);
		void render(const char *const );
		moth_fonts();
		~moth_fonts();
};

#endif
