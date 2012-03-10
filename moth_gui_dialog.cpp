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
#include "moth_gui_dialog.h"
#include <stdlib.h>
#include <string.h>
#include <errno.h>

moth_dialog::moth_dialog() throw()
{
    stream = NULL;
}

moth_dialog::~moth_dialog()
{
    if (stream)
	    pclose(stream);
    stream = NULL;
}

moth_dialog_response moth_dialog::show_dialog(std::string &cmd) throw()
{
	stream = popen(cmd.c_str(), "r");
    if (!stream || errno == ECHILD)
        return MOTH_DIALOG_ERROR;
    memset(line, '\0', len);
	fgets(line, len, stream);
    if (strlen(line) == 0) {
	    pclose(stream);
        stream = NULL;
        return MOTH_DIALOG_ERROR;
    }
	pclose(stream);
    stream = NULL;
    return MOTH_DIALOG_OK;
}

moth_dialog_response moth_dialog::choose_file(std::string &type,
        std::string &file) throw()
{
    std::string cmd("zenity --file-selection");
    if (type.length()) {
        cmd += " --file-filter=";
        cmd += type;
    }
    moth_dialog_response resp = show_dialog(cmd);
    if (resp == MOTH_DIALOG_OK) {
        file = line;
    }
    return resp;
}

moth_dialog_response moth_dialog::input(std::string &info,
        std::string &input) throw()
{
    std::string cmd("zenity --entry");
    if (info.length()) {
        cmd += " --text=";
        cmd += info;
    }
    moth_dialog_response resp = show_dialog(cmd);
    if (resp == MOTH_DIALOG_OK) {
        input = line;
    }
    return resp;
}

moth_dialog_response moth_dialog::info(std::string &info) throw()
{
    std::string cmd("zenity --info");
    if (info.length()) {
        cmd += " --text=";
        cmd += info;
    }
    moth_dialog_response resp = show_dialog(cmd);
    return resp;
}
