/*
 * Copyright 2016-2017 Kacper Kasper <kacperkasper@gmail.com>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include "FindWindow.h"

#include <ScintillaView.h>

#include <Application.h>
#include <Box.h>
#include <Button.h>
#include <Catalog.h>
#include <CheckBox.h>
#include <LayoutBuilder.h>
#include <Message.h>
#include <RadioButton.h>
#include <StringView.h>

#include "Utils.h"


#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "FindWindow"


FindWindow::FindWindow(BMessage *state)
	:
	BWindow(BRect(0, 0, 400, 300), B_TRANSLATE("Find/Replace"), B_TITLED_WINDOW,
		B_NOT_ZOOMABLE, 0),
	fFlagsChanged(false)
{
	_InitInterface();
	CenterOnScreen();

	SetChecked(fInSelectionCB, state->GetBool("inSelection", false));
	SetChecked(fMatchCaseCB, state->GetBool("matchCase", false));
	SetChecked(fMatchWordCB, state->GetBool("matchWord", false));
	SetChecked(fWrapAroundCB, state->GetBool("wrapAround", false));
	SetChecked(fRegexCB, state->GetBool("regex", false));
	SetChecked(fBackwardsCB, state->GetBool("backwards", false));

	fFindTC->SetText(state->GetString("findText"));
	fReplaceTC->SetText(state->GetString("replaceText"));
}


FindWindow::~FindWindow()
{
}


void
FindWindow::MessageReceived(BMessage* message)
{
	switch(message->what) {
		case FINDWINDOW_FIND:
		case FINDWINDOW_REPLACE:
		case FINDWINDOW_REPLACEFIND:
		case FINDWINDOW_REPLACEALL: {
			int32 findLength = fFindTC->TextLength() + 1;
			int32 replaceLength = fReplaceTC->TextLength() + 1;
			std::string findText(findLength + 1, '\0');
			std::string replaceText(replaceLength + 1, '\0');
			fFindTC->GetText(0, findLength, &findText[0]);
			fReplaceTC->GetText(0, replaceLength, &replaceText[0]);
			bool newSearch = (fFlagsChanged
				|| fOldFindText != findText
				|| fOldReplaceText != replaceText);
			message->AddBool("newSearch", newSearch);
			message->AddBool("inSelection", IsChecked(fInSelectionCB));
			message->AddBool("matchCase", IsChecked(fMatchCaseCB));
			message->AddBool("matchWord", IsChecked(fMatchWordCB));
			message->AddBool("wrapAround", IsChecked(fWrapAroundCB));
			message->AddBool("regex", IsChecked(fRegexCB));
			message->AddBool("backwards", IsChecked(fBackwardsCB));
			message->AddString("findText", findText.c_str());
			message->AddString("replaceText", replaceText.c_str());
			be_app->PostMessage(message);
			fOldFindText = findText;
			fOldReplaceText = replaceText;
			if(message->what == FINDWINDOW_REPLACEALL) {
				fFlagsChanged = true;
					// Force scope retargeting on next search
			} else {
				fFlagsChanged = false;
			}
		} break;
		case FINDWINDOW_QUITTING: {
			if(LockLooper())
				Quit();
		} break;
		case Actions::MATCH_CASE:
		case Actions::MATCH_WORD:
		case Actions::WRAP_AROUND:
		case Actions::BACKWARDS:
		case Actions::IN_SELECTION: {
			fFlagsChanged = true;
		} break;
		default: {
			BWindow::MessageReceived(message);
		} break;
	}
}


void
FindWindow::WindowActivated(bool active)
{
	if(active == true) {
		fFindTC->MakeFocus();
		fFindTC->SendMessage(SCI_SELECTALL);
	}
}


void
FindWindow::Quit()
{
	be_app->PostMessage(FINDWINDOW_QUITTING);

	BWindow::Quit();
}


void
FindWindow::SetFindText(const std::string text)
{
	fFindTC->SetText(text.c_str());
}


void
FindWindow::_InitInterface()
{
	fFindString = new BStringView("findString", B_TRANSLATE("Find:"));
	fReplaceString = new BStringView("replaceString", B_TRANSLATE("Replace:"));
	fFindTC = new BScintillaView("findText", 0, true, true);
	fFindTC->SetExplicitMinSize(BSize(200, 100));
	fFindTC->Target()->SetFlags(fFindTC->Target()->Flags() | B_NAVIGABLE);
	fFindTC->SendMessage(SCI_SETMARGINWIDTHN, 1, 0);
	fReplaceTC = new BScintillaView("replaceText", 0, true, true);
	fReplaceTC->SetExplicitMinSize(BSize(200, 100));
	fReplaceTC->Target()->SetFlags(fReplaceTC->Target()->Flags() | B_NAVIGABLE);
	fReplaceTC->SendMessage(SCI_SETMARGINWIDTHN, 1, 0);

	fFindButton = new BButton(B_TRANSLATE("Find"), new BMessage((uint32) FINDWINDOW_FIND));
	fFindButton->MakeDefault(true);
	fFindButton->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));
	fReplaceButton = new BButton(B_TRANSLATE("Replace"), new BMessage((uint32) FINDWINDOW_REPLACE));
	fReplaceButton->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));
	fReplaceFindButton = new BButton(B_TRANSLATE("Replace and find"), new BMessage((uint32) FINDWINDOW_REPLACEFIND));
	fReplaceFindButton->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));
	fReplaceAllButton = new BButton(B_TRANSLATE("Replace all"), new BMessage((uint32) FINDWINDOW_REPLACEALL));
	fReplaceAllButton->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));

	fMatchCaseCB = new BCheckBox("matchCase", B_TRANSLATE("Match case"), new BMessage((uint32) Actions::MATCH_CASE));
	fMatchWordCB = new BCheckBox("matchWord", B_TRANSLATE("Match entire words"), new BMessage((uint32) Actions::MATCH_WORD));
	fWrapAroundCB = new BCheckBox("wrapAround", B_TRANSLATE("Wrap around"), new BMessage((uint32) Actions::WRAP_AROUND));
	fInSelectionCB = new BCheckBox("inSelection", B_TRANSLATE("In selection"), new BMessage((uint32) Actions::IN_SELECTION));
	fBackwardsCB = new BCheckBox("backwards", B_TRANSLATE("Backwards"), new BMessage((uint32) Actions::BACKWARDS));
	fRegexCB = new BCheckBox("regex", B_TRANSLATE("Regex"), new BMessage((uint32) Actions::REGEX));

	AddCommonFilter(new KeyDownMessageFilter(FINDWINDOW_QUITTING, B_ESCAPE));

	BLayoutBuilder::Group<>(this, B_VERTICAL, B_USE_DEFAULT_SPACING)
		.AddGroup(B_HORIZONTAL, B_USE_DEFAULT_SPACING)
			.AddGrid(B_USE_HALF_ITEM_SPACING, B_USE_HALF_ITEM_SPACING)
				.Add(fFindString, 0, 0)
				.Add(fFindTC, 1, 0)
				.Add(fReplaceString, 0, 1)
				.Add(fReplaceTC, 1, 1)
			.End()
			.AddGroup(B_VERTICAL, B_USE_ITEM_SPACING)
				.Add(fFindButton)
				.Add(fReplaceButton)
				.Add(fReplaceFindButton)
				.Add(fReplaceAllButton)
				.AddGlue()
				.SetExplicitMaxSize(BSize(200, B_SIZE_UNSET))
			.End()
		.End()
		.AddGrid(0.0f, 0.0f)
			.Add(fMatchCaseCB, 0, 0)
			.Add(fRegexCB, 1, 0)
			.Add(fBackwardsCB, 2, 0)
			.Add(fMatchWordCB, 0, 1)
			.Add(fInSelectionCB, 1, 1)
			.Add(fWrapAroundCB, 2, 1)
//			.SetExplicitMaxSize(BSize(B_SIZE_UNSET, 50)) // doesn't work
		.End()
		.SetInsets(B_USE_SMALL_INSETS);
	BSize min = GetLayout()->MinSize();
	SetSizeLimits(min.Width(), B_SIZE_UNLIMITED, min.Height(), B_SIZE_UNLIMITED);
	ResizeTo(min.Width(), min.Height());
}
