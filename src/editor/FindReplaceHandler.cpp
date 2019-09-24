/*
 * Copyright 2019 Kacper Kasper <kacperkasper@gmail.com>
 * All rights reserved. Distributed under the terms of the MIT license.
 */


#include "FindReplaceHandler.h"

#include <Message.h>
#include <Messenger.h>
#include <ScintillaView.h>


namespace Sci = Scintilla;
using namespace Sci::Properties;


FindReplaceHandler::FindReplaceHandler(BScintillaView* editor)
	:
	fEditor(editor),
	fSearchTarget(-1, -1),
	fSearchLastResult(-1, -1),
	fSearchLast(""),
	fSearchLastFlags(0)
{
}


void
FindReplaceHandler::MessageReceived(BMessage* message)
{
	const search_info info = _UnpackSearchMessage(*message);

	const bool inSelection = info.inSelection;
	const bool matchCase = info.matchCase;
	const bool matchWord = info.matchWord;
	const bool wrapAround = info.wrapAround;
	const bool backwards = info.backwards;
	const bool regex = info.regex;
	const std::string find = info.find;
	const std::string replace = info.replace;

	Sci::Guard<SearchTarget, SearchFlags> guard(fEditor);

	const int length = fEditor->SendMessage(SCI_GETLENGTH);
	const Sci_Position anchor = fEditor->SendMessage(SCI_GETANCHOR);
	const Sci_Position current = fEditor->SendMessage(SCI_GETCURRENTPOS);

	switch(message->what) {
		case FIND: {
			if((fSearchLastInfo.backwards == true && (anchor != fSearchLastResult.first
					|| current != fSearchLastResult.second))
				|| (fSearchLastInfo.backwards == false && (anchor != fSearchLastResult.second
					|| current != fSearchLastResult.first))
				|| info != fSearchLastInfo) {
				fNewSearch = true;
			}

			if(fNewSearch == true) {
				if(inSelection == true) {
					fSearchTarget = Get<Selection>();
					if(backwards == true) {
						std::swap(fSearchTarget.first, fSearchTarget.second);
					}
				} else {
					fSearchTarget = backwards ? Sci::Range(anchor, 0) : Sci::Range(current, length);
				}
			}

			auto temp = fSearchTarget;

			if(fNewSearch == false) {
				temp.first = current;
			}

			Sci_Position pos = _Find(find, temp.first, temp.second, matchCase,
				matchWord, regex);
			fSearchLastResult = Get<SearchTarget>();
			if(backwards == true) {
				std::swap(fSearchLastResult.first, fSearchLastResult.second);
			}
			fEditor->SendMessage(SCI_SETANCHOR, fSearchLastResult.first);
			fEditor->SendMessage(SCI_SETCURRENTPOS, fSearchLastResult.second);

			if(pos == -1 && wrapAround == true) {
				Sci_Position startAgain;
				if(inSelection == true) {
					startAgain = fSearchTarget.first;
				} else {
					startAgain = (backwards ? length : 0);
				}
				pos = _Find(find, startAgain, fSearchTarget.second, matchCase,
					matchWord, regex);
				if(pos != -1) {
					fSearchLastResult = Get<SearchTarget>();
					if(backwards == true) {
						std::swap(fSearchLastResult.first, fSearchLastResult.second);
					}
					fEditor->SendMessage(SCI_SETANCHOR, fSearchLastResult.first);
					fEditor->SendMessage(SCI_SETCURRENTPOS, fSearchLastResult.second);
				}
			}
			fNewSearch = false;
			fSearchLastInfo = info;
		} break;
		case REPLACE: {
			int replaceMsg = (regex ? SCI_REPLACETARGETRE : SCI_REPLACETARGET);
			if(fSearchLastResult != Sci::Range{ -1, -1 }) {
				// we need to search again, because whitespace highlighting messes with
				// the results
				Set<SearchFlags>(fSearchLastFlags);
				Set<SearchTarget>(fSearchLastResult);
				fEditor->SendMessage(SCI_SEARCHINTARGET, (uptr_t) fSearchLast.size(), (sptr_t) fSearchLast.c_str());
				fEditor->SendMessage(replaceMsg, -1, (sptr_t) replace.c_str());
				Sci::Range target = Get<SearchTarget>();
				if(fSearchLastInfo.backwards == true) {
					std::swap(target.first, target.second);
				}
				fEditor->SendMessage(SCI_SETANCHOR, target.first);
				fEditor->SendMessage(SCI_SETCURRENTPOS, target.second);
				fSearchLastResult = { -1, -1 };
			}
		} break;
	}
}


Sci_Position
FindReplaceHandler::_Find(std::string search, Sci_Position start,
	Sci_Position end, bool matchCase, bool matchWord, bool regex)
{
	int searchFlags = 0;
	if(matchCase == true)
		searchFlags |= SCFIND_MATCHCASE;
	if(matchWord == true)
		searchFlags |= SCFIND_WHOLEWORD;
	if(regex == true)
		searchFlags |= SCFIND_REGEXP | SCFIND_CXX11REGEX;
	Set<SearchFlags>(searchFlags);
	fSearchLastFlags = searchFlags;

	Set<SearchTarget>({start, end});

	fSearchLast = search;
	Sci_Position pos = fEditor->SendMessage(SCI_SEARCHINTARGET,
		(uptr_t) search.size(), (sptr_t) search.c_str());
	return pos;
}


FindReplaceHandler::search_info
FindReplaceHandler::_UnpackSearchMessage(BMessage& message)
{
	search_info info;
	info.inSelection = message.GetBool("inSelection");
	info.matchCase = message.GetBool("matchCase");
	info.matchWord = message.GetBool("matchWord");
	info.wrapAround = message.GetBool("wrapAround");
	info.backwards = message.GetBool("backwards");
	info.regex = message.GetBool("regex");
	info.find = message.GetString("findText", "");
	info.replace = message.GetString("replaceText", "");
	return info;
}
