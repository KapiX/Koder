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

#include "Editor.h"

#include <Messenger.h>

#include <algorithm>

#include "Preferences.h"

Editor::Editor()
	:
	BScintillaView("EditorView", 0, true, true)
{
}

void
Editor::NotificationReceived(SCNotification* notification)
{
	BMessenger window_msg(NULL, (BLooper*) Window());
	switch(notification->nmhdr.code) {
		case SCN_SAVEPOINTLEFT:
			window_msg.SendMessage(EDITOR_SAVEPOINT_LEFT);
		break;
		case SCN_SAVEPOINTREACHED:
			window_msg.SendMessage(EDITOR_SAVEPOINT_REACHED);
		break;
		case SCN_PAINTED:
			_UpdateLineNumberWidth();
		break;
	}
}

// borrowed from Notepad++
void
Editor::_UpdateLineNumberWidth()
{
	if(fPreferences->fLineNumbers) {
		int linesVisible = SendMessage(SCI_LINESONSCREEN, 0, 0);
		if(linesVisible) {
			int firstVisibleLineVis = SendMessage(SCI_GETFIRSTVISIBLELINE, 0, 0);
			int lastVisibleLineVis = linesVisible + firstVisibleLineVis + 1;

			if(SendMessage(SCI_GETWRAPMODE, 0, 0) != SC_WRAP_NONE) {
				int numLinesDoc = SendMessage(SCI_GETLINECOUNT, 0, 0);
				int prevLineDoc = SendMessage(SCI_DOCLINEFROMVISIBLE, firstVisibleLineVis, 0);
				for(int i = firstVisibleLineVis + 1; i <= lastVisibleLineVis; ++i) {
					int lineDoc = SendMessage(SCI_DOCLINEFROMVISIBLE, i, 0);
					if(lineDoc == numLinesDoc)
						break;
					if(lineDoc == prevLineDoc)
						lastVisibleLineVis++;
					prevLineDoc = lineDoc;
				}
			}

			int lastVisibleLineDoc = SendMessage(SCI_DOCLINEFROMVISIBLE, lastVisibleLineVis, 0);
			int i = 0;

			while(lastVisibleLineDoc) {
				lastVisibleLineDoc /= 10;
				++i;
			}

			i = std::max(i, 3);
			int pixelWidth = 8 + i * SendMessage(SCI_TEXTWIDTH, STYLE_LINENUMBER, (sptr_t) "8");
			SendMessage(SCI_SETMARGINWIDTHN, Margin::NUMBER, pixelWidth);
		}
	}
}
