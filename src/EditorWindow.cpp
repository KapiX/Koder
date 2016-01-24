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

#include "EditorWindow.h"

#include <Alert.h>
#include <Application.h>
#include <Catalog.h>
#include <Entry.h>
#include <File.h>
#include <FilePanel.h>
#include <GroupLayout.h>
#include <LayoutBuilder.h>
#include <MenuBar.h>
#include <MimeType.h>
#include <Node.h>
#include <NodeInfo.h>
#include <ObjectList.h>
#include <Path.h>
#include <Roster.h>
#include <String.h>

#include "AppPreferencesWindow.h"
#include "Editor.h"
#include "FindWindow.h"
#include "GoToLineWindow.h"
#include "Languages.h"
#include "Preferences.h"
#include "Styler.h"


#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "EditorWindow"


Preferences* EditorWindow::fPreferences = NULL;
Styler* EditorWindow::fStyler = NULL;


EditorWindow::EditorWindow()
	:
	BWindow(fPreferences->fWindowRect, gAppName, B_DOCUMENT_WINDOW, 0)
{
	fActivatedGuard = false;

	fSearchLastResultStart = -1;
	fSearchLastResultEnd = -1;

	fGoToLineWindow = NULL;
	fOpenedFilePath = NULL;
	fOpenedFileMimeType.SetTo("text/plain");

	BMessenger* windowMessenger = new BMessenger(this);
	fOpenPanel = new BFilePanel(B_OPEN_PANEL, windowMessenger);
	fSavePanel = new BFilePanel(B_SAVE_PANEL, windowMessenger, NULL, 0, false);

	fMainMenu = new BMenuBar("MainMenu");
	BLayoutBuilder::Menu<>(fMainMenu)
		.AddMenu(B_TRANSLATE("File"))
			.AddItem(B_TRANSLATE("New"), MAINMENU_FILE_NEW, 'N')
			.AddSeparator()
			.AddItem(B_TRANSLATE("Open"), MAINMENU_FILE_OPEN, 'O')
			.AddItem(B_TRANSLATE("Save"), MAINMENU_FILE_SAVE, 'S')
			.AddItem(B_TRANSLATE("Save as" B_UTF8_ELLIPSIS), MAINMENU_FILE_SAVEAS)
			.AddSeparator()
			.AddItem(B_TRANSLATE("Close"), B_QUIT_REQUESTED, 'W')
			.AddItem(B_TRANSLATE("Quit"), MAINMENU_FILE_QUIT, 'Q')
		.End()
		.AddMenu(B_TRANSLATE("Edit"))
			.AddItem(B_TRANSLATE("Undo"), B_UNDO, 'Z')
			.AddItem(B_TRANSLATE("Redo"), B_REDO, 'Y')
			.AddSeparator()
			.AddItem(B_TRANSLATE("Cut"), B_CUT, 'X')
			.AddItem(B_TRANSLATE("Copy"), B_COPY, 'C')
			.AddItem(B_TRANSLATE("Paste"), B_PASTE, 'V')
			.AddSeparator()
			.AddItem(B_TRANSLATE("Select all"), B_SELECT_ALL, 'A')
			.AddSeparator()
			.AddMenu(B_TRANSLATE("Convert EOLs"))
				.AddItem(B_TRANSLATE("Unix format"), MAINMENU_EDIT_CONVERTEOLS_UNIX)
				.AddItem(B_TRANSLATE("Windows format"), MAINMENU_EDIT_CONVERTEOLS_WINDOWS)
				.AddItem(B_TRANSLATE("Old Mac format"), MAINMENU_EDIT_CONVERTEOLS_MAC)
			.End()
			.AddSeparator()
			.AddItem(B_TRANSLATE("File preferences" B_UTF8_ELLIPSIS), MAINMENU_EDIT_FILE_PREFERENCES)
			.AddItem(B_TRANSLATE("Application preferences" B_UTF8_ELLIPSIS), MAINMENU_EDIT_APP_PREFERENCES, ',')
		.End()
		.AddMenu(B_TRANSLATE("View"))
			.AddMenu(B_TRANSLATE("Special symbols"))
				.AddItem(B_TRANSLATE("Show white space"), MAINMENU_VIEW_SPECIAL_WHITESPACE)
				.AddItem(B_TRANSLATE("Show EOLs"), MAINMENU_VIEW_SPECIAL_EOL)
			.End()
		.End()
		.AddMenu(B_TRANSLATE("Search"))
			.AddItem(B_TRANSLATE("Find/Replace" B_UTF8_ELLIPSIS), MAINMENU_SEARCH_FINDREPLACE, 'F')
			.AddSeparator()
			.AddItem(B_TRANSLATE("Go to line" B_UTF8_ELLIPSIS), MAINMENU_SEARCH_GOTOLINE, 'G')
		.End()
		.AddMenu(B_TRANSLATE("Language"))
			.AddItem("Dummy", MAINMENU_LANGUAGE)
		.End()
		.AddMenu(B_TRANSLATE("Help"))
			.AddItem(B_TRANSLATE("About" B_UTF8_ELLIPSIS), B_ABOUT_REQUESTED)
		.End();

	fLanguageMenu = fMainMenu->FindItem(MAINMENU_LANGUAGE)->Menu();
	_PopulateLanguageMenu(fLanguageMenu);

	fEditor = new Editor();
	fEditor->SetPreferences(fPreferences);

	BGroupLayout *layout = new BGroupLayout(B_VERTICAL, 0);
	SetLayout(layout);
	layout->AddView(fMainMenu);
	layout->AddView(fEditor);
	layout->SetInsets(0, 0, -1, -1);
	SetKeyMenuBar(fMainMenu);

	_SyncWithPreferences();

	fEditor->SendMessage(SCI_SETADDITIONALSELECTIONTYPING, true, 0);

	fEditor->SendMessage(SCI_USEPOPUP, 0, 0);

	fStyler->ApplyGlobal(fEditor);

	RefreshTitle();
}


