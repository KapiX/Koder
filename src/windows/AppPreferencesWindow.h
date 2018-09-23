/*
 * Copyright 2014-2018 Kacper Kasper <kacperkasper@gmail.com>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#ifndef APPPREFERENCESWINDOW_H
#define APPPREFERENCESWINDOW_H


#include <Window.h>


class BBox;
class BButton;
class BCheckBox;
class BPopUpMenu;
class BMenuField;
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
		TOOLBAR					= 'tlbr',
		FULL_PATH_IN_TITLE		= 'fpit',
		TABS_TO_SPACES			= 'ttsp',
		TAB_WIDTH				= 'tbwd',
		LINE_HIGHLIGHTING		= 'lhlt',
		LINE_HIGHLIGHTING_BG	= 'lhlb',
		LINE_HIGHLIGHTING_FRAME	= 'lhlf',
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

		EDITOR_STYLE			= 'styl',

		ATTACH_WINDOWS			= 'atwn',
		HIGHLIGHT_TRAILING_WS	= 'hltw',
		TRIM_TRAILING_WS_SAVE	= 'ttws',

		USE_EDITORCONFIG		= 'uecf',

		APPLY					= 'appl',
		REVERT					= 'rvrt'
	};

	void			_InitInterface();
	void			_SyncPreferences(Preferences* preferences);

	void			_PreferencesModified();
	void			_SetLineLimitBoxEnabled(bool enabled);
	void			_SetLineHighlightingBoxEnabled(bool enabled);
	void			_SetIndentGuidesBoxEnabled(bool enabled);
	void			_PopulateStylesMenu();

	Preferences*	fStartPreferences;
	Preferences*	fCurrentPreferences;
	Preferences*	fTempPreferences;

	BBox*			fVisualBox;
	BBox*			fBehaviorBox;
	BBox*			fIndentationBox;
	BBox*			fTrailingWSBox;

	BCheckBox*		fCompactLangMenuCB;
	BCheckBox*		fToolbarCB;
	BCheckBox*		fFullPathInTitleCB;
	BCheckBox*		fTabsToSpacesCB;
	BTextControl*	fTabWidthTC;
	BBox*			fLineHighlightingBox;
	BCheckBox*		fLineHighlightingCB;
	BRadioButton*	fLineHighlightingBackgroundRadio;
	BRadioButton*	fLineHighlightingFrameRadio;
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

	BPopUpMenu*		fEditorStyleMenu;
	BMenuField*		fEditorStyleMF;

	BCheckBox*		fAttachNewWindowsCB;
	BCheckBox*		fHighlightTrailingWSCB;
	BCheckBox*		fTrimTrailingWSOnSaveCB;

	BCheckBox*		fUseEditorconfigCB;

	BButton*		fApplyButton;
	BButton*		fRevertButton;
};


#endif // APPPREFERENCESWINDOW_H
