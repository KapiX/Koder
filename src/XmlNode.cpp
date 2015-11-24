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

#include "XmlNode.h"


XmlNode::XmlNode()
	:
	fNode(nullptr)
{
}


XmlNode::~XmlNode()
{
	xmlFreeNodeList(fNode);
}


void
XmlNode::Init(xmlNodePtr node)
{
	fNode = xmlCopyNodeList(node);

	if(fNode == NULL) {
	}
}


BString
XmlNode::GetContent()
{
	if(fNode == nullptr) return BString();

	xmlChar* content = xmlNodeGetContent(fNode);
	const BString temp(reinterpret_cast<char*>(content));
	xmlFree(content);
	return temp;
}


BString
XmlNode::GetAttribute(const char* name)
{
	if(fNode == nullptr) return BString();

	xmlChar* attr = xmlGetProp(fNode, reinterpret_cast<const xmlChar*>(name));
	const BString temp(reinterpret_cast<char*>(attr));
	xmlFree(attr);
	return temp;
}
