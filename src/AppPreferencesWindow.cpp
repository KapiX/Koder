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

#include "AppPreferencesWindow.h"

#include <stdlib.h>

#include <Application.h>
#include <Box.h>
#include <Button.h>
#include <CheckBox.h>
#include <LayoutBuilder.h>
#include <Message.h>
#include <RadioButton.h>
#include <StringView.h>

#include "Preferences.h"


AppPreferencesWindow::AppPreferencesWindow(Preferences* preferences)
	:
	BWindow(BRect(0, 0, 400, 300), "Application preferences", B_TITLED_WINDOW, 0, 0)
{
	fCurrentPreferences = preferences;

	fStartPreferences = new Preferences();
	*fStartPreferences = *fCurrentPreferences;

	fTempPreferences = new Preferences();
	*fTempPreferences = *fCurrentPreferences;

	_InitInterface();
	CenterOnScreen();

	_SyncPreferences(fCurrentPreferences);
}


AppPreferencesWindow::~AppPreferencesWindow()
{
	delete fStartPreferences;
	delete fTempPreferences;
}


void
AppPreferencesWindow::MessageReceived(BMessage* message)
{
	switch(message->what) {
		case Actions::LINE_HIGHLIGHTING: {
			fTempPreferences->fLineHighlighting =
				(fLineHighlightingCB->Value() == B_CONTROL_ON ? true : false);
			_PreferencesModified();
		} break;
		case Actions::LINE_NUMBERS: {
			fTempPreferences->fLineNumbers =
				(fLineNumbersCB->Value() == B_CONTROL_ON ? true : false);
			_PreferencesModified();
		} break;
		case Actions::LINELIMIT_COLUMN: {
			fTempPreferences->fLineLimitColumn =
				atoi(fLineLimitColumnTC->Text());
			_PreferencesModified();
		} break;
		case Actions::LINELIMIT_SHOW: {
			bool show = (fLineLimitShowCB->Value() == B_CONTROL_ON ? true : false);
			fTempPreferences->fLineLimitShow = show;
			_SetLineLimitBoxEnabled(show);
			_PreferencesModified();
		} break;
		case Actions::LINELIMIT_BACKGROUND: {
			fTempPreferences->fLineLimitMode = 2;
			_PreferencesModified();
		} break;
		case Actions::LINELIMIT_LINE: {
			fTempPreferences->fLineLimitMode = 1;
			_PreferencesModified();
		} break;
		case Actions::APPLY: {
			*fCurrentPreferences = *fTempPreferences;
			fApplyButton->SetEnabled(false);
			BMessage changed(APP_PREFERENCES_CHANGED);
			be_app->PostMessage(&changed);
		} break;
		case Actions::REVERT: {
			*fTempPreferences = *fStartPreferences;
			fRevertButton->SetEnabled(false);
			fApplyButton->SetEnabled(true);
			_SyncPreferences(fTempPreferences);
		} break;
		default: {
			BWindow::MessageReceived(message);
		} break;
	}
}


void
AppPreferencesWindow::_InitInterface()
{
	fEditorBox = new BBox("editorPrefs");
	fEditorBox->SetLabel("Editor");
	fTabsToSpacesCB = new BCheckBox("tabsToSpaces", "Convert tabs to spaces", new BMessage((uint32) Actions::TABS_TO_SPACES));
	fLineHighlightingCB = new BCheckBox("lineHighlighting", "Highlight current line", new BMessage((uint32) Actions::LINE_HIGHLIGHTING));
	fLineNumbersCB = new BCheckBox("lineNumbers", "Display line numbers", new BMessage((uint32) Actions::LINE_NUMBERS));

	fLineLimitHeaderView = new BView("lineLimitHeader", 0);
	fLineLimitShowCB = new BCheckBox("lineLimitShow", "Mark lines exceeding ", new BMessage((uint32) Actions::LINELIMIT_SHOW));
	fLineLimitColumnTC = new BTextControl("lineLimitColumn", "", "80", new BMessage((uint32) Actions::LINELIMIT_COLUMN));
	fLineLimitText = new BStringView("lineLimitText", " characters");

	BLayoutBuilder::Group<>(fLineLimitHeaderView, B_HORIZONTAL, 0)
		.Add(fLineLimitShowCB)
		.Add(fLineLimitColumnTC)
		.Add(fLineLimitText);

	fLineLimitBox = new BBox("lineLimitPrefs");
	fLineLimitBackgroundRadio = new BRadioButton("lineLimitRadio", "Background", new BMessage((uint32) Actions::LINELIMIT_BACKGROUND));
	fLineLimitLineRadio = new BRadioButton("lineLimitRadio", "Line", new BMessage((uint32) Actions::LINELIMIT_LINE));

	BLayoutBuilder::Group<>(fLineLimitBox, B_VERTICAL, 5)
		.Add(fLineLimitBackgroundRadio)
		.Add(fLineLimitLineRadio)
		.SetInsets(10, 25, 15, 10);
	fLineLimitBox->SetLabel(fLineLimitHeaderView);

	fApplyButton = new BButton("Apply", new BMessage((uint32) Actions::APPLY));
	fRevertButton = new BButton("Revert", new BMessage((uint32) Actions::REVERT));

	fApplyButton->SetEnabled(false);
	fRevertButton->SetEnabled(false);

	BLayoutBuilder::Group<>(fEditorBox, B_VERTICAL, 5)
		//.Add(fTabsToSpacesCB)
		.Add(fLineHighlightingCB)
		.Add(fLineNumbersCB)
		.Add(fLineLimitBox)
		.AddGlue()
		.SetInsets(10, 15, 15, 10);

	BLayoutBuilder::Group<>(this, B_VERTICAL, 5)
		.Add(fEditorBox)
		.AddGroup(B_HORIZONTAL, 5)
			.Add(fRevertButton)
			.AddGlue()
			.Add(fApplyButton)
		.End()
		.SetInsets(5, 5, 5, 5);
}


void
AppPreferencesWindow::_SyncPreferences(Preferences* preferences)
{
	if(preferences->fLineNumbers == true) {
		fLineNumbersCB->SetValue(B_CONTROL_ON);
	} else {
		fLineNumbersCB->SetValue(B_CONTROL_OFF);
	}

	if(preferences->fLineHighlighting == true) {
		fLineHighlightingCB->SetValue(B_CONTROL_ON);
	} else {
		fLineHighlightingCB->SetValue(B_CONTROL_OFF);
	}

	BString columnString;
	columnString << preferences->fLineLimitColumn;
	fLineLimitColumnTC->SetText(columnString.String());
	if(preferences->fLineLimitShow == true) {
		fLineLimitShowCB->SetValue(B_CONTROL_ON);
		_SetLineLimitBoxEnabled(true);
	} else {
		fLineLimitShowCB->SetValue(B_CONTROL_OFF);
		_SetLineLimitBoxEnabled(false);
	}
}


void
AppPreferencesWindow::_PreferencesModified()
{
	fApplyButton->SetEnabled(true);
	fRevertButton->SetEnabled(true);
}


void
AppPreferencesWindow::_SetLineLimitBoxEnabled(bool enabled)
{
	fLineLimitColumnTC->SetEnabled(enabled);
	fLineLimitBackgroundRadio->SetEnabled(enabled);
	fLineLimitLineRadio->SetEnabled(enabled);

	switch(fTempPreferences->fLineLimitMode) {
		case 1:
			fLineLimitLineRadio->SetValue(B_CONTROL_ON);
		break;
		case 2:
			fLineLimitBackgroundRadio->SetValue(B_CONTROL_ON);
		break;
	}
}
