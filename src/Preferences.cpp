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

#include "Languages.h"

// TODO: B_TRANSLATE

Preferences::Preferences(const char* path)
	:
	fPath(path)
{
}


Preferences::~Preferences()
{
	Save();
}


void
Preferences::Load()
{
	BFile *file = new BFile(fPath.String(), B_READ_ONLY);
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
	if(storage.FindInt8("tabWidth", (int8*) &fTabWidth) != B_OK) {
		fTabWidth = 4;
	}
	if(storage.FindBool("tabsToSpaces", &fTabsToSpaces) != B_OK) {
		fTabsToSpaces = false;
	}
	if(storage.FindBool("lineHighlighting", &fLineHighlighting) != B_OK) {
		fLineHighlighting = true;
	}
	if(storage.FindBool("lineNumbers", &fLineNumbers) != B_OK) {
		fLineNumbers = true;
	}
	if(storage.FindInt8("indentationGuides", (int8*) &fIndentationGuides) != B_OK) {
		fIndentationGuides = 1;
	}
	if(storage.FindBool("whiteSpaceVisible", &fWhiteSpaceVisible) != B_OK) {
		fWhiteSpaceVisible = false;
	}
	if(storage.FindBool("EOLVisible", &fEOLVisible) != B_OK) {
		fEOLVisible = false;
	}
	if(storage.FindString("styleFile", &fStyleFile) != B_OK) {
		fStyleFile = "default.xml";
	}
	if(storage.FindMessage("extensions", &fExtensions) != B_OK) {
		fExtensions.AddUInt32("c", LANGUAGE_C);
		fExtensions.AddUInt32("h", LANGUAGE_CPP);
		fExtensions.AddUInt32("cpp", LANGUAGE_CPP);
		fExtensions.AddUInt32("cxx", LANGUAGE_CPP);
		fExtensions.AddUInt32("hpp", LANGUAGE_CPP);
		fExtensions.AddUInt32("hxx", LANGUAGE_CPP);
		fExtensions.AddUInt32("makefile", LANGUAGE_MAKEFILE);
		fExtensions.AddUInt32("Makefile", LANGUAGE_MAKEFILE);
		fExtensions.AddUInt32("py", LANGUAGE_PYTHON);
	}
	if(storage.FindRect("windowRect", &fWindowRect) != B_OK) {
		fWindowRect.Set(50, 50, 450, 450);
	}
	
	delete file;
}


void
Preferences::Save()
{
	BFile* file = new BFile(fPath.String(), B_WRITE_ONLY | B_CREATE_FILE | B_ERASE_FILE);
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
	storage.AddInt8("indentationGuides", fIndentationGuides);
	storage.AddString("styleFile", fStyleFile);
	storage.AddMessage("extensions", &fExtensions);
	storage.AddRect("windowRect", fWindowRect);
	storage.Flatten(file);
	
	delete file;
}
