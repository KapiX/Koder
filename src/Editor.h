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

#ifndef EDITOR_H
#define EDITOR_H

#include <ScintillaView.h>
#include <SciLexer.h>


class Preferences;


enum {
	EDITOR_SAVEPOINT_LEFT		= 'svpl',
	EDITOR_SAVEPOINT_REACHED	= 'svpr'
};


class Editor : public BScintillaView {
public:
	enum Margin {
		NUMBER 		= 0,
		FOLD
	};

			Editor();

	void	NotificationReceived(SCNotification* notification);

	void	SetPreferences(Preferences* preferences) { fPreferences = preferences; }

private:
	void	_MaintainIndentation(char ch);
	void	_UpdateLineNumberWidth();

	void				_SetLineIndentation(int line, int indent);
	Sci_CharacterRange	_GetSelection();
	void				_SetSelection(int anchor, int currentPos);

	Preferences*		fPreferences;
};

#endif // EDITOR_H