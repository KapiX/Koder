/*
 * Copyright 2016-2017 Kacper Kasper <kacperkasper@gmail.com>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#ifndef FINDWINDOW_H
#define FINDWINDOW_H


#include <string>

#include <Path.h>
#include <String.h>
#include <Window.h>


class BBox;
class BButton;
class BCheckBox;
class BMessage;
class BRadioButton;
class BStringView;
class FindScintillaView;


enum {
	FINDWINDOW_FIND			= 'fwfd',
	FINDWINDOW_REPLACE		= 'fwrp',
	FINDWINDOW_REPLACEFIND	= 'fwrf',
	FINDWINDOW_REPLACEALL	= 'fwra',
	FINDWINDOW_QUITTING		= 'FWQU'
};


class FindWindow : public BWindow {
public:
					FindWindow(BMessage *state, BPath settingsPath);
					~FindWindow();

	void			MessageReceived(BMessage* message);
	void			WindowActivated(bool active);
	void			Quit();

	void			SetFindText(const std::string text);

private:
	enum Actions {
		MATCH_CASE		= 'mtcs',
		MATCH_WORD		= 'mtwd',
		WRAP_AROUND		= 'wrar',
		BACKWARDS		= 'back',
		IN_SELECTION	= 'insl',
		REGEX			= 'rege'
	};
	enum HistoryRequests {
		GET_FIND_HISTORY		= 'fmru',
		GET_REPLACE_HISTORY		= 'rmru',
		CLEAR_FIND_HISTORY		= 'cfmr',
		CLEAR_REPLACE_HISTORY	= 'crmr',
		APPLY_FIND_ITEM			= 'afit',
		APPLY_REPLACE_ITEM		= 'arit'
	};
	void			_InitInterface();
	void			_LoadHistory();
	void			_SaveHistory();

	BStringView*	fFindString;
	FindScintillaView*	fFindTC;
	BStringView*	fReplaceString;
	FindScintillaView*	fReplaceTC;

	BButton*		fFindButton;
	BButton*		fReplaceButton;
	BButton*		fReplaceFindButton;
	BButton*		fReplaceAllButton;

	BCheckBox*		fMatchCaseCB;
	BCheckBox*		fMatchWordCB;
	BCheckBox*		fWrapAroundCB;
	BCheckBox*		fBackwardsCB;
	BCheckBox*		fInSelectionCB;
	BCheckBox*		fRegexCB;

	BPath			fSettingsPath;
	BMessage		fFindHistory;
	BMessage		fReplaceHistory;

	bool			fFlagsChanged;
	std::string		fOldFindText;
	std::string		fOldReplaceText;
};


#endif // FINDWINDOW_H
