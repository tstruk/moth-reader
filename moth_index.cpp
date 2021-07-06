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

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <cassert>
#include <linux/limits.h>

#include "moth.h"
#include "moth_index.h"
#include "moth_gui.h"

#define INDEX_GUI "moth_index_gtk "
#define CMD INDEX_GUI PIPE

int moth_index_gui::show(moth_gui *gui) throw()
{
	char *ptr;
	int sz;

	mkfifo(PIPE, 0666);
	stream = popen(CMD, "r");
	if (!stream || errno == ECHILD) {
		return FAIL;
	}
	pipe = open(PIPE, O_WRONLY);
	if (pipe < 0) {
		return FAIL;
	}
	print_index(&gui->index);
	sz = write(pipe, "\n", 1);
	assert(sz == 1);
	close(pipe);
	memset(line, '\0', line_len);
	do {
		ptr = fgets(line, line_len, stream);
		if (ptr) {
			gui->goto_page(atoi(line));
			for(int i = 0; i < 20; i++)
				gui->show_pages();
		}
	} while (ptr);
	pclose(stream);
	stream = NULL;
	unlink(PIPE);
	return SUCCESS;
}

void moth_index_gui::print_index(moth_index *ptr) throw()
{
	char buf[4];
	int sz;

	while (ptr)
	{
		sz = write(pipe, ptr->name.c_str(), ptr->name.length());
		assert(sz == static_cast<int>(ptr->name.length()));
		sz = write(pipe, " PAGE: ", 7);
		assert(sz == 7);
		sprintf(buf, "%d", ptr->page);
		sz = write(pipe, buf, strlen(buf));
		assert(sz == static_cast<int>(strlen(buf)));
		sz = write(pipe, "\n", 1);
		assert(sz == 1);

		if (ptr->child) {
			sz = write(pipe, ">", 1);
			assert(sz == 1);
			print_index(ptr->child);
			sz = write(pipe, "<", 1);
			assert(sz == 1);
		}
		ptr = ptr->next;
	}
}
