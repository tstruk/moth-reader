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
		pages = new PopplerPage*[num_pages];
	} catch(std::exception &e) {
		std::cerr << e.what() << std::endl;
		throw;
	}

	for(unsigned int i = 0; i < num_pages; i++) {
		pages[i] = poppler_document_get_page(doc, i);
	}
}

moth_reader_pdf::~moth_reader_pdf()
{
	for(unsigned int i = 0; i < num_pages; i++) {
		g_object_unref(pages[i]);
	}
	g_object_unref(doc);
	delete [] pages;
}

unsigned int moth_reader_pdf::get_pages()
{
	return num_pages;
}

int moth_reader_pdf::get_page(int num, GdkPixbuf *&pixbuff)
{
	double w, h;
	get_page_size(num, &w, &h);
	w *= 2;
	h *= 2;
	cairo_surface_t *s = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, w, h);
	if (CAIRO_STATUS_SUCCESS != cairo_surface_status(s)) {
		std::cerr << "get_page - failed cairo_image_surface_create" << std::endl;
		return FAIL;
	}

	cairo_t *cr = cairo_create(s);
	if (CAIRO_STATUS_SUCCESS != cairo_status(cr)) {
		std::cerr << "get_page - failed cairo_create" << std::endl;
		return FAIL;
	}
	cairo_save(cr);
	cairo_scale(cr, 2, 2);
	poppler_page_render(pages[num], cr);
	cairo_restore(cr);
	cairo_set_operator(cr, CAIRO_OPERATOR_DEST_OVER);
	cairo_set_source_rgb(cr, 1, 1, 1);
	cairo_paint(cr);
	cairo_destroy(cr);
	pixbuff = gdk_pixbuf_get_from_surface(s, 0, 0, w, h);
	cairo_surface_destroy(s);
	return pixbuff != NULL ? SUCCESS : FAIL;
}

int moth_reader_pdf::get_page_size(int page, double *w, double *h)
{
	poppler_page_get_size(pages[page], w, h);
	return SUCCESS;
}

int moth_reader_pdf::walk_index(moth_index &index, PopplerIndexIter *iter)
{
	moth_index *ptr = &index;
	do {
		if (!iter) {
			std::cerr << "walk index - bad iterator" << std::endl;
			throw moth_bad_pdf();
		}
		PopplerAction *action = poppler_index_iter_get_action(iter);
		PopplerActionGotoDest action_goto;
		PopplerDest *dest;
		switch(action->type)
		{
			case POPPLER_ACTION_GOTO_DEST:
				action_goto = action->goto_dest;
				ptr->name = action_goto.title;
				switch(action_goto.dest->type)
				{
					case POPPLER_DEST_NAMED:
						ptr->name = action_goto.title;
						dest = poppler_document_find_dest(doc,
										 action_goto.dest->named_dest);
						if (dest) {
							ptr->page = dest->page_num;
							poppler_dest_free(dest);
						}
					break;
					case POPPLER_DEST_XYZ:
					case POPPLER_DEST_FIT:
					case POPPLER_DEST_FITH:
					case POPPLER_DEST_FITV:
					case POPPLER_DEST_FITR:
					case POPPLER_DEST_FITB:
					case POPPLER_DEST_FITBH:
					case POPPLER_DEST_FITBV:
						ptr->name = action_goto.title;
						ptr->page = action_goto.dest->page_num;
					break;

					default:
						std::cerr << "walk index different goto dest type - "
									   << action_goto.dest->type << std::endl;
				}
				break;

			default:
				std::cerr << "walk index different action type - "
												 << action->type << std::endl;
		}
		poppler_action_free(action);
		PopplerIndexIter *child = poppler_index_iter_get_child(iter);
		if (child) {
			moth_index *new_child = new moth_index;
			ptr->child = new_child;
			walk_index(*new_child, child);
		}
		poppler_index_iter_free(child);
		PopplerIndexIter *iter_copy = poppler_index_iter_copy(iter);
		if (poppler_index_iter_next(iter_copy)) {
			ptr->next = new moth_index;
			ptr = ptr->next;
		}
		poppler_index_iter_free(iter_copy);
	} while(poppler_index_iter_next(iter));
	return SUCCESS;
}

int moth_reader_pdf::build_index(moth_index &index)
{
	int ret;
	PopplerIndexIter *iter = poppler_index_iter_new(doc);
	try {
		ret = walk_index(index, iter);
	}
	catch (moth_bad_pdf &) {
		ret = FAIL;
	}
	poppler_index_iter_free(iter);
	return ret;
}

int moth_reader_pdf::save_copy(std::string &url)
{
	GError *err = NULL;
	if (poppler_document_save_a_copy(doc, url.c_str(), &err)) {
		return SUCCESS;
	}
	else {
		std::cerr << "Could not save file " << err->message
				  << " code " << err->code << std::endl;
		g_error_free(err);
		return FAIL;
	}
}

int moth_reader_pdf::search(std::string& str, unsigned int page,
							std::vector<moth_highlight> &result)
{

	PopplerPage *p = poppler_document_get_page(doc, page);
	GList *list = poppler_page_find_text(p, str.c_str());
	g_object_unref(p);
	if(!list) {
		return FAIL;
	}
	int i = 0;
	do {
		PopplerRectangle *rec = static_cast<PopplerRectangle*>(list->data);
		moth_highlight h = {page, rec->x1 * 2, rec->y1 * 2, rec->x2 * 2, rec->y2 * 2};
		result.push_back(h);
		list = g_list_nth(list, ++i);
	} while(list);
	g_list_free(list);
	return SUCCESS;
}


