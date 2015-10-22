/*
 * Koder is a code editor for Haiku based on Scintilla.
 *
 * Copyright (C) 2015 Kacper Kasper <kacperkasper@gmail.com>
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
#include "XmlDocument.h"

#include "XmlNode.h"


XmlDocument::XmlDocument(const char* path)
{
	fDocument = xmlParseFile(path);

	if(fDocument == NULL) {
		
	}
}


XmlDocument::~XmlDocument()
{
	xmlFreeDoc(fDocument);
}


XmlNode*
XmlDocument::GetNodesByXPath(const char* xpath, uint32* count)
{
	xmlXPathContextPtr context;
	xmlXPathObjectPtr object;
	
	context = xmlXPathNewContext(fDocument);

	if(context == NULL) {
	}

	object = xmlXPathEvalExpression(reinterpret_cast<const xmlChar*>(xpath), context);

	xmlXPathFreeContext(context);

	if(object == NULL) {
	}

	if(xmlXPathNodeSetIsEmpty(object->nodesetval)) {
		xmlXPathFreeObject(object);
		*count = 0;
		return nullptr;
	}

	XmlNode* nodes = new XmlNode[object->nodesetval->nodeNr];
	for(uint32 i = 0; i < object->nodesetval->nodeNr; i++) {
		nodes[i].Init(object->nodesetval->nodeTab[i]);
	}
	*count = object->nodesetval->nodeNr;

	xmlXPathFreeObject(object);

	return nodes;
}
