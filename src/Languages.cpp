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

#include "Languages.h"

#include <algorithm>

#include <Catalog.h>
#include <String.h>

#include <SciLexer.h>

#include "Editor.h"
#include "XmlDocument.h"
#include "XmlNode.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "Languages"

#define LANGDEF(t, l, shrt, lng, lex) { LANGUAGE_##t, BString(l), BString(B_TRANSLATE(shrt)), BString(B_TRANSLATE(lng)), lex, { "", "", "", "", "", "", "", "", "" } }

std::vector<LanguageDefinition> Languages::sLanguages = {
	LANGDEF(TEXT, "normal", "Normal text", "Normal text file", SCLEX_NULL),
	LANGDEF(PHP, "php", "PHP", "PHP Hypertext Preprocessor file", SCLEX_HTML),
	LANGDEF(C, "c", "C", "C source file", SCLEX_CPP),
	LANGDEF(CPP, "cpp", "C++", "C++ source file", SCLEX_CPP),
	LANGDEF(CS, "cs", "C#", "C# source file", SCLEX_CPP),
	LANGDEF(OBJC, "objc", "Objective-C", "Objective-C source file", SCLEX_CPP),
	LANGDEF(JAVA, "java", "Java", "Java source file", SCLEX_CPP),
	LANGDEF(RC, "rc", "RC", "Windows Resource file", SCLEX_CPP),
	LANGDEF(HTML, "html", "HTML", "Hyper Text Markup Language file", SCLEX_HTML),
	LANGDEF(XML, "xml", "XML", "eXtensible Markup Language file", SCLEX_XML),
	LANGDEF(MAKEFILE, "makefile", "Makefile", "Makefile", SCLEX_MAKEFILE),
	LANGDEF(PASCAL, "pascal", "Pascal", "Pascal source file", SCLEX_PASCAL),
	LANGDEF(BATCH, "batch", "Batch", "Batch file", SCLEX_BATCH),
	LANGDEF(INI, "ini", "INI", "Microsoft INI file", SCLEX_PROPERTIES),
	LANGDEF(ASCII, "nfo", "NFO", "MSDOS Style/ASCII Art", SCLEX_NULL),
	LANGDEF(ASP, "asp", "ASP", "Active Server Pages script file", SCLEX_HTML),
	LANGDEF(SQL, "sql", "SQL", "Structured Query Language file", SCLEX_SQL),
	LANGDEF(VB, "vb", "VB", "Visual Basic file", SCLEX_VB),
	LANGDEF(JS, "javascript", "JavaScript", "JavaScript file", SCLEX_CPP),
	LANGDEF(CSS, "css", "CSS", "Cascade Style Sheets file", SCLEX_CSS),
	LANGDEF(PERL, "perl", "Perl", "Perl source file", SCLEX_PERL),
	LANGDEF(PYTHON, "python", "Python", "Python source file", SCLEX_PYTHON),
	LANGDEF(LUA, "lua", "Lua", "Lua source file", SCLEX_LUA),
	LANGDEF(TEX, "tex", "TeX", "TeX file", SCLEX_TEX),
	LANGDEF(FORTRAN, "fortran", "Fortran", "Fortran source file", SCLEX_FORTRAN),
	LANGDEF(BASH, "bash", "Shell", "Unix script file", SCLEX_BASH),
	LANGDEF(FLASH, "actionscript", "ActionScript", "ActionScript source file", SCLEX_CPP),
	LANGDEF(NSIS, "nsis", "NSIS", "Nullsoft Scriptable Install System script file", SCLEX_NSIS),
	LANGDEF(TCL, "tcl", "TCL", "Tool Command Language file", SCLEX_TCL),
	LANGDEF(LISP, "lisp", "Lisp", "List Processing Language source file", SCLEX_LISP),
	LANGDEF(SCHEME, "scheme", "Scheme", "Scheme source file", SCLEX_LISP),
	LANGDEF(ASM, "asm", "Assembly", "Assembly language source file", SCLEX_ASM),
	LANGDEF(DIFF, "diff", "Diff", "Diff file", SCLEX_DIFF),
	LANGDEF(PROPS, "props", "Properties file", "Properties file", SCLEX_PROPERTIES),
	LANGDEF(POSTSCRIPT, "postscript", "Postscript", "Postscript source file", SCLEX_PS),
	LANGDEF(RUBY, "ruby", "Ruby", "Ruby source file", SCLEX_RUBY),
	LANGDEF(SMALLTALK, "smalltalk", "Smalltalk", "Smalltalk source file", SCLEX_SMALLTALK),
	LANGDEF(VHDL, "vhdl", "VHDL", "VHSIC Hardware Description Language file", SCLEX_VHDL),
	LANGDEF(KIX, "kix", "KiXtart", "KiXtart file", SCLEX_KIX),
	LANGDEF(AU3, "autoit", "AutoIt", "AutoIt", SCLEX_AU3),
	LANGDEF(CAML, "caml", "CAML", "Categorical Abstract Machine Language source file", SCLEX_CAML),
	LANGDEF(ADA, "ada", "Ada", "Ada source file", SCLEX_ADA),
	LANGDEF(VERILOG, "verilog", "Verilog", "Verilog file", SCLEX_VERILOG),
	LANGDEF(MATLAB, "matlab", "MATLAB", "MATrix LABoratory", SCLEX_MATLAB),
	LANGDEF(HASKELL, "haskell", "Haskell", "Haskell file", SCLEX_HASKELL),
	LANGDEF(INNO, "inno", "Inno", "Inno Setup script", SCLEX_INNOSETUP),
	LANGDEF(CMAKE, "cmake", "CMake", "CMake file", SCLEX_CMAKE),
	LANGDEF(YAML, "yaml", "YAML", "YAML Ain't Markup Language file", SCLEX_YAML),
	LANGDEF(COBOL, "cobol", "COBOL", "COmmon Business Oriented Language file", SCLEX_COBOL),
	LANGDEF(GUI4CLI, "gui4cli", "Gui4Cli", "Gui4Cli file", SCLEX_GUI4CLI),
	LANGDEF(D, "d", "D", "D source file", SCLEX_D),
	LANGDEF(POWERSHELL, "powershell", "PowerShell", "Windows PowerShell script file", SCLEX_POWERSHELL),
	LANGDEF(R, "r", "R", "R source file", SCLEX_R),
	LANGDEF(JSP, "jsp", "JSP", "Java Server Pages script file", SCLEX_HTML),
	LANGDEF(COFFEESCRIPT, "coffeescript", "CoffeeScript", "CoffeeScript file", SCLEX_COFFEESCRIPT),
	LANGDEF(RECIPE, "recipe", "Recipe", "HaikuPorts' recipe file", SCLEX_BASH),
	LANGDEF(RUST, "rust", "Rust", "Rust source file", SCLEX_RUST)
};


