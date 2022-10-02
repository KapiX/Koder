/*
 * Copyright 2014-2019 Kacper Kasper <kacperkasper@gmail.com>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#ifndef EDITORWINDOW_H
#define EDITORWINDOW_H

#include <memory>
#include <optional>
#include <string>

#include <Catalog.h>
#include <MimeType.h>
#include <MessageFilter.h>
#include <Statable.h>
#include <String.h>
#include <Window.h>

#include <ScintillaView.h>

#include "Languages.h"


struct entry_ref;
class BFilePanel;
class BMenu;
class BMenuBar;
class BPath;
class BPopUpMenu;
class BookmarksWindow;
class Editor;
class FindReplaceHandler;
class GoToLineWindow;
class Preferences;
class StatusView;
class ToolBar;


const BString gAppName = B_TRANSLATE_SYSTEM_NAME("Koder");

const uint32 ACTIVE_WINDOW_CHANGED	= 'AWCH';
const uint32 SAVE_FILE				= 'SVFL';


enum {
	MAINMENU_FILE_NEW					= 'mnew',
	MAINMENU_FILE_OPEN					= 'mopn',
	MAINMENU_FILE_RELOAD				= 'mrld',
	MAINMENU_FILE_SAVE					= 'msav',
	MAINMENU_FILE_SAVEAS				= 'msva',
	MAINMENU_FILE_OPEN_CORRESPONDING	= 'mcrf',
	MAINMENU_FILE_QUIT					= 'mqut',

	MAINMENU_EDIT_CONVERTEOLS_UNIX		= 'ceun',
	MAINMENU_EDIT_CONVERTEOLS_WINDOWS	= 'cewi',
	MAINMENU_EDIT_CONVERTEOLS_MAC		= 'cema',

	EDIT_COMMENTLINE					= 'cmtl',
	EDIT_COMMENTBLOCK					= 'cmtb',

	MAINMENU_EDIT_TRIMWS				= 'metw',

	MAINMENU_EDIT_FILE_PREFERENCES		= 'mefp',
	MAINMENU_EDIT_APP_PREFERENCES		= 'meap',

	MAINMENU_VIEW_SPECIAL_WHITESPACE	= 'vsws',
	MAINMENU_VIEW_SPECIAL_EOL			= 'vseo',
	MAINMENU_VIEW_TOOLBAR				= 'vstl',
	MAINMENU_VIEW_WRAPLINES				= 'mvwl',

	MAINMENU_SEARCH_FINDREPLACE			= 'msfr',
	MAINMENU_SEARCH_FINDNEXT			= 'msfn',
	MAINMENU_SEARCH_FINDSELECTION		= 'msfs',
	MAINMENU_SEARCH_REPLACEANDFIND		= 'msrf',
	MAINMENU_SEARCH_INCREMENTAL			= 'msin',
	MAINMENU_SEARCH_BOOKMARKS			= 'msbk',
	MAINMENU_SEARCH_TOGGLEBOOKMARK		= 'mtbk',
	MAINMENU_SEARCH_NEXTBOOKMARK		= 'mnbk',
	MAINMENU_SEARCH_PREVBOOKMARK		= 'mpbk',
	MAINMENU_SEARCH_GOTOLINE			= 'msgl',

	MAINMENU_LANGUAGE					= 'ml00',
	MAINMENU_OPEN_RECENT				= 'mr00',

	TOOLBAR_SPECIAL_SYMBOLS				= 'tlss',

	FILE_OPEN							= 'flop',
	FILE_SAVE							= 'flsv',

	WINDOW_NEW							= 'ewnw',
	WINDOW_CLOSE						= 'ewcl',

	OPEN_TERMINAL						= 'otrm',
};


class EditorWindow : public BWindow {
public:
							EditorWindow(bool stagger = false);
	virtual					~EditorWindow();

			void			New();
			void			OpenFile(const entry_ref* ref,
								Sci_Position line = -1,
								Sci_Position column = -1);
			void			RefreshTitle();
			void			SaveFile(entry_ref* ref);

			bool			QuitRequested();
			void			MessageReceived(BMessage* message);
			void			WindowActivated(bool active);
			void			FrameMoved(BPoint origin);
			void			Show();

			bool			IsModified() { return fModified; }
			const char*		OpenedFilePath();

	static	void			SetPreferences(Preferences* preferences);

			void			SetOnQuitReplyToMessage(BMessage* message);

private:
	enum ModifiedAlertResult {
		CANCEL	= 0,
		DISCARD	= 1,
		SAVE	= 2
	};
	struct FilePreferences {
		std::optional<int> fTabWidth;
		std::optional<int> fIndentWidth;
		std::optional<bool> fTabsToSpaces;
		std::optional<bool> fTrimTrailingWhitespace;
		std::optional<uint8> fEOLMode;
	};
			BMenuBar*		fMainMenu;
			BPopUpMenu*		fContextMenu;
			BPath*			fOpenedFilePath;
			BMimeType		fOpenedFileMimeType;
			time_t			fOpenedFileModificationTime;
			bool			fModifiedOutside;
			bool			fModified;
			bool			fReadOnly;
			Editor*			fEditor;
			BFilePanel*		fOpenPanel;
			BFilePanel*		fSavePanel;
			BMenu*			fOpenRecentMenu;
			BMenu*			fLanguageMenu;
			std::string		fCurrentLanguage;
			ToolBar*		fToolbar;
			StatusView*		fStatusView;

			GoToLineWindow*		fGoToLineWindow;
			BookmarksWindow*	fBookmarksWindow;

			bool			fActivatedGuard;

			BMessage*		fOnQuitReplyToMessage;

			FindReplaceHandler*	fFindReplaceHandler;

	static	Preferences*	fPreferences;
			FilePreferences	fFilePreferences;

			std::string			fIncrementalSearchTerm;
			std::unique_ptr<BMessageFilter> fIncrementalSearchFilter;

			void			_PopulateOpenRecentMenu();
			void			_PopulateLanguageMenu();
			void			_ReloadFile(entry_ref* ref = nullptr);
			void			_SetLanguage(std::string lang);
			void			_SetLanguageByFilename(const char* filename);
			void			_OpenCorrespondingFile(const BPath &file, const std::string lang);
			void			_LoadEditorconfig();
			void			_SyncWithPreferences();
			void			_SyncEditMenus();
			int32			_ShowModifiedAlert();
			void			_ReloadAlert(const char* title, const char* message);
			void			_Save();
			void			_OpenTerminal(const char* path);

			void			OnSavePoint(bool left);

	static	filter_result	_IncrementalSearchFilter(BMessage* message,
								BHandler** target, BMessageFilter* messageFilter);
};


#endif // EDITORWINDOW_H
