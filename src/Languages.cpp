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

#include "Languages.h"

#include <algorithm>
#include <map>
#include <string>

#include <Catalog.h>
#include <FindDirectory.h>
#include <Path.h>
#include <String.h>

#include <SciLexer.h>
#include <yaml.h>

#include "Editor.h"
#include "EditorWindow.h"


#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "Languages"


std::vector<std::string>			Languages::sLanguages;
std::map<std::string, std::string>	Languages::sMenuItems;
std::map<std::string, std::string> 	Languages::sExtensions;

#include <cstdio>
/* static */ bool
Languages::GetLanguageForExtension(const std::string ext, std::string& lang)
{
	lang = "text";
	fprintf(stderr, "ext: %s\n", ext.c_str());
	if(sExtensions.count(ext) > 0) {
		lang = sExtensions[ext];
		return true;
	}
	return false;
}


/* static */ void
Languages::SortAlphabetically()
{
	std::sort(sLanguages.begin(), sLanguages.end());
}


/* static */ void
Languages::ApplyLanguage(Editor* editor, const char* lang)
{
	BPath dataPath;
	find_directory(B_SYSTEM_DATA_DIRECTORY, &dataPath);
	try {
		_ApplyLanguage(editor, lang, dataPath);
	} catch (YAML::BadFile &) {
	}
	find_directory(B_USER_DATA_DIRECTORY, &dataPath);
	try {
		_ApplyLanguage(editor, lang, dataPath);
	} catch (YAML::BadFile &) {
	}
	find_directory(B_SYSTEM_NONPACKAGED_DATA_DIRECTORY, &dataPath);
	try {
		_ApplyLanguage(editor, lang, dataPath);
	} catch (YAML::BadFile &) {
	}
	find_directory(B_USER_NONPACKAGED_DATA_DIRECTORY, &dataPath);
	try {
		_ApplyLanguage(editor, lang, dataPath);
	} catch (YAML::BadFile &) {
	}
}


/* static */ void
Languages::_ApplyLanguage(Editor* editor, const char* lang, const BPath &path)
{
	BPath p(path);
	p.Append(gAppName);
	p.Append("languages");
	p.Append(lang);
	const YAML::Node language = YAML::LoadFile(std::string(p.Path()) + ".yaml");
	int lexerID = language["lexer"].as<int>();
	editor->SendMessage(SCI_SETLEXER, static_cast<uptr_t>(lexerID), 0);

	const YAML::Node properties = language["properties"];
	for(YAML::const_iterator it = properties.begin(); it != properties.end(); ++it) {
		auto name = it->first.as<std::string>();
		auto value = it->second.as<std::string>();
		editor->SendMessage(SCI_SETPROPERTY, (uptr_t) name.c_str(), (sptr_t) value.c_str());
	}

	const YAML::Node keywords = language["keywords"];
	for(YAML::const_iterator it = keywords.begin(); it != keywords.end(); ++it) {
		auto num = it->first.as<int>();
		auto words = it->second.as<std::string>();
		editor->SendMessage(SCI_SETKEYWORDS, num, (sptr_t) words.c_str());
	}
}


/* static */ void
Languages::LoadLanguages()
{
	BPath dataPath;
	find_directory(B_SYSTEM_DATA_DIRECTORY, &dataPath);
	try {
		_LoadLanguages(dataPath);
	} catch (YAML::BadFile &) {
	}
	find_directory(B_USER_DATA_DIRECTORY, &dataPath);
	try {
		_LoadLanguages(dataPath);
	} catch (YAML::BadFile &) {
	}
	find_directory(B_SYSTEM_NONPACKAGED_DATA_DIRECTORY, &dataPath);
	try {
		_LoadLanguages(dataPath);
	} catch (YAML::BadFile &) {
	}
	find_directory(B_USER_NONPACKAGED_DATA_DIRECTORY, &dataPath);
	try {
		_LoadLanguages(dataPath);
	} catch (YAML::BadFile &) {
	}
}


/* static */ void
Languages::_LoadLanguages(const BPath& path)
{
	BPath p(path);
	p.Append(gAppName);
	p.Append("languages");
	const YAML::Node languages = YAML::LoadFile(std::string(p.Path()) + ".yaml");
	for(YAML::const_iterator it = languages.begin(); it != languages.end(); ++it) {
		auto name = it->first.as<std::string>();
		auto menuitem = it->second["name"].as<std::string>();
		auto extensions = it->second["extensions"].as<std::vector<std::string>>();
		for(auto extension : extensions) {
			sExtensions[extension] = name;
		}
		sLanguages.push_back(name);
		sMenuItems[name] = menuitem;
	}
}
