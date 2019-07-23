/*
 * Copyright 2014-2019 Kacper Kasper <kacperkasper@gmail.com>
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
#include <Language.h>
#include <LocaleRoster.h>
#include <Path.h>
#include <tracker_private.h>
#include <WindowStack.h>

#include <algorithm>
#include <iterator>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "AppPreferencesWindow.h"
#include "EditorWindow.h"
#include "FindWindow.h"
#include "GoToLineWindow.h"
#include "Preferences.h"
#include "Styler.h"
#include "Utils.h"
#include "QuitAlert.h"


#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "App"


App::App()
	:
	BApplication(gAppMime),
	fLastActiveWindow(nullptr),
	fAppPreferencesWindow(nullptr),
	fFindWindow(nullptr),
	fPreferences(nullptr),
	fSuppressInitialWindow(false)
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
	std::vector<std::string> specialThanks{
		B_TRANSLATE("Neil Hodgson, for Scintilla editing component and SciTE editor."),
		B_TRANSLATE("zuMi, for toolbar icons."),
		B_TRANSLATE("humdinger, for GUI design tips and artwork."),
		B_TRANSLATE("Konrad77, for dark theme."),
		B_TRANSLATE_COMMENT("translators to:", "to _ (language), e.g. German"),
	};

	const std::unordered_map<std::string, std::vector<std::string>> translatorsMap{
		{ "de", { "FabianReidinger", "humdinger" } },
		{ "en_GB", { "adamfowleruk" } },
		{ "es", { "un_spacyar" } },
//		{ "fi", { "Karvjorm" } },
		{ "fr", { "Briseur" } },
		{ "hu", { "miqlas" } },
//		{ "id", { "mazbrili" } },
//		{ "it", { "TheClue" } },
//		{ "lt", { "damoklas" } },
		{ "pt", { "Victor Domingos" } },
		{ "ro", { "al-popa", "Florentina" } },
		{ "ru", { "al-popa", "ArmanHayots", "diver", "TK-313" } },
//		{ "sv", { "deejam" } },
		{ "uk", { "Lan72" } }
	};

	BMessage languages;
	if(BLocaleRoster::Default()->GetAvailableLanguages(&languages) == B_OK) {
		BString langID;
		for(int i = 0; languages.FindString("language", i, &langID) == B_OK; i++) {
			BLanguage lang(langID.String());
			const auto translators = translatorsMap.find(lang.ID());
			// let's deal with pt_BR later
			if((lang.IsCountrySpecific() && langID != "en_GB")
				|| translators == translatorsMap.end())
				continue;
			BString name;
			lang.GetName(name);
			std::ostringstream languageRow;
			languageRow << "    " << name.String() << ": ";
			std::copy(translators->second.begin(), translators->second.end(),
				std::ostream_iterator<std::string>(languageRow, ", "));
			std::string str(languageRow.str());
			str.erase(str.end() - 2);
			specialThanks.push_back(str);
		}
	}

	// prepare something acceptable for BeAPI
	std::vector<const char*> specialThanksC;
	std::transform(specialThanks.begin(), specialThanks.end(),
		std::back_inserter(specialThanksC), [](const std::string& s) { return s.c_str(); });
	specialThanksC.push_back(nullptr);

	BAboutWindow* window = new BAboutWindow(gAppName, gAppMime);
	window->AddCopyright(2016, "Kacper Kasper");
	window->AddDescription(
		B_TRANSLATE("Code editor for Haiku based on Scintilla editing component."));
	window->AddSpecialThanks(specialThanksC.data());
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
	if(CountWindows() == 0 && fSuppressInitialWindow == false) {
		PostMessage(WINDOW_NEW);
	}
}


void
App::ArgvReceived(int32 argc, char** argv)
{
	BMessage* message = CurrentMessage();
	BString cwd = message->GetString("cwd", "~");
	std::unique_ptr<BWindowStack> windowStack;
	for(int32 i = 1; i < argc; ++i) {
		std::string arg = argv[i];
		// FIXME: this should be handled in main.cpp probably
		if(arg == "-w" || arg == "--wait" || arg == "-h" || arg == "--help")
			continue;
		int32 line, column;
		std::string filename = ParseFileArgument(argv[i], &line, &column);
		if(filename.find('/') != 0) {
			BPath absolute(cwd.String(), filename.c_str(), true);
			filename = absolute.Path();
		}
		entry_ref ref;
		BEntry(filename.c_str()).GetRef(&ref);
		_ActivateOrCreateWindow(message, ref, line, column, windowStack);
	}
}


void
App::RefsReceived(BMessage* message)
{
	int32 count;
	if(message->GetInfo("refs", nullptr, &count) != B_OK) {
		return;
	}

	std::unique_ptr<BWindowStack> windowStack;
	BMessenger messenger(kTrackerSignature);
	BMessage trackerMessage(B_REFS_RECEIVED);
	entry_ref ref;
	for(int32 i = 0; i < count; ++i) {
		if(message->FindRef("refs", i, &ref) == B_OK) {
			if(BNode(&ref).IsDirectory()) {
				trackerMessage.AddRef("refs", &ref);
				continue;
			}
			const int32 line = message->GetInt32("be:line", -1);
			const int32 column = message->GetInt32("be:column", -1);
			_ActivateOrCreateWindow(message, ref, line, column, windowStack);
		}
	}
	if(!trackerMessage.IsEmpty()) {
		messenger.SendMessage(&trackerMessage);
	}
}


void
App::MessageReceived(BMessage* message)
{
	switch(message->what) {
	case SUPPRESS_INITIAL_WINDOW: {
		fSuppressInitialWindow = true;
	} break;
	case ACTIVE_WINDOW_CHANGED: {
		if(message->FindPointer("window", (void**) &fLastActiveWindow) != B_OK) {
			fLastActiveWindow = nullptr;
		} else {
			fPreferences->fWindowRect = fLastActiveWindow->Frame();
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
		fPreferences->fFindWindowState = *message;
	} break;
	case FINDWINDOW_QUITTING: {
		fFindWindow = nullptr;
	} break;
	case MAINMENU_EDIT_APP_PREFERENCES: {
		if(fAppPreferencesWindow == nullptr) {
			fAppPreferencesWindow = new AppPreferencesWindow(fPreferences);
			for(uint32 i = 0, count = fWindows.CountItems(); i < count; i++) {
				fAppPreferencesWindow->StartWatching(fWindows.ItemAt(i), APP_PREFERENCES_CHANGED);
			}
		}
		fAppPreferencesWindow->Show();
		fAppPreferencesWindow->Activate();
	} break;
	case MAINMENU_SEARCH_FINDREPLACE: {
		if(fFindWindow == nullptr) {
			fFindWindow = new FindWindow(
				&fPreferences->fFindWindowState, fPreferences->fSettingsPath);
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
		std::unique_ptr<BWindowStack> stack;
		auto window = _CreateWindow(message, stack);
		window->Show();
	} break;
	case WINDOW_NEW_WITH_QUIT_REPLY: {
		BMessage* detached = DetachCurrentMessage();
		entry_ref ref;
		if(message->FindRef("refs", &ref) == B_OK) {
			const int32 line = message->GetInt32("be:line", -1);
			const int32 column = message->GetInt32("be:column", -1);
			_CreateWindowWithQuitReply(detached, &ref, line, column);
		} else {
			_CreateWindowWithQuitReply(detached);
		}
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


/**
 * Takes into account current preferences: if files should not always be opened
 * in new window it first looks for existing one with specified file and
 * activates it if found; otherwise creates a new one. Then jumps to specified
 * line and column. windowStack - see App::_CreateWindow.
 */
