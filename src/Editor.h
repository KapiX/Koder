/*
 * Copyright 2014-2017 Kacper Kasper <kacperkasper@gmail.com>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#ifndef EDITOR_H
#define EDITOR_H


#include <ScintillaView.h>
#include <SciLexer.h>

#include <string>


class Preferences;


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
		FOLD
	};
	enum Indicator {
		WHITESPACE	= 0
	};

						Editor();

	void				NotificationReceived(SCNotification* notification);
	void				ContextMenu(BPoint point);

	void				SetPreferences(Preferences* preferences);

	void				CommentLine(Sci_Position start, Sci_Position end);
	void				CommentBlock(Sci_Position start, Sci_Position end);

	void				SetCommentLineToken(std::string token);
	void				SetCommentBlockTokens(std::string start, std::string end);

	bool				CanCommentLine();
	bool				CanCommentBlock();

	void				HighlightTrailingWhitespace();
	void				ClearHighlightedWhitespace();
	void				TrimTrailingWhitespace();

	bool				Find(std::string search, bool matchCase, bool matchWord,
							bool backwards, bool wrapAround, bool inSelection,
							bool regex = false);
	void				Replace(std::string replacement, bool regex = false);
	int					ReplaceAll(std::string search, std::string replacement,
							bool matchCase, bool matchWord, bool inSelection,
							bool regex = false);
	void				ResetFindReplace();

private:
	void				_MaintainIndentation(char ch);
	void				_UpdateLineNumberWidth();
	void				_BraceHighlight();
	bool				_BraceMatch(int pos);
	void				_MarginClick(int margin, int pos);
	void				_HighlightTrailingWhitespace(Sci_Position start, Sci_Position end);

	void				_SetLineIndentation(int line, int indent);
	Sci_CharacterRange	_GetSelection();
	void				_SetSelection(int anchor, int currentPos);

	bool				_Find(std::string search, Sci_Position start,
							Sci_Position end, bool matchCase, bool matchWord,
							bool regex = false);

	Preferences*		fPreferences;

	std::string			fCommentLineToken;
	std::string			fCommentBlockStartToken;
	std::string			fCommentBlockEndToken;

	Sci_Position		fHighlightedWhitespaceStart;
	Sci_Position		fHighlightedWhitespaceEnd;
	Sci_Position		fHighlightedWhitespaceCurrentPos;

	Sci_Position		fSearchTargetStart;
	Sci_Position		fSearchTargetEnd;
	Sci_Position		fSearchLastResultStart;
	Sci_Position		fSearchLastResultEnd;
	std::string			fSearchLast;
	int					fSearchLastFlags;
	bool				fNewSearch;
};


#endif // EDITOR_H
