/*
 * Copyright 2019 Kacper Kasper <kacperkasper@gmail.com>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef BOOKMARKWINDOW_H
#define BOOKMARKWINDOW_H


#include <Window.h>


class BScrollView;
class BookmarksListView;


enum {
	BOOKMARK_ADDED		= 'bkma',
	BOOKMARK_REMOVED	= 'bkmr',
	BOOKMARKS_INVALIDATED	= 'bkmi',
	BOOKMARKS_WINDOW_QUITTING	= 'bkqt',
};


class BookmarksWindow : public BWindow {
public:
	BookmarksWindow(BWindow* owner, const BMessage bookmarks);
	~BookmarksWindow();

	void			MessageReceived(BMessage* message);
	void			Quit();
private:
	BScrollView*		fScroller;
	BookmarksListView*	fList;
	BWindow*			fOwner;
};


#endif // BOOKMARKWINDOW_H
