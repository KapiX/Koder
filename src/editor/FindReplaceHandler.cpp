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


FindReplaceHandler::FindReplaceHandler(BScintillaView* editor,
	BHandler* replyHandler)
	:
	fEditor(editor),
	fReplyHandler(replyHandler),
	fSearchTarget(-1, -1),
	fSearchLastResult(-1, -1),
	fSearchLast(""),
	fSearchLastFlags(0)
{
}


void
FindReplaceHandler::MessageReceived(BMessage* message)
{
	search_info info = _UnpackSearchMessage(*message);

	Sci::Guard<SearchTarget, SearchFlags> guard(fEditor);

	const int length = fEditor->SendMessage(SCI_GETLENGTH);
	const Sci_Position anchor = fEditor->SendMessage(SCI_GETANCHOR);
	const Sci_Position current = fEditor->SendMessage(SCI_GETCURRENTPOS);
	switch(message->what) {
		case REPLACEFIND:
			// fallthrough
		case REPLACE: {
			int replaceMsg = (info.regex ? SCI_REPLACETARGETRE : SCI_REPLACETARGET);
			if(fSearchLastResult != Sci::Range{ -1, -1 }) {
				// we need to search again, because whitespace highlighting messes with
				// the results
				Set<SearchFlags>(fSearchLastFlags);
				Set<SearchTarget>(fSearchLastResult);
				fEditor->SendMessage(SCI_SEARCHINTARGET, (uptr_t) fSearchLastInfo.find.size(), (sptr_t) fSearchLastInfo.find.c_str());
				fEditor->SendMessage(replaceMsg, -1, (sptr_t) info.replace.c_str());
				Sci::Range target = Get<SearchTarget>();
				if(fSearchLastInfo.backwards == true) {
					std::swap(target.first, target.second);
				}
				fEditor->SendMessage(SCI_SETANCHOR, target.first);
				fEditor->SendMessage(SCI_SETCURRENTPOS, target.second);
				fSearchLastResult = { -1, -1 };
			}
		}
		if(message->what != REPLACEFIND) break;
		case FIND: {
			if((fSearchLastInfo.backwards == true && (anchor != fSearchLastResult.first
					|| current != fSearchLastResult.second))
				|| (fSearchLastInfo.backwards == false && (anchor != fSearchLastResult.second
					|| current != fSearchLastResult.first))
				|| info != fSearchLastInfo) {
				fNewSearch = true;
			}
			if(message->what == REPLACEFIND) {
				info = fSearchLastInfo;
			}

			if(fNewSearch == true) {
				if(info.inSelection == true) {
					fSearchTarget = Get<Selection>();
					if(info.backwards == true) {
						std::swap(fSearchTarget.first, fSearchTarget.second);
					}
				} else {
					fSearchTarget = info.backwards ? Sci::Range(current, 0) : Sci::Range(current, length);
				}
			}

			auto temp = fSearchTarget;

			if(fNewSearch == false) {
				temp.first = current;
			}

			Sci_Position pos = _Find(info.find, temp.first, temp.second,
				info.matchCase, info.matchWord, info.regex);

			if(pos == -1 && info.wrapAround == true) {
				Sci_Position startAgain;
				if(info.inSelection == true) {
					startAgain = fSearchTarget.first;
				} else {
					startAgain = (info.backwards ? length : 0);
				}
				pos = _Find(info.find, startAgain, fSearchTarget.second,
					info.matchCase, info.matchWord, info.regex);
			}
			if(pos != -1) {
				fSearchLastResult = Get<SearchTarget>();
				if(info.backwards == true) {
					std::swap(fSearchLastResult.first, fSearchLastResult.second);
				}
				fEditor->SendMessage(SCI_SETANCHOR, fSearchLastResult.first);
				fEditor->SendMessage(SCI_SETCURRENTPOS, fSearchLastResult.second);
				fEditor->SendMessage(SCI_SCROLLCARET);
			}
			if(fReplyHandler != nullptr) {
				BMessage reply(FIND);
				reply.AddBool("found", pos != -1);
				message->SendReply(&reply, fReplyHandler);
			}
			fNewSearch = false;
			fSearchLastInfo = info;
		} break;
		case REPLACEALL: {
			Sci::UndoAction action(fEditor);
			int replaceMsg = (info.regex ? SCI_REPLACETARGETRE : SCI_REPLACETARGET);
			int occurences = 0;
			fEditor->SendMessage(info.inSelection ? SCI_TARGETFROMSELECTION : SCI_TARGETWHOLEDOCUMENT);
			auto target = Get<SearchTarget>();
			Sci_Position pos;
			do {
				pos = _Find(info.find, target.first, target.second,
					info.matchCase, info.matchWord, info.regex);
				if(pos != -1) {
					fEditor->SendMessage(replaceMsg, -1, (sptr_t) info.replace.c_str());
					target.first = Get<SearchTargetEnd>();
					target.second = fEditor->SendMessage(SCI_GETLENGTH);
					occurences++;
				}
			} while(pos != -1);
			if(fReplyHandler != nullptr) {
				BMessage reply(REPLACEALL);
				reply.AddInt32("replaced", occurences);
				message->SendReply(&reply, fReplyHandler);
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
