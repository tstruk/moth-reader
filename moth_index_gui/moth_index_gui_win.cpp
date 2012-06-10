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

#include "moth_index_gui_win.h"
#include <gdk/gdk.h>
#include <moth.h>
#include <cstdlib>

using namespace std;

moth_index_gui::moth_index_gui(string file)
	: button("Quit")
{
	set_title("moth " MOTH_VER_STRING " - Book Index");
	set_border_width(5);
	set_default_size(800, 800);
	add(vbox);
	scrolled_window.add(tree_view);
	scrolled_window.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
	vbox.pack_start(scrolled_window);
	vbox.pack_start(button_box, Gtk::PACK_SHRINK);
	button_box.pack_start(button, Gtk::PACK_SHRINK);
	button_box.set_border_width(5);
	button_box.set_layout(Gtk::BUTTONBOX_END);
	button.signal_clicked().connect(sigc::mem_fun(*this,
	                                       &moth_index_gui::on_button_quit) );
	tree_model = Gtk::TreeStore::create(columns);
	tree_view.set_model(tree_model);
    ifstream pipe(file.c_str());
    if (!pipe.is_open()) {
        cerr << "Unable to open file \"" << file << "\"" << endl;
        throw moth_exception();
    }
    size_t pos;
    static const int depth = 10;
    int current = 0, last, page;
    Gtk::TreeModel::Row parent[depth];
    Gtk::TreeModel::Row row;
    string line;
    bool down, up;

    while (pipe.good())
    {
        down = false;
        up = false;
        getline (pipe, line);
        pos = line.rfind("PAGE:");
        if (pos == string::npos)
            continue;
        page = atoi(line.substr(pos + 6).c_str());
        line = line.substr(0, pos - 1);
        if (line[0] == '>') {
            line = line.substr(1);
            last = current;
            current++;
            down = true;
        }
        else {
            last = current;
            for(int i = 0; i < line.length(); i++)
            {
                if (line[0] == '<') {
                    line = line.substr(1);
                    current--;
                    up = true;
                }
                else {
                    break;
                }
            }
        }
        if (current == 0) {
            row = *(tree_model->append());
            parent[0] = row;
        }
        else {
            if (down) {
                row = *(tree_model->append(parent[last].children()));
                parent[current] = row;
            }
            else if(up) {
                row = *(tree_model->append(parent[current-1].children()));
                parent[current] = row;
            }
            else {
                row = *(tree_model->append(parent[current-1].children()));
                parent[current] = row;
            }
        }

        row[columns.link] = line;
        row[columns.page_nr] = page;
    }
    pipe.close();
	tree_view.append_column("", columns.link);
	tree_view.append_column("", columns.page_nr);
	tree_view.signal_row_activated().connect(
            sigc::mem_fun(*this, &moth_index_gui::on_row_clicked));
	this->add_events(Gdk::FOCUS_CHANGE_MASK);
	show_all_children();
}

moth_index_gui::~moth_index_gui()
{
}

void moth_index_gui::on_button_quit()
{
	hide();
}
bool moth_index_gui::on_focus_out_event(GdkEventFocus* event)
{
	hide();
	return false;
}

void moth_index_gui::on_row_clicked(const Gtk::TreeModel::Path& path,
                Gtk::TreeViewColumn* column)
{
	Gtk::TreeModel::iterator iter = tree_model->get_iter(path);
	if(iter) {
		Gtk::TreeModel::Row row = *iter;
		cout << row[columns.page_nr] << endl;
	}
}
