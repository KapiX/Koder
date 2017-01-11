/*
 * Copyright 2014-2017 Kacper Kasper <kacperkasper@gmail.com>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include "Styler.h"

#include <cstdlib>
#include <yaml.h>

#include <FindDirectory.h>
#include <Path.h>
#include <String.h>

#include "Editor.h"
#include "EditorWindow.h"


/* static */ void
Styler::ApplyGlobal(Editor* editor, const char* style)
{
	BPath dataPath;
	find_directory(B_SYSTEM_DATA_DIRECTORY, &dataPath);
	try {
		_ApplyGlobal(editor, style, dataPath);
	} catch (YAML::BadFile &) {
	}
	find_directory(B_USER_DATA_DIRECTORY, &dataPath);
	try {
		_ApplyGlobal(editor, style, dataPath);
	} catch (YAML::BadFile &) {
	}
	find_directory(B_SYSTEM_NONPACKAGED_DATA_DIRECTORY, &dataPath);
	try {
		_ApplyGlobal(editor, style, dataPath);
	} catch (YAML::BadFile &) {
	}
	find_directory(B_USER_NONPACKAGED_DATA_DIRECTORY, &dataPath);
	try {
		_ApplyGlobal(editor, style, dataPath);
	} catch (YAML::BadFile &) {
	}
}


/* static */ void
Styler::_ApplyGlobal(Editor* editor, const char* style, const BPath &path)
{
	BPath p(path);
	p.Append(gAppName);
	p.Append("styles");
	p.Append(style);
	const YAML::Node global = YAML::LoadFile(std::string(p.Path()) + ".yaml");

	int id, fg, bg, fs;
	if(global["Default"]) {
		_GetAttributesFromNode(global["Default"], &id, &fg, &bg, &fs);

		font_family fixed;
		be_fixed_font->GetFamilyAndStyle(&fixed, NULL);
		editor->SendMessage(SCI_STYLESETFONT, id, (sptr_t) fixed);
		editor->SendMessage(SCI_STYLESETSIZE, id, (sptr_t) be_fixed_font->Size());
		_SetAttributesInEditor(editor, id, fg, bg, fs);
		editor->SendMessage(SCI_STYLECLEARALL, 0, 0);
		editor->SendMessage(SCI_STYLESETFONT, 36, (sptr_t) fixed);
		editor->SendMessage(SCI_STYLESETSIZE, 36, (sptr_t) (be_fixed_font->Size() / 1.3));
		editor->SendMessage(SCI_SETWHITESPACESIZE, be_fixed_font->Size() / 6, 0);
	}
	for(YAML::const_iterator it = global.begin(); it != global.end(); ++it) {
		std::string name = it->first.as<std::string>();
		_GetAttributesFromNode(global[name], &id, &fg, &bg, &fs);
		if(id != -1) {
			_SetAttributesInEditor(editor, id, fg, bg, fs);
		} else {
			if(name == "Current line") {
				editor->SendMessage(SCI_SETCARETLINEBACK, bg, 0);
				//editor->SendMessage(SCI_SETCARETLINEBACKALPHA, 128, 0);
			}
			else if(name == "Whitespace") {
				if(fg != -1) {
					editor->SendMessage(SCI_SETWHITESPACEFORE, true, fg);
				}
				if(bg != -1) {
					editor->SendMessage(SCI_SETWHITESPACEBACK, true, bg);
				}
			}
			else if(name == "Selected text") {
				if(fg != -1) {
					editor->SendMessage(SCI_SETSELFORE, true, fg);
				}
				if(bg != -1) {
					editor->SendMessage(SCI_SETSELBACK, true, bg);
				}
			}
			else if(name == "Caret") {
				editor->SendMessage(SCI_SETCARETFORE, fg, 0);
			}
			else if(name == "Edge") {
				editor->SendMessage(SCI_SETEDGECOLOUR, fg, 0);
			}
			else if(name == "Fold") {
				if(fg != -1) {
					editor->SendMessage(SCI_SETFOLDMARGINHICOLOUR, true, fg);
				}
				if(bg != -1) {
					editor->SendMessage(SCI_SETFOLDMARGINCOLOUR, true, bg);
				}
			}
		}
	}
}


/* static */ void
Styler::ApplyLanguage(Editor* editor, const char* style, const char* lang)
{
	BPath dataPath;
	find_directory(B_SYSTEM_DATA_DIRECTORY, &dataPath);
	try {
		_ApplyLanguage(editor, style, lang, dataPath);
	} catch (YAML::BadFile &) {
	}
	find_directory(B_USER_DATA_DIRECTORY, &dataPath);
	try {
		_ApplyLanguage(editor, style, lang, dataPath);
	} catch (YAML::BadFile &) {
	}
	find_directory(B_SYSTEM_NONPACKAGED_DATA_DIRECTORY, &dataPath);
	try {
		_ApplyLanguage(editor, style, lang, dataPath);
	} catch (YAML::BadFile &) {
	}
	find_directory(B_USER_NONPACKAGED_DATA_DIRECTORY, &dataPath);
	try {
		_ApplyLanguage(editor, style, lang, dataPath);
	} catch (YAML::BadFile &) {
	}
}


/* static */ void
Styler::_ApplyLanguage(Editor* editor, const char* style, const char* lang, const BPath &path)
{
	BPath p(path);
	p.Append(gAppName);
	p.Append("styles");
	p.Append(style);
	p.Append(lang);
	const YAML::Node language = YAML::LoadFile(std::string(p.Path()) + ".yaml");
	int id, fg, bg, fs;
	for(YAML::const_iterator it = language.begin(); it != language.end(); ++it) {
		std::string name = it->first.as<std::string>();
		_GetAttributesFromNode(language[name], &id, &fg, &bg, &fs);
		_SetAttributesInEditor(editor, id, fg, bg, fs);
	}
}


void
Styler::_GetAttributesFromNode(const YAML::Node &node, int* styleId, int* fgColor,
	int* bgColor, int* fontStyle)
{
	*styleId = -1;
	*fgColor = -1;
	*bgColor = -1;
	*fontStyle = -1;

	if(node["id"]) {
		*styleId = node["id"].as<int>();
	}
	if(node["foreground"]) {
		*fgColor = strtol(node["foreground"].as<std::string>().c_str(), NULL, 16);
	}
	if(node["background"]) {
		*bgColor = strtol(node["background"].as<std::string>().c_str(), NULL, 16);
	}
	if(node["style"]) {
		*fontStyle = 0;
		auto styles = node["style"].as<std::vector<std::string>>();
		for(auto style : styles) {
			if(style == "bold") *fontStyle |= 1;
			else if(style == "italic") *fontStyle |= 2;
			else if(style == "underline") *fontStyle |= 4;
		}
	}
}


void
Styler::_SetAttributesInEditor(Editor* editor, int styleId, int fgColor,
	int bgColor, int fontStyle)
{
	if(fgColor != -1) {
		editor->SendMessage(SCI_STYLESETFORE, styleId, fgColor);
	}
	if(bgColor != -1) {
		editor->SendMessage(SCI_STYLESETBACK, styleId, bgColor);
	}
	if(fontStyle != -1) {
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
}
