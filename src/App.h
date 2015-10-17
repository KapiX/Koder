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

#ifndef APP_H
#define APP_H

#include <Application.h>
#include <ObjectList.h>

class AppPreferencesWindow;
class EditorWindow;
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
	void						RefsReceived(BMessage* message);
	void						MessageReceived(BMessage* message);
private:
	BObjectList<EditorWindow>	fWindows;
	EditorWindow*				fLastActiveWindow;
	AppPreferencesWindow*		fAppPreferencesWindow;
	Preferences*				fPreferences;
	Styler*						fStyler;
};

#endif // APP_H
