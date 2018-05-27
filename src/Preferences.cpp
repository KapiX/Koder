/*
 * Copyright 2014-2017 Kacper Kasper <kacperkasper@gmail.com>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include "Preferences.h"

#include <Alert.h>
#include <Catalog.h>
#include <File.h>

#include "Languages.h"


#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "Preferences"


void
Preferences::Load(const char* filename)
{
	BFile *file = new BFile(filename, B_READ_ONLY);
	status_t result = file->InitCheck();
	switch (result) {
		case B_BAD_VALUE:
		{
			BAlert* alert = new BAlert(B_TRANSLATE("Configuration file"),
				B_TRANSLATE("Couldn't open configuration file because the path is not specified. It usually "
				"means that the programmer made a mistake. There is nothing you can do about it. "
				"Your personal settings will not be loaded. Sorry."), B_TRANSLATE("OK"), nullptr, nullptr,
				B_WIDTH_AS_USUAL, B_WARNING_ALERT);
			alert->Go();
			break;
		}
		case B_PERMISSION_DENIED:
		{
			BAlert* alert = new BAlert(B_TRANSLATE("Configuration file"),
				B_TRANSLATE("Couldn't open configuration file because permission was denied. It usually "
				"means that you don't have read permissions to your settings directory. "
				"If you want to have your personal settings loaded, check your OS documentation "
				"to find out which directory it is and try changing its permissions."), B_TRANSLATE("OK"),
				nullptr, nullptr, B_WIDTH_AS_USUAL, B_WARNING_ALERT);
			alert->Go();
			break;
		}
		case B_NO_MEMORY:
		{
			BAlert* alert = new BAlert(B_TRANSLATE("Configuration file"),
				B_TRANSLATE("There is not enough memory available on your system to load the configuration "
				"file. If you want to have your personal settings loaded, try closing few "
				"applications and restart Koder."), B_TRANSLATE("OK"), nullptr, nullptr,
				B_WIDTH_AS_USUAL, B_WARNING_ALERT);
			alert->Go();
			break;
		}
		default:
			break;
	}

	BMessage storage;
	if(result == B_OK)
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
	fWrapLines = storage.GetBool("wrapLines", false);
	fBracesHighlighting = storage.GetBool("bracesHighlighting", true);
	fFullPathInTitle = storage.GetBool("fullPathInTitle", true);
	fCompactLangMenu = storage.GetBool("compactLangMenu", true);
	fToolbar = storage.GetBool("toolbar", true);
	fOpenWindowsInStack = storage.GetBool("openWindowsInStack", true);
	fHighlightTrailingWhitespace = storage.GetBool("highlightTrailingWhitespace", false);
	fTrimTrailingWhitespaceOnSave = storage.GetBool("trimTrailingWhitespaceOnSave", false);
	fStyle = storage.GetString("style", "default");
	fWindowRect = storage.GetRect("windowRect", BRect(50, 50, 450, 450));
	if(storage.FindMessage("findWindowState", &fFindWindowState) != B_OK)
		fFindWindowState = BMessage();

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
			BAlert* alert = new BAlert(B_TRANSLATE("Configuration file"),
				B_TRANSLATE("Couldn't open configuration file because the path is not specified. It usually "
				"means that the programmer made a mistake. There is nothing you can do about it. "
				"Your personal settings will not be saved. Sorry."), B_TRANSLATE("OK"), nullptr, nullptr,
				B_WIDTH_AS_USUAL, B_WARNING_ALERT);
			alert->Go();
			break;
		}
		case B_PERMISSION_DENIED:
		{
			BAlert* alert = new BAlert(B_TRANSLATE("Configuration file"),
				B_TRANSLATE("Couldn't open configuration file because permission was denied. It usually "
				"means that you don't have write permissions to your settings directory. "
				"If you want to have your personal settings loaded, check your OS documentation "
				"to find out which directory it is and try changing its permissions."), B_TRANSLATE("OK"),
				nullptr, nullptr, B_WIDTH_AS_USUAL, B_WARNING_ALERT);
			alert->Go();
			break;
		}
		case B_NO_MEMORY:
		{
			BAlert* alert = new BAlert(B_TRANSLATE("Configuration file"),
				B_TRANSLATE("There is not enough memory available on your system to save the configuration "
				"file. If you want to have your personal settings saved, try closing few "
				"applications and try again."), B_TRANSLATE("OK"), nullptr, nullptr,
				B_WIDTH_AS_USUAL, B_WARNING_ALERT);
			alert->Go();
			break;
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
	storage.AddUInt8("indentGuidesMode", fIndentGuidesMode);
	storage.AddBool("lineLimitShow", fLineLimitShow);
	storage.AddUInt8("lineLimitMode", fLineLimitMode);
	storage.AddUInt32("lineLimitColumn", fLineLimitColumn);
	storage.AddBool("wrapLines", fWrapLines);
	storage.AddBool("bracesHighlighting", fBracesHighlighting);
	storage.AddBool("fullPathInTitle", fFullPathInTitle);
	storage.AddBool("compactLangMenu", fCompactLangMenu);
	storage.AddBool("toolbar", fToolbar);
	storage.AddBool("openWindowsInStack", fOpenWindowsInStack);
	storage.AddBool("highlightTrailingWhitespace", fHighlightTrailingWhitespace);
	storage.AddBool("trimTrailingWhitespaceOnSave", fTrimTrailingWhitespaceOnSave);
	storage.AddString("style", fStyle.c_str());
	storage.AddRect("windowRect", fWindowRect);
	storage.AddMessage("findWindowState", &fFindWindowState);
	storage.Flatten(file);

	delete file;
}


Preferences&
Preferences::operator =(Preferences p)
{
	fSettingsPath = p.fSettingsPath;
	fTabWidth = p.fTabWidth;
	fTabsToSpaces = p.fTabsToSpaces;
	fLineHighlighting = p.fLineHighlighting;
	fLineNumbers = p.fLineNumbers;
	fEOLVisible = p.fEOLVisible;
	fWhiteSpaceVisible = p.fWhiteSpaceVisible;
	fIndentGuidesShow = p.fIndentGuidesShow;
	fIndentGuidesMode = p.fIndentGuidesMode;
	fLineLimitShow = p.fLineLimitShow;
	fLineLimitMode = p.fLineLimitMode;
	fLineLimitColumn = p.fLineLimitColumn;
	fWrapLines = p.fWrapLines;
	fBracesHighlighting = p.fBracesHighlighting;
	fFullPathInTitle = p.fFullPathInTitle;
	fCompactLangMenu = p.fCompactLangMenu;
	fToolbar = p.fToolbar;
	fOpenWindowsInStack = p.fOpenWindowsInStack;
	fHighlightTrailingWhitespace = p.fHighlightTrailingWhitespace;
	fTrimTrailingWhitespaceOnSave = p.fTrimTrailingWhitespaceOnSave;
	fStyle = p.fStyle;
	fWindowRect = p.fWindowRect;
	fFindWindowState = p.fFindWindowState;
}
