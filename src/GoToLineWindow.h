/*
 * Copyright 2014-2017 Kacper Kasper <kacperkasper@gmail.com>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#ifndef GOTOLINEWINDOW_H
#define GOTOLINEWINDOW_H


#include <MessageFilter.h>
#include <Window.h>


class BButton;
class BTextControl;


enum {
	GTLW_CANCEL				= 'gtlc',
	GTLW_GO					= 'gtlg'
};


class GoToLineWindow : public BWindow {
public:
							GoToLineWindow(BWindow* owner);

			void			MessageReceived(BMessage* message);
			void			ShowCentered(BRect ownerRect);
			void			WindowActivated(bool active);
	
	static	filter_result	KeyDownFilter(BMessage* message, BHandler** target,
								BMessageFilter* messageFilter);

private:
			BTextControl*	fLine;
			BButton*		fGo;
			BButton*		fCancel;

			BWindow*		fOwner;
};


#endif // GOTOLINEWINDOW_H
