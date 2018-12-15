/*
 * Copyright 2018 Kacper Kasper <kacperkasper@gmail.com>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef TOOLBAR_H
#define TOOLBAR_H


#include <ToolBar.h>


class ToolBar : public BPrivate::BToolBar {
public:
	ToolBar();

	virtual void Draw(BRect updateRect);
};


#endif // TOOLBAR_H
