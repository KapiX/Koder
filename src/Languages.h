/*
 * Koder is a code editor for Haiku based on Scintilla.
 *
 * Copyright (C) 2015-2016 Kacper Kasper <kacperkasper@gmail.com>
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

#ifndef LANGUAGES_H
#define LANGUAGES_H


#include <map>
#include <string>
#include <vector>


class BPath;
class Editor;


class Languages {
public:
	static	size_t								GetCount() { return sLanguages.size(); }
	static	std::string							GetLanguage(int index) { return sLanguages[index]; }
	static	std::string							GetMenuItemName(std::string lang) { return sMenuItems[lang]; }
	static	std::string							GetLanguageForExtension(std::string ext) { if(sExtensions.count(ext)) return sExtensions[ext]; return "text"; }
	static	void								SortAlphabetically();
	static	void								ApplyLanguage(Editor* editor, const char* lang);
	static	void								LoadLanguages();

private:
	static	void								_LoadLanguages(const BPath& path);
	static	void								_ApplyLanguage(Editor* editor, const char* lang, const BPath &path);
	static	std::vector<std::string>			sLanguages;
	static	std::map<std::string, std::string>	sMenuItems;
	static	std::map<std::string, std::string> 	sExtensions;
};


#endif // LANGUAGES_H
