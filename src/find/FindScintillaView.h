/*
 * Copyright 2019 Kacper Kasper <kacperkasper@gmail.com>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#ifndef FINDSCINTILLAVIEW_H
#define FINDSCINTILLAVIEW_H


#include <ScintillaView.h>


namespace find {

class StatusView;


class ScintillaView : public BScintillaView
{
public:
	ScintillaView(const char* name, uint32 getMessage, uint32 clearMessage,
		uint32 applyMessage, uint32 flags = 0, bool horizontal = true,
		bool vertical = true, border_style border = B_FANCY_BORDER);

	virtual void DoLayout();
	virtual void FrameResized(float width, float height);
	virtual void MessageReceived(BMessage* message);

private:
	void _UpdateColors();

	StatusView* fStatusView;
};

} // namespace find

#endif // FINDSCINTILLAVIEW_H