void
EditorWindow::New()
{
	be_app->PostMessage(WINDOW_NEW);
}


void
EditorWindow::OpenFile(entry_ref* ref)
{
	BEntry entry(ref);

	char mimeType[256];
	int32 caretPos = 0;
	BNode node(&entry);
	node.ReadAttr("be:caret_position", B_INT32_TYPE, 0, &caretPos, 4);
	node.ReadAttr("BEOS:TYPE", B_MIME_TYPE, 0, mimeType, 256);

	BFile file(&entry, B_READ_ONLY);
	off_t size;
	file.GetSize(&size);
	char* buffer = new char[size + 1];
	file.Read(buffer, size);
	buffer[size] = 0;
	fEditor->SetText(buffer);
	fEditor->SendMessage(SCI_SETSAVEPOINT, 0, 0);
	fEditor->SendMessage(SCI_EMPTYUNDOBUFFER, 0, 0);
	delete []buffer;

	fEditor->SendMessage(SCI_GOTOPOS, caretPos, 0);
	fOpenedFileMimeType.SetTo(mimeType);

	char name[B_FILE_NAME_LENGTH];
	entry.GetName(name);
	char* extension = strrchr(name, '.') + 1;
	if((int) extension == 1) {
		extension = name;
	}

	uint32 lang;
	if(fPreferences->fExtensions.FindUInt32(extension, &lang) == B_OK) {
		_SetLanguage(static_cast<LanguageType>(lang));
	}

	be_roster->AddToRecentDocuments(ref, gAppMime);
	
	if(fOpenedFilePath == NULL)
		fOpenedFilePath = new BPath(&entry);
	else
		fOpenedFilePath->SetTo(&entry);
	RefreshTitle();
}


void
EditorWindow::RefreshTitle()
{
	BString title;
	if(fEditor->SendMessage(SCI_GETMODIFY, 0, 0))
		title << "*";
	if(fOpenedFilePath != NULL)
		if(fPreferences->fFullPathInTitle == true) {
			title << fOpenedFilePath->Path();
		} else {
			title << fOpenedFilePath->Leaf();
		}
	else {
		title << B_TRANSLATE("Untitled");
	}
	SetTitle(title);
}


