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
								Preferences(const char* path);
								~Preferences();
								
	void						Load();
	void						Save();
	
	BPath						fSettingsPath;

	// file specific
	uint8						fTabWidth;
	bool						fTabsToSpaces;
	// broadcast to all editors
	bool						fLineHighlighting;
	bool						fLineNumbers;
	uint8						fIndentationGuides;
	BString						fStyleFile;
	
private:
	BString						fPath;
};

#endif // PREFERENCES_H
