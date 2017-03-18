/*
 * Copyright 2014-2017 Kacper Kasper <kacperkasper@gmail.com>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include "Editor.h"

#include <Messenger.h>

#include <algorithm>

#include "Preferences.h"


Editor::Editor()
	:
	BScintillaView("EditorView", 0, true, true, B_NO_BORDER),
	fCommentLineToken(""),
	fCommentBlockStartToken(""),
	fCommentBlockEndToken("")
{
}


void
Editor::NotificationReceived(SCNotification* notification)
{
	BMessenger window_msg(nullptr, (BLooper*) Window());
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


void
Editor::CommentLine(Sci_Position start, Sci_Position end)
{
	if(end < start) return;

	SendMessage(SCI_BEGINUNDOACTION, 0, 0);
	Sci_Position targetStart = SendMessage(SCI_GETTARGETSTART, 0, 0);
	Sci_Position targetEnd = SendMessage(SCI_GETTARGETEND, 0, 0);
	int startLine = SendMessage(SCI_LINEFROMPOSITION, start, 0);
	int endLine = SendMessage(SCI_LINEFROMPOSITION, end, 0);
	Sci_Position lineStartPos = SendMessage(SCI_POSITIONFROMLINE, startLine, 0);
	const size_t tokenLength = fCommentLineToken.length();
	const char* token = fCommentLineToken.c_str();

	// check for comment tokens at the beggining of the lines
	SendMessage(SCI_SETTARGETRANGE, lineStartPos, end);
	Sci_Position pos = SendMessage(SCI_SEARCHINTARGET, (uptr_t) tokenLength, (sptr_t) token);
	// check only the first line here, so fragments with one line comments can
	// be commented
	Sci_Position maxPos = SendMessage(SCI_GETLINEINDENTPOSITION, startLine, 0);
	if(pos != -1 && pos <= maxPos) {
		int charactersRemoved = 0;
		int line = startLine;
		while(pos != -1) {
			SendMessage(SCI_SETTARGETRANGE, SendMessage(SCI_POSITIONFROMLINE, line, 0), end - charactersRemoved);
			pos = SendMessage(SCI_SEARCHINTARGET, (uptr_t) tokenLength, (sptr_t) token);
			maxPos = SendMessage(SCI_GETLINEINDENTPOSITION, line, 0);
			line++;
			if(pos != -1 && pos <= maxPos) {
				SendMessage(SCI_REPLACETARGET, 0, (sptr_t) "");
				charactersRemoved += 2;
			}
		}
	} else {
		int addedCharacters = 0;
		while(startLine <= endLine) {
			Sci_Position linePos = SendMessage(SCI_POSITIONFROMLINE, startLine, 0);
			SendMessage(SCI_INSERTTEXT, linePos, (sptr_t) token);
			addedCharacters += tokenLength;
			startLine++;
		}
		_SetSelection(start + tokenLength, end + addedCharacters);
	}
	SendMessage(SCI_SETTARGETRANGE, targetStart, targetEnd);
	SendMessage(SCI_ENDUNDOACTION, 0, 0);
}


void
Editor::CommentBlock(Sci_Position start, Sci_Position end)
{
	if(start == end || end < start) return;

	const size_t startTokenLen = fCommentBlockStartToken.length();
	const size_t endTokenLen = fCommentBlockEndToken.length();
	bool startTokenPresent = true;
	bool endTokenPresent = true;
	for(int i = 0; i < startTokenLen; i++) {
		if(SendMessage(SCI_GETCHARAT, start + i, 0) != fCommentBlockStartToken[i])
			startTokenPresent = false;
	}
	for(int i = 0; i < endTokenLen; i++) {
		if(SendMessage(SCI_GETCHARAT, end - endTokenLen + i, 0) != fCommentBlockEndToken[i])
			endTokenPresent = false;
	}

	SendMessage(SCI_BEGINUNDOACTION, 0, 0);
	if(startTokenPresent && endTokenPresent) {
		// order is important here
		SendMessage(SCI_DELETERANGE, end - endTokenLen, endTokenLen);
		SendMessage(SCI_DELETERANGE, start, startTokenLen);
		_SetSelection(start, end - startTokenLen - endTokenLen);
	} else {
		SendMessage(SCI_INSERTTEXT, start, (sptr_t) fCommentBlockStartToken.c_str());
		SendMessage(SCI_INSERTTEXT, end + startTokenLen, (sptr_t) fCommentBlockEndToken.c_str());
		_SetSelection(start, end + startTokenLen + endTokenLen);
	}
	SendMessage(SCI_ENDUNDOACTION, 0, 0);
}


void
Editor::SetCommentLineToken(std::string token)
{
	fCommentLineToken = token;
}


void
Editor::SetCommentBlockTokens(std::string start, std::string end)
{
	fCommentBlockStartToken = start;
	fCommentBlockEndToken = end;
}


bool
Editor::CanCommentLine()
{
	return fCommentLineToken != "";
}


bool
Editor::CanCommentBlock()
{
	return fCommentBlockStartToken != "" && fCommentBlockEndToken != "";
}


// borrowed from SciTE
// Copyright (c) Neil Hodgson
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
		Sci_Position pos = SendMessage(SCI_GETCURRENTPOS, 0, 0);
		// highlight indent guide
		int line = SendMessage(SCI_LINEFROMPOSITION, pos, 0);
		int indentation = SendMessage(SCI_GETLINEINDENTATION, line, 0);
		SendMessage(SCI_SETHIGHLIGHTGUIDE, indentation, 0);
		// highlight braces
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
// Copyright (c) Neil Hodgson
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
