/*
 * Koder is a code editor for Haiku based on Scintilla.
 *
 * Copyright (C) 2014-2015 Kacper Kasper <kacperkasper@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "Styler.h"

#include <cstdlib>
#include <libxml/parser.h>
#include <libxml/xpath.h>

#include <String.h>

#include "Editor.h"

Styler::Styler(const char* path)
{
	fPath = strdup(path);
}

Styler::~Styler()
{
	delete[] fPath;
}

void
Styler::ApplyGlobal(Editor* editor)
{
	const xmlChar* globalOverridePath = (xmlChar*) "/NotepadPlus/GlobalStyles/WidgetStyle[@styleID='32']";
	const xmlChar* globalStylePath = (xmlChar*) "/NotepadPlus/GlobalStyles/WidgetStyle";
	xmlDocPtr doc;
	xmlXPathContextPtr context;
	xmlXPathObjectPtr defaultStyle, globalStyle;
	
	doc = xmlParseFile(fPath);
	
	if(doc == NULL) {
	}
	
	context = xmlXPathNewContext(doc);
	if(context == NULL) {	
	}
	
	defaultStyle = xmlXPathEvalExpression((const xmlChar*) globalOverridePath, context);
	globalStyle = xmlXPathEvalExpression((const xmlChar*) globalStylePath, context);
	
	xmlXPathFreeContext(context);
	
	if(defaultStyle == NULL) {	
	}
	
	if(xmlXPathNodeSetIsEmpty(defaultStyle->nodesetval)) {
		xmlXPathFreeObject(defaultStyle);
	}
	
	if(globalStyle == NULL) {	
	}
	
	if(xmlXPathNodeSetIsEmpty(globalStyle->nodesetval)) {
		xmlXPathFreeObject(globalStyle);
	}
	
	int id, fg, bg, fs;
	xmlNodePtr node = defaultStyle->nodesetval->nodeTab[0];
	_GetAttributes(node, &id, &fg, &bg, &fs);
	
	editor->SendMessage(SCI_STYLESETFONT, id, (sptr_t) "DejaVu Sans Mono");
	_SetAttributes(editor, id, fg, bg, fs);
	editor->SendMessage(SCI_STYLECLEARALL, 0, 0);

	xmlXPathFreeObject(defaultStyle);
	
	for(int i = 0; i < globalStyle->nodesetval->nodeNr; i++) {
		node = globalStyle->nodesetval->nodeTab[i];
		_GetAttributes(node, &id, &fg, &bg, &fs);
		
		if(id != 0 && id != 2069) {
			_SetAttributes(editor, id, fg, bg, fs);
		}
		else
		{
			xmlChar* name = xmlGetProp(node, (xmlChar*) "name");
			if(xmlStrcmp(name, (xmlChar*) "Current line background colour") == 0) {
				editor->SendMessage(SCI_SETCARETLINEBACK, bg, 0);
				//editor->SendMessage(SCI_SETCARETLINEBACKALPHA, 128, 0);
			}
			else if(xmlStrcmp(name, (xmlChar*) "White space symbol") == 0) {
				editor->SendMessage(SCI_SETWHITESPACEFORE, true, fg);
				editor->SendMessage(SCI_SETWHITESPACEBACK, true, bg);
			}
			else if(xmlStrcmp(name, (xmlChar*) "Selected text colour") == 0) {
				editor->SendMessage(SCI_SETSELFORE, true, fg);
				editor->SendMessage(SCI_SETSELBACK, true, bg);
			}
			else if(xmlStrcmp(name, (xmlChar*) "Caret colour") == 0) {
				editor->SendMessage(SCI_SETCARETFORE, fg, 0);
			}
			else if(xmlStrcmp(name, (xmlChar*) "Edge colour") == 0) {
				editor->SendMessage(SCI_SETEDGECOLOUR, bg, 0);
			}
			xmlFree(name);
		}
	}
	
	xmlXPathFreeObject(globalStyle);
	
	xmlFreeDoc(doc);
}

void
Styler::ApplyLanguage(Editor* editor, const char* lang)
{
	BString xpath("/NotepadPlus/LexerStyles/LexerType[@name='%s']/WordsStyle");
	xpath.ReplaceFirst("%s", lang);
	xmlDocPtr doc;
	xmlXPathContextPtr context;
	xmlXPathObjectPtr lexerStyle;
	
	doc = xmlParseFile(fPath);
	
	if(doc == NULL) {
	}
	
	context = xmlXPathNewContext(doc);
	if(context == NULL) {	
	}
	
	lexerStyle = xmlXPathEvalExpression((const xmlChar*) xpath.String(), context);
	
	xmlXPathFreeContext(context);
	
	if(lexerStyle == NULL) {	
	}
	
	if(xmlXPathNodeSetIsEmpty(lexerStyle->nodesetval)) {
		xmlXPathFreeObject(lexerStyle);
	}
	
	int id, fg, bg, fs;
	for(int i = 0; i < lexerStyle->nodesetval->nodeNr; i++) {
		xmlNodePtr node = lexerStyle->nodesetval->nodeTab[i];
		_GetAttributes(node, &id, &fg, &bg, &fs);
		_SetAttributes(editor, id, fg, bg, fs);
	}
	
	xmlXPathFreeObject(lexerStyle);
	xmlFreeDoc(doc);
}

void
Styler::_GetAttributes(xmlNodePtr node, int* styleId, int* fgColor, int* bgColor, int* fontStyle)
{
	xmlChar* id;
	xmlChar* fg;
	xmlChar* bg;
	xmlChar* fs;

	id = xmlGetProp(node, (xmlChar*) "styleID");
	fg = xmlGetProp(node, (xmlChar*) "fgColor");
	bg = xmlGetProp(node, (xmlChar*) "bgColor");
	fs = xmlGetProp(node, (xmlChar*) "fontStyle");
	
	if(id != NULL) *styleId = strtol((char*) id, NULL, 10);
	if(fg != NULL) *fgColor = strtol((char*) fg, NULL, 16);
	if(bg != NULL) *bgColor = strtol((char*) bg, NULL, 16);
	if(fs != NULL) *fontStyle = strtol((char*) fs, NULL, 10);
	
	xmlFree(id);
	xmlFree(fg);
	xmlFree(bg);
	xmlFree(fs);
}

void
Styler::_SetAttributes(Editor* editor, int styleId, int fgColor, int bgColor, int fontStyle)
{
	editor->SendMessage(SCI_STYLESETFORE, styleId, fgColor);
	editor->SendMessage(SCI_STYLESETBACK, styleId, bgColor);
	if(fontStyle & 1) {
		editor->SendMessage(SCI_STYLESETBOLD, styleId, true);
	}
	if(fontStyle & 2) {
		editor->SendMessage(SCI_STYLESETITALIC, styleId, true);
	}
	if(fontStyle & 4) {
		editor->SendMessage(SCI_STYLESETUNDERLINE, styleId, true);
	}
}