void
EditorWindow::SaveFile(BPath* path)
{
	// TODO error checking
	BEntry backup;
	if(fOpenedFilePath != NULL && *fOpenedFilePath == *path) {
		backup.SetTo(path->Path());
		backup.Rename(BString(path->Path()).Append("~"));
	}
	BFile file(path->Path(), B_WRITE_ONLY | B_CREATE_FILE);
	int length = fEditor->TextLength() + 1;
	char* buffer = new char[length];
	fEditor->GetText(0, length, buffer);
	file.Write(buffer, length - 1);
	fEditor->SendMessage(SCI_SETSAVEPOINT, 0, 0);
	delete []buffer;

	const char* mimeType = fOpenedFileMimeType.Type();
	BNode node(path->Path());
	BNodeInfo nodeInfo(&node);
	nodeInfo.SetType(mimeType);

	if(fOpenedFilePath != NULL && *fOpenedFilePath == *path) {
		backup.Remove();
	}

	if(fOpenedFilePath == NULL) {
		fOpenedFilePath = new BPath(*path);
	} else if(*fOpenedFilePath != *path) {
		*fOpenedFilePath = *path;
	}
	RefreshTitle();
}


bool
EditorWindow::QuitRequested()
{
	bool close = true;
	if(fEditor->SendMessage(SCI_GETMODIFY, 0, 0)) {
		int32 result = _ShowModifiedAlert();
		switch(result) {
		case ModifiedAlertResult::CANCEL:
			close = false;
		break;
		case ModifiedAlertResult::SAVE:
			_Save();
		case ModifiedAlertResult::DISCARD:
			close = true;
		break;
		}
	}
	if(close == true) {
		if(fOpenedFilePath != NULL) {
			int32 caretPos = fEditor->SendMessage(SCI_GETCURRENTPOS, 0, 0);
			BNode node(fOpenedFilePath->Path());
			node.WriteAttr("be:caret_position", B_INT32_TYPE, 0, &caretPos, 4);
		}

		if(fGoToLineWindow != NULL) {
			fGoToLineWindow->LockLooper();
			fGoToLineWindow->Quit();
		}
		
		delete fOpenPanel;
		delete fSavePanel;

		BMessage closing(WINDOW_CLOSE);
		closing.AddPointer("window", this);
		be_app->PostMessage(&closing);
	}
	return close;
}


