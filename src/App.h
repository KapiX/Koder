/*
 * Copyright 2014-2019 Kacper Kasper <kacperkasper@gmail.com>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#ifndef APP_H
#define APP_H


#include <memory>

#include <Application.h>
#include <ObjectList.h>
#include <Path.h>
#include <String.h>
#include <WindowStack.h>


class AppPreferencesWindow;
class EditorWindow;
class FindWindow;
class Preferences;
class Styler;


enum {
	SUPPRESS_INITIAL_WINDOW		= 'Siwn',
	WINDOW_NEW_WITH_QUIT_REPLY	= 'NWwn',
	ACTIVATE_WINDOW				= 'actw'
};


const BString gAppMime = "application/x-vnd.KapiX-Koder";


class App : public BApplication {
public:
								App();
								~App();

	void						Init();

	void						AboutRequested();
	bool						QuitRequested();
	void						ReadyToRun();
	void						RefsReceived(BMessage* message);
	void						MessageReceived(BMessage* message);

private:
	void						_ActivateOrCreateWindow(const BMessage* message,
									const entry_ref& ref, const int32 line,
									const int32 column,
									std::unique_ptr<BWindowStack>& windowStack);
	EditorWindow*				_CreateWindow(const BMessage* message,
									std::unique_ptr<BWindowStack>& windowStack);
	void						_CreateWindowWithQuitReply(BMessage* message,
									const entry_ref* ref = nullptr,
									const int32 line = -1,
									const int32 column = -1);

	BObjectList<EditorWindow>	fWindows;
	EditorWindow*				fLastActiveWindow;
	AppPreferencesWindow*		fAppPreferencesWindow;
	FindWindow*					fFindWindow;
	Preferences*				fPreferences;
	Styler*						fStyler;

	BPath						fPreferencesFile;
	bool						fSuppressInitialWindow;
};


#endif // APP_H
