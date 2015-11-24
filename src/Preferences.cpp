/*
 * Koder is a code editor for Haiku based on Scintilla.
 *
 * Copyright (C) 2014-2015 Kacper Kasper <kacperkasper@gmail.com>
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

#include "Preferences.h"

#include <Alert.h>
#include <File.h>

#include "Languages.h"


// TODO: B_TRANSLATE


void
Preferences::Load(const char* filename)
{
	BFile *file = new BFile(filename, B_READ_ONLY);
	status_t result = file->InitCheck();
	switch (result) {
		case B_BAD_VALUE:
		{
			BAlert* alert = new BAlert("Configuration file", 
				"Couldn't open configuration file because path is not specified. It usually "
				"means that programmer made a mistake. There is nothing you can do about it. "
				"Your personal settings will not be loaded. Sorry.", "Continue", NULL, NULL,
				B_WIDTH_AS_USUAL, B_WARNING_ALERT);
			alert->Go();
			
			return;
		}
		case B_PERMISSION_DENIED:
		{
			BAlert* alert = new BAlert("Configuration file",
				"Couldn't open configuration file because permission was denied. It usually "
				"means that you don't have read permissions to your settings directory. "
				"If you want to have your personal settings loaded, check your OS documentation "
				"to find out which directory it is and try changing its permissions.", "Continue",
				NULL, NULL, B_WIDTH_AS_USUAL, B_WARNING_ALERT);
			alert->Go();
			
			return;
		}
		case B_NO_MEMORY:
		{
			BAlert* alert = new BAlert("Configuration file",
				"There is not enough memory available on your system to load configuration "
				"file. If you want to have your personal settings loaded, try closing few "
				"applications and restart Koder.", "Continue", NULL, NULL,
				B_WIDTH_AS_USUAL, B_WARNING_ALERT);
			alert->Go();
			
			return;
		}
		default:
			break;
	}

	BMessage storage;
	storage.Unflatten(file);
	fTabWidth = storage.GetUInt8("tabWidth", 4);
	fTabsToSpaces = storage.GetBool("tabsToSpaces", false);
	fLineHighlighting = storage.GetBool("lineHighlighting", true);
	fLineNumbers = storage.GetBool("lineNumbers", true);
	fIndentGuidesShow = storage.GetBool("indentGuidesShow", true);
	fIndentGuidesMode = storage.GetUInt8("indentGuidesMode", 1); // SC_IV_REAL
	fWhiteSpaceVisible = storage.GetBool("whiteSpaceVisible", false);
	fEOLVisible = storage.GetBool("EOLVisible", false);
	fLineLimitShow = storage.GetBool("lineLimitShow", false);
	fLineLimitMode = storage.GetUInt8("lineLimitMode", 1); // EDGE_LINE
	fLineLimitColumn = storage.GetUInt32("lineLimitColumn", 80);
	fBracesHighlighting = storage.GetBool("bracesHighlighting", true);
	fFullPathInTitle = storage.GetBool("fullPathInTitle", true);
	fCompactLangMenu = storage.GetBool("compactLangMenu", true);
	fStyleFile = storage.GetString("styleFile", "default.xml");
	if(storage.FindMessage("extensions", &fExtensions) != B_OK) {
		fExtensions.AddUInt32("as", LANGUAGE_FLASH);
		fExtensions.AddUInt32("mx", LANGUAGE_FLASH);
		fExtensions.AddUInt32("ada", LANGUAGE_ADA);
		fExtensions.AddUInt32("ads", LANGUAGE_ADA);
		fExtensions.AddUInt32("adb", LANGUAGE_ADA);
		fExtensions.AddUInt32("asm", LANGUAGE_ASM);
		fExtensions.AddUInt32("S", LANGUAGE_ASM);
		fExtensions.AddUInt32("asp", LANGUAGE_ASP);
		fExtensions.AddUInt32("au3", LANGUAGE_AU3);
		fExtensions.AddUInt32("bash", LANGUAGE_BASH);
		fExtensions.AddUInt32("sh", LANGUAGE_BASH);
		fExtensions.AddUInt32("bsh", LANGUAGE_BASH);
		fExtensions.AddUInt32("csh", LANGUAGE_BASH);
		fExtensions.AddUInt32("bat", LANGUAGE_BATCH);
		fExtensions.AddUInt32("cmd", LANGUAGE_BATCH);
		fExtensions.AddUInt32("nt", LANGUAGE_BATCH);
		fExtensions.AddUInt32("c", LANGUAGE_C);
		fExtensions.AddUInt32("ml", LANGUAGE_CAML);
		fExtensions.AddUInt32("mli", LANGUAGE_CAML);
		fExtensions.AddUInt32("sml", LANGUAGE_CAML);
		fExtensions.AddUInt32("thy", LANGUAGE_CAML);
		fExtensions.AddUInt32("cmake", LANGUAGE_CMAKE);
		fExtensions.AddUInt32("cbl", LANGUAGE_COBOL);
		fExtensions.AddUInt32("cbd", LANGUAGE_COBOL);
		fExtensions.AddUInt32("cdb", LANGUAGE_COBOL);
		fExtensions.AddUInt32("cdc", LANGUAGE_COBOL);
		fExtensions.AddUInt32("cob", LANGUAGE_COBOL);
		fExtensions.AddUInt32("coffee", LANGUAGE_COFFEESCRIPT);
		fExtensions.AddUInt32("h", LANGUAGE_CPP);
		fExtensions.AddUInt32("cc", LANGUAGE_CPP);
		fExtensions.AddUInt32("cpp", LANGUAGE_CPP);
		fExtensions.AddUInt32("cxx", LANGUAGE_CPP);
		fExtensions.AddUInt32("hpp", LANGUAGE_CPP);
		fExtensions.AddUInt32("hxx", LANGUAGE_CPP);
		fExtensions.AddUInt32("cs", LANGUAGE_CS);
		fExtensions.AddUInt32("d", LANGUAGE_D);
		fExtensions.AddUInt32("diff", LANGUAGE_DIFF);
		fExtensions.AddUInt32("patch", LANGUAGE_DIFF);
		fExtensions.AddUInt32("f", LANGUAGE_FORTRAN);
		fExtensions.AddUInt32("for", LANGUAGE_FORTRAN);
		fExtensions.AddUInt32("f90", LANGUAGE_FORTRAN);
		fExtensions.AddUInt32("f95", LANGUAGE_FORTRAN);
		fExtensions.AddUInt32("f2k", LANGUAGE_FORTRAN);
		fExtensions.AddUInt32("hs", LANGUAGE_HASKELL);
		fExtensions.AddUInt32("lhs", LANGUAGE_HASKELL);
		fExtensions.AddUInt32("las", LANGUAGE_HASKELL);
		fExtensions.AddUInt32("html", LANGUAGE_HTML);
		fExtensions.AddUInt32("htm", LANGUAGE_HTML);
		fExtensions.AddUInt32("shtml", LANGUAGE_HTML);
		fExtensions.AddUInt32("shtm", LANGUAGE_HTML);
		fExtensions.AddUInt32("xhtml", LANGUAGE_HTML);
		fExtensions.AddUInt32("xht", LANGUAGE_HTML);
		fExtensions.AddUInt32("hta", LANGUAGE_HTML);
		fExtensions.AddUInt32("ini", LANGUAGE_INI);
		fExtensions.AddUInt32("inf", LANGUAGE_INI);
		fExtensions.AddUInt32("reg", LANGUAGE_INI);
		fExtensions.AddUInt32("url", LANGUAGE_INI);
		fExtensions.AddUInt32("iss", LANGUAGE_INNO);
		fExtensions.AddUInt32("java", LANGUAGE_JAVA);
		fExtensions.AddUInt32("js", LANGUAGE_JS);
		fExtensions.AddUInt32("jsm", LANGUAGE_JS);
		fExtensions.AddUInt32("json", LANGUAGE_JS);
		fExtensions.AddUInt32("jsp", LANGUAGE_JSP);
		fExtensions.AddUInt32("kix", LANGUAGE_KIX);
		fExtensions.AddUInt32("lsp", LANGUAGE_LISP);
		fExtensions.AddUInt32("lisp", LANGUAGE_LISP);
		fExtensions.AddUInt32("lua", LANGUAGE_LUA);
		fExtensions.AddUInt32("mak", LANGUAGE_MAKEFILE);
		fExtensions.AddUInt32("makefile", LANGUAGE_MAKEFILE);
		fExtensions.AddUInt32("Makefile", LANGUAGE_MAKEFILE);
		fExtensions.AddUInt32("m", LANGUAGE_MATLAB);
		fExtensions.AddUInt32("nfo", LANGUAGE_ASCII);
		fExtensions.AddUInt32("nsi", LANGUAGE_NSIS);
		fExtensions.AddUInt32("nsh", LANGUAGE_NSIS);
		fExtensions.AddUInt32("pas", LANGUAGE_PASCAL);
		fExtensions.AddUInt32("inc", LANGUAGE_PASCAL);
		fExtensions.AddUInt32("pl", LANGUAGE_PERL);
		fExtensions.AddUInt32("pm", LANGUAGE_PERL);
		fExtensions.AddUInt32("plx", LANGUAGE_PERL);
		fExtensions.AddUInt32("php", LANGUAGE_PHP);
		fExtensions.AddUInt32("php3", LANGUAGE_PHP);
		fExtensions.AddUInt32("php4", LANGUAGE_PHP);
		fExtensions.AddUInt32("php5", LANGUAGE_PHP);
		fExtensions.AddUInt32("phps", LANGUAGE_PHP);
		fExtensions.AddUInt32("phpt", LANGUAGE_PHP);
		fExtensions.AddUInt32("phtml", LANGUAGE_PHP);
		fExtensions.AddUInt32("ps", LANGUAGE_POSTSCRIPT);
		fExtensions.AddUInt32("ps1", LANGUAGE_POWERSHELL);
		fExtensions.AddUInt32("psm1", LANGUAGE_POWERSHELL);
		fExtensions.AddUInt32("properties", LANGUAGE_PROPS);
		fExtensions.AddUInt32("py", LANGUAGE_PYTHON);
		fExtensions.AddUInt32("pyw", LANGUAGE_PYTHON);
		fExtensions.AddUInt32("r", LANGUAGE_R);
		fExtensions.AddUInt32("s", LANGUAGE_R);
		fExtensions.AddUInt32("splus", LANGUAGE_R);
		fExtensions.AddUInt32("rc", LANGUAGE_RC);
		fExtensions.AddUInt32("recipe", LANGUAGE_RECIPE);
		fExtensions.AddUInt32("rb", LANGUAGE_RUBY);
		fExtensions.AddUInt32("rbw", LANGUAGE_RUBY);
		fExtensions.AddUInt32("scm", LANGUAGE_SCHEME);
		fExtensions.AddUInt32("smd", LANGUAGE_SCHEME);
		fExtensions.AddUInt32("ss", LANGUAGE_SCHEME);
		fExtensions.AddUInt32("st", LANGUAGE_SMALLTALK);
		fExtensions.AddUInt32("sql", LANGUAGE_SQL);
		fExtensions.AddUInt32("tcl", LANGUAGE_TCL);
		fExtensions.AddUInt32("tex", LANGUAGE_TEX);
		fExtensions.AddUInt32("vb", LANGUAGE_VB);
		fExtensions.AddUInt32("vbs", LANGUAGE_VB);
		fExtensions.AddUInt32("v", LANGUAGE_VERILOG);
		fExtensions.AddUInt32("sv", LANGUAGE_VERILOG);
		fExtensions.AddUInt32("vh", LANGUAGE_VERILOG);
		fExtensions.AddUInt32("svh", LANGUAGE_VERILOG);
		fExtensions.AddUInt32("vhd", LANGUAGE_VHDL);
		fExtensions.AddUInt32("vhdl", LANGUAGE_VHDL);
		fExtensions.AddUInt32("xml", LANGUAGE_XML);
		fExtensions.AddUInt32("xaml", LANGUAGE_XML);
		fExtensions.AddUInt32("xsl", LANGUAGE_XML);
		fExtensions.AddUInt32("xslt", LANGUAGE_XML);
		fExtensions.AddUInt32("xsd", LANGUAGE_XML);
		fExtensions.AddUInt32("xul", LANGUAGE_XML);
		fExtensions.AddUInt32("kml", LANGUAGE_XML);
		fExtensions.AddUInt32("svg", LANGUAGE_XML);
		fExtensions.AddUInt32("mxml", LANGUAGE_XML);
		fExtensions.AddUInt32("xsml", LANGUAGE_XML);
		fExtensions.AddUInt32("wsdl", LANGUAGE_XML);
		fExtensions.AddUInt32("xlf", LANGUAGE_XML);
		fExtensions.AddUInt32("xliff", LANGUAGE_XML);
		fExtensions.AddUInt32("xbl", LANGUAGE_XML);
		fExtensions.AddUInt32("sxbl", LANGUAGE_XML);
		fExtensions.AddUInt32("pimx", LANGUAGE_XML);
		fExtensions.AddUInt32("rdlc", LANGUAGE_XML);
		fExtensions.AddUInt32("swidtag", LANGUAGE_XML);
		fExtensions.AddUInt32("gml", LANGUAGE_XML);
		fExtensions.AddUInt32("gtt", LANGUAGE_XML);
		fExtensions.AddUInt32("gpx", LANGUAGE_XML);
		fExtensions.AddUInt32("yml", LANGUAGE_YAML);
		fExtensions.AddUInt32("yaml", LANGUAGE_YAML);
	}
	fWindowRect = storage.GetRect("windowRect", BRect(50, 50, 450, 450));

	delete file;
}


void
Preferences::Save(const char* filename)
{
	BFile* file = new BFile(filename, B_WRITE_ONLY | B_CREATE_FILE | B_ERASE_FILE);
	status_t result = file->InitCheck();
	switch (result) {
		case B_BAD_VALUE:
		{
			BAlert* alert = new BAlert("Configuration file", 
				"Couldn't open configuration file because path is not specified. It usually "
				"means that programmer made a mistake. There is nothing you can do about it. "
				"Your personal settings will not be saved. Sorry.", "Continue", NULL, NULL,
				B_WIDTH_AS_USUAL, B_WARNING_ALERT);
			alert->Go();
			
			return;
		}
		case B_PERMISSION_DENIED:
		{
			BAlert* alert = new BAlert("Configuration file",
				"Couldn't open configuration file because permission was denied. It usually "
				"means that you don't have write permissions to your settings directory. "
				"If you want to have your personal settings loaded, check your OS documentation "
				"to find out which directory it is and try changing its permissions.", "Continue",
				NULL, NULL, B_WIDTH_AS_USUAL, B_WARNING_ALERT);
			alert->Go();
			
			return;
		}
		case B_NO_MEMORY:
		{
			BAlert* alert = new BAlert("Configuration file",
				"There is not enough memory available on your system to save configuration "
				"file. If you want to have your personal settings saved, try closing few "
				"applications and try again.", "Continue", NULL, NULL,
				B_WIDTH_AS_USUAL, B_WARNING_ALERT);
			alert->Go();
			
			return;
		}
		default:
			break;
	}

	BMessage storage;
	storage.AddInt8("tabWidth", fTabWidth);
	storage.AddBool("tabsToSpaces", fTabsToSpaces);
	storage.AddBool("lineHighlighting", fLineHighlighting);
	storage.AddBool("lineNumbers", fLineNumbers);
	storage.AddBool("whiteSpaceVisible", fWhiteSpaceVisible);
	storage.AddBool("EOLVisible", fEOLVisible);
	storage.AddBool("indentGuidesShow", fIndentGuidesShow);
	storage.AddInt8("indentGuidesMode", fIndentGuidesMode);
	storage.AddBool("lineLimitShow", fLineLimitShow);
	storage.AddInt8("lineLimitMode", fLineLimitMode);
	storage.AddInt32("lineLimitColumn", fLineLimitColumn);
	storage.AddBool("bracesHighlighting", fBracesHighlighting);
	storage.AddBool("fullPathInTitle", fFullPathInTitle);
	storage.AddBool("compactLangMenu", fCompactLangMenu);
	storage.AddString("styleFile", fStyleFile);
	storage.AddMessage("extensions", &fExtensions);
	storage.AddRect("windowRect", fWindowRect);
	storage.Flatten(file);

	delete file;
}


Preferences&
Preferences::operator =(Preferences p)
{
	fSettingsPath = p.fSettingsPath;
	fTabWidth = p.fTabWidth;
	fTabsToSpaces = p.fTabsToSpaces;
	fExtensions = p.fExtensions;
	fLineHighlighting = p.fLineHighlighting;
	fLineNumbers = p.fLineNumbers;
	fEOLVisible = p.fEOLVisible;
	fWhiteSpaceVisible = p.fWhiteSpaceVisible;
	fIndentGuidesShow = p.fIndentGuidesShow;
	fIndentGuidesMode = p.fIndentGuidesMode;
	fLineLimitShow = p.fLineLimitShow;
	fLineLimitMode = p.fLineLimitMode;
	fLineLimitColumn = p.fLineLimitColumn;
	fBracesHighlighting = p.fBracesHighlighting;
	fFullPathInTitle = p.fFullPathInTitle;
	fCompactLangMenu = p.fCompactLangMenu;
	fStyleFile = p.fStyleFile;
	fWindowRect = p.fWindowRect;
}
