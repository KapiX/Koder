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
 * Copyright 2018-2019 Kacper Kasper <kacperkasper@gmail.com>
 * Distributed under the terms of the MIT License.
 */
#ifndef FIND_STATUS_VIEW_H
#define FIND_STATUS_VIEW_H


#include <Entry.h>
#include <String.h>
#include <View.h>

#include "StatusView.h"


class BScrollView;

namespace find {

class StatusView : public controls::StatusView {
public:
							StatusView(BScrollView* scrollView,
								uint32 getMessage, uint32 clearMessage,
								uint32 applyMessage);
							~StatusView();

	virtual	void			Draw(BRect bounds);
	virtual	void			MouseDown(BPoint point);
	virtual	void			MessageReceived(BMessage* message);

protected:
	virtual	float			Width();

private:
			void			_ShowHistoryMenu();
			void			_DrawButton(BRect rect);

private:
			bool			fPressed;
			float			fButtonWidth;
			uint32			fGetMessage;
			uint32			fClearMessage;
			uint32			fApplyMessage;
};

} // namespace find

#endif  // FIND_STATUS_VIEW_H
