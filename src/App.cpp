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

#include "App.h"

#include <Alert.h>
#include <Entry.h>
#include <File.h>

#include "AppPreferencesWindow.h"
#include "EditorWindow.h"
#include "Preferences.h"

App::App()
	:
	BApplication(gAppMime),
	fLastActiveWindow(NULL),
	fPreferences(NULL)
{
	// TODO: use BPathFinder
	fPreferences = new Preferences("/boot/home/config/settings/Koder");
	fPreferences->Load();
	EditorWindow::SetPreferences(fPreferences);
}

App::~App()
{
	EditorWindow* window;
	while(fWindows.CountItems() > 0) {
		window = fWindows.RemoveItemAt(0);
		if(window->LockLooper())
		{
			window->Quit();
		}
	}
	
	delete fPreferences;
}

void
App::AboutRequested()
{
	BAlert* alert = new BAlert("About", "Koder\nAuthor: Kacper Kasper", "Close", NULL, NULL, B_WIDTH_AS_USUAL, B_INFO_ALERT);
	alert->Go();
}

bool
App::QuitRequested()
{
	// TODO: Window asking to save changes in many windows
	//       will usually be called when application closing
	//       from Tracker
}

void
App::ReadyToRun()
{
	if(CountWindows() == 0) {
		PostMessage(WINDOW_NEW);
	}
}

void
App::RefsReceived(BMessage* message)
{
	entry_ref ref;
	if(message->FindRef("refs", &ref) == B_OK) {
		EditorWindow* window = new EditorWindow();
		window->OpenFile(&ref);
		window->Show();
		fWindows.AddItem(window);
	}
}

void
App::MessageReceived(BMessage* message)
{
	switch(message->what) {
	case MAINMENU_EDIT_APP_PREFERENCES: {
		fAppPreferencesWindow = new AppPreferencesWindow(fPreferences);
		fAppPreferencesWindow->Show();
	} break;
	case WINDOW_NEW: {
		EditorWindow* window = new EditorWindow();
		window->Show();
		fWindows.AddItem(window);
	} break;
	case WINDOW_CLOSE: {
		EditorWindow* window;
		if(message->FindPointer("window", (void**) &window) == B_OK) {
			fWindows.RemoveItem(window);
		}
		if(fWindows.CountItems() == 0) {
			Quit();
		}
	} break;
	default:
		BApplication::MessageReceived(message);
	break;
	}
}
