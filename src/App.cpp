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
#include <Directory.h>
#include <Entry.h>
#include <File.h>
#include <FindDirectory.h>
#include <Path.h>

#include "AppPreferencesWindow.h"
#include "EditorWindow.h"
#include "FindWindow.h"
#include "Preferences.h"
#include "Styler.h"


App::App()
	:
	BApplication(gAppMime),
	fLastActiveWindow(NULL),
	fAppPreferencesWindow(nullptr),
	fFindWindow(nullptr),
	fPreferences(NULL),
	fStyler(NULL)
{
}


App::~App()
{
	EditorWindow* window;
	while(fWindows.CountItems() > 0) {
		window = fWindows.RemoveItemAt(0);
		if(fWindows.CountItems() == 0) {
			fPreferences->fWindowRect = window->Frame();
		}
		if(window->LockLooper())
		{
			window->Quit();
		}
	}

	fPreferences->Save(fPreferencesFile.Path());
	delete fPreferences;
	delete fStyler;
}


void
App::Init()
{
	BPath settingsPath;
	find_directory(B_USER_SETTINGS_DIRECTORY, &settingsPath);
	settingsPath.Append("Koder");
	BDirectory preferencesDir(settingsPath.Path());
	if(preferencesDir.InitCheck() == B_ENTRY_NOT_FOUND) {
		preferencesDir.CreateDirectory(".", nullptr);
	}

	fPreferencesFile.SetTo(&preferencesDir, "settings");
	fPreferences = new Preferences();
	fPreferences->Load(fPreferencesFile.Path());
	fPreferences->fSettingsPath = settingsPath;
	EditorWindow::SetPreferences(fPreferences);

	BPath styleFile(&preferencesDir, "styles");
	styleFile.Append(fPreferences->fStyleFile);
	fStyler = new Styler(styleFile.Path());
	EditorWindow::SetStyler(fStyler);
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
	case ACTIVE_WINDOW_CHANGED: {
		if(message->FindPointer("window", (void**) &fLastActiveWindow) != B_OK) {
			fLastActiveWindow = nullptr;
		}
	} break;
	case APP_PREFERENCES_CHANGED: {
		for(uint32 i = 0, count = fWindows.CountItems(); i < count; i++) {
			BMessenger messenger((BWindow*) fWindows.ItemAt(i));
			messenger.SendMessage(message);
		}
	} break;
	case APP_PREFERENCES_QUITTING: {
		fAppPreferencesWindow = nullptr;
	} break;
	case FINDWINDOW_FIND:
	case FINDWINDOW_REPLACE:
	case FINDWINDOW_REPLACEFIND:
	case FINDWINDOW_REPLACEALL: {
		// TODO: == nullptr should never happen, alert if it somehow does?
		if(fLastActiveWindow != nullptr) {
			BMessenger messenger((BWindow*) fLastActiveWindow);
			messenger.SendMessage(message);
		}
	} break;
	case FINDWINDOW_QUITTING: {
		fFindWindow = nullptr;
	} break;
	case MAINMENU_EDIT_APP_PREFERENCES: {
		if(fAppPreferencesWindow == nullptr) {
			fAppPreferencesWindow = new AppPreferencesWindow(fPreferences);
		}
		fAppPreferencesWindow->Show();
		fAppPreferencesWindow->Activate();
	} break;
	case MAINMENU_SEARCH_FINDREPLACE: {
		if(fFindWindow == nullptr) {
			fFindWindow = new FindWindow();
		}
		fFindWindow->Show();
		fFindWindow->Activate();
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
			fPreferences->fWindowRect = window->Frame();
			Quit();
		}
	} break;
	default:
		BApplication::MessageReceived(message);
	break;
	}
}
