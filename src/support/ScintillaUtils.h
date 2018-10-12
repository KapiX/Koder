/*
 * Copyright 2018 Kacper Kasper <kacperkasper@gmail.com>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#ifndef SCINTILLAUTILS_H
#define SCINTILLAUTILS_H


#include <ScintillaView.h>


namespace Scintilla {

template<typename Type, int GetMessageId, int SetMessageId>
class Property {
public:
	static Type Get(BScintillaView* view) {
		return view->SendMessage(GetMessageId);
	}
	static void Set(BScintillaView* view, Type value) {
		view->SendMessage(SetMessageId, value);
	}
	typedef Type type;
};


namespace Properties {

typedef Property<int, SCI_GETSEARCHFLAGS, SCI_SETSEARCHFLAGS>
	SearchFlags;
typedef Property<Sci_Position, SCI_GETTARGETSTART, SCI_SETTARGETSTART>
	TargetStart;
typedef Property<Sci_Position, SCI_GETTARGETEND, SCI_SETTARGETEND>
	TargetEnd;
typedef Property<int, SCI_GETINDICATORCURRENT, SCI_SETINDICATORCURRENT>
	CurrentIndicator;

}


template<typename ...Ts>
class Guard {
public:
	Guard(BScintillaView* view) {}
};


template<typename T, typename ...Ts>
class Guard<T, Ts...> : public Guard<Ts...> {
public:
	Guard(BScintillaView* view)
		: Guard<Ts...>(view), fView(view) {
		fValue = T::Get(fView);
	}
	virtual ~Guard() {
		T::Set(fView, fValue);
	}
private:
	BScintillaView* fView;
	typename T::type fValue;
};


class UndoAction {
public:
	UndoAction(BScintillaView* view)
		: fView(view) {
		fView->SendMessage(SCI_BEGINUNDOACTION);
	}
	~UndoAction() {
		fView->SendMessage(SCI_ENDUNDOACTION);
	}
private:
	BScintillaView* fView;
};

}


#endif // SCINTILLAUTILS_H
