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
#include "XmlDocument.h"
#include "XmlNode.h"

Styler::Styler(const char* path)
	:
	fDocument(new XmlDocument(path))
{
}

Styler::~Styler()
{
	delete fDocument;
}

void
Styler::ApplyGlobal(Editor* editor)
{
	uint32 count;
	XmlNode* defaultStyle = fDocument->GetNodesByXPath("/NotepadPlus/GlobalStyles/WidgetStyle[@styleID='32']", &count);
	
	int id, fg, bg, fs;
	_GetAttributesFromNode(defaultStyle[0], &id, &fg, &bg, &fs);
	
	editor->SendMessage(SCI_STYLESETFONT, id, (sptr_t) "DejaVu Sans Mono");
	_SetAttributesInEditor(editor, id, fg, bg, fs);
	editor->SendMessage(SCI_STYLECLEARALL, 0, 0);

	delete []defaultStyle;
	
	XmlNode* globalStyle = fDocument->GetNodesByXPath("/NotepadPlus/GlobalStyles/WidgetStyle", &count);

	for(int i = 0; i < count; i++) {
		_GetAttributesFromNode(globalStyle[i], &id, &fg, &bg, &fs);
		
		if(id != 0 && id != 2069) {
			_SetAttributesInEditor(editor, id, fg, bg, fs);
		}
		else
		{
			BString name = globalStyle[i].GetAttribute("name");
			if(name == "Current line background colour") {
				editor->SendMessage(SCI_SETCARETLINEBACK, bg, 0);
				//editor->SendMessage(SCI_SETCARETLINEBACKALPHA, 128, 0);
			}
			else if(name == "White space symbol") {
				editor->SendMessage(SCI_SETWHITESPACEFORE, true, fg);
				editor->SendMessage(SCI_SETWHITESPACEBACK, true, bg);
			}
			else if(name == "Selected text colour") {
				editor->SendMessage(SCI_SETSELFORE, true, fg);
				editor->SendMessage(SCI_SETSELBACK, true, bg);
			}
			else if(name == "Caret colour") {
				editor->SendMessage(SCI_SETCARETFORE, fg, 0);
			}
			else if(name == "Edge colour") {
				editor->SendMessage(SCI_SETEDGECOLOUR, bg, 0);
			}
		}
	}

	delete []globalStyle;
}

void
Styler::ApplyLanguage(Editor* editor, const char* lang)
{
	BString xpath("/NotepadPlus/LexerStyles/LexerType[@name='%s']/WordsStyle");
	xpath.ReplaceFirst("%s", lang);
	uint32 count;
	XmlNode* nodes = fDocument->GetNodesByXPath(xpath.String(), &count);
	int id, fg, bg, fs;
	for(int i = 0; i < count; i++) {
		_GetAttributesFromNode(nodes[i], &id, &fg, &bg, &fs);
		_SetAttributesInEditor(editor, id, fg, bg, fs);
	}

	delete []nodes;
}

void
Styler::_GetAttributesFromNode(XmlNode &node, int* styleId, int* fgColor, int* bgColor, int* fontStyle)
{
	*styleId = strtol(node.GetAttribute("styleID").String(), NULL, 10);
	*fgColor = strtol(node.GetAttribute("fgColor").String(), NULL, 16);
	*bgColor = strtol(node.GetAttribute("bgColor").String(), NULL, 16);
	*fontStyle = strtol(node.GetAttribute("fontStyle").String(), NULL, 10);
}

void
Styler::_SetAttributesInEditor(Editor* editor, int styleId, int fgColor, int bgColor, int fontStyle)
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
