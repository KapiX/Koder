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

#ifndef EDITORWINDOW_H
#define EDITORWINDOW_H

#include <MimeType.h>
#include <String.h>
#include <Window.h>

#include "Languages.h"


struct entry_ref;
class BFilePanel;
class BMenu;
class BMenuBar;
class BPath;
class Editor;
class GoToLineWindow;
class Preferences;
class Styler;


const BString gAppName = "Koder";
const BString gAppMime = "application/x-vnd.KapiX-Koder";


enum {
	MAINMENU_FILE_NEW			= 'mnew',
	MAINMENU_FILE_OPEN			= 'mopn',
	MAINMENU_FILE_SAVE			= 'msav',
	MAINMENU_FILE_SAVEAS		= 'msva',
	MAINMENU_FILE_QUIT			= 'mqut',

	MAINMENU_EDIT_FILE_PREFERENCES	= 'mefp',
	MAINMENU_EDIT_APP_PREFERENCES	= 'meap',
	
	MAINMENU_VIEW_SPECIAL_WHITESPACE	= 'vsws',
	MAINMENU_VIEW_SPECIAL_EOL			= 'vseo',

	MAINMENU_SEARCH_GOTOLINE	= 'msgl',

	MAINMENU_VIEW_LINEHIGHLIGHT	= 'mlhl',
	MAINMENU_VIEW_LINENUMBERS	= 'mvln',

	MAINMENU_LANGUAGE			= 'ml00',

	FILE_OPEN					= 'flop',
	FILE_SAVE					= 'flsv',

	WINDOW_NEW					= 'ewnw',
	WINDOW_CLOSE				= 'ewcl',
};

class EditorWindow : public BWindow {
public:
				EditorWindow();

	void		New();
	void		OpenFile(entry_ref* entry);
	void		RefreshTitle();
	void		SaveFile(BPath* path);

	bool		QuitRequested();
	void		MessageReceived(BMessage* message);

	static void	SetPreferences(Preferences* preferences) { fPreferences = preferences; }
	static void	SetStyler(Styler* styler) { fStyler = styler; }
private:
	BMenuBar*	fMainMenu;
	BPath*		fOpenedFilePath;
	BMimeType	fOpenedFileMimeType;
	Editor*		fEditor;
	BFilePanel*	fOpenPanel;
	BFilePanel*	fSavePanel;

	GoToLineWindow*	fGoToLineWindow;

	static Preferences*	fPreferences;
	static Styler*		fStyler;

	void _PopulateLanguageMenu(BMenu* languageMenu);
	void _SetLanguage(LanguageType lang);
};

#endif // EDITORWINDOW_H
