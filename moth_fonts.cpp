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

#include <sys/types.h>
#include <dirent.h>
#include <cassert>
#include "moth_fonts.h"
#include "moth_gui_dialog.h"

static const char *const font_dirs[] = {
	"/usr/share/fonts/truetype/freefont/",
	"/usr/share/fonts/TrueType/freefont/",
	"/usr/share/fonts/gnu-free/"
};

static const char *const font_files[] = {
	"FreeSerifItalic.ttf",
	"FreeSansOblique.ttf",
	"FreeMonoOblique.ttf",
	"FreeSans.ttf",
	"FreeSerif.ttf",
};

void moth_fonts::find_system_fonts()
{
	DIR *dir;
	unsigned int x;
	for (x = 0; x < (sizeof(font_dirs)/sizeof(font_dirs[0])); x++)
	{
		dir = opendir(font_dirs[x]);
		if (dir)
			break;
		else {
			switch(errno) {
				case EACCES:
					{
						std::cerr << "Can not open font dir " << font_dirs[x]
								  << std::endl;
						std::cerr << "Permission denied" << std::endl;
						throw moth_bad_font();
					}
				case ENOENT:
					break;
			}
		}
	}
	if (dir) {
		struct dirent *f;
		for (unsigned int i = 0;
				i < (sizeof(font_files)/sizeof(font_files[0])); i++)
		{
			while((f = readdir(dir))) {
				if (strcmp(f->d_name, font_files[i]) == 0) {
					ttf_file = std::string(font_dirs[x]);
					ttf_file += font_files[i];
					std::cout << "using fonts " << ttf_file << std::endl;
					closedir(dir);
					return;
				}
			}
			rewinddir(dir);
		}
		closedir(dir);
	}
	moth_dialog dialog;
	std::string info("\"No ttf font found.\"");
	dialog.info(info);
	std::cerr << "No ttf font found" << std::endl;
	throw moth_bad_font();
}

moth_fonts::moth_fonts()
{
	find_system_fonts();
	font = new FTExtrudeFont(ttf_file.c_str());
}

moth_fonts::~moth_fonts()
{
	delete font;
}

void moth_fonts::set_size(unsigned int size)
{
	assert(font);
	font->FaceSize(size);
}

void moth_fonts::set_depth(unsigned int depth)
{
	assert(font);
	font->Depth(depth);
}

void moth_fonts::render(const char *const buf)
{
	assert(font);
	font->Render(buf);
}
