/*
 * Copyright 2014-2017 Kacper Kasper <kacperkasper@gmail.com>
 * All rights reserved. Distributed under the terms of the MIT license.
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
const uint32 APP_PREFERENCES_QUITTING = 'APQU';


class AppPreferencesWindow : public BWindow {
public:
					AppPreferencesWindow(Preferences* preferences);
					~AppPreferencesWindow();

	void			MessageReceived(BMessage* message);
	void			Quit();

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
	BCheckBox*		fLineHighlightingCB;
	BCheckBox*		fLineNumbersCB;

	BView*			fLineLimitHeaderView;
	BCheckBox*		fLineLimitShowCB;
	BTextControl*	fLineLimitColumnTC;

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
