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

std::string moth_reader::get_url(const char * const path)
{
    FILE *f = fopen(path, "r");
    if(!f)
    {
        throw moth_bad_file();
    }
    fclose(f);
    std::string url = "file://";
    const char * const dir_up = "../";
    char slash = '/';
    char *pwd = getenv("PWD");
    int up_ctr = 0;
    int loop;
    const char *new_path = path;
    do{
        loop = strncmp(new_path, dir_up, 3);
        if(loop == 0){
            up_ctr++;
            new_path += 3;
        }
    } while(loop == 0);
    if(0 == up_ctr)
    {
        pwd = (char*)"";
    }
    while(new_path && up_ctr)
    {
        char *p = rindex(pwd, slash);
        *p = '\0';
        up_ctr--;
    }
    url += pwd;
    url += new_path;
    return url;
}
