/*
 * Copyright 2014-2018 Kacper Kasper <kacperkasper@gmail.com>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include "AppPreferencesWindow.h"

#include <set>
#include <string>

#include <Application.h>
#include <Box.h>
#include <Button.h>
#include <Catalog.h>
#include <CheckBox.h>
#include <ControlLook.h>
#include <Font.h>
#include <LayoutBuilder.h>
#include <PopUpMenu.h>
#include <MenuField.h>
#include <Message.h>
#include <RadioButton.h>
#include <Spinner.h>
#include <StringView.h>

#include <Scintilla.h>

#include "Preferences.h"
#include "Styler.h"
#include "Utils.h"


#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "AppPreferencesWindow"


AppPreferencesWindow::AppPreferencesWindow(Preferences* preferences)
	:
	BWindow(BRect(0, 0, 400, 300), B_TRANSLATE("Koder preferences"), B_TITLED_WINDOW,
		B_NOT_ZOOMABLE | B_AUTO_UPDATE_SIZE_LIMITS, 0)
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
		case Actions::COMPACT_LANG_MENU: {
			fTempPreferences->fCompactLangMenu = IsChecked(fCompactLangMenuCB);
			_PreferencesModified();
		} break;
		case Actions::TOOLBAR: {
			fTempPreferences->fToolbar = IsChecked(fToolbarCB);
			_PreferencesModified();
		} break;
		case Actions::FULL_PATH_IN_TITLE: {
			fTempPreferences->fFullPathInTitle = IsChecked(fFullPathInTitleCB);
			_PreferencesModified();
		} break;
		case Actions::TABS_TO_SPACES: {
			fTempPreferences->fTabsToSpaces = IsChecked(fTabsToSpacesCB);
			_PreferencesModified();
		} break;
		case Actions::TAB_WIDTH: {
			fTempPreferences->fTabWidth = std::stoi(fTabWidthTC->Text());
			_PreferencesModified();
		} break;
		case Actions::LINE_HIGHLIGHTING: {
			bool show = IsChecked(fLineHighlightingCB);
			fTempPreferences->fLineHighlighting = show;
			_SetLineHighlightingBoxEnabled(show);
			_PreferencesModified();
		} break;
		case Actions::LINE_HIGHLIGHTING_BG: {
			fTempPreferences->fLineHighlightingMode = 0;
			_PreferencesModified();
		} break;
		case Actions::LINE_HIGHLIGHTING_FRAME: {
			fTempPreferences->fLineHighlightingMode = 1;
			_PreferencesModified();
		} break;
		case Actions::LINE_NUMBERS: {
			fTempPreferences->fLineNumbers = IsChecked(fLineNumbersCB);
			_PreferencesModified();
		} break;
		case Actions::LINELIMIT_COLUMN: {
			fTempPreferences->fLineLimitColumn =
				std::stoi(fLineLimitColumnTC->Text());
			_PreferencesModified();
		} break;
		case Actions::LINELIMIT_SHOW: {
			bool show = IsChecked(fLineLimitShowCB);
			fTempPreferences->fLineLimitShow = show;
			_SetLineLimitBoxEnabled(show);
			_PreferencesModified();
		} break;
		case Actions::LINELIMIT_BACKGROUND: {
			fTempPreferences->fLineLimitMode = EDGE_BACKGROUND;
			_PreferencesModified();
		} break;
		case Actions::LINELIMIT_LINE: {
			fTempPreferences->fLineLimitMode = EDGE_LINE;
			_PreferencesModified();
		} break;
		case Actions::INDENTGUIDES_SHOW: {
			bool show = IsChecked(fIndentGuidesShowCB);
			fTempPreferences->fIndentGuidesShow = show;
			_SetIndentGuidesBoxEnabled(show);
			_PreferencesModified();
		} break;
		case Actions::INDENTGUIDES_REAL: {
			fTempPreferences->fIndentGuidesMode = SC_IV_REAL;
			_PreferencesModified();
		} break;
		case Actions::INDENTGUIDES_FORWARD: {
			fTempPreferences->fIndentGuidesMode = SC_IV_LOOKFORWARD;
			_PreferencesModified();
		} break;
		case Actions::INDENTGUIDES_BOTH: {
			fTempPreferences->fIndentGuidesMode = SC_IV_LOOKBOTH;
			_PreferencesModified();
		} break;
		case Actions::BRACES_HIGHLIGHTING: {
			fTempPreferences->fBracesHighlighting = IsChecked(fBracesHighlightingCB);
			_PreferencesModified();
		} break;
		case Actions::EDITOR_STYLE: {
			fTempPreferences->fStyle = message->GetString("style", "default");
			_PreferencesModified();
		} break;
		case Actions::ATTACH_WINDOWS: {
			fTempPreferences->fOpenWindowsInStack = IsChecked(fAttachNewWindowsCB);
			_PreferencesModified();
		} break;
		case Actions::HIGHLIGHT_TRAILING_WS: {
			fTempPreferences->fHighlightTrailingWhitespace =
				IsChecked(fHighlightTrailingWSCB);
			_PreferencesModified();
		} break;
		case Actions::TRIM_TRAILING_WS_SAVE: {
			fTempPreferences->fTrimTrailingWhitespaceOnSave =
				IsChecked(fTrimTrailingWSOnSaveCB);
			_PreferencesModified();
		} break;
		case Actions::USE_EDITORCONFIG: {
			fTempPreferences->fUseEditorconfig = IsChecked(fUseEditorconfigCB);
			_PreferencesModified();
		} break;
		case Actions::ALWAYS_OPEN_IN_NEW_WINDOW: {
			fTempPreferences->fAlwaysOpenInNewWindow =
				IsChecked(fAlwaysOpenInNewWindowCB);
			_PreferencesModified();
		} break;
		case Actions::USE_CUSTOM_FONT: {
			bool use = IsChecked(fUseCustomFontCB);
			fTempPreferences->fUseCustomFont = use;
			_SetFontBoxEnabled(use);
			_PreferencesModified();
		} break;
		case Actions::FONT_CHANGED: {
			fTempPreferences->fFontFamily = message->GetString("family");
			_UpdateFontMenu();
			_PreferencesModified();
		} break;
		case Actions::FONT_SIZE_CHANGED: {
			fTempPreferences->fFontSize = fFontSizeSpinner->Value();
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
AppPreferencesWindow::Quit()
{
	be_app->PostMessage(APP_PREFERENCES_QUITTING);

	BWindow::Quit();
}


void
AppPreferencesWindow::_InitInterface()
{
	fVisualBox = new BBox("visualPrefs");
	fVisualBox->SetLabel(B_TRANSLATE("Visual"));
	fBehaviorBox = new BBox("behaviorPrefs");
	fBehaviorBox->SetLabel(B_TRANSLATE("Behavior"));
	fIndentationBox = new BBox("indentationPrefs");
	fIndentationBox->SetLabel(B_TRANSLATE("Indentation"));
	fTrailingWSBox = new BBox("trailingWSPrefs");
	fTrailingWSBox->SetLabel(B_TRANSLATE("Trailing whitespace"));

	fCompactLangMenuCB = new BCheckBox("compactLangMenu", B_TRANSLATE("Compact language menu"), new BMessage((uint32) Actions::COMPACT_LANG_MENU));
	fToolbarCB = new BCheckBox("toolbar", B_TRANSLATE("Show toolbar"), new BMessage((uint32) Actions::TOOLBAR));
	fFullPathInTitleCB = new BCheckBox("fullPathInTitle", B_TRANSLATE("Show full path in title"), new BMessage((uint32) Actions::FULL_PATH_IN_TITLE));
	fTabsToSpacesCB = new BCheckBox("tabsToSpaces", B_TRANSLATE("Convert tabs to spaces"), new BMessage((uint32) Actions::TABS_TO_SPACES));
	fTabWidthTC = new BTextControl("tabWidth", B_TRANSLATE("Spaces per tab:"), "4", new BMessage((uint32) Actions::TAB_WIDTH));
	fLineNumbersCB = new BCheckBox("lineNumbers", B_TRANSLATE("Show line numbers"), new BMessage((uint32) Actions::LINE_NUMBERS));

	fLineLimitHeaderView = new BView("lineLimitHeader", 0);
	fLineLimitShowCB = new BCheckBox("lineLimitShow", B_TRANSLATE("Mark overly long lines"), new BMessage((uint32) Actions::LINELIMIT_SHOW));

	BLayoutBuilder::Group<>(fLineLimitHeaderView, B_HORIZONTAL, 0)
		.Add(fLineLimitShowCB);

	fLineLimitBox = new BBox("lineLimitPrefs");
	fLineLimitColumnTC = new BTextControl("lineLimitColumn", B_TRANSLATE("Max. characters per line:"), "80", new BMessage((uint32) Actions::LINELIMIT_COLUMN));
	fLineLimitBackgroundRadio = new BRadioButton("lineLimitRadio", B_TRANSLATE("Background"), new BMessage((uint32) Actions::LINELIMIT_BACKGROUND));
	fLineLimitLineRadio = new BRadioButton("lineLimitRadio", B_TRANSLATE("Line"), new BMessage((uint32) Actions::LINELIMIT_LINE));

	BLayoutBuilder::Group<>(fLineLimitBox, B_VERTICAL, 0)
		.AddStrut(B_USE_ITEM_SPACING)
		.Add(fLineLimitBackgroundRadio)
		.Add(fLineLimitLineRadio)
		.Add(fLineLimitColumnTC)
		.SetInsets(B_USE_ITEM_INSETS);
	fLineLimitBox->SetLabel(fLineLimitHeaderView);

	fLineHighlightingBox = new BBox("lineHighlightingPrefs");
	fLineHighlightingCB = new BCheckBox("lineHighlighting", B_TRANSLATE("Highlight current line"), new BMessage((uint32) Actions::LINE_HIGHLIGHTING));
	fLineHighlightingBackgroundRadio = new BRadioButton("lineHighlightingRadio",
		B_TRANSLATE_COMMENT("Background", "Current line highlight"),
		new BMessage((uint32) Actions::LINE_HIGHLIGHTING_BG));
	fLineHighlightingFrameRadio = new BRadioButton("lineHighlightingRadio",
		B_TRANSLATE_COMMENT("Frame", "Current line highlight"),
		new BMessage((uint32) Actions::LINE_HIGHLIGHTING_FRAME));

	BLayoutBuilder::Group<>(fLineHighlightingBox, B_VERTICAL, 0)
		.AddStrut(B_USE_ITEM_SPACING)
		.Add(fLineHighlightingBackgroundRadio)
		.Add(fLineHighlightingFrameRadio)
		.SetInsets(B_USE_ITEM_INSETS);
	fLineHighlightingBox->SetLabel(fLineHighlightingCB);

	fIndentGuidesBox = new BBox("indentGuidesPrefs");
	fIndentGuidesShowCB = new BCheckBox("indentGuidesShow", B_TRANSLATE("Show indentation guides"), new BMessage((uint32) Actions::INDENTGUIDES_SHOW));
	fIndentGuidesRealRadio = new BRadioButton("indentGuidesReal", B_TRANSLATE("Only in actually indented lines"), new BMessage((uint32) Actions::INDENTGUIDES_REAL));
	fIndentGuidesLookForwardRadio = new BRadioButton("indentGuidesForward", B_TRANSLATE("Up to the next non-empty line"), new BMessage((uint32) Actions::INDENTGUIDES_FORWARD));
	fIndentGuidesLookBothRadio = new BRadioButton("indentGuidesBoth", B_TRANSLATE("Up to the next/previous non-empty line"), new BMessage((uint32) Actions::INDENTGUIDES_BOTH));

	BLayoutBuilder::Group<>(fIndentGuidesBox, B_VERTICAL, 0)
		.AddStrut(B_USE_ITEM_SPACING)
		.Add(fIndentGuidesRealRadio)
		.Add(fIndentGuidesLookForwardRadio)
		.Add(fIndentGuidesLookBothRadio)
		.SetInsets(B_USE_ITEM_INSETS);
	fIndentGuidesBox->SetLabel(fIndentGuidesShowCB);

	fBracesHighlightingCB = new BCheckBox("bracesHighlighting", B_TRANSLATE("Highlight braces"), new BMessage((uint32) Actions::BRACES_HIGHLIGHTING));

	fEditorStyleMenu = new BPopUpMenu("style");
	fEditorStyleMF = new BMenuField("style", B_TRANSLATE("Style"), fEditorStyleMenu);

	fAttachNewWindowsCB = new BCheckBox("attachWindows", B_TRANSLATE("Stack new windows"), new BMessage((uint32) Actions::ATTACH_WINDOWS));
	fHighlightTrailingWSCB = new BCheckBox("highlightTrailingWS", B_TRANSLATE("Highlight trailing whitespace"), new BMessage((uint32) Actions::HIGHLIGHT_TRAILING_WS));
	fTrimTrailingWSOnSaveCB  = new BCheckBox("trimTrailingWSOnSave", B_TRANSLATE("Trim trailing whitespace on save"), new BMessage((uint32) Actions::TRIM_TRAILING_WS_SAVE));
	fUseEditorconfigCB  = new BCheckBox("useEditorconfig", B_TRANSLATE("Use .editorconfig if possible"), new BMessage((uint32) Actions::USE_EDITORCONFIG));
	fAlwaysOpenInNewWindowCB  = new BCheckBox("alwaysOpenInNewWindow", B_TRANSLATE("Always open files in new window"), new BMessage((uint32) Actions::ALWAYS_OPEN_IN_NEW_WINDOW));

	fUseCustomFontCB = new BCheckBox("customFont", B_TRANSLATE("Use custom font"), new BMessage((uint32) Actions::USE_CUSTOM_FONT));
	fFontMenu = new BPopUpMenu("font");
	fFontMenu->SetLabelFromMarked(true);
	fFontMenu->SetRadioMode(true);
	fFontBox = new BBox("fontPrefs");
	fFontMF = new BMenuField("font", "", fFontMenu);
	fFontSizeSpinner = new BSpinner("fontSize", "", new BMessage((uint32) Actions::FONT_SIZE_CHANGED));
	fFontSizeSpinner->SetExplicitMaxSize(BSize(80.0f, B_SIZE_UNSET));
	fFontSizeSpinner->SetRange(6, 72);

	BLayoutBuilder::Group<>(fFontBox, B_VERTICAL, 0)
		.AddStrut(B_USE_ITEM_SPACING)
		.AddGroup(B_HORIZONTAL, B_USE_HALF_ITEM_SPACING)
			.Add(fFontMF)
			.Add(fFontSizeSpinner)
		.End()
		.SetInsets(B_USE_ITEM_INSETS);
	fFontBox->SetLabel(fUseCustomFontCB);

	fApplyButton = new BButton(B_TRANSLATE("Apply"), new BMessage((uint32) Actions::APPLY));
	fRevertButton = new BButton(B_TRANSLATE("Revert"), new BMessage((uint32) Actions::REVERT));

	fApplyButton->SetEnabled(false);
	fRevertButton->SetEnabled(false);

	BLayoutBuilder::Group<>(fVisualBox, B_VERTICAL, 0)
		.AddStrut(B_USE_ITEM_SPACING)
		.Add(fCompactLangMenuCB)
		.Add(fToolbarCB)
		.Add(fFullPathInTitleCB)
		.Add(fLineNumbersCB)
		.Add(fBracesHighlightingCB)
		.Add(fLineLimitBox)
		.AddStrut(B_USE_HALF_ITEM_SPACING)
		.Add(fLineHighlightingBox)
		.AddStrut(B_USE_HALF_ITEM_SPACING)
		.Add(fEditorStyleMF)
		.SetInsets(B_USE_ITEM_INSETS);

	BLayoutBuilder::Group<>(fIndentationBox, B_VERTICAL, 0)
		.AddStrut(B_USE_ITEM_SPACING)
		.Add(fTabsToSpacesCB)
		.Add(fTabWidthTC)
		.Add(fIndentGuidesBox)
		.SetInsets(B_USE_ITEM_INSETS);

	BLayoutBuilder::Group<>(fTrailingWSBox, B_VERTICAL, 0)
		.AddStrut(B_USE_ITEM_SPACING)
		.Add(fHighlightTrailingWSCB)
		.Add(fTrimTrailingWSOnSaveCB)
		.SetInsets(B_USE_ITEM_INSETS);

	BLayoutBuilder::Group<>(fBehaviorBox, B_VERTICAL, 0)
		.AddStrut(B_USE_ITEM_SPACING)
		.Add(fAlwaysOpenInNewWindowCB)
		.Add(fAttachNewWindowsCB)
		.Add(fUseEditorconfigCB)
		.SetInsets(B_USE_ITEM_INSETS);

	BLayoutBuilder::Group<>(this, B_VERTICAL, B_USE_DEFAULT_SPACING)
		.AddGroup(B_HORIZONTAL, B_USE_DEFAULT_SPACING)
			.AddGroup(B_VERTICAL, B_USE_DEFAULT_SPACING)
				.Add(fVisualBox)
				.Add(fFontBox)
			.End()
			.AddGroup(B_VERTICAL, B_USE_DEFAULT_SPACING)
				.Add(fIndentationBox)
				.Add(fTrailingWSBox)
				.Add(fBehaviorBox)
				.AddGlue()
			.End()
		.End()
		.AddGroup(B_HORIZONTAL, B_USE_DEFAULT_SPACING)
			.Add(fRevertButton)
			.AddGlue()
			.Add(fApplyButton)
		.End()
		.SetInsets(B_USE_SMALL_INSETS);

	_PopulateStylesMenu();
	_UpdateFontMenu();
}


void
AppPreferencesWindow::_SyncPreferences(Preferences* preferences)
{
	SetChecked(fCompactLangMenuCB, preferences->fCompactLangMenu);
	SetChecked(fToolbarCB, preferences->fToolbar);
	SetChecked(fFullPathInTitleCB, preferences->fFullPathInTitle);
	SetChecked(fTabsToSpacesCB, preferences->fTabsToSpaces);

	BString tabWidthString;
	tabWidthString << preferences->fTabWidth;
	fTabWidthTC->SetText(tabWidthString.String());

	SetChecked(fLineNumbersCB, preferences->fLineNumbers);

	BString columnString;
	columnString << preferences->fLineLimitColumn;
	fLineLimitColumnTC->SetText(columnString.String());

	SetChecked(fLineLimitShowCB, preferences->fLineLimitShow);
	_SetLineLimitBoxEnabled(preferences->fLineLimitShow);

	SetChecked(fLineHighlightingCB, preferences->fLineHighlighting);
	_SetLineHighlightingBoxEnabled(preferences->fLineHighlighting);

	SetChecked(fIndentGuidesShowCB, preferences->fIndentGuidesShow);
	_SetIndentGuidesBoxEnabled(preferences->fIndentGuidesShow);

	SetChecked(fUseCustomFontCB, preferences->fUseCustomFont);
	_SetFontBoxEnabled(preferences->fUseCustomFont);
	fFontSizeSpinner->SetValue(preferences->fFontSize);
	_UpdateFontMenu();

	SetChecked(fBracesHighlightingCB, preferences->fBracesHighlighting);
	SetChecked(fAttachNewWindowsCB, preferences->fOpenWindowsInStack);
	SetChecked(fHighlightTrailingWSCB, preferences->fHighlightTrailingWhitespace);
	SetChecked(fTrimTrailingWSOnSaveCB, preferences->fTrimTrailingWhitespaceOnSave);
	SetChecked(fUseEditorconfigCB, preferences->fUseEditorconfig);
	SetChecked(fAlwaysOpenInNewWindowCB, preferences->fAlwaysOpenInNewWindow);
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
		case 1: SetChecked(fLineLimitLineRadio); break;
		case 2: SetChecked(fLineLimitBackgroundRadio); break;
	}
}


void
AppPreferencesWindow::_SetLineHighlightingBoxEnabled(bool enabled)
{
	fLineHighlightingBackgroundRadio->SetEnabled(enabled);
	fLineHighlightingFrameRadio->SetEnabled(enabled);

	switch(fTempPreferences->fLineHighlightingMode) {
		case 0: SetChecked(fLineHighlightingBackgroundRadio); break;
		case 1: SetChecked(fLineHighlightingFrameRadio); break;
	}
}


void
AppPreferencesWindow::_SetIndentGuidesBoxEnabled(bool enabled)
{
	fIndentGuidesRealRadio->SetEnabled(enabled);
	fIndentGuidesLookForwardRadio->SetEnabled(enabled);
	fIndentGuidesLookBothRadio->SetEnabled(enabled);

	switch(fTempPreferences->fIndentGuidesMode) {
		case 1: SetChecked(fIndentGuidesRealRadio); break;
		case 2: SetChecked(fIndentGuidesLookForwardRadio); break;
		case 3: SetChecked(fIndentGuidesLookBothRadio); break;
	}
}


void
AppPreferencesWindow::_SetFontBoxEnabled(bool enabled)
{
	fFontMF->SetEnabled(enabled);
	fFontSizeSpinner->SetEnabled(enabled);
}


void
AppPreferencesWindow::_PopulateStylesMenu()
{
	std::set<std::string> styles;
	Styler::GetAvailableStyles(styles);
	for(auto& style : styles) {
		BMessage* msg = new BMessage(EDITOR_STYLE);
		msg->AddString("style", style.c_str());
		BMenuItem* menuItem = new BMenuItem(style.c_str(), msg);
		if(style == fTempPreferences->fStyle)
			menuItem->SetMarked(true);
		fEditorStyleMenu->AddItem(menuItem);
	}
}


void
AppPreferencesWindow::_UpdateFontMenu()
{
	fFontMenu->RemoveItems(0, fFontMenu->CountItems(), true);

	int32 numFamilies = count_font_families();
	for(int32 i = 0; i < numFamilies; i++) {
		font_family family;
		if(get_font_family(i, &family) == B_OK) {
			BMessage* familyMsg = new BMessage(FONT_CHANGED);
			familyMsg->AddString("family", family);

			BMenuItem* familyItem = new BMenuItem(family, familyMsg);
			if(fTempPreferences->fFontFamily == family) {
				familyItem->SetMarked(true);
			}
			fFontMenu->AddItem(familyItem);
		}
	}
	fFontMenu->SetTargetForItems(this);
}
