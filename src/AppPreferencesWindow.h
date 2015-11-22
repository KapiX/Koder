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

#ifndef APPPREFERENCESWINDOW_H
#define APPPREFERENCESWINDOW_H


#include <Window.h>


class BBox;
class BButton;
class BCheckBox;
class BMessage;
class BRadioButton;
class BStringView;
class BTextControl;
class BView;
class Preferences;

const uint32 APP_PREFERENCES_CHANGED = 'apch';

class AppPreferencesWindow : public BWindow {
public:
					AppPreferencesWindow(Preferences* preferences);
					~AppPreferencesWindow();

	void			MessageReceived(BMessage* message);
private:
	enum Actions {
		COMPACT_LANG_MENU		= 'clnm',
		FULL_PATH_IN_TITLE		= 'fpit',
		TABS_TO_SPACES			= 'ttsp',
		TAB_WIDTH				= 'tbwd',
		LINE_HIGHLIGHTING		= 'lhlt',
		LINE_NUMBERS			= 'lnum',

		LINELIMIT_COLUMN		= 'llcl',
		LINELIMIT_SHOW			= 'llsh',
		LINELIMIT_BACKGROUND	= 'llbk',
		LINELIMIT_LINE			= 'llln',

		INDENTGUIDES_SHOW		= 'igsh',
		INDENTGUIDES_REAL		= 'igrl',
		INDENTGUIDES_FORWARD	= 'igfw',
		INDENTGUIDES_BOTH		= 'igbo',

		BRACES_HIGHLIGHTING		= 'bhlt',

		APPLY					= 'appl',
		REVERT					= 'rvrt'
	};
	void			_InitInterface();
	void			_SyncPreferences(Preferences* preferences);

	void			_PreferencesModified();
	void			_SetLineLimitBoxEnabled(bool enabled);
	void			_SetIndentGuidesBoxEnabled(bool enabled);

	Preferences*	fStartPreferences;
	Preferences*	fCurrentPreferences;
	Preferences*	fTempPreferences;

	BBox*			fEditorBox;
	BCheckBox*		fCompactLangMenuCB;
	BCheckBox*		fFullPathInTitleCB;
	BCheckBox*		fTabsToSpacesCB;
	BTextControl*	fTabWidthTC;
	BStringView*	fTabWidthText;
	BCheckBox*		fLineHighlightingCB;
	BCheckBox*		fLineNumbersCB;

	BView*			fLineLimitHeaderView;
	BCheckBox*		fLineLimitShowCB;
	BTextControl*	fLineLimitColumnTC;
	BStringView*	fLineLimitText;

	BBox*			fLineLimitBox;

	BRadioButton*	fLineLimitBackgroundRadio;
	BRadioButton*	fLineLimitLineRadio;

	BBox*			fIndentGuidesBox;
	BCheckBox*		fIndentGuidesShowCB;
	BRadioButton*	fIndentGuidesRealRadio;
	BRadioButton*	fIndentGuidesLookForwardRadio;
	BRadioButton*	fIndentGuidesLookBothRadio;

	BCheckBox*		fBracesHighlightingCB;

	BButton*		fApplyButton;
	BButton*		fRevertButton;
};


#endif // APPPREFERENCESWINDOW_H