void
EditorWindow::MessageReceived(BMessage* message)
{
	if(message->what >= MAINMENU_LANGUAGE && message->what < MAINMENU_LANGUAGE + LANGUAGE_COUNT) {
		uint32 lang = message->what - MAINMENU_LANGUAGE;
		_SetLanguage(static_cast<LanguageType>(lang));
		return;
	}
	switch(message->what) {
		case APP_PREFERENCES_CHANGED: {
			_SyncWithPreferences();
		} break;
		case MAINMENU_FILE_NEW:
			New();
		break;
		case MAINMENU_FILE_OPEN: {
			if(fOpenedFilePath != NULL) {
				BPath parent(*fOpenedFilePath);
				parent.GetParent(&parent);
				fOpenPanel->SetPanelDirectory(parent.Path());
			}
			fOpenPanel->Show();
		} break;
		case MAINMENU_FILE_SAVE: {
			if(fEditor->SendMessage(SCI_GETMODIFY, 0, 0)) {
				_Save();
			}
		} break;
		case MAINMENU_FILE_SAVEAS: {
			if(fOpenedFilePath != NULL) {
				BPath parent(*fOpenedFilePath);
				parent.GetParent(&parent);
				fSavePanel->SetPanelDirectory(parent.Path());
			}
			fSavePanel->Show();
		} break;
		case MAINMENU_FILE_QUIT: {
			be_app->PostMessage(B_QUIT_REQUESTED);
		} break;
		case MAINMENU_EDIT_CONVERTEOLS_UNIX: {
			fEditor->SendMessage(SCI_CONVERTEOLS, SC_EOL_LF, 0);
			fEditor->SendMessage(SCI_SETEOLMODE, SC_EOL_LF, 0);
		} break;
		case MAINMENU_EDIT_CONVERTEOLS_WINDOWS: {
			fEditor->SendMessage(SCI_CONVERTEOLS, SC_EOL_CRLF, 0);
			fEditor->SendMessage(SCI_SETEOLMODE, SC_EOL_CRLF, 0);
		} break;
		case MAINMENU_EDIT_CONVERTEOLS_MAC: {
			fEditor->SendMessage(SCI_CONVERTEOLS, SC_EOL_CR, 0);
			fEditor->SendMessage(SCI_SETEOLMODE, SC_EOL_CR, 0);
		} break;
		case MAINMENU_EDIT_APP_PREFERENCES:
		case MAINMENU_SEARCH_FINDREPLACE: {
			be_app->PostMessage(message);
		} break;
		case MAINMENU_SEARCH_GOTOLINE: {
			if(fGoToLineWindow == NULL) {
				fGoToLineWindow = new GoToLineWindow(this);
			}
			fGoToLineWindow->ShowCentered(Frame());
		} break;
		case MAINMENU_VIEW_SPECIAL_WHITESPACE: {
			fPreferences->fWhiteSpaceVisible = !fPreferences->fWhiteSpaceVisible;
			fMainMenu->FindItem(message->what)->SetMarked(fPreferences->fWhiteSpaceVisible);
			fEditor->SendMessage(SCI_SETVIEWWS, fPreferences->fWhiteSpaceVisible, 0);
		} break;
		case MAINMENU_VIEW_SPECIAL_EOL: {
			fPreferences->fEOLVisible = !fPreferences->fEOLVisible;
			fMainMenu->FindItem(message->what)->SetMarked(fPreferences->fEOLVisible);
			fEditor->SendMessage(SCI_SETVIEWEOL, fPreferences->fEOLVisible, 0);
		} break;
		case B_SAVE_REQUESTED: {
			entry_ref ref;
			if(message->FindRef("directory", &ref) == B_OK) {
				BString name;
				message->FindString("name", &name);
				BPath path(&ref);
				path.Append(name);
				
				SaveFile(&path);
			}
		} break;
		case B_CUT: {
			fEditor->SendMessage(SCI_CUT, 0, 0);
		} break;
		case B_COPY: {
			fEditor->SendMessage(SCI_COPY, 0, 0);
		} break;
		case B_PASTE: {
			fEditor->SendMessage(SCI_PASTE, 0, 0);
		} break;
		case B_SELECT_ALL: {
			fEditor->SendMessage(SCI_SELECTALL, 0, 0);
		} break;
		case B_UNDO: {
			fEditor->SendMessage(SCI_UNDO, 0, 0);
			// TODO: disable menuitem
		} break;
		case B_REDO: {
			fEditor->SendMessage(SCI_REDO, 0, 0);
		} break;
		case EDITOR_SAVEPOINT_LEFT:
		case EDITOR_SAVEPOINT_REACHED:
			RefreshTitle();
		break;
		case B_ABOUT_REQUESTED:
			be_app->PostMessage(message);
		break;
		case B_REFS_RECEIVED: {
			entry_ref ref;
			if(message->FindRef("refs", &ref) == B_OK) {
				if(fEditor->SendMessage(SCI_GETMODIFY, 0, 0)) {
					int32 result = _ShowModifiedAlert();
					switch(result) {
					case ModifiedAlertResult::CANCEL: break;
					case ModifiedAlertResult::SAVE:
						_Save();
					case ModifiedAlertResult::DISCARD:
						OpenFile(&ref);
					break;
					}
				} else {
					OpenFile(&ref);
				}
			}
		}
		case GTLW_GO: {
			int32 line;
			if(message->FindInt32("line", &line) == B_OK) {
				fEditor->SendMessage(SCI_ENSUREVISIBLEENFORCEPOLICY, line - 1, 0);
				fEditor->SendMessage(SCI_GOTOLINE, line - 1, 0);
			}
		} break;
		case FINDWINDOW_FIND:
		case FINDWINDOW_REPLACE:
		case FINDWINDOW_REPLACEALL: {
			_FindReplace(message);
		} break;
		case FINDWINDOW_REPLACEFIND: {
			message->what = FINDWINDOW_REPLACE;
			_FindReplace(message);
			message->what = FINDWINDOW_FIND;
			_FindReplace(message);
		}
		default:
			BWindow::MessageReceived(message);
		break;
	}
}


void
EditorWindow::WindowActivated(bool active)
{
	if(active == true) {
		if(fActivatedGuard == false) {
			// Ensure that caret will be visible after opening file in a new
			// window GOTOPOS in OpenFile does not do that, because in that time
			// Scintilla view does not have proper dimensions, and the control
			// cannot calculate scroll position correctly.
			// After the window is activated for the first time, we are sure
			// layouting has been completed.
			fEditor->SendMessage(SCI_SCROLLCARET, 0, 0);
			fActivatedGuard = true;
		}
		BMessage message(ACTIVE_WINDOW_CHANGED);
		message.AddPointer("window", this);
		be_app->PostMessage(&message);
	}
}


