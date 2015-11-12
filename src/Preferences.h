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

#ifndef PREFERENCES_H
#define PREFERENCES_H

#include <File.h>
#include <Message.h>
#include <Path.h>
#include <String.h>


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
	BMessage		fExtensions;
	bool			fLineHighlighting;
	bool			fLineNumbers;
	bool			fEOLVisible;
	bool			fWhiteSpaceVisible;
	bool			fIndentGuidesShow;
	uint8			fIndentGuidesMode;
	bool			fLineLimitShow;
	uint8			fLineLimitMode;
	uint32			fLineLimitColumn;
	bool			fBracesHighlighting;
	BString			fStyleFile;
	BRect			fWindowRect;
};

#endif // PREFERENCES_H
