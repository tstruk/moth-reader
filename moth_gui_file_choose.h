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


#ifndef __MOTH_GUI_FILE_CH__
#define __MOTH_GUI_FILE_CH__

#include <string>

class moth_gui_file_ch
{
public:
  void choose_file(std::string&);
  moth_gui_file_ch();
  virtual ~moth_gui_file_ch();
};
#endif