/* static */ void
EditorWindow::SetPreferences(Preferences* preferences)
{
	fPreferences = preferences;
}


/* static */ void
EditorWindow::SetStyler(Styler* styler)
{
	fStyler = styler;
}


void
EditorWindow::_FindReplace(BMessage* message)
{
	bool newSearch = message->GetBool("newSearch");
	bool inSelection = message->GetBool("inSelection");
	bool matchCase = message->GetBool("matchCase");
	bool matchWord = message->GetBool("matchWord");
	bool wrapAround = message->GetBool("wrapAround");
	bool backwards = message->GetBool("backwards");
	const char* findText = message->GetString("findText", "");
	const char* replaceText = message->GetString("replaceText", "");

	int searchFlags = 0;
	if(matchCase == true)
		searchFlags |= SCFIND_MATCHCASE;
	if(matchWord == true)
		searchFlags |= SCFIND_WHOLEWORD;
	fEditor->SendMessage(SCI_SETSEARCHFLAGS, searchFlags, 0);

	if(message->what != FINDWINDOW_REPLACEALL) {
		// Detect if user has changed cursor position
		Sci_Position anchor = fEditor->SendMessage(SCI_GETANCHOR, 0, 0);
		Sci_Position current = fEditor->SendMessage(SCI_GETCURRENTPOS, 0, 0);
		if(anchor != fSearchLastResultStart || current != fSearchLastResultEnd) {
			newSearch = true;
		}

		if(newSearch == true) {
			if(inSelection == true) {
				if(backwards == false) {
					fSearchTargetStart = fEditor->SendMessage(SCI_GETSELECTIONSTART, 0, 0);
					fSearchTargetEnd = fEditor->SendMessage(SCI_GETSELECTIONEND, 0, 0);
				} else {
					fSearchTargetStart = fEditor->SendMessage(SCI_GETSELECTIONEND, 0, 0);
					fSearchTargetEnd = fEditor->SendMessage(SCI_GETSELECTIONSTART, 0, 0);
				}
			} else {
				if(backwards == true) {
					fSearchTargetStart = anchor;
					fSearchTargetEnd = 0;
				} else {
					fSearchTargetStart = current;
					fSearchTargetEnd = fEditor->SendMessage(SCI_GETLENGTH, 0, 0);
				}
			}
			fEditor->SendMessage(SCI_SETTARGETRANGE, fSearchTargetStart, fSearchTargetEnd);
		}

		switch(message->what) {
			case FINDWINDOW_FIND: {
				Sci_Position pos = fEditor->SendMessage(SCI_SEARCHINTARGET, (uptr_t) strlen(findText), (sptr_t) findText);
				if(pos != -1) {
					fSearchLastResultStart = pos;
					fSearchLastResultEnd = pos + strlen(findText);
					fEditor->SendMessage(SCI_SETSEL, fSearchLastResultStart, fSearchLastResultEnd);
					fEditor->SendMessage(SCI_SETTARGETRANGE, (backwards == false ? pos + strlen(findText) : pos), fSearchTargetEnd);
				} else {
					// TODO: _method?
					BAlert* alert = new BAlert(B_TRANSLATE("Searching finished"),
						B_TRANSLATE("End of document was reached. No results found."),
						B_TRANSLATE("OK"), nullptr, nullptr, B_WIDTH_AS_USUAL, B_OFFSET_SPACING, B_INFO_ALERT);
					alert->SetShortcut(0, B_ESCAPE);
					alert->Go();
					if(wrapAround == true) {
						Sci_Position s;
						if(inSelection == true) {
							s = fSearchTargetStart;
						} else {
							if(backwards == true) {
								s = fEditor->SendMessage(SCI_GETLENGTH, 0, 0);
							} else {
								s = 0;
							}
						}
						fEditor->SendMessage(SCI_SETTARGETRANGE, s, fSearchTargetEnd);
					}
				}
			} break;
			case FINDWINDOW_REPLACE: {
				if(fSearchLastResultStart != -1 && fSearchLastResultEnd != -1) {
					fEditor->SendMessage(SCI_SETSEL, fSearchLastResultStart, fSearchLastResultEnd);
					fEditor->SendMessage(SCI_REPLACESEL, 0, (sptr_t) replaceText);
					fSearchLastResultStart = -1;
					fSearchLastResultEnd = -1;
				}
			} break;
		}
	} else {
		if(inSelection == true) {
			fEditor->SendMessage(SCI_TARGETFROMSELECTION, 0, 0);
		} else {
			fEditor->SendMessage(SCI_TARGETWHOLEDOCUMENT, 0, 0);
		}
		Sci_Position pos;
		Sci_Position targetEnd = fEditor->SendMessage(SCI_GETTARGETEND, 0, 0);
		fEditor->SendMessage(SCI_BEGINUNDOACTION, 0, 0);
		do {
			pos = fEditor->SendMessage(SCI_SEARCHINTARGET, (uptr_t) strlen(findText), (sptr_t) findText);
			if(pos != -1) {
				fEditor->SendMessage(SCI_REPLACETARGET, -1, (sptr_t) replaceText);
				fEditor->SendMessage(SCI_SETTARGETRANGE, pos + strlen(replaceText), targetEnd);
			}
		} while(pos != -1);
		fEditor->SendMessage(SCI_ENDUNDOACTION, 0, 0);
		fSearchLastResultStart = -1;
		fSearchLastResultEnd = -1;
	}
}


