/*
 * Copyright 2014-2017 Kacper Kasper <kacperkasper@gmail.com>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include "App.h"

#include <AboutWindow.h>
#include <Alert.h>
#include <Catalog.h>
#include <Directory.h>
#include <Entry.h>
#include <File.h>
#include <FindDirectory.h>
#include <Path.h>
#include <WindowStack.h>

#include <memory>
#include <string>
#include <vector>

#include "AppPreferencesWindow.h"
#include "EditorWindow.h"
#include "FindWindow.h"
#include "Preferences.h"
#include "Styler.h"
#include "QuitAlert.h"


#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "App"


App::App()
	:
	BApplication(gAppMime),
	fLastActiveWindow(nullptr),
	fAppPreferencesWindow(nullptr),
	fFindWindow(nullptr),
	fPreferences(nullptr)
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
}


void
App::Init()
{
	BPath settingsPath;
	find_directory(B_USER_SETTINGS_DIRECTORY, &settingsPath);
	settingsPath.Append(gAppName);
	BDirectory preferencesDir(settingsPath.Path());
	if(preferencesDir.InitCheck() == B_ENTRY_NOT_FOUND) {
		preferencesDir.CreateDirectory(settingsPath.Path(), &preferencesDir);
	}

	fPreferencesFile.SetTo(&preferencesDir, "settings");
	fPreferences = new Preferences();
	fPreferences->Load(fPreferencesFile.Path());
	fPreferences->fSettingsPath = settingsPath;
	EditorWindow::SetPreferences(fPreferences);

	Languages::LoadLanguages();
}


void
App::AboutRequested()
{
	const char* specialThanks[] = {
		B_TRANSLATE("Neil Hodgson, for Scintilla editing component and SciTE editor."),
		B_TRANSLATE("zuMi, for toolbar icons."),
		B_TRANSLATE("humdinger, for German translation and GUI design tips."),
		B_TRANSLATE("miqlas, for Hungarian translation and app icon."),
		B_TRANSLATE("damoklas, for Lithuanian translation."),
		B_TRANSLATE("ArmanHayots, for Russian translation."),
		B_TRANSLATE("un_spacyar, for Spanish translation."),
		B_TRANSLATE("Lan72, for Ukrainian translation."),
		nullptr
	};

	BAboutWindow* window = new BAboutWindow(gAppName, gAppMime);
	window->AddCopyright(2016, "Kacper Kasper");
	window->AddDescription(
		B_TRANSLATE("Code editor for Haiku based on Scintilla editing component."));
	window->AddSpecialThanks(specialThanks);
	window->AddExtraInfo(B_TRANSLATE("Distributed on MIT license terms."));
	window->Show();
}


bool
App::QuitRequested()
{
	BObjectList<EditorWindow> unsaved;
	std::vector<std::string> unsavedPaths;
	EditorWindow* current;
	for(int i = 0; current = fWindows.ItemAt(i); ++i) {
		if(current->IsModified()) {
			unsaved.AddItem(current);
			unsavedPaths.push_back(std::string(current->OpenedFilePath()));
		}
	}
	if(unsaved.IsEmpty()) {
		return true;
	}
	QuitAlert* quitAlert = new QuitAlert(unsavedPaths);
	auto filesToSave = quitAlert->Go();
	if(filesToSave.empty()) {
		fLastActiveWindow->Activate();
		return false;
	}
	for(int i = 0; current = unsaved.ItemAt(i); ++i) {
		if(filesToSave[i] == false) continue;
		BMessage reply;
		BMessage save(SAVE_FILE);
		BMessenger messenger((BWindow*) current);
		messenger.SendMessage(&save, &reply);
			// FIXME: this is smelly
	}
	return true;
}


void
App::ReadyToRun()
{
	if(CountWindows() == 0) {
		PostMessage(WINDOW_NEW);
	}
}


void
App::ArgvReceived(int32 argc, char** argv)
{
	entry_ref ref;
	BEntry entry;
	std::unique_ptr<BWindowStack> windowStack(nullptr);
	for(int32 i = 1; i < argc; ++i) {
		Sci_Position line = -1;
		Sci_Position column = -1;
		BString argument(argv[i]);
		BString filename;
		BString lineStr, columnStr;
		// first :
		int32 pos = argument.FindFirst(':');
		if(pos != B_ERROR) {
			argument.CopyInto(filename, 0, pos);
			// second :
			int32 pos2 = argument.FindFirst(':', pos + 1);
			if(pos2 != B_ERROR) {
				argument.CopyInto(lineStr, pos + 1, pos2);
				argument.CopyInto(columnStr, pos2 + 1, argument.Length());
			} else {
				argument.CopyInto(lineStr, pos + 1, argument.Length());
			}
			line = strtol(lineStr.String(), nullptr, 10) - 1;
			column = strtol(columnStr.String(), nullptr, 10) - 1;
		} else {
			filename = argument;
		}
		entry.SetTo(filename);
		entry.GetRef(&ref);
		bool stagger = fWindows.CountItems() > 0 && (!fPreferences->fOpenWindowsInStack || !windowStack.get());
		EditorWindow* window = new EditorWindow(stagger);
		if(fPreferences->fOpenWindowsInStack) {
			if(windowStack.get() == nullptr)
				windowStack.reset(new BWindowStack(window));
			else
				windowStack->AddWindow(window);
		}
		window->OpenFile(&ref, line, column);
		window->Show();
		fWindows.AddItem(window);
	}
}


void
App::RefsReceived(BMessage* message)
{
	int32 count;
	if(message->GetInfo("refs", nullptr, &count) != B_OK) {
		return;
	}
	BWindow* windowPtr = nullptr;
	std::unique_ptr<BWindowStack> windowStack(nullptr);
	if(message->FindPointer("window", (void**) &windowPtr) == B_OK)
		windowStack.reset(new BWindowStack(windowPtr));
	entry_ref ref;
	for(int32 i = 0; i < count; ++i) {
		if(message->FindRef("refs", i, &ref) == B_OK) {
			Sci_Position line = message->GetInt32("be:line", 0) - 1;
			Sci_Position column = message->GetInt32("be:column", 0) - 1;
			bool stagger = fWindows.CountItems() > 0 && (!fPreferences->fOpenWindowsInStack || !windowStack.get());
			EditorWindow* window = new EditorWindow(stagger);
			if(fPreferences->fOpenWindowsInStack) {
				if(windowStack.get() == nullptr)
					windowStack.reset(new BWindowStack(window));
				else
					windowStack->AddWindow(window);
			}
			window->OpenFile(&ref, line, column);
			window->Show();
			fWindows.AddItem(window);
		}
	}
}


void
App::MessageReceived(BMessage* message)
{
	switch(message->what) {
	case ACTIVE_WINDOW_CHANGED: {
		if(message->FindPointer("window", (void**) &fLastActiveWindow) != B_OK) {
			fLastActiveWindow = nullptr;
		} else {
			fPreferences->fWindowRect = fLastActiveWindow->Frame();
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
		std::string findText = message->GetString("selection", "");
		if(findText.empty() == false) {
			if(fFindWindow->LockLooper()) {
				fFindWindow->SetFindText(findText);
				fFindWindow->UnlockLooper();
			}
		}
	} break;
	case WINDOW_NEW: {
		BWindow* windowPtr = nullptr;
		std::unique_ptr<BWindowStack> windowStack(nullptr);
		if(message->FindPointer("window", (void**) &windowPtr) == B_OK)
			windowStack.reset(new BWindowStack(windowPtr));
		bool stagger = fWindows.CountItems() > 0 && (!fPreferences->fOpenWindowsInStack || !windowStack.get());
		EditorWindow* window = new EditorWindow(stagger);
		if(fPreferences->fOpenWindowsInStack && windowStack.get() != nullptr)
			windowStack->AddWindow(window);
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
