/*
 * Koder is a code editor for Haiku based on Scintilla.
 *
 * Copyright (C) 2014-2015 Kacper Kasper <kacperkasper@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "AppPreferencesWindow.h"

#include <Box.h>
#include <Button.h>
#include <CheckBox.h>
#include <LayoutBuilder.h>

AppPreferencesWindow::AppPreferencesWindow(Preferences* preferences)
	:
	BWindow(BRect(0, 0, 400, 300), "Application preferences", B_TITLED_WINDOW, 0, 0)
{
	BBox* editor = new BBox("editorPrefs");
	editor->SetLabel("Editor");
	BCheckBox* tabsToSpaces = new BCheckBox("tabsToSpaces", "Convert tabs to spaces", new BMessage((uint32) 0));
	BCheckBox* lineHighlighting = new BCheckBox("lineHighlighting", "Highlight current line", new BMessage((uint32) 0));
	BCheckBox* lineNumbers = new BCheckBox("lineNumbers", "Display line numbers", new BMessage((uint32) 0));
	BButton* apply = new BButton("Apply");
	BLayoutBuilder::Group<>(editor, B_VERTICAL, 5)
		.Add(tabsToSpaces)
		.Add(lineHighlighting)
		.Add(lineNumbers)
		.AddGlue()
		.SetInsets(10, 15, 15, 10);
		
	BLayoutBuilder::Group<>(this, B_VERTICAL, 5)
		.Add(editor)
		.AddGroup(B_HORIZONTAL, 5)
			.AddGlue()
			.Add(apply)
		.End()
		.SetInsets(5, 5, 5, 5);

	CenterOnScreen();
}

AppPreferencesWindow::~AppPreferencesWindow()
{
}

