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
 * Copyright 2018 Kacper Kasper <kacperkasper@gmail.com>
 * Distributed under the terms of the MIT License.
 */
#ifndef STATUS_VIEW_H
#define STATUS_VIEW_H


#include <Entry.h>
#include <String.h>
#include <View.h>


class BScrollView;

class FindStatusView : public BView {
public:
							FindStatusView(BScrollView* scrollView,
								uint32 getMessage, uint32 clearMessage,
								uint32 applyMessage);
							~FindStatusView();

	virtual	void			AttachedToWindow();
	virtual void			GetPreferredSize(float* _width, float* _height);
	virtual	void			ResizeToPreferred();
	virtual	void			Draw(BRect bounds);
	virtual	void			MouseDown(BPoint point);
	virtual	void			WindowActivated(bool active);
	virtual	void			MessageReceived(BMessage* message);

private:
			void			_ValidatePreferredSize();
			void			_ShowHistoryMenu();
			void			_DrawNavigationButton(BRect rect);

private:
			BScrollView*	fScrollView;
			BSize			fPreferredSize;
			bool			fReadOnly;
			bool			fPressed;
			float			fButtonWidth;
			uint32			fGetMessage;
			uint32			fClearMessage;
			uint32			fApplyMessage;
};

#endif  // STATUS_VIEW_H
