/*
 * Koder is a code editor for Haiku based on Scintilla.
 *
 * Copyright (C) 2014-2016 Kacper Kasper <kacperkasper@gmail.com>
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

#ifndef STYLER_H
#define STYLER_H


#include <yaml.h>


class BPath;
class Editor;


class Styler {
public:
	static	void	ApplyGlobal(Editor* editor, const char* style);
	static	void	ApplyLanguage(Editor* editor, const char* style, const char* lang);

private:
	static	void	_ApplyGlobal(Editor* editor, const char* style, const BPath &path);
	static	void	_ApplyLanguage(Editor* editor, const char* style, const char* lang, const BPath &path);
	static	void	_GetAttributesFromNode(const YAML::Node &node, int* styleId,
						int* fgColor, int* bgColor, int* fontStyle);
	static	void	_SetAttributesInEditor(Editor* editor, int styleId,
						int fgColor, int bgColor, int fontStyle);
};


#endif // STYLER_H
