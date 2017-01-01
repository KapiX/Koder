/*
 * Copyright 2014-2017 Kacper Kasper <kacperkasper@gmail.com>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#ifndef APP_H
#define APP_H


#include <Application.h>
#include <ObjectList.h>
#include <Path.h>


class AppPreferencesWindow;
class EditorWindow;
class FindWindow;
class Preferences;
class Styler;


class App : public BApplication {
public:
								App();
								~App();

	void						Init();

	void						AboutRequested();
	bool						QuitRequested();
	void						ReadyToRun();
	void						ArgvReceived(int32 argc, char** argv);
	void						RefsReceived(BMessage* message);
	void						MessageReceived(BMessage* message);

private:
	BObjectList<EditorWindow>	fWindows;
	EditorWindow*				fLastActiveWindow;
	AppPreferencesWindow*		fAppPreferencesWindow;
	FindWindow*					fFindWindow;
	Preferences*				fPreferences;
	Styler*						fStyler;

	BPath						fPreferencesFile;
};


#endif // APP_H
