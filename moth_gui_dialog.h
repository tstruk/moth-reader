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
#ifndef __MOTH_GUI_DIALOG__
#define __MOTH_GUI_DIALOG__

#include <string>
extern "C"{
#include <stdio.h>
}
enum moth_dialog_response {
    MOTH_DIALOG_OK,
    MOTH_DIALOG_CANCEL,
    MOTH_DIALOG_YES,
    MOTH_DIALOG_NO,
    MOTH_DIALOG_ERROR
};

const static uint16_t len = 256;

class moth_dialog {
	FILE *stream;
	char line[len];
    moth_dialog_response show_dialog(std::string &cmd) throw();

    public:
        moth_dialog() throw();
        virtual ~moth_dialog();

        moth_dialog_response choose_file(std::string &type,
                                         std::string &file) throw();
        moth_dialog_response save_file(std::string &file) throw();
        moth_dialog_response input(std::string &info,
                                   std::string &input) throw();
        moth_dialog_response info(std::string &info) throw();
        moth_dialog_response error(std::string &error) throw();
};
#endif
