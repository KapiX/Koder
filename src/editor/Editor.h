/*
 * Copyright 2014-2018 Kacper Kasper <kacperkasper@gmail.com>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#ifndef EDITOR_H
#define EDITOR_H


#include <Message.h>

#include <ScintillaView.h>
#include <SciLexer.h>

#include <memory>
#include <string>
#include <vector>

#include "ScintillaUtils.h"


namespace editor {
	class StatusView;
}


enum {
	EDITOR_SAVEPOINT_LEFT		= 'svpl',
	EDITOR_SAVEPOINT_REACHED	= 'svpr',
	EDITOR_MODIFIED				= 'modi',
	EDITOR_CONTEXT_MENU			= 'conm',
	EDITOR_UPDATEUI				= 'updu'
};


class Editor : public BScintillaView {
public:
	enum Margin {
		NUMBER 		= 0,
		FOLD,
		BOOKMARKS,
		CHANGES
	};
	enum Marker {
		BOOKMARK	= 0
	};
	enum Indicator {
		WHITESPACE	= 0
	};

						Editor();

	virtual	void		DoLayout();
	virtual	void		FrameResized(float width, float height);

	void				NotificationReceived(SCNotification* notification);
	void				ContextMenu(BPoint point);

	void				SetType(std::string type);
	void				SetRef(const entry_ref& ref);
	void				SetReadOnly(bool readOnly);

	void				CommentLine(Scintilla::Range range);
	void				CommentBlock(Scintilla::Range range);

	void				SetCommentLineToken(std::string token);
	void				SetCommentBlockTokens(std::string start, std::string end);

	bool				CanCommentLine();
	bool				CanCommentBlock();

	void				HighlightTrailingWhitespace();
	void				ClearHighlightedWhitespace();
	void				TrimTrailingWhitespace();

	void				AppendNLAtTheEndIfNotPresent();

	void				UpdateLineNumberWidth();

	void				GoToLine(int64 line);

	void				SetBookmarks(const BMessage &lines);
	void				SetBookmarksFromSearch(const BMessage &searchMessage);
	BMessage			Bookmarks();
	BMessage			BookmarksWithText();

	bool				ToggleBookmark(int64 line = -1);
	void				GoToNextBookmark();
	void				GoToPreviousBookmark();

	void				SetNumberMarginEnabled(bool enabled);
	void				SetFoldMarginEnabled(bool enabled);
	void				SetBookmarkMarginEnabled(bool enabled);
	void				SetChangeMarginEnabled(bool enabled);
	void				SetBracesHighlightingEnabled(bool enabled);
	void				SetTrailingWSHighlightingEnabled(bool enabled);

	std::string			SelectionText();

	template<typename T>
	typename T::type	Get() { return T::Get(this); }
	template<typename T>
	void				Set(typename T::type value) { T::Set(this, value); }

private:
	void				_MaintainIndentation(char ch);
	void				_UpdateStatusView();
	void				_BraceHighlight();
	bool				_BraceMatch(int pos);
	void				_MarginClick(int margin, int pos);
	void				_HighlightTrailingWhitespace(Sci_Position start, Sci_Position end);
	std::string			_LineFeedString(int eolMode);

	void				_SetLineIndentation(int line, int indent);

	editor::StatusView*	fStatusView;

	std::string			fCommentLineToken;
	std::string			fCommentBlockStartToken;
	std::string			fCommentBlockEndToken;

	Sci_Position		fHighlightedWhitespaceStart;
	Sci_Position		fHighlightedWhitespaceEnd;
	Sci_Position		fHighlightedWhitespaceCurrentPos;

	bool				fNumberMarginEnabled;
	bool				fFoldMarginEnabled;
	bool				fBookmarkMarginEnabled;
	bool				fChangeMarginEnabled;
	bool				fBracesHighlightingEnabled;
	bool				fTrailingWSHighlightingEnabled;

	// needed for StatusView
	std::string			fType;
	bool				fReadOnly;
};


#endif // EDITOR_H
