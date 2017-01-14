/*
 * Copyright 2014-2017 Kacper Kasper <kacperkasper@gmail.com>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include "GoToLineWindow.h"

#include <cstdlib>

#include <Button.h>
#include <Catalog.h>
#include <GroupLayout.h>
#include <LayoutBuilder.h>
#include <MessageFilter.h>
#include <TextControl.h>


#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "GoToLineWindow"


GoToLineWindow::GoToLineWindow(BWindow* owner)
	:
	BWindow(BRect(0, 0, 0, 0), B_TRANSLATE("Go to line"), B_MODAL_WINDOW_LOOK,
		B_MODAL_SUBSET_WINDOW_FEEL,
		B_NOT_RESIZABLE | B_NOT_MOVABLE | B_AUTO_UPDATE_SIZE_LIMITS),
	fOwner(owner)
{
	fLine = new BTextControl("GoToLineTC", B_TRANSLATE("Go to line:"), "1", nullptr);
	fGo = new BButton("GoButton", B_TRANSLATE("OK"), new BMessage(GTLW_GO));
	fGo->MakeDefault(true);
	fCancel = new BButton("CancelButton", B_TRANSLATE("Cancel"), new BMessage(GTLW_CANCEL));

	AddCommonFilter(new BMessageFilter(B_KEY_DOWN, KeyDownFilter));

	AddToSubset(fOwner);

	BGroupLayout* layout = new BGroupLayout(B_VERTICAL, 5);
	layout->SetInsets(5, 5, 5, 5);
	SetLayout(layout);
	layout->View()->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	BLayoutBuilder::Group<>(layout)
		.Add(fLine)
		.AddGroup(B_HORIZONTAL, 5)
			.Add(fCancel)
			.Add(fGo)
		.End();
}


void
GoToLineWindow::MessageReceived(BMessage* message)
{
	switch(message->what) {
	case GTLW_GO: {
		int32 line = atoi(fLine->Text());
		message->AddInt32("line", line);
		fOwner->PostMessage(message);
	}
	case GTLW_CANCEL:
		Hide();
	break;
	default:
		BWindow::MessageReceived(message);
	break;
	}
}


void
GoToLineWindow::ShowCentered(BRect ownerRect)
{
	CenterIn(ownerRect);
	Show();
}


void
GoToLineWindow::WindowActivated(bool active)
{
	fLine->MakeFocus();
	fLine->TextView()->SelectAll();
}


filter_result
GoToLineWindow::KeyDownFilter(BMessage* message, BHandler** target,
	BMessageFilter* messageFilter)
{
	if(message->what == B_KEY_DOWN) {
		const char* bytes;
		message->FindString("bytes", &bytes);
		if(bytes[0] == B_ESCAPE) {
			messageFilter->Looper()->PostMessage(GTLW_CANCEL);
			return B_SKIP_MESSAGE;
		}
	}
	return B_DISPATCH_MESSAGE;
}
