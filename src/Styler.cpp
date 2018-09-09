/*
 * Copyright 2014-2017 Kacper Kasper <kacperkasper@gmail.com>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include "Styler.h"

#include <unordered_map>

#include <yaml-cpp/yaml.h>

#include <Alert.h>
#include <Directory.h>
#include <FindDirectory.h>
#include <Path.h>
#include <String.h>

#include "Editor.h"
#include "EditorWindow.h"
#include "Utils.h"


#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "Styler"


std::unordered_map<int, Styler::Style>	Styler::sStylesMapping;


/* static */ void
Styler::ApplyGlobal(Editor* editor, const char* style)
{
	static bool alertShown = false;
	bool found = false;
	BPath dataPath;
	find_directory(B_SYSTEM_DATA_DIRECTORY, &dataPath);
	try {
		_ApplyGlobal(editor, style, dataPath);
		found = true;
	} catch (YAML::BadFile &) {
	}
	find_directory(B_USER_DATA_DIRECTORY, &dataPath);
	try {
		_ApplyGlobal(editor, style, dataPath);
		found = true;
	} catch (YAML::BadFile &) {
	}
	find_directory(B_SYSTEM_NONPACKAGED_DATA_DIRECTORY, &dataPath);
	try {
		_ApplyGlobal(editor, style, dataPath);
		found = true;
	} catch (YAML::BadFile &) {
	}
	find_directory(B_USER_NONPACKAGED_DATA_DIRECTORY, &dataPath);
	try {
		_ApplyGlobal(editor, style, dataPath);
		found = true;
	} catch (YAML::BadFile &) {
	}
	if(found == false && alertShown == false) {
		alertShown = true;
		BAlert* alert = new BAlert(B_TRANSLATE("Style files"),
			B_TRANSLATE("Couldn't find style files. Make sure you have them "
				"installed in one of your data directories."),
				B_TRANSLATE("OK"), nullptr, nullptr, B_WIDTH_AS_USUAL,
				B_WARNING_ALERT);
		alert->Go();
	}
}


/* static */ void
Styler::_ApplyGlobal(Editor* editor, const char* style, const BPath &path)
{
	BPath p(path);
	p.Append(gAppName);
	p.Append("styles");
	p.Append(style);
	const YAML::Node styles = YAML::LoadFile(std::string(p.Path()) + ".yaml");
	YAML::Node global;
	if(styles["Global"]) {
		global = styles["Global"];
	}

	int id, fg, bg, fs;
	if(global["Default"]) {
		_GetAttributesFromNode(global["Default"], &id, &fg, &bg, &fs);

		font_family fixed;
		be_fixed_font->GetFamilyAndStyle(&fixed, nullptr);
		editor->SendMessage(SCI_STYLESETFONT, id, (sptr_t) fixed);
		editor->SendMessage(SCI_STYLESETSIZE, id, (sptr_t) be_fixed_font->Size());
		_SetAttributesInEditor(editor, id, fg, bg, fs);
		editor->SendMessage(SCI_STYLECLEARALL, 0, 0);
		editor->SendMessage(SCI_STYLESETFONT, 36, (sptr_t) fixed);
		editor->SendMessage(SCI_STYLESETSIZE, 36, (sptr_t) (be_fixed_font->Size() / 1.3));
		editor->SendMessage(SCI_SETWHITESPACESIZE, be_fixed_font->Size() / 6, 0);

		// whitespace
		editor->SendMessage(SCI_INDICSETSTYLE, 0, INDIC_ROUNDBOX);
		editor->SendMessage(SCI_INDICSETFORE, 0, 0x0000FF);
		editor->SendMessage(SCI_INDICSETALPHA, 0, 100);
		// IME
		editor->SendMessage(SCI_INDICSETSTYLE, INDIC_IME, INDIC_FULLBOX);
		editor->SendMessage(SCI_INDICSETFORE, INDIC_IME, 0xFF0000);
		editor->SendMessage(SCI_INDICSETSTYLE, INDIC_IME+1, INDIC_FULLBOX);
		editor->SendMessage(SCI_INDICSETFORE, INDIC_IME+1, 0x0000FF);
	}
	for(YAML::const_iterator it = global.begin(); it != global.end(); ++it) {
		std::string name = it->first.as<std::string>();
		_GetAttributesFromNode(global[name], &id, &fg, &bg, &fs);
		if(id != -1) {
			_SetAttributesInEditor(editor, id, fg, bg, fs);
			sStylesMapping.emplace(id, Style(fg, bg, fs));
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
	for(const auto& style : styles) {
		if(style.first.as<std::string>() == "Global")
			continue;
		_GetAttributesFromNode(style.second, &id, &fg, &bg, &fs);
		sStylesMapping.emplace(id, Style(fg, bg, fs));
	}
}


/* static */ void
Styler::ApplyLanguage(Editor* editor, const std::map<int, int>& styleMapping)
{
	for(const auto& mapping : styleMapping) {
		int scintillaId = mapping.first;
		int styleId = mapping.second;
		const auto it = sStylesMapping.find(styleId);
		if(it != sStylesMapping.end()) {
			Style s = it->second;
			_SetAttributesInEditor(editor, scintillaId, s.fgColor, s.bgColor, s.style);
		}
	}
}


/* static */ void
Styler::GetAvailableStyles(std::set<std::string> &styles)
{
	BPath dataPath;
	find_directory(B_SYSTEM_DATA_DIRECTORY, &dataPath);
	_GetAvailableStyles(styles, dataPath);
	find_directory(B_USER_DATA_DIRECTORY, &dataPath);
	_GetAvailableStyles(styles, dataPath);
	find_directory(B_SYSTEM_NONPACKAGED_DATA_DIRECTORY, &dataPath);
	_GetAvailableStyles(styles, dataPath);
	find_directory(B_USER_NONPACKAGED_DATA_DIRECTORY, &dataPath);
	_GetAvailableStyles(styles, dataPath);
}


/* static */ void
Styler::_GetAvailableStyles(std::set<std::string> &styles, const BPath &path)
{
	BPath p(path);
	p.Append(gAppName);
	p.Append("styles");
	BDirectory directory(p.Path());
	BEntry entry;
	char name[B_FILE_NAME_LENGTH];
	while(directory.GetNextEntry(&entry) == B_OK) {
		if(entry.IsDirectory())
			continue;
		entry.GetName(name);
		if(GetFileExtension(name) == "yaml") {
			styles.insert(GetFileName(name));
		}
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
		*fgColor = _CSSToInt(node["foreground"].as<std::string>());
	}
	if(node["background"]) {
		*bgColor = _CSSToInt(node["background"].as<std::string>());
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


/* static */ int
Styler::_CSSToInt(const std::string cssColor)
{
	if(cssColor[0] != '#' || cssColor.length() != 7)
		return -1;

	std::string red = cssColor.substr(1, 2);
	std::string green = cssColor.substr(3, 2);
	std::string blue = cssColor.substr(5, 2);

	return std::stoi(blue + green + red, nullptr, 16);
}
