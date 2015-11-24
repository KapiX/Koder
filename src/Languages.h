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

#ifndef LANGUAGES_H
#define LANGUAGES_H


#include <vector>

#include <String.h>


class Editor;
class XmlDocument;


enum LanguageType {
	LANGUAGE_TEXT = 0,
	LANGUAGE_PHP,
	LANGUAGE_C,
	LANGUAGE_CPP,
	LANGUAGE_CS,
	LANGUAGE_OBJC,
	LANGUAGE_JAVA,
	LANGUAGE_RC,
	LANGUAGE_HTML,
	LANGUAGE_XML,
	LANGUAGE_MAKEFILE,
	LANGUAGE_PASCAL,
	LANGUAGE_BATCH,
	LANGUAGE_INI,
	LANGUAGE_ASCII,
	LANGUAGE_ASP,
	LANGUAGE_SQL,
	LANGUAGE_VB,
	LANGUAGE_JS,
	LANGUAGE_CSS,
	LANGUAGE_PERL,
	LANGUAGE_PYTHON,
	LANGUAGE_LUA,
	LANGUAGE_TEX,
	LANGUAGE_FORTRAN,
	LANGUAGE_BASH,
	LANGUAGE_FLASH,
	LANGUAGE_NSIS,
	LANGUAGE_TCL,
	LANGUAGE_LISP,
	LANGUAGE_SCHEME,
	LANGUAGE_ASM,
	LANGUAGE_DIFF,
	LANGUAGE_PROPS,
	LANGUAGE_POSTSCRIPT,
	LANGUAGE_RUBY,
	LANGUAGE_SMALLTALK,
	LANGUAGE_VHDL,
	LANGUAGE_KIX,
	LANGUAGE_AU3,
	LANGUAGE_CAML,
	LANGUAGE_ADA,
	LANGUAGE_VERILOG,
	LANGUAGE_MATLAB,
	LANGUAGE_HASKELL,
	LANGUAGE_INNO,
	LANGUAGE_CMAKE,
	LANGUAGE_YAML,
	LANGUAGE_COBOL,
	LANGUAGE_GUI4CLI,
	LANGUAGE_D,
	LANGUAGE_POWERSHELL,
	LANGUAGE_R,
	LANGUAGE_JSP,
	LANGUAGE_COFFEESCRIPT,
	LANGUAGE_RECIPE,
	LANGUAGE_RUST,

	LANGUAGE_COUNT
};


struct LanguageDefinition {
	LanguageType fType;
	BString fLexerName;
	BString fShortName;
	BString fLongName;
	int fLexerID;
	BString fKeywords[9];
};


class Languages {
public:
			LanguageDefinition&					GetLanguage(LanguageType lang);
			std::vector<LanguageDefinition>&	GetLanguages() { return sLanguages; }
			void								SortAlphabetically();
			void								ApplyLanguage(Editor* editor, const char* path, const char* lang, XmlDocument* doc = nullptr);

private:
	static	std::vector<LanguageDefinition>		sLanguages;
};


#endif // LANGUAGES_H