void
EditorWindow::_PopulateLanguageMenu(BMenu* languageMenu)
{
	// Clear the menu first
	int32 count = languageMenu->CountItems();
	languageMenu->RemoveItems(0, count, true);

	Languages languages;
	languages.SortAlphabetically();
	char submenuName[] = "\0";
	BObjectList<BMenu> menus;
	for(int32 i = 0; i < LANGUAGE_COUNT; ++i) {
		LanguageDefinition& langDef = languages.GetLanguages().at(i);
		BMenuItem *menuItem = new BMenuItem(langDef.fShortName, new BMessage(MAINMENU_LANGUAGE + langDef.fType));

		if(fPreferences->fCompactLangMenu == true) {
			if(submenuName[0] != langDef.fShortName[0]) {
				submenuName[0] = langDef.fShortName[0];
				BMenu *submenu = new BMenu(submenuName);
				menus.AddItem(submenu);
			}
			menus.LastItem()->AddItem(menuItem);
		} else {
			languageMenu->AddItem(menuItem);
		}
	}
	if(fPreferences->fCompactLangMenu == true) {
		int32 menusCount = menus.CountItems();
		for(int32 i = 0; i < menusCount; i++) {
			if(menus.ItemAt(i)->CountItems() > 1) {
				languageMenu->AddItem(menus.ItemAt(i));
			} else {
				languageMenu->AddItem(menus.ItemAt(i)->RemoveItem((int32) 0));
			}
		}
	}
}


void
EditorWindow::_SetLanguage(LanguageType lang)
{
	Languages languages;
	LanguageDefinition& langDef = languages.GetLanguage(static_cast<LanguageType>(lang));
	fEditor->SendMessage(SCI_SETLEXER, static_cast<uptr_t>(langDef.fLexerID), 0);
	fEditor->SendMessage(SCI_SETPROPERTY, (uptr_t) "fold", (sptr_t) "1");
	if(langDef.fLexerID == SCLEX_CPP) {
		fEditor->SendMessage(SCI_SETPROPERTY, (uptr_t) "lexer.cpp.track.preprocessor", (sptr_t) "0");
		fEditor->SendMessage(SCI_SETPROPERTY, (uptr_t) "styling.within.preprocessor", (sptr_t) "1");
	}
	fStyler->ApplyLanguage(fEditor, langDef.fLexerName.String());
	BPath langsPath(fPreferences->fSettingsPath);
	langsPath.Append("langs.xml");
	languages.ApplyLanguage(fEditor, langsPath.Path(), langDef.fLexerName.String());
}


