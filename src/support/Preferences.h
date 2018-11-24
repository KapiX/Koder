/*
 * Copyright 2014-2018 Kacper Kasper <kacperkasper@gmail.com>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#ifndef PREFERENCES_H
#define PREFERENCES_H


#include <memory>
#include <string>

#include <Message.h>
#include <Path.h>


class BFile;


class Preferences {
private:
	std::shared_ptr<BFile>	_OpenFile(const char* filename, uint32 openMode);

public:
	void			Load(const char* filename);
	void			Save(const char* filename);

	Preferences&	operator =(Preferences p);

	BPath			fSettingsPath;

	// file specific
	uint8			fTabWidth;
	bool			fTabsToSpaces;
	// broadcast to all editors
	bool			fLineHighlighting;
	uint8			fLineHighlightingMode;
	bool			fLineNumbers;
	bool			fFoldMargin;
	bool			fBookmarkMargin;
	bool			fEOLVisible;
	bool			fWhiteSpaceVisible;
	bool			fIndentGuidesShow;
	uint8			fIndentGuidesMode;
	bool			fLineLimitShow;
	uint8			fLineLimitMode;
	uint32			fLineLimitColumn;
	bool			fWrapLines;
	bool			fBracesHighlighting;
	bool			fFullPathInTitle;
	bool			fCompactLangMenu;
	bool			fToolbar;
	bool			fOpenWindowsInStack;
	bool			fHighlightTrailingWhitespace;
	bool			fTrimTrailingWhitespaceOnSave;
	bool			fAppendNLAtTheEndIfNotPresent;
	bool			fUseEditorconfig;
	bool			fAlwaysOpenInNewWindow;
	bool			fUseCustomFont;
	std::string		fFontFamily;
	uint8			fFontSize;
	std::string		fStyle;
	// application state
	BRect			fWindowRect;
	BMessage		fFindWindowState;
};


#endif // PREFERENCES_H
