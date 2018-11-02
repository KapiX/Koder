/*
 * Copyright 2015-2018 Kacper Kasper <kacperkasper@gmail.com>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include "Languages.h"

#include <algorithm>
#include <functional>
#include <map>
#include <string>

#include <Catalog.h>
#include <Directory.h>
#include <FindDirectory.h>
#include <Path.h>
#include <String.h>

#include <SciLexer.h>
#include <yaml-cpp/yaml.h>

#include "Editor.h"
#include "EditorWindow.h"


#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "Languages"


std::vector<std::string>			Languages::sLanguages;
std::map<std::string, std::string>	Languages::sMenuItems;
std::map<std::string, std::string> 	Languages::sExtensions;


namespace {

/**
 * Executes a specified function for each data directory, going from system to
 * user, packaged to non-packaged. The path is available as a parameter to the
 * user supplied function.
 */
void
DoInAllDataDirectories(std::function<void(const BPath&)> func) {
	BPath dataPath;
	find_directory(B_SYSTEM_DATA_DIRECTORY, &dataPath);
	func(dataPath);
	find_directory(B_USER_DATA_DIRECTORY, &dataPath);
	func(dataPath);
	find_directory(B_SYSTEM_NONPACKAGED_DATA_DIRECTORY, &dataPath);
	func(dataPath);
	find_directory(B_USER_NONPACKAGED_DATA_DIRECTORY, &dataPath);
	func(dataPath);
}

}


/* static */ bool
Languages::GetLanguageForExtension(const std::string ext, std::string& lang)
{
	lang = "text";
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


/**
 * Reads YAML files from all data directories and creates a single style map,
 * where repeated keys are overridden (user non-packaged being final).
 */
/* static */ std::map<int, int>
Languages::ApplyLanguage(Editor* editor, const char* lang)
{
	std::map<int, int> styleMapping;
	DoInAllDataDirectories([&](const BPath& path) {
			try {
				auto m = _ApplyLanguage(editor, lang, path);
				m.merge(styleMapping);
				std::swap(styleMapping, m);
			} catch (YAML::BadFile &) {}
		});
	return styleMapping;
}

/**
 * Loads YAML file with language specification:
 *   lexer: int if supplied with Scintilla, string if external (required)
 *   properties: string/string map passed to SCI_SETPROPERTY
 *   keywords: index(int)/string map passed to SCI_SETKEYWORDS
 *   comments:
 *     line: string
 *     block: pair of strings
 *   styles: lexer style id(int)/Koder style id(int) map
 */
/* static */ std::map<int, int>
Languages::_ApplyLanguage(Editor* editor, const char* lang, const BPath &path)
{
	BPath p(path);
	p.Append(gAppName);
	p.Append("languages");
	p.Append(lang);
	const YAML::Node language = YAML::LoadFile(std::string(p.Path()) + ".yaml");
	try {
		int lexerID = language["lexer"].as<int>();
		editor->SendMessage(SCI_SETLEXER, static_cast<uptr_t>(lexerID), 0);
	} catch(YAML::TypedBadConversion<int>&) {
		std::string lexerName = language["lexer"].as<std::string>();
		editor->SendMessage(SCI_SETLEXERLANGUAGE, 0,
			reinterpret_cast<const sptr_t>(lexerName.c_str()));
	}

	for(const auto& property : language["properties"]) {
		auto name = property.first.as<std::string>();
		auto value = property.second.as<std::string>();
		editor->SendMessage(SCI_SETPROPERTY, (uptr_t) name.c_str(), (sptr_t) value.c_str());
	}

	for(const auto& keyword : language["keywords"]) {
		auto num = keyword.first.as<int>();
		auto words = keyword.second.as<std::string>();
		editor->SendMessage(SCI_SETKEYWORDS, num, (sptr_t) words.c_str());
	}

	const YAML::Node comments = language["comments"];
	if(comments) {
		const YAML::Node line = comments["line"];
		if(line)
			editor->SetCommentLineToken(line.as<std::string>());
		const YAML::Node block = comments["block"];
		if(block && block.IsSequence())
			editor->SetCommentBlockTokens(block[0].as<std::string>(), block[1].as<std::string>());
	}

	const YAML::Node styles = language["styles"];
	if(styles) {
		return styles.as<std::map<int, int>>();
	}
	return std::map<int, int>();
}


/* static */ void
Languages::LoadLanguages()
{
	DoInAllDataDirectories([](const BPath& path) {
			try {
				_LoadLanguages(path);
			} catch (YAML::BadFile &) {}
		});
}


/* static */ void
Languages::_LoadLanguages(const BPath& path)
{
	BPath p(path);
	p.Append(gAppName);
	p.Append("languages");
	const YAML::Node languages = YAML::LoadFile(std::string(p.Path()) + ".yaml");
	for(const auto& language : languages) {
		auto name = language.first.as<std::string>();
		auto menuitem = language.second["name"].as<std::string>();
		auto extensions = language.second["extensions"].as<std::vector<std::string>>();
		for(auto extension : extensions) {
			sExtensions[extension] = name;
		}
		if(std::find(sLanguages.begin(), sLanguages.end(), name) == sLanguages.end())
			sLanguages.push_back(name);
		sMenuItems[name] = menuitem;
	}
}


/* static */ void
Languages::LoadExternalLexers(Editor* editor)
{
	DoInAllDataDirectories([&](const BPath& path) {
			_LoadExternalLexers(path, editor);
		});
}


/**
 * Iterates through all files in path/scintilla/lexers and loads them as lexers
 * into editor.
 */
/* static */ void
Languages::_LoadExternalLexers(const BPath& path, Editor* editor)
{
	BPath p(path);
	p.Append("scintilla");
	p.Append("lexers");
	BDirectory lexersDir(p.Path());
	if (lexersDir.InitCheck() != B_OK)
		return;

	BEntry lexerEntry;
	while(lexersDir.GetNextEntry(&lexerEntry, true) == B_OK) {
		if(lexerEntry.IsDirectory())
			continue;
		BPath lexerPath;
		lexerEntry.GetPath(&lexerPath);
		editor->SendMessage(SCI_LOADLEXERLIBRARY, 0,
			reinterpret_cast<const sptr_t>(lexerPath.Path()));
	}
}