void
EditorWindow::_SyncWithPreferences()
{
	if(fPreferences != NULL) {
		fMainMenu->FindItem(MAINMENU_VIEW_SPECIAL_WHITESPACE)->SetMarked(fPreferences->fWhiteSpaceVisible);
		fMainMenu->FindItem(MAINMENU_VIEW_SPECIAL_EOL)->SetMarked(fPreferences->fEOLVisible);

		fEditor->SendMessage(SCI_SETVIEWEOL, fPreferences->fEOLVisible, 0);
		fEditor->SendMessage(SCI_SETVIEWWS, fPreferences->fWhiteSpaceVisible, 0);
		fEditor->SendMessage(SCI_SETTABWIDTH, fPreferences->fTabWidth, 0);
		fEditor->SendMessage(SCI_SETUSETABS, !fPreferences->fTabsToSpaces, 0);
		fEditor->SendMessage(SCI_SETCARETLINEVISIBLE, fPreferences->fLineHighlighting, 0);

		if(fPreferences->fLineNumbers == true) {
			fEditor->SendMessage(SCI_SETMARGINTYPEN, Editor::Margin::NUMBER, (long int) SC_MARGIN_NUMBER);
		} else {
			fEditor->SendMessage(SCI_SETMARGINWIDTHN, Editor::Margin::NUMBER, 0);
		}

		if(fPreferences->fLineLimitShow == true) {
			fEditor->SendMessage(SCI_SETEDGEMODE, fPreferences->fLineLimitMode, 0);
			fEditor->SendMessage(SCI_SETEDGECOLUMN, fPreferences->fLineLimitColumn, 0);
		} else {
			fEditor->SendMessage(SCI_SETEDGEMODE, 0, 0);
		}

		if(fPreferences->fIndentGuidesShow == true) {
			fEditor->SendMessage(SCI_SETINDENTATIONGUIDES, fPreferences->fIndentGuidesMode, 0);
		} else {
			fEditor->SendMessage(SCI_SETINDENTATIONGUIDES, 0, 0);
		}

		fEditor->SendMessage(SCI_SETMARGINTYPEN, Editor::Margin::FOLD, SC_MARGIN_SYMBOL);
		fEditor->SendMessage(SCI_SETMARGINMASKN, Editor::Margin::FOLD, SC_MASK_FOLDERS);
		fEditor->SendMessage(SCI_SETMARGINWIDTHN, Editor::Margin::FOLD, 20);
		fEditor->SendMessage(SCI_SETMARGINSENSITIVEN, Editor::Margin::FOLD, 1);

		fEditor->SendMessage(SCI_MARKERDEFINE, SC_MARKNUM_FOLDER, SC_MARK_PLUS);
		fEditor->SendMessage(SCI_MARKERDEFINE, SC_MARKNUM_FOLDEROPEN, SC_MARK_MINUS);
		fEditor->SendMessage(SCI_MARKERDEFINE, SC_MARKNUM_FOLDEREND, SC_MARK_EMPTY);
		fEditor->SendMessage(SCI_MARKERDEFINE, SC_MARKNUM_FOLDERMIDTAIL, SC_MARK_EMPTY);
		fEditor->SendMessage(SCI_MARKERDEFINE, SC_MARKNUM_FOLDEROPENMID, SC_MARK_EMPTY);
		fEditor->SendMessage(SCI_MARKERDEFINE, SC_MARKNUM_FOLDERSUB, SC_MARK_EMPTY);
		fEditor->SendMessage(SCI_MARKERDEFINE, SC_MARKNUM_FOLDERTAIL, SC_MARK_EMPTY);

		fEditor->SendMessage(SCI_SETFOLDFLAGS, 16, 0);

		RefreshTitle();

		// TODO Do this only if language menu preference has changed
		_PopulateLanguageMenu(fLanguageMenu);
	}
}


int32
EditorWindow::_ShowModifiedAlert()
{
	BAlert* modifiedAlert = new BAlert(B_TRANSLATE("Unsaved changes"),
		B_TRANSLATE("The file contains unsaved changes. What do you want to do?"),
		B_TRANSLATE("Cancel"), B_TRANSLATE("Discard"), B_TRANSLATE("Save"),
		B_WIDTH_AS_USUAL, B_OFFSET_SPACING, B_STOP_ALERT);
	modifiedAlert->SetShortcut(0, B_ESCAPE);
	return modifiedAlert->Go();
}


void
EditorWindow::_Save()
{
	if(fOpenedFilePath == NULL)
		fSavePanel->Show();
	else
		SaveFile(fOpenedFilePath);
}
