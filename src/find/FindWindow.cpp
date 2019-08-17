/*
 * Copyright 2016-2019 Kacper Kasper <kacperkasper@gmail.com>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include "FindWindow.h"

#include <memory>

#include <Application.h>
#include <Box.h>
#include <Button.h>
#include <ControlLook.h>
#include <Catalog.h>
#include <CheckBox.h>
#include <LayoutBuilder.h>
#include <Message.h>
#include <RadioButton.h>
#include <ScintillaView.h>
#include <StringView.h>

#include "File.h"
#include "FindStatusView.h"
#include "Utils.h"


#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "FindWindow"


class FindScintillaView : public BScintillaView
{
private:
	void UpdateColors()
	{
		rgb_color fore = ui_color(B_DOCUMENT_TEXT_COLOR);
		rgb_color back = ui_color(B_DOCUMENT_BACKGROUND_COLOR);
		SendMessage(SCI_STYLESETFORE, STYLE_DEFAULT, rgb_colorToSciColor(fore));
		SendMessage(SCI_STYLESETBACK, STYLE_DEFAULT, rgb_colorToSciColor(back));
		SendMessage(SCI_STYLESETFORE, 0, rgb_colorToSciColor(fore));
		SendMessage(SCI_STYLESETBACK, 0, rgb_colorToSciColor(back));
	}

public:
	FindScintillaView(const char* name, uint32 getMessage, uint32 clearMessage,
		uint32 applyMessage, uint32 flags = 0, bool horizontal = true,
		bool vertical = true, border_style border = B_FANCY_BORDER)
		:
		BScintillaView(name, flags, horizontal, vertical, border)
	{
		UpdateColors();
		fStatusView = new find::StatusView(this,
			getMessage, clearMessage, applyMessage);
	}

	virtual void DoLayout()
	{
		BScintillaView::DoLayout();

		fStatusView->ResizeToPreferred();
	}


	virtual void FrameResized(float width, float height)
	{
		BScintillaView::FrameResized(width, height);

		fStatusView->ResizeToPreferred();
	}

	virtual void MessageReceived(BMessage* message)
	{
		switch(message->what) {
			case B_COLORS_UPDATED: {
				UpdateColors();
			} break;
			default: {
				BScintillaView::MessageReceived(message);
			} break;
		}
	}
private:
	find::StatusView* fStatusView;
};


FindWindow::FindWindow(BMessage *state, BPath settingsPath)
	:
	BWindow(BRect(0, 0, 400, 300), B_TRANSLATE("Find/Replace"), B_TITLED_WINDOW,
		B_NOT_ZOOMABLE, 0),
	fFlagsChanged(false),
	fSettingsPath(settingsPath)
{
	_InitInterface();
	_LoadHistory();
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
			std::string findText(fFindTC->TextLength(), '\0');
			std::string replaceText(fReplaceTC->TextLength(), '\0');
			fFindTC->GetText(0, findText.size() + 1, &findText[0]);
			fReplaceTC->GetText(0, replaceText.size() + 1, &replaceText[0]);
			{
				const int32 mruMax = 10;
				int32 count = 0;
				std::string lastFind;
				if(fFindHistory.GetInfo("mru", nullptr, &count) == B_OK) {
					lastFind = fFindHistory.GetString("mru", count - 1, "");
				}
				if(lastFind != findText)
					fFindHistory.AddString("mru", findText.c_str());
				while(count >= mruMax) {
					fFindHistory.RemoveData("mru", 0);
					count--;
				}

				if (message->what != FINDWINDOW_FIND) {
					count = 0;
					std::string lastReplace;
					if(fReplaceHistory.GetInfo("mru", nullptr, &count) == B_OK) {
						lastReplace = fReplaceHistory.GetString("mru", count - 1, "");
					}
					if(lastReplace != replaceText)
						fReplaceHistory.AddString("mru", replaceText.c_str());
					while(count >= mruMax) {
						fReplaceHistory.RemoveData("mru", 0);
						count--;
					}
				}
			}
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
		case HistoryRequests::GET_FIND_HISTORY: {
			BMessage reply(fFindHistory);
			reply.what = message->what;
			message->SendReply(&reply);
		} break;
		case HistoryRequests::CLEAR_FIND_HISTORY: {
			fFindHistory.MakeEmpty();
		} break;
		case HistoryRequests::APPLY_FIND_ITEM: {
			int32 index = message->GetInt32("item", 0);
			BString item = fFindHistory.GetString("mru", index, "");
			fFindHistory.RemoveData("mru", index);
			fFindTC->SetText(item.String());
			fFindHistory.AddString("mru", item.String());
		} break;
		case HistoryRequests::GET_REPLACE_HISTORY: {
			BMessage reply(fReplaceHistory);
			reply.what = message->what;
			message->SendReply(&reply);
		} break;
		case HistoryRequests::CLEAR_REPLACE_HISTORY: {
			fReplaceHistory.MakeEmpty();
		} break;
		case HistoryRequests::APPLY_REPLACE_ITEM: {
			int32 index = message->GetInt32("item", 0);
			BString item = fReplaceHistory.GetString("mru", index, "");
			fReplaceHistory.RemoveData("mru", index);
			fReplaceTC->SetText(item.String());
			fReplaceHistory.AddString("mru", item.String());
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
	_SaveHistory();

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
	fFindTC = new FindScintillaView("findText",
		HistoryRequests::GET_FIND_HISTORY,
		HistoryRequests::CLEAR_FIND_HISTORY,
		HistoryRequests::APPLY_FIND_ITEM);
	fFindTC->SetExplicitMinSize(BSize(200, 100));
	fFindTC->Target()->SetFlags(fFindTC->Target()->Flags() | B_NAVIGABLE);
	fFindTC->SendMessage(SCI_SETMARGINWIDTHN, 1, 0);

	fReplaceTC = new FindScintillaView("replaceText",
		HistoryRequests::GET_REPLACE_HISTORY,
		HistoryRequests::CLEAR_REPLACE_HISTORY,
		HistoryRequests::APPLY_REPLACE_ITEM);
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

void
FindWindow::_LoadHistory()
{
	auto file = std::make_shared<BFile>(
		BPath(fSettingsPath.Path(), "findreplace_mru").Path(), B_READ_ONLY);
	// TODO: proper error checking
	if(file && file->InitCheck() == B_OK) {
		BMessage history;
		history.Unflatten(file.get());
		history.FindMessage("find", &fFindHistory);
		history.FindMessage("replace", &fReplaceHistory);
	}
}

void
FindWindow::_SaveHistory()
{
	BPath historyPath(fSettingsPath.Path(), "findreplace_mru");

	BMessage history;
	history.AddMessage("find", &fFindHistory);
	history.AddMessage("replace", &fReplaceHistory);

	BackupFileGuard backupFileGuard(historyPath.Path());

	auto file = std::make_shared<BFile>(historyPath.Path(),
		B_WRITE_ONLY | B_CREATE_FILE | B_ERASE_FILE);
	// TODO: proper error checking
	if(file && file->InitCheck() == B_OK) {
		history.Flatten(file.get());
		backupFileGuard.SaveSuccessful();
	}
}
