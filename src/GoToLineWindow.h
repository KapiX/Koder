/*
 * Koder is a code editor for Haiku based on Scintilla.
 *
 * Copyright (C) 2014-2015 Kacper Kasper <kacperkasper@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
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
	
	static	filter_result	KeyDownFilter(BMessage* message, BHandler** target, BMessageFilter* messageFilter);
private:
			BTextControl*	fLine;
			BButton*		fGo;
			BButton*		fCancel;
	
			BWindow*		fOwner;
};


#endif // GOTOLINEWINDOW_H
