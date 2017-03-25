/*
 * Copyright 2014-2017 Kacper Kasper <kacperkasper@gmail.com>
 * All rights reserved. Distributed under the terms of the MIT license.
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
#include <NodeMonitor.h>
#include <ObjectList.h>
#include <Path.h>
#include <Roster.h>
#include <String.h>

#include <string>
#include <yaml.h>

#include "AppPreferencesWindow.h"
#include "Editor.h"
#include "FindWindow.h"
#include "GoToLineWindow.h"
#include "Languages.h"
#include "Preferences.h"
#include "Styler.h"


#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "EditorWindow"


Preferences* EditorWindow::fPreferences = nullptr;


EditorWindow::EditorWindow()
	:
	BWindow(fPreferences->fWindowRect, gAppName, B_DOCUMENT_WINDOW, 0)
{
	fActivatedGuard = false;

	fModifiedOutside = false;
	fModified = false;
	fReadOnly = false;

	fSearchLastResultStart = -1;
	fSearchLastResultEnd = -1;

	fGoToLineWindow = nullptr;
	fOpenedFilePath = nullptr;
	fOpenedFileMimeType.SetTo("text/plain");

	fCurrentLanguage = "text";

	BMessenger* windowMessenger = new BMessenger(this);
	fOpenPanel = new BFilePanel(B_OPEN_PANEL, windowMessenger);
	fSavePanel = new BFilePanel(B_SAVE_PANEL, windowMessenger, nullptr, 0, false);

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
			.AddItem(B_TRANSLATE("Comment line"), MAINMENU_EDIT_COMMENTLINE, '/')
			.AddItem(B_TRANSLATE("Comment block"), MAINMENU_EDIT_COMMENTBLOCK, '/', B_SHIFT_KEY)
			.AddSeparator()
			.AddMenu(B_TRANSLATE("Line endings"))
				.AddItem(B_TRANSLATE("Unix format"), MAINMENU_EDIT_CONVERTEOLS_UNIX)
				.AddItem(B_TRANSLATE("Windows format"), MAINMENU_EDIT_CONVERTEOLS_WINDOWS)
				.AddItem(B_TRANSLATE("Old Mac format"), MAINMENU_EDIT_CONVERTEOLS_MAC)
			.End()
			.AddSeparator()
			//.AddItem(B_TRANSLATE("File preferences" B_UTF8_ELLIPSIS), MAINMENU_EDIT_FILE_PREFERENCES)
			.AddItem(B_TRANSLATE("Koder preferences" B_UTF8_ELLIPSIS), MAINMENU_EDIT_APP_PREFERENCES, ',')
		.End()
		.AddMenu(B_TRANSLATE("View"))
			.AddMenu(B_TRANSLATE("Special symbols"))
				.AddItem(B_TRANSLATE("Show white space"), MAINMENU_VIEW_SPECIAL_WHITESPACE)
				.AddItem(B_TRANSLATE("Show line endings"), MAINMENU_VIEW_SPECIAL_EOL)
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
	fEditor->SendMessage(SCI_SETIMEINTERACTION, SC_IME_INLINE, 0);
	fEditor->SendMessage(SCI_SETSCROLLWIDTHTRACKING, true, 0);

	fEditor->SendMessage(SCI_USEPOPUP, 0, 0);

	Styler::ApplyGlobal(fEditor, fPreferences->fStyle);
	fEditor->SendMessage(SCI_STYLESETFORE, 253, 0xFF00000);
	fEditor->SendMessage(SCI_STYLESETFORE, 254, 0x00000FF);

	RefreshTitle();
}


void
EditorWindow::New()
{
	be_app->PostMessage(WINDOW_NEW);
}


void
EditorWindow::OpenFile(entry_ref* ref, Sci_Position line, Sci_Position column)
{
	fEditor->SendMessage(SCI_SETREADONLY, false, 0);
		// let us load new file
	if(fOpenedFilePath != nullptr) {
		// stop watching previously opened file
		BEntry open(fOpenedFilePath->Path());
		_MonitorFile(&open, false);
	}

	BEntry entry(ref);
	_MonitorFile(&entry, true);
	entry.GetModificationTime(&fOpenedFileModificationTime);
	fModifiedOutside = false;

	char mimeType[256];
	int32 caretPos = 0;
	BNode node(&entry);
	node.ReadAttr("be:caret_position", B_INT32_TYPE, 0, &caretPos, 4);
	node.ReadAttr("BEOS:TYPE", B_MIME_TYPE, 0, mimeType, 256);

	fReadOnly = true;
	bool canWrite = _CheckPermissions(&node, S_IWUSR | S_IWGRP | S_IWOTH);
	if(canWrite) {
		fReadOnly = false;
	} else {
		BAlert* alert = new BAlert(B_TRANSLATE("Warning"),
			B_TRANSLATE("You don't have permissions to edit this file. The editor will be set to read-only mode."),
			B_TRANSLATE("OK"), nullptr, nullptr, B_WIDTH_AS_USUAL, B_WARNING_ALERT);
		alert->SetShortcut(0, B_ESCAPE);
		alert->Go();
	}

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

	Sci_Position gotoPos;
	if(line != -1) {
		Sci_Position linePos = fEditor->SendMessage(SCI_POSITIONFROMLINE, line, 0);
		if(column != -1) {
			gotoPos = linePos + column;
		} else {
			gotoPos = linePos;
		}
	} else {
		gotoPos = caretPos;
	}
	fEditor->SendMessage(SCI_GOTOPOS, gotoPos, 0);
	fOpenedFileMimeType.SetTo(mimeType);

	char name[B_FILE_NAME_LENGTH];
	entry.GetName(name);
	_SetLanguageByFilename(name);

	fEditor->SendMessage(SCI_SETREADONLY, fReadOnly, 0);

	be_roster->AddToRecentDocuments(ref, gAppMime);

	if(fOpenedFilePath == nullptr)
		fOpenedFilePath = new BPath(&entry);
	else
		fOpenedFilePath->SetTo(&entry);
	RefreshTitle();
}


void
EditorWindow::RefreshTitle()
{
	BString title;
	if(fModified == true)
		title << "*";
	if(fOpenedFilePath != nullptr)
		if(fPreferences->fFullPathInTitle == true) {
			title << fOpenedFilePath->Path();
		} else {
			title << fOpenedFilePath->Leaf();
		}
	else {
		title << B_TRANSLATE("Untitled");
	}
	if(fReadOnly) {
		title << " " << B_TRANSLATE("[read-only]");
	}
	SetTitle(title);
}


void
EditorWindow::SaveFile(entry_ref* ref)
{
	// TODO error checking
	BFile file(ref, B_WRITE_ONLY | B_CREATE_FILE | B_ERASE_FILE);
	if(file.InitCheck() == B_PERMISSION_DENIED) {
		BAlert* alert = new BAlert(B_TRANSLATE("Access denied"),
			B_TRANSLATE("You don't have sufficient permissions to edit this file."),
			B_TRANSLATE("OK"), nullptr, nullptr, B_WIDTH_AS_USUAL, B_STOP_ALERT);
		alert->SetShortcut(0, B_ESCAPE);
		return;
	}
	BNode node(ref);
	_MonitorFile(&node, false);
	int length = fEditor->TextLength() + 1;
	char* buffer = new char[length];
	fEditor->GetText(0, length, buffer);
	file.Write(buffer, length - 1);
	fEditor->SendMessage(SCI_SETSAVEPOINT, 0, 0);
	delete []buffer;

	const char* mimeType = fOpenedFileMimeType.Type();
	_MonitorFile(&node, true);
	node.GetModificationTime(&fOpenedFileModificationTime);
	fModifiedOutside = false;
	BNodeInfo nodeInfo(&node);
	nodeInfo.SetType(mimeType);

	if(fOpenedFilePath != nullptr) {
		delete fOpenedFilePath;
	}
	fOpenedFilePath = new BPath(ref);
	RefreshTitle();
}


bool
EditorWindow::QuitRequested()
{
	bool close = true;
	if(fModified == true) {
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
		if(fOpenedFilePath != nullptr) {
			int32 caretPos = fEditor->SendMessage(SCI_GETCURRENTPOS, 0, 0);
			BNode node(fOpenedFilePath->Path());
			node.WriteAttr("be:caret_position", B_INT32_TYPE, 0, &caretPos, 4);
		}

		if(fGoToLineWindow != nullptr) {
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
	if(message->WasDropped()) {
		BMessage refsReceived(*message);
		refsReceived.what = B_REFS_RECEIVED;
		be_app->PostMessage(&refsReceived);
		return;
	}
	switch(message->what) {
		case SAVE_FILE: {
			_Save();
			message->SendReply((uint32) B_OK);
				// TODO: error handling
		} break;
		case APP_PREFERENCES_CHANGED: {
			_SyncWithPreferences();
		} break;
		case MAINMENU_FILE_NEW:
			New();
		break;
		case MAINMENU_FILE_OPEN: {
			if(fOpenedFilePath != nullptr) {
				BPath parent(*fOpenedFilePath);
				parent.GetParent(&parent);
				fOpenPanel->SetPanelDirectory(parent.Path());
			}
			fOpenPanel->Show();
		} break;
		case MAINMENU_FILE_SAVE: {
			if(fModified == true) {
				if(fReadOnly == true) {
					// TODO: alert
				} else {
					_Save();
				}
			}
		} break;
		case MAINMENU_FILE_SAVEAS: {
			if(fOpenedFilePath != nullptr) {
				BPath parent(*fOpenedFilePath);
				parent.GetParent(&parent);
				fSavePanel->SetPanelDirectory(parent.Path());
			}
			fSavePanel->Show();
		} break;
		case MAINMENU_FILE_QUIT: {
			be_app->PostMessage(B_QUIT_REQUESTED);
		} break;
		case MAINMENU_EDIT_COMMENTLINE: {
			Sci_Position start = fEditor->SendMessage(SCI_GETSELECTIONSTART, 0, 0);
			Sci_Position end = fEditor->SendMessage(SCI_GETSELECTIONEND, 0, 0);
			fEditor->CommentLine(start, end);
		} break;
		case MAINMENU_EDIT_COMMENTBLOCK: {
			Sci_Position start = fEditor->SendMessage(SCI_GETSELECTIONSTART, 0, 0);
			Sci_Position end = fEditor->SendMessage(SCI_GETSELECTIONEND, 0, 0);
			fEditor->CommentBlock(start, end);
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
			if(fGoToLineWindow == nullptr) {
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
		case MAINMENU_LANGUAGE: {
			_SetLanguage(message->GetString("lang", "text"));
		} break;
		case B_SAVE_REQUESTED: {
			entry_ref ref;
			if(message->FindRef("directory", &ref) == B_OK) {
				BString name;
				message->FindString("name", &name);
				BPath path(&ref);
				path.Append(name);
				BEntry entry(path.Path());
				entry.GetRef(&ref);
				SaveFile(&ref);
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
		case EDITOR_SAVEPOINT_LEFT: {
			fModified = true;
			RefreshTitle();
		} break;
		case EDITOR_SAVEPOINT_REACHED: {
			fModified = false;
			RefreshTitle();
		} break;
		case B_ABOUT_REQUESTED:
			be_app->PostMessage(message);
		break;
		case B_REFS_RECEIVED: {
			entry_ref ref;
			if(message->FindRef("refs", &ref) == B_OK) {
				if(fModified == true) {
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
		} break;
		case B_NODE_MONITOR: {
			int32 opcode = message->GetInt32("opcode", 0);
			if(opcode == B_STAT_CHANGED) {
				BEntry entry;
				if(fOpenedFilePath != nullptr) {
					entry.SetTo(fOpenedFilePath->Path());
					time_t mt;
					entry.GetModificationTime(&mt);
					if(mt > fOpenedFileModificationTime) {
						fModifiedOutside = true;
						fOpenedFileModificationTime = mt;
					}

					bool canWrite = _CheckPermissions(&entry, S_IWUSR | S_IWGRP | S_IWOTH);
					fReadOnly = !canWrite;
					fEditor->SendMessage(SCI_SETREADONLY, fReadOnly, 0);
				}
				RefreshTitle();
				// Notification about this is sent when window is activated
			} else if(opcode == B_ENTRY_MOVED) {
				entry_ref ref;
				const char* name;
				message->FindInt32("device", &ref.device);
				message->FindInt64("to directory", &ref.directory);
				message->FindString("name", &name);
				ref.set_name(name);
				_ReloadFile(&ref);
			} else if(opcode == B_ENTRY_REMOVED) {
				// Do not delete fOpenedFilePath here.
				// git removes the file when changing branches. Losing the path
				// because of that is not useful.
				fModified = true;
				RefreshTitle();
			}
		} break;
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
		} break;
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

		if(fModifiedOutside == true) {
			// reload opened file
			BAlert* alert = new BAlert(B_TRANSLATE("File modified"),
				B_TRANSLATE("The file has been modified by another application. What to do?"),
				B_TRANSLATE("Reload"), B_TRANSLATE("Do nothing"), nullptr, B_WIDTH_AS_USUAL, B_EVEN_SPACING, B_INFO_ALERT);
			alert->SetShortcut(1, B_ESCAPE);
			int result = alert->Go();
			if(result == 0) {
				_ReloadFile();
			} else {
				fModified = true;
				RefreshTitle();
			}
			fModifiedOutside = false;
		}
	}
}


const char*
EditorWindow::OpenedFilePath()
{
	if(fOpenedFilePath != nullptr)
		return fOpenedFilePath->Path();
	return "Untitled";
}


/* static */ void
EditorWindow::SetPreferences(Preferences* preferences)
{
	fPreferences = preferences;
}


