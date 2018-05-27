/*
 * Copyright 2014-2017 Kacper Kasper <kacperkasper@gmail.com>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#ifndef PREFERENCES_H
#define PREFERENCES_H


#include <string>

#include <File.h>
#include <Message.h>
#include <Path.h>


class Preferences {
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
	bool			fLineNumbers;
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
	std::string		fStyle;
	// application state
	BRect			fWindowRect;
	BMessage		fFindWindowState;
};


#endif // PREFERENCES_H
