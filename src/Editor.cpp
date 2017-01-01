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
	BScintillaView("EditorView", 0, true, true, B_NO_BORDER)
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
		case SCN_CHARADDED: {
			char ch = static_cast<char>(notification->ch);
			_MaintainIndentation(ch);
		} break;
		case SCN_UPDATEUI:
			_BraceHighlight();
			_UpdateLineNumberWidth();
		break;
		case SCN_MARGINCLICK:
			_MarginClick(notification->margin, notification->position);
		break;
	}
}


void
Editor::SetPreferences(Preferences* preferences)
{
	fPreferences = preferences;
}


// borrowed from SciTE
void
Editor::_MaintainIndentation(char ch)
{
	int eolMode = SendMessage(SCI_GETEOLMODE, 0, 0);
	int currentLine = SendMessage(SCI_LINEFROMPOSITION, SendMessage(SCI_GETCURRENTPOS, 0, 0), 0);
	int lastLine = currentLine - 1;

	if(((eolMode == SC_EOL_CRLF || eolMode == SC_EOL_LF) && ch == '\n') ||
		(eolMode == SC_EOL_CR && ch == '\r')) {
		int indentAmount = 0;
		if(lastLine >= 0) {
			indentAmount = SendMessage(SCI_GETLINEINDENTATION, lastLine, 0);
		}
		if(indentAmount > 0) {
			_SetLineIndentation(currentLine, indentAmount);
		}
	}
}


void
Editor::_UpdateLineNumberWidth()
{
	if(fPreferences->fLineNumbers) {
		int numLines = SendMessage(SCI_GETLINECOUNT, 0, 0);
		int i;
		for(i = 1; numLines > 0; numLines /= 10, ++i);
		int charWidth = SendMessage(SCI_TEXTWIDTH, STYLE_LINENUMBER, (sptr_t) "0");
		SendMessage(SCI_SETMARGINWIDTHN, Margin::NUMBER, std::max(i, 3) * charWidth);
	}
}


void
Editor::_BraceHighlight()
{
	if(fPreferences->fBracesHighlighting == true) {
		int pos = SendMessage(SCI_GETCURRENTPOS, 0, 0);
		if(_BraceMatch(pos - 1) == false) {
			_BraceMatch(pos);
		}
	} else {
		SendMessage(SCI_BRACEBADLIGHT, -1, 0);
	}
}


bool
Editor::_BraceMatch(int pos)
{
	char ch = SendMessage(SCI_GETCHARAT, pos, 0);
	if(ch == '(' || ch == ')' || ch == '[' || ch == ']' || ch == '{' || ch == '}') {
		int match = SendMessage(SCI_BRACEMATCH, pos, 0);
		if(match == -1) {
			SendMessage(SCI_BRACEBADLIGHT, pos, 0);
		} else {
			SendMessage(SCI_BRACEHIGHLIGHT, pos, match);
		}
	} else {
		SendMessage(SCI_BRACEBADLIGHT, -1, 0);
		return false;
	}
	return true;
}


void
Editor::_MarginClick(int margin, int pos)
{
	switch(margin) {
		case Margin::FOLD: {
			int lineNumber = SendMessage(SCI_LINEFROMPOSITION, pos, 0);
			SendMessage(SCI_TOGGLEFOLD, lineNumber, 0);
		} break;
	}
}


// borrowed from SciTE
void
Editor::_SetLineIndentation(int line, int indent)
{
	if(indent < 0)
		return;

	Sci_CharacterRange crange = _GetSelection();
	Sci_CharacterRange crangeStart = crange;
	int posBefore = SendMessage(SCI_GETLINEINDENTPOSITION, line, 0);
	SendMessage(SCI_SETLINEINDENTATION, line, indent);
	int posAfter = SendMessage(SCI_GETLINEINDENTPOSITION, line, 0);
	int posDifference = posAfter - posBefore;
	if(posAfter > posBefore) {
		if(crange.cpMin >= posBefore) {
			crange.cpMin += posDifference;
		}
		if(crange.cpMax >= posBefore) {
			crange.cpMax += posDifference;
		}
	} else if(posAfter < posBefore) {
		if(crange.cpMin >= posAfter) {
			if(crange.cpMin >= posBefore) {
				crange.cpMin += posDifference;
			} else {
				crange.cpMin = posAfter;
			}
		}
		if(crange.cpMax >= posAfter) {
			if(crange.cpMax >= posBefore) {
				crange.cpMax += posDifference;
			} else {
				crange.cpMax = posAfter;
			}
		}
	}
	if((crangeStart.cpMin != crange.cpMin) || (crangeStart.cpMax != crange.cpMax)) {
		_SetSelection(static_cast<int>(crange.cpMin), static_cast<int>(crange.cpMax));
	}
}


Sci_CharacterRange
Editor::_GetSelection()
{
	Sci_CharacterRange crange;
	crange.cpMin = SendMessage(SCI_GETSELECTIONSTART, 0, 0);
	crange.cpMax = SendMessage(SCI_GETSELECTIONEND, 0, 0);
	return crange;
}


void
Editor::_SetSelection(int anchor, int currentPos)
{
	SendMessage(SCI_SETSEL, anchor, currentPos);
}
