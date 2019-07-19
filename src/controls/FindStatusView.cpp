/*
 * Copyright 2002-2012, Haiku, Inc. All Rights Reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Vlad Slepukhin
 *		Siarzhuk Zharski
 *
 * Copied from Haiku commit a609673ce8c942d91e14f24d1d8832951ab27964.
 * Modifications:
 * Copyright 2019 Kacper Kasper <kacperkasper@gmail.com>
 * Distributed under the terms of the MIT License.
 */


#include "FindStatusView.h"

#include <stdio.h>
#include <stdlib.h>
#include <string>

#include <Catalog.h>
#include <ControlLook.h>
#include <MenuItem.h>
#include <Message.h>
#include <Messenger.h>
#include <PopUpMenu.h>
#include <ScrollView.h>
#include <StringView.h>
#include <Window.h>

#include "FindWindow.h"


#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "FindStatusView"


namespace {
	const float kHorzSpacing = 5.f;
	const char* kLabel = B_TRANSLATE_MARK("History");
}


FindStatusView::FindStatusView(BScrollView* scrollView, uint32 getMessage,
	uint32 clearMessage, uint32 applyMessage)
	:
	BView(BRect(), "statusview", B_FOLLOW_BOTTOM | B_FOLLOW_LEFT, B_WILL_DRAW),
	fScrollView(scrollView),
	fPreferredSize(0., 0.),
	fPressed(false),
	fButtonWidth(B_H_SCROLL_BAR_HEIGHT + kHorzSpacing * 2),
	fGetMessage(getMessage),
	fClearMessage(clearMessage),
	fApplyMessage(applyMessage)
{
}


FindStatusView::~FindStatusView()
{
}


void
FindStatusView::AttachedToWindow()
{
	SetFont(be_plain_font);
	SetFontSize(10.);

	fButtonWidth += StringWidth(B_TRANSLATE(kLabel));

	BScrollBar* scrollBar = fScrollView->ScrollBar(B_HORIZONTAL);
	MoveTo(0., scrollBar->Frame().top);

	SetViewUIColor(B_PANEL_BACKGROUND_COLOR);

	ResizeToPreferred();
}


void
FindStatusView::GetPreferredSize(float* _width, float* _height)
{
	_ValidatePreferredSize();

	if (_width)
		*_width = fPreferredSize.width;

	if (_height)
		*_height = fPreferredSize.height;
}


void
FindStatusView::ResizeToPreferred()
{
	float width, height;
	GetPreferredSize(&width, &height);

	if (Bounds().Width() > width)
		width = Bounds().Width();

	BView::ResizeTo(width, height);
}


void
FindStatusView::Draw(BRect updateRect)
{
	if (fPreferredSize.width <= 0)
		return;

	rgb_color highColor = HighColor();
	BRect bounds(Bounds());
	bounds.bottom = fPreferredSize.height;
	bounds.right = fPreferredSize.width;

	rgb_color base = tint_color(ViewColor(), B_DARKEN_2_TINT);

	// Navigation button
	BRect navRect(bounds);
	navRect.right = fButtonWidth;
	navRect.bottom--;
	_DrawNavigationButton(navRect);

	be_control_look->DrawScrollViewFrame(this, bounds, bounds, BRect(), BRect(),
		ViewColor(), B_FANCY_BORDER, 0,
		BControlLook::B_LEFT_BORDER | BControlLook::B_BOTTOM_BORDER);

	if(fScrollView->IsBorderHighlighted() && Window()->IsActive()) {
		SetHighUIColor(B_KEYBOARD_NAVIGATION_COLOR);
	} else {
		SetHighUIColor(B_PANEL_BACKGROUND_COLOR, B_DARKEN_2_TINT);
	}
	StrokeLine(bounds.LeftTop(), bounds.RightTop());

	// BControlLook mutates color
	SetHighColor(base);
	navRect.top++;
	StrokeLine(navRect.RightTop(), navRect.RightBottom());

	font_height fontHeight;
	GetFontHeight(&fontHeight);

	float x = 0.0f;
	float y = (bounds.bottom + bounds.top
		+ ceilf(fontHeight.ascent) - ceilf(fontHeight.descent)) / 2;
	SetHighUIColor(B_PANEL_TEXT_COLOR);
	DrawString(B_TRANSLATE(kLabel), BPoint(x + kHorzSpacing, y));
	SetHighColor(highColor);
}


