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

#ifndef _MOTH_INDEX_GUI_GTK_H_
#define _MOTH_INDEX_GUI_GTK_H_

#include <iostream>
#include <fstream>
#include <string>
#include <gtkmm.h>

class moth_index_gui : public Gtk::Window
{
public:
	moth_index_gui(std::string file);
	virtual ~moth_index_gui();

protected:
	void on_button_quit();
	void on_treeview_row_activated(const Gtk::TreeModel::Path& path, Gtk::TreeViewColumn* column);
	class ModelColumns : public Gtk::TreeModel::ColumnRecord
	{
	public:

		ModelColumns() {
			add(link);
			add(page_nr);
		}

		Gtk::TreeModelColumn<Glib::ustring> link;
		Gtk::TreeModelColumn<int> page_nr;
	};

	ModelColumns columns;
	Gtk::VBox vbox;
	Gtk::ScrolledWindow scrolled_window;
	Gtk::TreeView tree_view;
	Glib::RefPtr<Gtk::TreeStore> tree_model;
	Gtk::HButtonBox button_box;
	Gtk::Button button;
};
#endif
