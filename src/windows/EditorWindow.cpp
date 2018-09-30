/*
 * Copyright 2014-2018 Kacper Kasper <kacperkasper@gmail.com>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include "EditorWindow.h"

#include <string>

#include <Alert.h>
#include <Application.h>
#include <Bitmap.h>
#include <Button.h>
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
#include <PopUpMenu.h>
#include <Roster.h>
#include <String.h>
#include <StringFormat.h>
#include <ToolBar.h>

#include <yaml-cpp/yaml.h>

#include "AppPreferencesWindow.h"
#include "Editor.h"
#include "Editorconfig.h"
#include "FindWindow.h"
#include "GoToLineWindow.h"
#include "Languages.h"
#include "Preferences.h"
#include "StatusView.h"
#include "Styler.h"
#include "Utils.h"


#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "EditorWindow"


using BToolBar = BPrivate::BToolBar;


const float kWindowStagger = 17.0f;


Preferences* EditorWindow::fPreferences = nullptr;


namespace {
	enum {
		INCREMENTAL_SEARCH_CHAR			= 'incs',
		INCREMENTAL_SEARCH_BACKSPACE	= 'incb',
		INCREMENTAL_SEARCH_CANCEL		= 'ince',
		INCREMENTAL_SEARCH_COMMIT		= 'incc'
	};
}


EditorWindow::EditorWindow(bool stagger)
	:
	BWindow(fPreferences->fWindowRect, gAppName, B_DOCUMENT_WINDOW, 0),
	fIncrementalSearchTerm(""),
	fIncrementalSearchFilter(new BMessageFilter(B_KEY_DOWN, _IncrementalSearchFilter))
{
	fActivatedGuard = false;

	fModifiedOutside = false;
	fModified = false;
	fReadOnly = false;

	fGoToLineWindow = nullptr;
	fOpenedFilePath = nullptr;
	fOpenedFileMimeType.SetTo("text/plain");

	fCurrentLanguage = "text";

	BMessage openMessage(B_REFS_RECEIVED);
	openMessage.AddPointer("window", this);
	BMessenger* windowMessenger = new BMessenger(this);
	fOpenPanel = new BFilePanel(B_OPEN_PANEL, windowMessenger);
	fOpenPanel->SetMessage(&openMessage);
	fSavePanel = new BFilePanel(B_SAVE_PANEL, windowMessenger, nullptr, 0, false);

	fMainMenu = new BMenuBar("MainMenu");
	BLayoutBuilder::Menu<>(fMainMenu)
		.AddMenu(B_TRANSLATE("File"))
			.AddItem(B_TRANSLATE("New"), MAINMENU_FILE_NEW, 'N')
			.AddSeparator()
			.AddItem(B_TRANSLATE("Open" B_UTF8_ELLIPSIS), MAINMENU_FILE_OPEN, 'O')
			.AddItem(B_TRANSLATE("Reload"), MAINMENU_FILE_RELOAD)
			.AddItem(B_TRANSLATE("Save"), MAINMENU_FILE_SAVE, 'S')
			.AddItem(B_TRANSLATE("Save as" B_UTF8_ELLIPSIS), MAINMENU_FILE_SAVEAS)
			.AddSeparator()
			.AddItem(B_TRANSLATE("Open corresponding" B_UTF8_ELLIPSIS), MAINMENU_FILE_OPEN_CORRESPONDING)
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
			.AddItem(B_TRANSLATE("Comment line"), EDIT_COMMENTLINE, '/')
			.AddItem(B_TRANSLATE("Comment block"), EDIT_COMMENTBLOCK, '/', B_SHIFT_KEY)
			.AddSeparator()
			.AddItem(B_TRANSLATE("Trim trailing whitespace"), MAINMENU_EDIT_TRIMWS)
			.AddSeparator()
			.AddMenu(B_TRANSLATE("Line endings"))
				.AddItem(B_TRANSLATE("Unix format"), MAINMENU_EDIT_CONVERTEOLS_UNIX)
				.AddItem(B_TRANSLATE("Windows format"), MAINMENU_EDIT_CONVERTEOLS_WINDOWS)
				.AddItem(B_TRANSLATE("Old Mac format"), MAINMENU_EDIT_CONVERTEOLS_MAC)
			.End()
			.AddSeparator()
			//.AddItem(B_TRANSLATE("File preferences" B_UTF8_ELLIPSIS), MAINMENU_EDIT_FILE_PREFERENCES)
			.AddItem(B_TRANSLATE("Koder preferences" B_UTF8_ELLIPSIS), MAINMENU_EDIT_APP_PREFERENCES)
		.End()
		.AddMenu(B_TRANSLATE("View"))
			.AddMenu(B_TRANSLATE("Special symbols"))
				.AddItem(B_TRANSLATE("Show white space"), MAINMENU_VIEW_SPECIAL_WHITESPACE)
				.AddItem(B_TRANSLATE("Show line endings"), MAINMENU_VIEW_SPECIAL_EOL)
			.End()
			.AddItem(B_TRANSLATE("Show toolbar"), MAINMENU_VIEW_TOOLBAR)
			.AddItem(B_TRANSLATE("Wrap lines"), MAINMENU_VIEW_WRAPLINES)
		.End()
		.AddMenu(B_TRANSLATE("Search"))
			.AddItem(B_TRANSLATE("Find/Replace" B_UTF8_ELLIPSIS), MAINMENU_SEARCH_FINDREPLACE, 'F')
			.AddItem(B_TRANSLATE("Find next"), MAINMENU_SEARCH_FINDNEXT, 'G')
			.AddItem(B_TRANSLATE("Find selection"), MAINMENU_SEARCH_FINDSELECTION, 'H')
			.AddItem(B_TRANSLATE("Replace and find"), MAINMENU_SEARCH_REPLACEANDFIND, 'T')
			.AddItem(B_TRANSLATE("Incremental search"), MAINMENU_SEARCH_INCREMENTAL, 'I')
			.AddSeparator()
			.AddItem(B_TRANSLATE("Go to line" B_UTF8_ELLIPSIS), MAINMENU_SEARCH_GOTOLINE, ',')
		.End()
		.AddMenu(B_TRANSLATE("Language"))
			.AddItem("Dummy", MAINMENU_LANGUAGE)
		.End()
		.AddMenu(B_TRANSLATE("Help"))
			.AddItem(B_TRANSLATE("About" B_UTF8_ELLIPSIS), B_ABOUT_REQUESTED)
		.End();

	fLanguageMenu = fMainMenu->FindItem(MAINMENU_LANGUAGE)->Menu();
	_PopulateLanguageMenu();

	fContextMenu = new BPopUpMenu("ContextMenu", false, false);
	BLayoutBuilder::Menu<>(fContextMenu)
		.AddItem(B_TRANSLATE("Undo"), B_UNDO, 'Z')
		.AddItem(B_TRANSLATE("Redo"), B_REDO, 'Y')
		.AddSeparator()
		.AddItem(B_TRANSLATE("Cut"), B_CUT, 'X')
		.AddItem(B_TRANSLATE("Copy"), B_COPY, 'C')
		.AddItem(B_TRANSLATE("Paste"), B_PASTE, 'V')
		.AddSeparator()
		.AddItem(B_TRANSLATE("Select all"), B_SELECT_ALL, 'A')
		.AddSeparator()
		.AddItem(B_TRANSLATE("Comment line"), EDIT_COMMENTLINE, '/')
		.AddItem(B_TRANSLATE("Comment block"), EDIT_COMMENTBLOCK, '/', B_SHIFT_KEY);
	fContextMenu->SetTargetForItems(*windowMessenger);

	fEditor = new Editor();
	fEditor->SetPreferences(fPreferences);

	BBitmap icon(BRect(0, 0, 23, 23), 0, B_RGBA32);
	fToolbar = new BToolBar(B_HORIZONTAL);
	GetVectorIcon("open", &icon);
	fToolbar->AddAction(MAINMENU_FILE_OPEN, this, &icon, B_TRANSLATE("Open" B_UTF8_ELLIPSIS));
	GetVectorIcon("reload", &icon);
	fToolbar->AddAction(MAINMENU_FILE_RELOAD, this, &icon, B_TRANSLATE("Reload"));
	GetVectorIcon("save", &icon);
	fToolbar->AddAction(MAINMENU_FILE_SAVE, this, &icon, B_TRANSLATE("Save"));
	GetVectorIcon("save as", &icon);
	fToolbar->AddAction(MAINMENU_FILE_SAVEAS, this, &icon, B_TRANSLATE("Save as" B_UTF8_ELLIPSIS));
	fToolbar->AddSeparator();
	GetVectorIcon("undo", &icon);
	fToolbar->AddAction(B_UNDO, this, &icon, B_TRANSLATE("Undo"));
	GetVectorIcon("redo", &icon);
	fToolbar->AddAction(B_REDO, this, &icon, B_TRANSLATE("Redo"));
	fToolbar->AddSeparator();
	GetVectorIcon("whitespace", &icon);
	fToolbar->AddAction(TOOLBAR_SPECIAL_SYMBOLS, this, &icon, B_TRANSLATE("Special symbols"), nullptr, true);
	fToolbar->AddSeparator();
	GetVectorIcon("preferences", &icon);
	fToolbar->AddAction(MAINMENU_EDIT_APP_PREFERENCES, this, &icon, B_TRANSLATE("Koder preferences" B_UTF8_ELLIPSIS));
	GetVectorIcon("find", &icon);
	fToolbar->AddAction(MAINMENU_SEARCH_FINDREPLACE, this, &icon, B_TRANSLATE("Find/Replace" B_UTF8_ELLIPSIS));
	fToolbar->AddGlue();

	BGroupLayout *layout = new BGroupLayout(B_VERTICAL, 0);
	SetLayout(layout);
	layout->AddView(fMainMenu);
	layout->AddView(fToolbar);
	layout->AddView(fEditor);
	layout->SetInsets(0, 0, -1, -1);
	SetKeyMenuBar(fMainMenu);

	fEditor->SendMessage(SCI_GRABFOCUS);

	_SyncWithPreferences();

	fEditor->SendMessage(SCI_SETADDITIONALSELECTIONTYPING, true, 0);
	fEditor->SendMessage(SCI_SETIMEINTERACTION, SC_IME_INLINE, 0);
	fEditor->SendMessage(SCI_SETSCROLLWIDTH, fEditor->Bounds().Width());
	fEditor->SendMessage(SCI_SETSCROLLWIDTHTRACKING, true, 0);
	fEditor->SendMessage(SCI_SETWRAPVISUALFLAGS, SC_WRAPVISUALFLAG_MARGIN, 0);
	fEditor->SendMessage(SCI_SETSAVEPOINT, 0, 0);

	fEditor->SendMessage(SCI_USEPOPUP, 0, 0);

	Styler::ApplyGlobal(fEditor, fPreferences->fStyle.c_str());

	RefreshTitle();

	if(stagger == true) {
		MoveBy(kWindowStagger, kWindowStagger);
	}
}


EditorWindow::~EditorWindow()
{
	RemoveCommonFilter(fIncrementalSearchFilter.get());
}


void
EditorWindow::New()
{
	BMessage message(WINDOW_NEW);
	message.AddPointer("window", this);
	be_app->PostMessage(&message);
}


void
EditorWindow::OpenFile(entry_ref* ref, Sci_Position line, Sci_Position column)
{
	fEditor->SetReadOnly(false);
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

	fEditor->SetReadOnly(fReadOnly);
	fEditor->SetRef(*ref);

	be_roster->AddToRecentDocuments(ref, gAppMime);

	if(fOpenedFilePath == nullptr)
		fOpenedFilePath = new BPath(&entry);
	else
		fOpenedFilePath->SetTo(&entry);
	RefreshTitle();

	// load .editorconfig and apply settings
	_SyncWithPreferences();
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

	if(fFilePreferences.fTrimTrailingWhitespace.value_or(
			fPreferences->fTrimTrailingWhitespaceOnSave) == true) {
		fEditor->TrimTrailingWhitespace();
	}

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
		message->what = B_REFS_RECEIVED;
	}
	switch(message->what) {
		case INCREMENTAL_SEARCH_CHAR: {
			const char* character = message->GetString("character", "");
			fIncrementalSearchTerm.append(character);
			fEditor->IncrementalSearch(fIncrementalSearchTerm);
		} break;
		case INCREMENTAL_SEARCH_BACKSPACE: {
			fIncrementalSearchTerm.pop_back();
			fEditor->IncrementalSearch(fIncrementalSearchTerm);
		} break;
		case INCREMENTAL_SEARCH_CANCEL: {
			fEditor->IncrementalSearchCancel();
			fIncrementalSearchTerm = "";
			RemoveCommonFilter(fIncrementalSearchFilter.get());
		} break;
		case INCREMENTAL_SEARCH_COMMIT: {
			fEditor->IncrementalSearchCommit(fIncrementalSearchTerm);
			fIncrementalSearchTerm = "";
			RemoveCommonFilter(fIncrementalSearchFilter.get());
		} break;
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
		case MAINMENU_FILE_RELOAD: {
			BAlert* alert = new BAlert(B_TRANSLATE("Unsaved changes"),
				B_TRANSLATE("Your changes will be lost."),
				B_TRANSLATE("Cancel"), B_TRANSLATE("Reload"), nullptr,
				B_WIDTH_AS_USUAL, B_EVEN_SPACING, B_STOP_ALERT);
			alert->SetShortcut(0, B_ESCAPE);
			int32 result = alert->Go();
			switch(result) {
			case 0: break;
			case 1:
				_ReloadFile();
			break;
			}
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
		case MAINMENU_FILE_OPEN_CORRESPONDING: {
			if(fOpenedFilePath != nullptr) {
				_OpenCorrespondingFile(*fOpenedFilePath, fCurrentLanguage);
			}
		} break;
		case MAINMENU_FILE_QUIT: {
			be_app->PostMessage(B_QUIT_REQUESTED);
		} break;
		case EDIT_COMMENTLINE: {
			Sci_Position start = fEditor->SendMessage(SCI_GETSELECTIONSTART, 0, 0);
			Sci_Position end = fEditor->SendMessage(SCI_GETSELECTIONEND, 0, 0);
			fEditor->CommentLine(start, end);
		} break;
		case EDIT_COMMENTBLOCK: {
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
		case MAINMENU_EDIT_TRIMWS: {
			fEditor->TrimTrailingWhitespace();
		} break;
		case MAINMENU_EDIT_APP_PREFERENCES: {
			be_app->PostMessage(message);
		} break;
		case MAINMENU_SEARCH_FINDREPLACE: {
			if(fEditor->SendMessage(SCI_GETSELECTIONEMPTY, 0, 0) == false) {
				int length = fEditor->SendMessage(SCI_GETSELTEXT, 0, 0);
				std::string selection(length, '\0');
				fEditor->SendMessage(SCI_GETSELTEXT, 0, (sptr_t) &selection[0]);
				message->AddString("selection", selection.c_str());
			}
			be_app->PostMessage(message);
		} break;
		case MAINMENU_SEARCH_FINDNEXT: {
			fEditor->FindNext();
		} break;
		case MAINMENU_SEARCH_FINDSELECTION: {
			fEditor->FindSelection();
		} break;
		case MAINMENU_SEARCH_REPLACEANDFIND: {
			fEditor->ReplaceAndFind();
		} break;
		case MAINMENU_SEARCH_INCREMENTAL: {
			AddCommonFilter(fIncrementalSearchFilter.get());
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
			bool pressed = fPreferences->fWhiteSpaceVisible && fPreferences->fEOLVisible;
			fToolbar->SetActionPressed(TOOLBAR_SPECIAL_SYMBOLS, pressed);
		} break;
		case MAINMENU_VIEW_SPECIAL_EOL: {
			fPreferences->fEOLVisible = !fPreferences->fEOLVisible;
			fMainMenu->FindItem(message->what)->SetMarked(fPreferences->fEOLVisible);
			fEditor->SendMessage(SCI_SETVIEWEOL, fPreferences->fEOLVisible, 0);
			bool pressed = fPreferences->fWhiteSpaceVisible && fPreferences->fEOLVisible;
			fToolbar->SetActionPressed(TOOLBAR_SPECIAL_SYMBOLS, pressed);
		} break;
		case MAINMENU_VIEW_TOOLBAR: {
			fPreferences->fToolbar = !fPreferences->fToolbar;
			fMainMenu->FindItem(message->what)->SetMarked(fPreferences->fToolbar);
			if(fPreferences->fToolbar == true)
				fToolbar->Show();
			else
				fToolbar->Hide();
		} break;
		case MAINMENU_VIEW_WRAPLINES: {
			fPreferences->fWrapLines = !fPreferences->fWrapLines;
			fMainMenu->FindItem(message->what)->SetMarked(fPreferences->fWrapLines);
			fEditor->SendMessage(SCI_SETWRAPMODE, fPreferences->fWrapLines ?
				SC_WRAP_WORD : SC_WRAP_NONE, 0);
		} break;
		case MAINMENU_LANGUAGE: {
			_SetLanguage(message->GetString("lang", "text"));
		} break;
		case TOOLBAR_SPECIAL_SYMBOLS: {
			bool pressed = fPreferences->fWhiteSpaceVisible && fPreferences->fEOLVisible;
			if(pressed == true) {
				fPreferences->fWhiteSpaceVisible = false;
				fPreferences->fEOLVisible = false;
			} else {
				fPreferences->fWhiteSpaceVisible = true;
				fPreferences->fEOLVisible = true;
			}
			fMainMenu->FindItem(MAINMENU_VIEW_SPECIAL_WHITESPACE)->SetMarked(fPreferences->fWhiteSpaceVisible);
			fMainMenu->FindItem(MAINMENU_VIEW_SPECIAL_EOL)->SetMarked(fPreferences->fEOLVisible);
			fEditor->SendMessage(SCI_SETVIEWWS, fPreferences->fWhiteSpaceVisible, 0);
			fEditor->SendMessage(SCI_SETVIEWEOL, fPreferences->fEOLVisible, 0);
			fToolbar->SetActionPressed(TOOLBAR_SPECIAL_SYMBOLS, !pressed);
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
			_SyncEditMenus();
		} break;
		case B_COPY: {
			fEditor->SendMessage(SCI_COPY, 0, 0);
		} break;
		case B_PASTE: {
			fEditor->SendMessage(SCI_PASTE, 0, 0);
			_SyncEditMenus();
		} break;
		case B_SELECT_ALL: {
			fEditor->SendMessage(SCI_SELECTALL, 0, 0);
		} break;
		case B_UNDO: {
			fEditor->SendMessage(SCI_UNDO, 0, 0);
			_SyncEditMenus();
		} break;
		case B_REDO: {
			fEditor->SendMessage(SCI_REDO, 0, 0);
			_SyncEditMenus();
		} break;
		case EDITOR_SAVEPOINT_LEFT: {
			fModified = true;
			RefreshTitle();
			fMainMenu->FindItem(MAINMENU_FILE_RELOAD)->SetEnabled(fModified);
			fMainMenu->FindItem(MAINMENU_FILE_SAVE)->SetEnabled(fModified);
			fToolbar->SetActionEnabled(MAINMENU_FILE_RELOAD, fModified);
			fToolbar->SetActionEnabled(MAINMENU_FILE_SAVE, fModified);
		} break;
		case EDITOR_SAVEPOINT_REACHED: {
			fModified = false;
			RefreshTitle();
			fMainMenu->FindItem(MAINMENU_FILE_RELOAD)->SetEnabled(fModified);
			fMainMenu->FindItem(MAINMENU_FILE_SAVE)->SetEnabled(fModified);
			fToolbar->SetActionEnabled(MAINMENU_FILE_RELOAD, fModified);
			fToolbar->SetActionEnabled(MAINMENU_FILE_SAVE, fModified);
		} break;
		case EDITOR_UPDATEUI: {
			if(fPreferences->fHighlightTrailingWhitespace) {
				fEditor->HighlightTrailingWhitespace();
			}
			_SyncEditMenus();
		} break;
		case EDITOR_CONTEXT_MENU: {
			BPoint where;
			if(message->FindPoint("where", &where) == B_OK) {
				where = ConvertToScreen(where);
				fContextMenu->Go(where, true, true);
			}
		} break;
		case B_ABOUT_REQUESTED:
			be_app->PostMessage(message);
		break;
		case B_REFS_RECEIVED: {
			entry_ref ref;
			if(message->FindRef("refs", &ref) == B_OK) {
				if(fOpenedFilePath == nullptr && fModified == false
					&& !fPreferences->fAlwaysOpenInNewWindow) {
					OpenFile(&ref);
				} else {
					message->AddPointer("window", this);
					be_app->PostMessage(message);
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
					fEditor->SetReadOnly(fReadOnly);
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
		case FINDWINDOW_REPLACEALL:
		case FINDWINDOW_REPLACEFIND: {
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


void
EditorWindow::FrameMoved(BPoint origin)
{
	fPreferences->fWindowRect.OffsetTo(origin);
}


void
EditorWindow::FrameResized(float width, float height)
{
	// Workaround: layouted views don't get that event
	fEditor->FrameResized(width, height);
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
	bool regex = message->GetBool("regex");
	const char* findText = message->GetString("findText", "");
	const char* replaceText = message->GetString("replaceText", "");

	switch(message->what) {
		case FINDWINDOW_REPLACE:
		case FINDWINDOW_REPLACEFIND:
			fEditor->Replace(replaceText, regex);
			if(message->what == FINDWINDOW_REPLACE) break;
		case FINDWINDOW_FIND: {
			if(newSearch == true) {
				fEditor->ResetFindReplace();
			}
			bool found;
			found = fEditor->Find(message);
			if(found == false) {
				BAlert* alert = new BAlert(B_TRANSLATE("Searching finished"),
					B_TRANSLATE("Reached the end of the target. No results found."),
					B_TRANSLATE("OK"), nullptr, nullptr, B_WIDTH_AS_USUAL, B_OFFSET_SPACING, B_INFO_ALERT);
				alert->SetShortcut(0, B_ESCAPE);
				alert->Go();
			}
		} break;
		case FINDWINDOW_REPLACEALL: {
			int occurences = fEditor->ReplaceAll(findText, replaceText,
				matchCase, matchWord, inSelection, regex);
			BString alertMessage;
			static BStringFormat format(B_TRANSLATE("Replaced "
				"{0, plural, one{# occurence} other{# occurences}}."));
			format.Format(alertMessage, occurences);
			BAlert* alert = new BAlert(B_TRANSLATE("Replacement finished"),
				alertMessage, B_TRANSLATE("OK"), nullptr, nullptr,
				B_WIDTH_AS_USUAL, B_OFFSET_SPACING, B_INFO_ALERT);
			alert->SetShortcut(0, B_ESCAPE);
			alert->Go();
		} break;
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
EditorWindow::_PopulateLanguageMenu()
{
	// Clear the menu first
	int32 count = fLanguageMenu->CountItems();
	fLanguageMenu->RemoveItems(0, count, true);

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
			fLanguageMenu->AddItem(menuItem);
		}
	}
	if(fPreferences->fCompactLangMenu == true) {
		int32 menusCount = menus.CountItems();
		for(int32 i = 0; i < menusCount; i++) {
			if(menus.ItemAt(i)->CountItems() > 1) {
				fLanguageMenu->AddItem(menus.ItemAt(i));
			} else {
				fLanguageMenu->AddItem(menus.ItemAt(i)->RemoveItem((int32) 0));
			}
		}
	}
}


void
EditorWindow::_ReloadFile(entry_ref* ref)
{
	if(fOpenedFilePath == nullptr) return;

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
	const auto mapping = Languages::ApplyLanguage(fEditor, lang.c_str());
	Styler::ApplyGlobal(fEditor, fPreferences->fStyle.c_str());
	Styler::ApplyLanguage(fEditor, mapping);

	fEditor->SetType(Languages::GetMenuItemName(lang));

	fMainMenu->FindItem(EDIT_COMMENTLINE)->SetEnabled(fEditor->CanCommentLine());
	fMainMenu->FindItem(EDIT_COMMENTBLOCK)->SetEnabled(fEditor->CanCommentBlock());
	fMainMenu->FindItem(MAINMENU_FILE_OPEN_CORRESPONDING)->SetEnabled(lang == "c" || lang == "cpp");

	fContextMenu->FindItem(EDIT_COMMENTLINE)->SetEnabled(fEditor->CanCommentLine());
	fContextMenu->FindItem(EDIT_COMMENTBLOCK)->SetEnabled(fEditor->CanCommentBlock());
}


void
EditorWindow::_SetLanguageByFilename(const char* filename)
{
	std::string lang;
	// try to match whole filename first, this is needed for e.g. CMake
	bool found = Languages::GetLanguageForExtension(filename, lang);
	if(found == false) {
		const std::string extension = GetFileExtension(filename);
		if(!extension.empty())
			Languages::GetLanguageForExtension(extension.c_str(), lang);
	}
	_SetLanguage(lang);
}


void
EditorWindow::_OpenCorrespondingFile(const BPath &file, const std::string lang)
{
	if(lang != "c" && lang != "cpp")
		return;

	const std::vector<std::string> extensionsToTryC{"h", "hh", "hxx", "hpp"};
	const std::vector<std::string> extensionsToTryH{"c", "cc", "cxx", "cpp"};
	const std::vector<std::string>* extensionsToTry = &extensionsToTryC;
	if(GetFileExtension(file.Leaf())[0] == 'h') {
		extensionsToTry = &extensionsToTryH;
	}
	BPath parent;
	if(file.GetParent(&parent) == B_OK) {
		const BDirectory parentDir(parent.Path());

		const std::string filename = GetFileName(file.Leaf());
		for(auto &ext : *extensionsToTry) {
			BEntry fileToTry(&parentDir, (filename + '.' + ext).c_str());
			if(fileToTry.Exists()) {
				BMessage openFile(B_REFS_RECEIVED);
				entry_ref ref;
				if(fileToTry.GetRef(&ref) == B_OK) {
					openFile.AddRef("refs", &ref);
					openFile.AddPointer("window", this);
					be_app->PostMessage(&openFile);
					return;
				}
			}
		}
		BAlert* notFoundAlert = new BAlert(B_TRANSLATE("Open corresponding file"),
			B_TRANSLATE("Corresponding file not found."), B_TRANSLATE("OK"),
			nullptr, nullptr, B_WIDTH_AS_USUAL, B_OFFSET_SPACING, B_STOP_ALERT);
		notFoundAlert->Go();
	}
}


void
EditorWindow::_LoadEditorconfig()
{
	if(fOpenedFilePath == nullptr) return;

	BPath editorconfig;
	if(Editorconfig::Find(fOpenedFilePath, &editorconfig)) {
		BMessage allProps;
		if(Editorconfig::Parse(editorconfig.Path(), &allProps)) {
			BMessage props;
			Editorconfig::MatchFilename(fOpenedFilePath->Path(), &allProps, &props);
			char* name;
			uint32 type;
			int32 count;
			for(int32 i = 0;
					props.GetInfo(B_STRING_TYPE, i, &name, &type, &count) == B_OK;
					i++) {
				BString propName(name);
				BString value;
				props.FindString(propName, &value);
				propName.ToLower();
				if(propName == "end_of_line") {
					value.ToLower();
					uint8 eolMode = SC_EOL_LF;
					if(value == "lf")
						eolMode = SC_EOL_LF;
					else if(value == "cr")
						eolMode = SC_EOL_CR;
					else if(value == "crlf")
						eolMode = SC_EOL_CRLF;
					fFilePreferences.fEOLMode = std::make_optional(eolMode);
				} else if(propName == "tab_width") {
					uint8 tabWidth = std::stoi(value.String());
					fFilePreferences.fTabWidth = std::make_optional(tabWidth);
				} else if(propName == "indent_style") {
					value.ToLower();
					bool tabsToSpaces = (value == "space");
					fFilePreferences.fTabsToSpaces = std::make_optional(tabsToSpaces);
				} else if(propName == "indent_size") {
					if(value.ToLower() == "tab")
						fFilePreferences.fIndentWidth = fFilePreferences.fTabWidth;
					else {
						uint8 indentWidth = std::stoi(value.String());
						fFilePreferences.fIndentWidth = std::make_optional(indentWidth);
					}
				} else if(propName == "trim_trailing_whitespace") {
					bool trim = (value.ToLower() == "true");
					fFilePreferences.fTrimTrailingWhitespace = std::make_optional(trim);
				}
			}
		}
	}
}


void
EditorWindow::_SyncWithPreferences()
{
	if(fPreferences != nullptr) {
		if(fPreferences->fUseEditorconfig) {
			_LoadEditorconfig();
		} else {
			// reset file preferences so they aren't picked up later
			fFilePreferences.fTabWidth.reset();
			fFilePreferences.fIndentWidth.reset();
			fFilePreferences.fTabsToSpaces.reset();
			fFilePreferences.fTrimTrailingWhitespace.reset();
			fFilePreferences.fEOLMode.reset();
		}

		bool pressed = fPreferences->fWhiteSpaceVisible && fPreferences->fEOLVisible;
		fToolbar->SetActionPressed(TOOLBAR_SPECIAL_SYMBOLS, pressed);
		fMainMenu->FindItem(MAINMENU_VIEW_SPECIAL_WHITESPACE)->SetMarked(fPreferences->fWhiteSpaceVisible);
		fMainMenu->FindItem(MAINMENU_VIEW_SPECIAL_EOL)->SetMarked(fPreferences->fEOLVisible);
		fMainMenu->FindItem(MAINMENU_VIEW_TOOLBAR)->SetMarked(fPreferences->fToolbar);
		fMainMenu->FindItem(MAINMENU_VIEW_WRAPLINES)->SetMarked(fPreferences->fWrapLines);

		// reapply styles
		_SetLanguage(fCurrentLanguage);

		fEditor->SendMessage(SCI_SETVIEWEOL, fPreferences->fEOLVisible, 0);
		fEditor->SendMessage(SCI_SETVIEWWS, fPreferences->fWhiteSpaceVisible, 0);
		fEditor->SendMessage(SCI_SETTABWIDTH,
			fFilePreferences.fTabWidth.value_or(fPreferences->fTabWidth), 0);
		fEditor->SendMessage(SCI_SETUSETABS,
			!fFilePreferences.fTabsToSpaces.value_or(fPreferences->fTabsToSpaces), 0);
		fEditor->SendMessage(SCI_SETINDENT,
			fFilePreferences.fIndentWidth.value_or(0), 0);
		fEditor->SendMessage(SCI_SETCARETLINEVISIBLE, fPreferences->fLineHighlighting, 0);
		fEditor->SendMessage(SCI_SETCARETLINEVISIBLEALWAYS, true, 0);
		fEditor->SendMessage(SCI_SETCARETLINEFRAME, fPreferences->fLineHighlightingMode ? 2 : 0);

		if(fFilePreferences.fEOLMode) {
			fEditor->SendMessage(SCI_SETEOLMODE, fFilePreferences.fEOLMode.value_or(SC_EOL_LF), 0);
		}

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

		if(fPreferences->fWrapLines == true) {
			fEditor->SendMessage(SCI_SETWRAPMODE, SC_WRAP_WORD, 0);
		} else {
			fEditor->SendMessage(SCI_SETWRAPMODE, SC_WRAP_NONE, 0);
		}

		if(fPreferences->fHighlightTrailingWhitespace == true) {
			fEditor->HighlightTrailingWhitespace();
		} else {
			fEditor->ClearHighlightedWhitespace();
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

		if(!IsHidden()) {
			if(fPreferences->fToolbar == true)
				while(fToolbar->IsHidden()) fToolbar->Show();
			else
				while(!fToolbar->IsHidden()) fToolbar->Hide();
		}

		RefreshTitle();

		// TODO Do this only if language menu preference has changed
		_PopulateLanguageMenu();
	}
}


void
EditorWindow::_SyncEditMenus()
{
	bool canUndo = fEditor->SendMessage(SCI_CANUNDO, 0, 0);
	bool canRedo = fEditor->SendMessage(SCI_CANREDO, 0, 0);
	bool canPaste = fEditor->SendMessage(SCI_CANPASTE, 0, 0);
	bool selectionEmpty = fEditor->SendMessage(SCI_GETSELECTIONEMPTY, 0, 0);
	fMainMenu->FindItem(B_UNDO)->SetEnabled(canUndo);
	fMainMenu->FindItem(B_REDO)->SetEnabled(canRedo);
	fMainMenu->FindItem(B_PASTE)->SetEnabled(canPaste);
	fMainMenu->FindItem(B_CUT)->SetEnabled(!selectionEmpty);
	fMainMenu->FindItem(B_COPY)->SetEnabled(!selectionEmpty);
	fContextMenu->FindItem(B_UNDO)->SetEnabled(canUndo);
	fContextMenu->FindItem(B_REDO)->SetEnabled(canRedo);
	fContextMenu->FindItem(B_PASTE)->SetEnabled(canPaste);
	fContextMenu->FindItem(B_CUT)->SetEnabled(!selectionEmpty);
	fContextMenu->FindItem(B_COPY)->SetEnabled(!selectionEmpty);
	fToolbar->SetActionEnabled(B_UNDO, canUndo);
	fToolbar->SetActionEnabled(B_REDO, canRedo);
	if(fEditor->CanCommentBlock()) {
		fMainMenu->FindItem(EDIT_COMMENTBLOCK)->SetEnabled(!selectionEmpty);
		fContextMenu->FindItem(EDIT_COMMENTBLOCK)->SetEnabled(!selectionEmpty);
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


filter_result
EditorWindow::_IncrementalSearchFilter(BMessage* message, BHandler** target,
	BMessageFilter* messageFilter)
{
	if(message->what == B_KEY_DOWN) {
		BLooper *looper = messageFilter->Looper();
		const char* bytes;
		message->FindString("bytes", &bytes);
		if(bytes[0] == B_RETURN) {
			looper->PostMessage(INCREMENTAL_SEARCH_COMMIT);
		} else if(bytes[0] == B_ESCAPE) {
			looper->PostMessage(INCREMENTAL_SEARCH_CANCEL);
		} else if(bytes[0] == B_BACKSPACE) {
			looper->PostMessage(INCREMENTAL_SEARCH_BACKSPACE);
		} else {
			BMessage msg(INCREMENTAL_SEARCH_CHAR);
			msg.AddString("character", &bytes[0]);
			messageFilter->Looper()->PostMessage(&msg);
		}
		return B_SKIP_MESSAGE;
	}
	return B_DISPATCH_MESSAGE;
}
