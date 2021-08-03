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

#include <cstdlib>
#include <cstring>
#include <cstdio>
#include "moth.h"
#include "moth_reader.h"

moth_reader::moth_reader()
{
}

void moth_reader::get_url(const std::string &path,
			  std::string &url)
{
	FILE *f = fopen(path.c_str(), "r");

	if (!f) {
		throw moth_bad_file();
	}
	fclose(f);
	url = "file://";
	const char *const dir_up = "../";
	const char *const current_dir = "./";
	const char slash = '/';
	char *pwd = getenv("PWD");
	int up_ctr = 0;
	int loop;
	const char *new_path = path.c_str();
	/* Count dirs up in relative path */
	do {
		loop = strncmp(new_path, dir_up, 3);
		if (loop == 0) {
			up_ctr++;
			new_path += 3;
		}
	} while (loop == 0);

	if(0 == up_ctr) {
		if((strncmp(new_path, current_dir, 2) != 0) && (path[0] == slash))
			pwd = (char*) "";
	}
	while (new_path && up_ctr) {
		char *p = rindex(pwd, slash);
		*p = '\0';
		up_ctr--;
	}
	url += pwd;
	url += "/";
	url += new_path;
}
