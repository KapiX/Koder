/*
 * Copyright 2016-2017 Kacper Kasper <kacperkasper@gmail.com>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include "QuitAlert.h"

#include <Application.h>
#include <Button.h>
#include <Catalog.h>
#include <CheckBox.h>
#include <LayoutBuilder.h>
#include <StringView.h>

#include <string>
#include <vector>

#include "EditorWindow.h"
#include "Utils.h"


#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "QuitAlert"


namespace {
const int kSemTimeOut = 50000;
}


QuitAlert::QuitAlert(const std::vector<std::string> &unsavedFiles)
	:
	BWindow(BRect(100, 100, 200, 200), B_TRANSLATE("Unsaved files"), B_MODAL_WINDOW,
		B_NOT_CLOSABLE | B_NOT_ZOOMABLE | B_NOT_RESIZABLE | B_AUTO_UPDATE_SIZE_LIMITS, 0),
	fUnsavedFiles(unsavedFiles),
	fAlertValue(0),
	fCheckboxes(unsavedFiles.size(), nullptr)
{
	_InitInterface();
	CenterOnScreen();
}


QuitAlert::~QuitAlert()
{
	if (fAlertSem >= B_OK)
		delete_sem(fAlertSem);
}


void
QuitAlert::_InitInterface()
{
	fMessageString = new BStringView("message", B_TRANSLATE("There are unsaved changes.\nSelect the files to save."));
	fSaveAll = new BButton(B_TRANSLATE("Save all"), new BMessage((uint32) Actions::SAVE_ALL));
	fSaveSelected = new BButton(B_TRANSLATE("Save selected"), new BMessage((uint32) Actions::SAVE_SELECTED));
	fDontSave = new BButton(B_TRANSLATE("Don't save"), new BMessage((uint32) Actions::DONT_SAVE));
	fCancel = new BButton(B_TRANSLATE("Cancel"), new BMessage(B_QUIT_REQUESTED));
	fCancel->MakeDefault(true);
	BGroupLayout* files = new BGroupLayout(B_VERTICAL, 5);
	BLayoutBuilder::Group<>(this, B_VERTICAL, 5)
		.Add(fMessageString)
		.Add(files)
		.AddGroup(B_HORIZONTAL, 5)
			.Add(fSaveAll)
			.Add(fSaveSelected)
			.Add(fDontSave)
			.AddGlue()
			.Add(fCancel)
		.End()
		.SetInsets(5, 5, 5, 5);

	EditorWindow* current;
	for(int i = 0; i < fUnsavedFiles.size(); ++i) {
		fCheckboxes[i] = new BCheckBox("file", fUnsavedFiles[i].c_str(), new BMessage((uint32) i));
		SetChecked(fCheckboxes[i]);
		files->AddView(fCheckboxes[i]);
	}
}


// borrowed from BAlert
std::vector<bool>
QuitAlert::Go()
{
	fAlertSem = create_sem(0, "AlertSem");
	if (fAlertSem < 0) {
		Quit();
		return std::vector<bool>(0);
	}

	// Get the originating window, if it exists
	BWindow* window = dynamic_cast<BWindow*>(
		BLooper::LooperForThread(find_thread(nullptr)));

	Show();

	if (window != nullptr) {
		status_t status;
		for (;;) {
			do {
				status = acquire_sem_etc(fAlertSem, 1, B_RELATIVE_TIMEOUT,
					kSemTimeOut);
				// We've (probably) had our time slice taken away from us
			} while (status == B_INTERRUPTED);

			if (status == B_BAD_SEM_ID) {
				// Semaphore was finally nuked in MessageReceived
				break;
			}
			window->UpdateIfNeeded();
		}
	} else {
		// No window to update, so just hang out until we're done.
		while (acquire_sem(fAlertSem) == B_INTERRUPTED) {
		}
	}

	// Have to cache the value since we delete on Quit()
	auto value = fAlertValue;
	if (Lock())
		Quit();

	return value;
}


void
QuitAlert::MessageReceived(BMessage* message)
{
	switch(message->what) {
		case Actions::SAVE_ALL: {
			fAlertValue = std::vector<bool>(fUnsavedFiles.size(), true);
		} break;
		case Actions::SAVE_SELECTED: {
			fAlertValue = std::vector<bool>(fUnsavedFiles.size(), false);
			for(uint32 i = 0; i < fCheckboxes.size(); ++i) {
				fAlertValue[i] = fCheckboxes[i]->Value() ? true : false;
			}
		} break;
		case Actions::DONT_SAVE: {
			fAlertValue = std::vector<bool>(fUnsavedFiles.size(), false);
		} break;
		default: {
			BWindow::MessageReceived(message);
		} return;
	}
	delete_sem(fAlertSem);
	fAlertSem = -1;
}
