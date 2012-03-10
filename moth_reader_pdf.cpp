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

#include "moth.h"
#include "moth_reader.h"
#include "moth_reader_pdf.h"

moth_reader_pdf::moth_reader_pdf(const std::string &file)
{
	std::string url;
	pages = NULL;
	num_pages = 0;
	get_url(file, url);
	doc = poppler_document_new_from_file(url.c_str(), NULL, NULL);
	if(NULL == doc) {
		throw moth_bad_file();
	}
	num_pages = poppler_document_get_n_pages(doc);

	try {
		pages = new PopplerPage* [num_pages];
	} catch(std::exception &e) {
		std::cerr << e.what() << std::endl;
		throw;
	}

	for(int i = 0; i < num_pages; i++) {
		pages[i] = poppler_document_get_page(doc, i);
	}
	/* TODO remove debug info */
	std::cout << "Oppened doc: there are " << num_pages << " pages in it" << std::endl;
}

moth_reader_pdf::~moth_reader_pdf()
{
	for(int i = 0; i < num_pages; i++) {
		g_object_unref(pages[i]);
	}
	g_object_unref(doc);
	delete [] pages;
}

int moth_reader_pdf::get_pages()
{
	return num_pages;
}

int moth_reader_pdf::get_page(int num, GdkPixbuf *&pixbuff)
{
	double w, h;
	static cairo_surface_t *cario;
	get_page_size(num, &w, &h);
	poppler_page_render_to_pixbuf(pages[num], 0, 0, w, h, 2, 0, pixbuff);
	return SUCCESS;
}

int moth_reader_pdf::get_page_size(int page, double *w, double *h)
{
	poppler_page_get_size(pages[page], w, h);
	return SUCCESS;
}