void
App::_ActivateOrCreateWindow(const BMessage* message, const entry_ref& ref,
	const int32 line, const int32 column,
	std::unique_ptr<BWindowStack>& windowStack)
{
	if(!fPreferences->fAlwaysOpenInNewWindow) {
		EditorWindow* current;
		for(int i = 0; current = fWindows.ItemAt(i); ++i) {
			if(std::string(current->OpenedFilePath()) == BPath(&ref).Path()) {
				current->Activate();
				if(line != -1) {
					BMessage gotoMsg(GTLW_GO);
					gotoMsg.AddInt32("line", line);
					current->PostMessage(&gotoMsg);
				}
				return;
			}
		}
	}
	auto window = _CreateWindow(message, windowStack);
	window->OpenFile(&ref, line, column);
	window->Show();
}


/**
 * Creates a window with parameters specified in message:
 *   (Pointer) window - new window will be attached this one
 * Takes current preferences into account.
 * Adds created window to the list of all open windows.
 * Returns a window and a window stack to which it is attached.
 */
EditorWindow*
App::_CreateWindow(const BMessage* message, std::unique_ptr<BWindowStack>& windowStack)
{
	if(message != nullptr) {
		BWindow* windowPtr;
		if(message->FindPointer("window", (void**) &windowPtr) == B_OK)
			windowStack.reset(new BWindowStack(windowPtr));
	}
	bool stagger = fWindows.CountItems() > 0 &&
		(!fPreferences->fOpenWindowsInStack || !windowStack);
	EditorWindow* window = new EditorWindow(stagger);
	if(fPreferences->fOpenWindowsInStack)
		if(!windowStack)
			windowStack.reset(new BWindowStack(window));
		else
			windowStack->AddWindow(window);
	if(fAppPreferencesWindow != nullptr) {
		fAppPreferencesWindow->StartWatching(window, APP_PREFERENCES_CHANGED);
	}
	fWindows.AddItem(window);
	return window;
}


/**
 * Creates a window and tells it to reply to message when it quits.
 * Then optionally opens the file and jumps to specified line and column.
 * IMPORTANT! message has to be detached from any looper.
 */
void
App::_CreateWindowWithQuitReply(BMessage* message, const entry_ref* ref,
	const int32 line, const int32 column)
{
	auto emptyStack = std::unique_ptr<BWindowStack>();
	EditorWindow* window = _CreateWindow(message, emptyStack);
	window->SetOnQuitReplyToMessage(message);
	if(ref != nullptr)
		window->OpenFile(ref, line, column);
	window->Show();
}
