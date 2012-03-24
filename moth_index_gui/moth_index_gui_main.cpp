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

#include <iostream>
#include <fstream>
#include <gtkmm.h>
#include <moth.h>
#include "moth_index_gui_win.h"

int main(int argc, char *argv[])
{
    if (argc < 2)
        return -1;

	Gtk::Main kit(argc, argv);
	moth_index_gui gui(argv[1]);
	Gtk::Main::run(gui);
	return 0;
}
