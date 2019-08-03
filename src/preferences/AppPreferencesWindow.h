/*
 * Copyright 2014-2019 Kacper Kasper <kacperkasper@gmail.com>
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
class BSpinner;
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
		TOOLBAR_ICON_SIZE		= 'tlis',
		FULL_PATH_IN_TITLE		= 'fpit',
		TABS_TO_SPACES			= 'ttsp',
		TAB_WIDTH				= 'tbwd',
		LINE_HIGHLIGHTING		= 'lhlt',
		LINE_HIGHLIGHTING_BG	= 'lhlb',
		LINE_HIGHLIGHTING_FRAME	= 'lhlf',

		LINE_NUMBERS			= 'lnum',
		FOLD_MARGIN				= 'fldm',
		BOOKMARK_MARGIN			= 'bkmm',

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
		APPEND_NL_AT_THE_END	= 'apae',
		ALWAYS_OPEN_IN_NEW_WINDOW='aonw',
		USE_CUSTOM_FONT			= 'ucfn',
		FONT_CHANGED			= 'fnch',
		FONT_SIZE_CHANGED		= 'fsch',

		REVERT					= 'rvrt'
	};

	void			_InitInterface();
	void			_SyncPreferences(Preferences* preferences);

	void			_PreferencesModified();
	void			_SetLineLimitBoxEnabled(bool enabled);
	void			_SetLineHighlightingBoxEnabled(bool enabled);
	void			_SetIndentGuidesBoxEnabled(bool enabled);
	void			_SetFontBoxEnabled(bool enabled);
	void			_SetToolbarBoxEnabled(bool enabled);
	void			_PopulateStylesMenu();
	void			_UpdateFontMenu();

	Preferences*	fStartPreferences;
	Preferences*	fPreferences;

	BBox*			fVisualBox;
	BBox*			fBehaviorBox;
	BBox*			fIndentationBox;
	BBox*			fTrailingWSBox;

	BCheckBox*		fCompactLangMenuCB;
	BCheckBox*		fFullPathInTitleCB;
	BCheckBox*		fTabsToSpacesCB;
	BTextControl*	fTabWidthTC;
	BBox*			fLineHighlightingBox;
	BCheckBox*		fLineHighlightingCB;
	BRadioButton*	fLineHighlightingBackgroundRadio;
	BRadioButton*	fLineHighlightingFrameRadio;

	BBox*			fToolbarBox;
	BCheckBox*		fToolbarCB;
	BPopUpMenu*		fToolbarIconSizeMenu;
	BMenuField*		fToolbarIconSizeMF;

	BBox*			fMarginsBox;
	BCheckBox*		fLineNumbersCB;
	BCheckBox*		fFoldMarginCB;
	BCheckBox*		fBookmarkMarginCB;

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
	BCheckBox*		fAlwaysOpenInNewWindowCB;
	BCheckBox*		fAppendNLAtTheEndCB;

	BBox*			fFontBox;
	BCheckBox*		fUseCustomFontCB;
	BMenuField*		fFontMF;
	BSpinner*		fFontSizeSpinner;
	BPopUpMenu*		fFontMenu;

	BButton*		fRevertButton;
};


#endif // APPPREFERENCESWINDOW_H