LanguageDefinition&
Languages::GetLanguage(LanguageType lang) {
	auto it = std::find_if(sLanguages.begin(), sLanguages.end(), [lang](const LanguageDefinition& a) {
		return a.fType == lang;
	});
	return (*it);
}


void
Languages::SortAlphabetically()
{
	auto compareFunc = [](const LanguageDefinition& a, const LanguageDefinition& b) {
		return a.fShortName.ICompare(b.fShortName) < 0;
	};
	std::sort(sLanguages.begin(), sLanguages.end(), compareFunc);
}


void
Languages::ApplyLanguage(Editor* editor, const char* path, const char* lang, XmlDocument* doc)
{
	XmlDocument* document;
	if(doc == nullptr)
		document = new XmlDocument(path);
	else
		document = doc;

	BString xpath("/Koder/Languages/Language[@name='%s']");
	xpath.ReplaceFirst("%s", lang);
	uint32 count;
	XmlNode* language = document->GetNodesByXPath(xpath, &count);
	BString derive = language[0].GetAttribute("derive");
	if(derive.IsEmpty() == false) {
		ApplyLanguage(editor, path, derive, document);
	}
	xpath.Append("/Keywords");
	XmlNode* nodes = document->GetNodesByXPath(xpath, &count);

	for(int i = 0; i < count; i++) {
		BString name = nodes[i].GetAttribute("name");
		BString content = nodes[i].GetContent();
		if(!name.IsEmpty()) {
			int list = atoi(name.String());
			editor->SendMessage(SCI_SETKEYWORDS, list, (sptr_t) content.String());
		}
	}

	delete []nodes;
	if(doc == nullptr)
		delete document;
}
