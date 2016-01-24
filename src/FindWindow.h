/*
 * Koder is a code editor for Haiku based on Scintilla.
 *
 * Copyright (C) 2016 Kacper Kasper <kacperkasper@gmail.com>
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

#ifndef FINDWINDOW_H
#define FINDWINDOW_H


#include <String.h>
#include <Window.h>


class BBox;
class BButton;
class BCheckBox;
class BMessage;
class BRadioButton;
class BStringView;
class BTextControl;


enum {
	FINDWINDOW_FIND			= 'fwfd',
	FINDWINDOW_REPLACE		= 'fwrp',
	FINDWINDOW_REPLACEFIND	= 'fwrf',
	FINDWINDOW_REPLACEALL	= 'fwra',
	FINDWINDOW_QUITTING		= 'FWQU'
};


class FindWindow : public BWindow {
public:
					FindWindow();
					~FindWindow();

	void			MessageReceived(BMessage* message);
	void			WindowActivated(bool active);
	void			Quit();

private:
	enum Actions {
		MATCH_CASE		= 'mtcs',
		MATCH_WORD		= 'mtwd',
		WRAP_AROUND		= 'wrar',
		DIRECTION_UP	= 'diru',
		DIRECTION_DOWN	= 'dird',
		IN_SELECTION	= 'insl'
	};
	void			_InitInterface();

	BStringView*	fFindString;
	BTextControl*	fFindTC;
	BStringView*	fReplaceString;
	BTextControl*	fReplaceTC;

	BButton*		fFindButton;
	BButton*		fReplaceButton;
	BButton*		fReplaceFindButton;
	BButton*		fReplaceAllButton;

	BCheckBox*		fMatchCaseCB;
	BCheckBox*		fMatchWordCB;
	BCheckBox*		fWrapAroundCB;
	BCheckBox*		fBackwardsCB;
	BCheckBox*		fInSelectionCB;

	BBox*			fDirectionBox;
	BRadioButton*	fDirectionUpRadio;
	BRadioButton*	fDirectionDownRadio;

	bool			fFlagsChanged;
	BString			fOldFindText;
	BString			fOldReplaceText;
};


#endif // FINDWINDOW_H
