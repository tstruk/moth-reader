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

#include "moth_gui.h"
#include "moth_gui_file_choose.h"

moth_gui_file_ch::moth_gui_file_ch()
{

}

moth_gui_file_ch::~moth_gui_file_ch()
{
}

void moth_gui_file_ch::choose_file(std::string& file, Gtk::Window &win)
{
	Gtk::FileChooserDialog dialog("Please choose a file",
	                              Gtk::FILE_CHOOSER_ACTION_OPEN);
	dialog.set_transient_for(win);

	dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
	dialog.add_button(Gtk::Stock::OPEN, Gtk::RESPONSE_OK);
	Gtk::FileFilter filter_pdf;
	filter_pdf.set_name("PDF files");
	filter_pdf.add_mime_type("application/pdf");
	dialog.add_filter(filter_pdf);
	int result = dialog.run();
	switch(result) {
	case(Gtk::RESPONSE_OK): {
		file = dialog.get_filename();
		break;
	}
	case(Gtk::RESPONSE_CANCEL):
		break;
	default:
		break;
	}
	dialog.hide();
}
