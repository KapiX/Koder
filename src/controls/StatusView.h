/*
 * Copyright 2002-2012, Haiku, Inc. All Rights Reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Vlad Slepukhin
 *		Siarzhuk Zharski
 *
 * Copied from Haiku commit a609673ce8c942d91e14f24d1d8832951ab27964.
 * Modifications:
 * Copyright 2018 Kacper Kasper <kacperkasper@gmail.com>
 * Distributed under the terms of the MIT License.
 */
#ifndef STATUS_VIEW_H
#define STATUS_VIEW_H


#include <Entry.h>
#include <String.h>
#include <View.h>


class BScrollView;

class StatusView : public BView {
public:
	enum {
		UPDATE_STATUS		= 'upda'
	};

							StatusView(BScrollView* fScrollView);
							~StatusView();

			void			SetStatus(BMessage* mesage);
			void			SetRef(const entry_ref& ref);
	virtual	void			AttachedToWindow();
	virtual void			GetPreferredSize(float* _width, float* _height);
	virtual	void			ResizeToPreferred();
	virtual	void			Draw(BRect bounds);
	virtual	void			MouseDown(BPoint point);
	virtual	void			WindowActivated(bool active);

private:
			void			_ValidatePreferredSize();
			void			_ShowDirMenu();
			void			_DrawNavigationButton(BRect rect);
			bool			_HasRef();

private:
	enum {
		kPositionCell,
		kTypeCell,
		kFileStateCell,
		kStatusCellCount
	};
			BScrollView*	fScrollView;
			BSize			fPreferredSize;
			BString			fCellText[kStatusCellCount];
			float			fCellWidth[kStatusCellCount];
			bool			fReadOnly;
			bool			fNavigationPressed;
			BString			fType;
			entry_ref		fRef;
	const	float			fNavigationButtonWidth;
};

#endif  // STATUS_VIEW_H
