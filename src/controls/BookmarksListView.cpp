/*
 * Copyright 2019 Kacper Kasper <kacperkasper@gmail.com>
 * All rights reserved. Distributed under the terms of the MIT license.
 */


#include "BookmarksListView.h"


#include <Catalog.h>
#include <ControlLook.h>
#include <LayoutBuilder.h>
#include <ListView.h>
#include <PopUpMenu.h>
#include <StringItem.h>

#include <string>

#include "BookmarksWindow.h"
#include "GoToLineWindow.h"


#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "BookmarksListView"


BookmarksListView::BookmarksListView(const char* name)
	:
	BListView(name)
{
}


BookmarksListView::~BookmarksListView()
{
}


void
BookmarksListView::AttachedToWindow()
{
	SetFlags(Flags() | B_FULL_UPDATE_ON_RESIZE);
}


status_t
BookmarksListView::Invoke(BMessage* message)
{
	BMessage clone;
	if(InvocationMessage() != nullptr)
		clone = *InvocationMessage();
	int32 index = CurrentSelection();
	BookmarkItem* item = static_cast<BookmarkItem*>(ItemAt(index));
	if(item != nullptr) {
		clone.AddInt32("line", item->Line() + 1);
		return BListView::Invoke(&clone);
	}
	return B_BAD_VALUE;
}


void
BookmarksListView::MouseDown(BPoint where)
{
	BMessage* message = Looper()->CurrentMessage();
	int32 buttons = message->GetInt32("buttons", 0);
	if((buttons & B_PRIMARY_MOUSE_BUTTON) != 0) {
		BListView::MouseDown(where);
	} else if((buttons & B_SECONDARY_MOUSE_BUTTON) != 0) {
		Select(IndexOf(where));
		_ShowContextMenu(where);
	}
}


void
BookmarksListView::UpdateBookmarks(const BMessage bookmarks)
{
	MakeEmpty();
	type_code type;
	int32 count;
	if(bookmarks.GetInfo("line", &type, &count) == B_OK) {
		for(int32 i = 0; i < count; i++) {
			int64 line = bookmarks.GetInt64("line", i, -1);
			const char* text = bookmarks.GetString("text", i, "");
			if(line != -1) {
				BListView::AddItem(new BookmarkItem(line, text));
			}
		}
	}
}


void
BookmarksListView::_ShowContextMenu(BPoint where)
{
	BMessage* gotoMessage = new BMessage(GTLW_GO);
	BMessage* deleteMessage = new BMessage(BOOKMARK_REMOVED);
	int32 index = CurrentSelection();
	BookmarkItem* item = static_cast<BookmarkItem*>(ItemAt(index));
	if(item != nullptr) {
		gotoMessage->AddInt32("line", item->Line() + 1);
		deleteMessage->AddInt32("line", item->Line());
	}
	BPopUpMenu* contextMenu = new BPopUpMenu("ContextMenu", false, false);
	BLayoutBuilder::Menu<>(contextMenu)
		.AddItem(B_TRANSLATE("Go to"), gotoMessage)
		.AddItem(B_TRANSLATE("Delete"), deleteMessage);
	contextMenu->SetTargetForItems(Target());
	contextMenu->Go(ConvertToScreen(where), true, true);
}


BookmarksListView::BookmarkItem::BookmarkItem(int64 line, BString text)
	:
	BStringItem(text.String()),
	fLine(line),
	fText(text)
{
}


void
BookmarksListView::BookmarkItem::DrawItem(BView* owner, BRect frame, bool complete)
{
	rgb_color lowColor = owner->LowColor();

	if (IsSelected() || complete) {
		rgb_color color;
		if (IsSelected())
			color = ui_color(B_LIST_SELECTED_BACKGROUND_COLOR);
		else
			color = owner->ViewColor();

		owner->SetLowColor(color);
		owner->FillRect(frame, B_SOLID_LOW);
	} else
		owner->SetLowColor(owner->ViewColor());

	owner->MovePenTo(frame.left + be_control_look->DefaultLabelSpacing(),
		frame.top + BaselineOffset());

	BString lineStr;
	lineStr << fLine + 1;
	float lineStrWidth = owner->StringWidth(lineStr.String());
	owner->DrawString(lineStr.String());
	owner->MovePenBy(lineStrWidth, 0.0f);

	BString textToDraw = fText;
	float width = frame.right - owner->PenLocation().x;
	owner->TruncateString(&textToDraw, B_TRUNCATE_END, width);
	owner->DrawString(textToDraw);

	owner->SetLowColor(lowColor);
}


int64
BookmarksListView::BookmarkItem::Line() const
{
	return fLine;
}
