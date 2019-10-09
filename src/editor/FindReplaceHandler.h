/*
 * Copyright 2019 Kacper Kasper <kacperkasper@gmail.com>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef FINDREPLACEHANLDER_H
#define FINDREPLACEHANDLER_H


#include <string>

#include <Handler.h>
#include <Message.h>

#include "ScintillaUtils.h"


class BScintillaView;


class FindReplaceHandler : public BHandler {
public:
	enum {
		FIND		= 'find',
		REPLACE		= 'repl',
		REPLACEFIND	= 'fnrp',
		REPLACEALL	= 'rpla',
	};
					FindReplaceHandler(BScintillaView* editor,
						BHandler* replyHandler = nullptr);
	virtual void	MessageReceived(BMessage* message);

private:
	struct search_info {
		bool inSelection : 1;
		bool matchCase : 1;
		bool matchWord : 1;
		bool wrapAround : 1;
		bool backwards : 1;
		bool regex : 1;
		std::string find;
		std::string replace;

		bool operator ==(const search_info& rhs) const {
			return inSelection == rhs.inSelection
				&& matchCase == rhs.matchCase
				&& matchWord == rhs.matchWord
				&& wrapAround == rhs.wrapAround
				&& backwards == rhs.backwards
				&& regex == rhs.regex
				&& find == rhs.find
				&& replace == rhs.replace;
		}
		bool operator !=(const search_info& rhs) const {
			return !(*this == rhs);
		}
	};
	Sci_Position	_Find(std::string search, Sci_Position start,
							Sci_Position end, bool matchCase, bool matchWord,
							bool regex);
	search_info		_UnpackSearchMessage(BMessage& message);

	template<typename T>
	typename T::type	Get() { return T::Get(fEditor); }
	template<typename T>
	void				Set(typename T::type value) { T::Set(fEditor, value); }

	BScintillaView*	fEditor;
	BHandler*		fReplyHandler;

	Scintilla::Range	fSearchTarget;
	Scintilla::Range	fSearchLastResult;
	std::string			fSearchLast;
	int					fSearchLastFlags;
	bool				fNewSearch;
	search_info			fSearchLastInfo;
	bool				fIncrementalSearch;
	Scintilla::Range	fSavedSelection;
};


#endif // FINDREPLACEHANDLER_H