void
FindStatusView::MouseDown(BPoint where)
{
	fPressed = true;
	Invalidate();
	_ShowHistoryMenu();
}


void
FindStatusView::WindowActivated(bool active)
{
	// Workaround: doesn't redraw automatically
	Invalidate();
}


void
FindStatusView::MessageReceived(BMessage* message)
{
	if(message->what == fGetMessage && message->IsReply()) {
		BPopUpMenu* menu = new BPopUpMenu("HistoryMenu", false, false);

		int32 count;
		if(message->GetInfo("mru", nullptr, &count) != B_OK) {
			BMenuItem* mi = new BMenuItem(B_TRANSLATE("<empty>"), nullptr);
			mi->SetEnabled(false);
			menu->AddItem(mi);
		} else {
			for(int32 i = count; i >= 0; --i) {
				BString item;
				if(message->FindString("mru", i, &item) == B_OK) {
					BMessage* msg = new BMessage((uint32) fApplyMessage);
					msg->AddInt32("item", i);
					BMenuItem* mi = new BMenuItem(item.String(), msg);
					menu->AddItem(mi);
				}
			}
			menu->AddSeparatorItem();
			BMenuItem* clear = new BMenuItem(B_TRANSLATE("Clear"),
				new BMessage((uint32) fClearMessage));
			menu->AddItem(clear);
			menu->SetTargetForItems(Window());
		}

		BPoint point = Parent()->Bounds().LeftBottom();
		point.x += 2; // border width
		point.y += 3 + B_H_SCROLL_BAR_HEIGHT;
		ConvertToScreen(&point);
		BRect clickToOpenRect(Parent()->Bounds());
		ConvertToScreen(&clickToOpenRect);
		menu->Go(point, true, true, clickToOpenRect);
		fPressed = false;
		delete menu;
	} else {
		BView::MessageReceived(message);
	}
}


void
FindStatusView::_ValidatePreferredSize()
{
	// width
	fPreferredSize.width = fButtonWidth;

	// height
	font_height fontHeight;
	GetFontHeight(&fontHeight);

	fPreferredSize.height = ceilf(fontHeight.ascent + fontHeight.descent
		+ fontHeight.leading);

	if (fPreferredSize.height < B_H_SCROLL_BAR_HEIGHT + 1)
		fPreferredSize.height = B_H_SCROLL_BAR_HEIGHT + 1;

	ResizeBy(fPreferredSize.width, 0);
	BScrollBar* scrollBar = fScrollView->ScrollBar(B_HORIZONTAL);
	float diff = scrollBar->Frame().left - fPreferredSize.width;
	if(fabs(diff) > 0.5) {
		scrollBar->ResizeBy(diff, 0);
		scrollBar->MoveBy(-diff, 0);
	}
}


void
FindStatusView::_DrawNavigationButton(BRect rect)
{
	rgb_color baseColor = tint_color(ui_color(B_PANEL_BACKGROUND_COLOR),
		B_LIGHTEN_1_TINT);
	uint32 flags = 0;
	if(fPressed)
		flags |= BControlLook::B_ACTIVATED;
	if(Window()->IsActive() == false)
		flags |= BControlLook::B_DISABLED;
	be_control_look->DrawButtonBackground(this, rect, rect, baseColor, flags,
		BControlLook::B_ALL_BORDERS, B_HORIZONTAL);
	rect.left += fButtonWidth - B_H_SCROLL_BAR_HEIGHT;
	be_control_look->DrawArrowShape(this, rect, rect, baseColor,
		BControlLook::B_DOWN_ARROW, flags, B_DARKEN_MAX_TINT);
}


void
FindStatusView::_ShowHistoryMenu()
{
	BMessenger windowMsgr(Window());
	windowMsgr.SendMessage(fGetMessage, this);
}
