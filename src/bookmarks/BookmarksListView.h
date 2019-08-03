/*
 * Copyright 2019 Kacper Kasper <kacperkasper@gmail.com>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef BOOKMARKSLISTVIEW_H
#define BOOKMARKSLISTVIEW_H


#include <ListView.h>
#include <StringItem.h>
#include <String.h>


class BookmarksListView : public BListView {
public:
	BookmarksListView(const char* name);
	~BookmarksListView();

	void		AttachedToWindow();
	status_t	Invoke(BMessage* message = nullptr);
	void		MouseDown(BPoint where);

	void		UpdateBookmarks(const BMessage bookmarks);
private:
	class BookmarkItem : public BStringItem {
	public:
		BookmarkItem(int64 line, BString text);

		virtual void DrawItem(BView* owner, BRect frame, bool complete = false);

		int64 Line() const;
	private:
		int64 fLine;
		BString fText;
	};
	void				_ShowContextMenu(BPoint where);
};


#endif // BOOKMARKSLISTVIEW_H
