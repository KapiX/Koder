/*
 * Copyright 2019 Kacper Kasper <kacperkasper@gmail.com>
 * All rights reserved. Distributed under the terms of the MIT license.
 */


#include "BookmarksWindow.h"


#include <Catalog.h>
#include <GroupLayout.h>
#include <ListView.h>
#include <ScrollView.h>

#include <string>

#include "BookmarksListView.h"
#include "GoToLineWindow.h"


#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "BookmarksWindow"


class CustomScrollView : public BScrollView {
public:
	CustomScrollView(const char* name, BView* target);

	virtual void DoLayout();
};


CustomScrollView::CustomScrollView(const char* name, BView* target)
	:
	BScrollView(name, target, 0, false, true)
{
}


void
CustomScrollView::DoLayout()
{
	BScrollView::DoLayout();

	BScrollBar* scrollBar = ScrollBar(B_VERTICAL);
	scrollBar->ResizeBy(0, 2);
	scrollBar->MoveBy(0, -1);
}


#pragma mark -


BookmarksWindow::BookmarksWindow(BWindow* owner, const BMessage bookmarks)
	:
	BWindow(BRect(0, 0, 0, 0), B_TRANSLATE("Bookmarks"),
		B_FLOATING_WINDOW_LOOK, B_FLOATING_SUBSET_WINDOW_FEEL, 0),
	fOwner(owner)
{
	AddToSubset(fOwner);

	fOwner->StartWatching(this, BOOKMARK_ADDED);
	fOwner->StartWatching(this, BOOKMARK_REMOVED);
	fOwner->StartWatching(this, BOOKMARKS_INVALIDATED);

	BMessage* goToLineMessage = new BMessage(GTLW_GO);
	fList = new BookmarksListView("bookmarks list");
	fList->SetInvocationMessage(goToLineMessage);
	fList->SetTarget(fOwner);
	fScroller = new CustomScrollView("bookmarks", fList);
	fScroller->SetBorder(B_NO_BORDER);

	BGroupLayout* layout = new BGroupLayout(B_VERTICAL, 0);
	SetLayout(layout);
	layout->AddView(fScroller);
	layout->SetInsets(0.f, 0.f, -1.0f, 0.f);

	fList->UpdateBookmarks(bookmarks);

	BRect frame = fOwner->Frame();
	BRect decorFrame = fOwner->DecoratorFrame();
	float frameThickness = frame.left - decorFrame.left;
	MoveTo(frame.left + frame.Width() + frameThickness * 2, frame.top);
	ResizeTo(200.0f, fOwner->Bounds().Height());
}


BookmarksWindow::~BookmarksWindow()
{
}


void
BookmarksWindow::MessageReceived(BMessage* message)
{
	switch(message->what) {
		case B_OBSERVER_NOTICE_CHANGE: {
			int32 what = message->GetInt32("be:observe_change_what", 0);
			switch(what) {
				case BOOKMARK_ADDED:
				case BOOKMARK_REMOVED:
				case BOOKMARKS_INVALIDATED: {
					fList->UpdateBookmarks(*message);
				} break;
			}
		} break;
		default:
			BWindow::MessageReceived(message);
		break;
	}
}


void
BookmarksWindow::Quit()
{
	fOwner->PostMessage(BOOKMARKS_WINDOW_QUITTING);

	BWindow::Quit();
}
