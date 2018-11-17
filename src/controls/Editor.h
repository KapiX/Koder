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


class Preferences;
class StatusView;


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
		BOOKMARKS
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

	void				SetPreferences(Preferences* preferences);

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

	bool				Find(BMessage* message);
	void				FindNext();
	void				FindSelection();
	void				Replace(std::string replacement, bool regex = false);
	int					ReplaceAll(std::string search, std::string replacement,
							bool matchCase, bool matchWord, bool inSelection,
							bool regex = false);
	void				ReplaceAndFind();
	void				ResetFindReplace();
	void				IncrementalSearch(std::string term);
	void				IncrementalSearchCancel();
	void				IncrementalSearchCommit(std::string term);

	void				UpdateLineNumberWidth();

	void				SetBookmarks(const BMessage &lines);
	BMessage			Bookmarks();

	void				ToggleBookmark(int64 line = -1);
	void				NextBookmark();
	void				PreviousBookmark();

	void				SetNumberMarginEnabled(bool enabled);
	void				SetFoldMarginEnabled(bool enabled);
	void				SetBookmarkMarginEnabled(bool enabled);

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

	void				_SetLineIndentation(int line, int indent);

	bool				_Find(std::string search, Sci_Position start,
							Sci_Position end, bool matchCase, bool matchWord,
							bool regex = false);

	Preferences*		fPreferences;
	StatusView*			fStatusView;

	std::string			fCommentLineToken;
	std::string			fCommentBlockStartToken;
	std::string			fCommentBlockEndToken;

	Sci_Position		fHighlightedWhitespaceStart;
	Sci_Position		fHighlightedWhitespaceEnd;
	Sci_Position		fHighlightedWhitespaceCurrentPos;

	Scintilla::Range	fSearchTarget;
	Scintilla::Range	fSearchLastResult;
	std::string			fSearchLast;
	int					fSearchLastFlags;
	bool				fNewSearch;
	BMessage			fSearchLastMessage;
	bool				fIncrementalSearch;
	Scintilla::Range	fSavedSelection;

	bool				fNumberMarginEnabled;
	bool				fFoldMarginEnabled;
	bool				fBookmarkMarginEnabled;

	// needed for StatusView
	std::string			fType;
	bool				fReadOnly;
};


#endif // EDITOR_H
