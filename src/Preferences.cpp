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


namespace {
	const char* kErrorOK = B_TRANSLATE_MARK("OK");
	const char* kErrorTitle = B_TRANSLATE_MARK("Editor settings");
	const char* kErrorUnknown = B_TRANSLATE_MARK("Unknown error.");
	const char* kErrorBadValue[] = {
		B_TRANSLATE_MARK("Something wrong has happened while opening the "
			"configuration file. Your personal settings will not be %s%."),
		B_TRANSLATE_MARK_COMMENT("loaded", "settings will not be _"),
		B_TRANSLATE_MARK_COMMENT("saved", "settings will not be _")
	};
	const char* kErrorAccessDenied[] = {
		B_TRANSLATE_MARK("Access was denied while opening the configuration "
			"file. Make sure you have %s% permission for your settings "
			"directory."),
		B_TRANSLATE_MARK_COMMENT("read", "Make sure you have _ permission for"),
		B_TRANSLATE_MARK_COMMENT("write", "Make sure you have _ permission for")
	};
	const char* kErrorNoMemory[] = {
		B_TRANSLATE_MARK("There is not enough memory available to %s% the "
			"configuration file. Try closing a few applications and restart "
			"the editor."),
		B_TRANSLATE_MARK_COMMENT("load", "to _ the configuration"),
		B_TRANSLATE_MARK_COMMENT("save", "to _ the configuration")
	};
}


std::shared_ptr<BFile>
Preferences::_OpenFile(const char* filename, uint32 openMode)
{
	if (openMode == B_WRITE_ONLY)
		openMode |= B_CREATE_FILE | B_ERASE_FILE;
	const int index = (openMode == B_READ_ONLY) ? 1 : 2;
	auto file = std::make_shared<BFile>(filename, openMode);
	status_t result = file->InitCheck();
	if (result != B_OK) {
		BString error = B_TRANSLATE(kErrorUnknown);
		BString ok = B_TRANSLATE(kErrorOK);
		switch (result) {
			case B_BAD_VALUE:
				error = B_TRANSLATE(kErrorBadValue[0]);
				error.ReplaceFirst("%s%", B_TRANSLATE(kErrorBadValue[index]));
				break;
			case B_PERMISSION_DENIED:
				error = B_TRANSLATE(kErrorAccessDenied[0]);
				error.ReplaceFirst("%s%", B_TRANSLATE(kErrorAccessDenied[index]));
				break;
			case B_NO_MEMORY:
				error = B_TRANSLATE(kErrorNoMemory[0]);
				error.ReplaceFirst("%s%", B_TRANSLATE(kErrorNoMemory[index]));
				break;
		}
		BAlert* alert = new BAlert(B_TRANSLATE(kErrorTitle), error, ok,
			nullptr, nullptr, B_WIDTH_AS_USUAL, B_WARNING_ALERT);
		alert->Go();
		return std::shared_ptr<BFile>();
	}
	return file;
}


void
Preferences::Load(const char* filename)
{
	auto file = _OpenFile(filename, B_READ_ONLY);
	BMessage storage;
	if (file)
		storage.Unflatten(file.get());
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
}


void
Preferences::Save(const char* filename)
{
	auto file = _OpenFile(filename, B_WRITE_ONLY);
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
	if (file)
		storage.Flatten(file.get());
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
