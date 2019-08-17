/*
 * Copyright 2019 Kacper Kasper <kacperkasper@gmail.com>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include "FindScintillaView.h"

#include <ControlLook.h>

#include "FindStatusView.h"
#include "Utils.h"


namespace find {

ScintillaView::ScintillaView(const char* name, uint32 getMessage,
	uint32 clearMessage, uint32 applyMessage, uint32 flags, bool horizontal,
	bool vertical, border_style border)
	:
	BScintillaView(name, flags, horizontal, vertical, border)
{
	_UpdateColors();
	fStatusView = new find::StatusView(this,
		getMessage, clearMessage, applyMessage);
}


void
ScintillaView::DoLayout()
{
	BScintillaView::DoLayout();

	fStatusView->ResizeToPreferred();
}


void
ScintillaView::FrameResized(float width, float height)
{
	BScintillaView::FrameResized(width, height);

	fStatusView->ResizeToPreferred();
}


void
ScintillaView::MessageReceived(BMessage* message)
{
	switch(message->what) {
		case B_COLORS_UPDATED: {
			_UpdateColors();
		} break;
		default: {
			BScintillaView::MessageReceived(message);
		} break;
	}
}


void
ScintillaView::_UpdateColors()
{
	rgb_color fore = ui_color(B_DOCUMENT_TEXT_COLOR);
	rgb_color back = ui_color(B_DOCUMENT_BACKGROUND_COLOR);
	SendMessage(SCI_STYLESETFORE, STYLE_DEFAULT, rgb_colorToSciColor(fore));
	SendMessage(SCI_STYLESETBACK, STYLE_DEFAULT, rgb_colorToSciColor(back));
	SendMessage(SCI_STYLESETFORE, 0, rgb_colorToSciColor(fore));
	SendMessage(SCI_STYLESETBACK, 0, rgb_colorToSciColor(back));
}

} // namespace find
