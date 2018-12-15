/*
 * Copyright 2018 Kacper Kasper <kacperkasper@gmail.com>
 * All rights reserved. Distributed under the terms of the MIT license.
 *
 * Border drawing code lifted from kits/tracker/Navigator.cpp
 * Copyright 2015 John Scipione
 */


#include "ToolBar.h"

#include <ControlLook.h>


ToolBar::ToolBar()
	:
	BToolBar(B_HORIZONTAL)
{
	GroupLayout()->SetInsets(0.0f, 0.0f, B_USE_HALF_ITEM_INSETS, 1.0f);
		// 1px bottom inset used for border

	// Needed to draw the bottom border
	SetFlags(Flags() | B_WILL_DRAW);
	SetLowColor(ui_color(B_PANEL_BACKGROUND_COLOR));
}

void
ToolBar::Draw(BRect updateRect)
{
	// Draw a 1px bottom border, like BMenuBar
	BRect rect(Bounds());
	rgb_color base = LowColor();
	uint32 flags = 0;

	be_control_look->DrawBorder(this, rect, updateRect, base,
		B_PLAIN_BORDER, flags, BControlLook::B_BOTTOM_BORDER);

	BToolBar::Draw(rect & updateRect);
}