bool
EditorWindow::_CheckPermissions(BStatable* file, mode_t permissions)
{
	mode_t perms;
	if(file->GetPermissions(&perms) < B_OK) {
		BAlert* alert = new BAlert(B_TRANSLATE("Error"),
			B_TRANSLATE("Failed to read file permissions."),
			B_TRANSLATE("OK"), nullptr, nullptr, B_WIDTH_AS_USUAL, B_STOP_ALERT);
		alert->SetShortcut(0, B_ESCAPE);
		alert->Go();
		return false;
	}

	if(perms & permissions) {
		return true;
	}
	return false;
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
						B_TRANSLATE("Reached the end of the document. No results found."),
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


status_t
EditorWindow::_MonitorFile(BStatable* file, bool enable)
{
	uint32 flags = (enable == true ? B_WATCH_NAME | B_WATCH_STAT : B_STOP_WATCHING);
	node_ref nref;
	file->GetNodeRef(&nref);
	return watch_node(&nref, flags, this);
}


void
EditorWindow::_PopulateLanguageMenu(BMenu* languageMenu)
{
	// Clear the menu first
	int32 count = languageMenu->CountItems();
	languageMenu->RemoveItems(0, count, true);

	Languages::SortAlphabetically();
	char submenuName[] = "\0";
	BObjectList<BMenu> menus;
	for(int32 i = 0; i < Languages::GetCount(); ++i) {
		std::string lang = Languages::GetLanguage(i);
		std::string name = Languages::GetMenuItemName(lang);

		BMessage *msg = new BMessage(MAINMENU_LANGUAGE);
		msg->AddString("lang", lang.c_str());
		BMenuItem *menuItem = new BMenuItem(name.c_str(), msg);

		if(fPreferences->fCompactLangMenu == true) {
			if(submenuName[0] != name[0]) {
				submenuName[0] = name[0];
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
EditorWindow::_ReloadFile(entry_ref* ref)
{
	if(ref == nullptr) {
		// reload file from current location
		entry_ref e;
		BEntry entry(fOpenedFilePath->Path());
		entry.GetRef(&e);
		OpenFile(&e);
	} else {
		// file has been moved
		BEntry entry(ref);
		char name[B_FILE_NAME_LENGTH];
		entry.GetName(name);
		_SetLanguageByFilename(name);

		fOpenedFilePath->SetTo(&entry);
		RefreshTitle();
	}
}


void
EditorWindow::_SetLanguage(std::string lang)
{
	fCurrentLanguage = lang;
	Languages::ApplyLanguage(fEditor, lang.c_str());
	Styler::ApplyGlobal(fEditor, fPreferences->fStyle);
	Styler::ApplyLanguage(fEditor, fPreferences->fStyle, lang.c_str());

	fMainMenu->FindItem(MAINMENU_EDIT_COMMENTLINE)->SetEnabled(fEditor->CanCommentLine());
	fMainMenu->FindItem(MAINMENU_EDIT_COMMENTBLOCK)->SetEnabled(fEditor->CanCommentBlock());
}


void
EditorWindow::_SetLanguageByFilename(const char* filename)
{
	std::string lang;
	// try to match whole filename first, this is needed for e.g. CMake
	bool found = Languages::GetLanguageForExtension(filename, lang);
	if(found == false) {
		const char* extension = strrchr(filename, '.');
		if(extension != nullptr)
			Languages::GetLanguageForExtension(extension + 1, lang);
	}
	_SetLanguage(lang);
}


void
EditorWindow::_SyncWithPreferences()
{
	if(fPreferences != nullptr) {
		fMainMenu->FindItem(MAINMENU_VIEW_SPECIAL_WHITESPACE)->SetMarked(fPreferences->fWhiteSpaceVisible);
		fMainMenu->FindItem(MAINMENU_VIEW_SPECIAL_EOL)->SetMarked(fPreferences->fEOLVisible);

		// reapply styles
		_SetLanguage(fCurrentLanguage);

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
	const char* alertText;
	const char* button0 = B_TRANSLATE("Cancel");
	const char* button1 = B_TRANSLATE("Discard");
	const char* button2;
	if(fReadOnly == true) {
		alertText = B_TRANSLATE("The file contains unsaved changes, but is read-only. What to do?");
		//button2 = B_TRANSLATE("Save as" B_UTF8_ELLIPSIS);
			// FIXME: Race condition when opening file
	} else {
		alertText = B_TRANSLATE("The file contains unsaved changes. What to do?");
		button2 = B_TRANSLATE("Save");
	}
	BAlert* modifiedAlert = new BAlert(B_TRANSLATE("Unsaved changes"),
		alertText, button0, button1, button2, B_WIDTH_AS_USUAL, B_OFFSET_SPACING, B_STOP_ALERT);
	modifiedAlert->SetShortcut(0, B_ESCAPE);
	return modifiedAlert->Go();
}


void
EditorWindow::_Save()
{
	if(fOpenedFilePath == nullptr || fReadOnly == true)
		fSavePanel->Show();
	else {
		BEntry entry(fOpenedFilePath->Path());
		entry_ref ref;
		entry.GetRef(&ref);
		SaveFile(&ref);
	}
	// block until user has chosen location
	while(fSavePanel->IsShowing()) UpdateIfNeeded();
}
