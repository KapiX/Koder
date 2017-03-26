/*
 * Copyright 2014-2017 Kacper Kasper <kacperkasper@gmail.com>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#ifndef EDITORWINDOW_H
#define EDITORWINDOW_H

#include <Catalog.h>
#include <MimeType.h>
#include <Statable.h>
#include <String.h>
#include <Window.h>

#include <ScintillaView.h>

#include <string>

#include "Languages.h"


struct entry_ref;
class BFilePanel;
class BMenu;
class BMenuBar;
class BPath;
class Editor;
class GoToLineWindow;
class Preferences;


const BString gAppName = B_TRANSLATE_SYSTEM_NAME("Koder");
const BString gAppMime = "application/x-vnd.KapiX-Koder";

const uint32 ACTIVE_WINDOW_CHANGED	= 'AWCH';
const uint32 SAVE_FILE				= 'SVFL';


enum {
	MAINMENU_FILE_NEW					= 'mnew',
	MAINMENU_FILE_OPEN					= 'mopn',
	MAINMENU_FILE_SAVE					= 'msav',
	MAINMENU_FILE_SAVEAS				= 'msva',
	MAINMENU_FILE_QUIT					= 'mqut',

	MAINMENU_EDIT_CONVERTEOLS_UNIX		= 'ceun',
	MAINMENU_EDIT_CONVERTEOLS_WINDOWS	= 'cewi',
	MAINMENU_EDIT_CONVERTEOLS_MAC		= 'cema',

	MAINMENU_EDIT_COMMENTLINE			= 'cmtl',
	MAINMENU_EDIT_COMMENTBLOCK			= 'cmtb',

	MAINMENU_EDIT_FILE_PREFERENCES		= 'mefp',
	MAINMENU_EDIT_APP_PREFERENCES		= 'meap',

	MAINMENU_VIEW_SPECIAL_WHITESPACE	= 'vsws',
	MAINMENU_VIEW_SPECIAL_EOL			= 'vseo',

	MAINMENU_SEARCH_FINDREPLACE			= 'msfr',
	MAINMENU_SEARCH_GOTOLINE			= 'msgl',

	MAINMENU_VIEW_LINEHIGHLIGHT			= 'mlhl',
	MAINMENU_VIEW_LINENUMBERS			= 'mvln',

	MAINMENU_LANGUAGE					= 'ml00',

	FILE_OPEN							= 'flop',
	FILE_SAVE							= 'flsv',

	WINDOW_NEW							= 'ewnw',
	WINDOW_CLOSE						= 'ewcl',
};


class EditorWindow : public BWindow {
public:
							EditorWindow();

			void			New();
			void			OpenFile(entry_ref* ref, Sci_Position line = -1, Sci_Position column = -1);
			void			RefreshTitle();
			void			SaveFile(entry_ref* ref);

			bool			QuitRequested();
			void			MessageReceived(BMessage* message);
			void			WindowActivated(bool active);

			bool			IsModified() { return fModified; }
			const char*		OpenedFilePath();

	static	void			SetPreferences(Preferences* preferences);

private:
	enum ModifiedAlertResult {
		CANCEL	= 0,
		DISCARD	= 1,
		SAVE	= 2
	};
			BMenuBar*		fMainMenu;
			BPath*			fOpenedFilePath;
			BMimeType		fOpenedFileMimeType;
			time_t			fOpenedFileModificationTime;
			bool			fModifiedOutside;
			bool			fModified;
			bool			fReadOnly;
			Editor*			fEditor;
			BFilePanel*		fOpenPanel;
			BFilePanel*		fSavePanel;
			BMenu*			fLanguageMenu;
			std::string		fCurrentLanguage;

			Sci_Position	fSearchTargetStart;
			Sci_Position	fSearchTargetEnd;
			Sci_Position	fSearchLastResultStart;
			Sci_Position	fSearchLastResultEnd;

			GoToLineWindow*	fGoToLineWindow;

			bool			fActivatedGuard;

	static	Preferences*	fPreferences;

			bool			_CheckPermissions(BStatable* file, mode_t permissions);
			void			_FindReplace(BMessage* message);
			status_t		_MonitorFile(BStatable* file, bool enable);
			void			_PopulateLanguageMenu();
			void			_ReloadFile(entry_ref* ref = nullptr);
			void			_SetLanguage(std::string lang);
			void			_SetLanguageByFilename(const char* filename);
			void			_SyncWithPreferences();
			int32			_ShowModifiedAlert();
			void			_Save();
};


#endif // EDITORWINDOW_H
