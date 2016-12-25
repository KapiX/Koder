/*
 * Koder is a code editor for Haiku based on Scintilla.
 *
 * Copyright (C) 2016 Kacper Kasper <kacperkasper@gmail.com>
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

#ifndef QUITALERT_H
#define QUITALERT_H


#include <ObjectList.h>
#include <Window.h>

#include <string>
#include <vector>


class BButton;
class BCheckBox;
class BStringView;

class EditorWindow;


class QuitAlert : public BWindow {
public:
								QuitAlert(const std::vector<std::string> &unsavedFiles);
								~QuitAlert();

	void						MessageReceived(BMessage* message);
	std::vector<bool>			Go();
private:
	enum Actions {
		SAVE_ALL		= 'sval',
		SAVE_SELECTED	= 'svsl',
		DONT_SAVE		= 'dnsv'
	};
	const std::vector<std::string>	fUnsavedFiles;
	BStringView*					fMessageString;
	BButton*						fSaveAll;
	BButton*						fSaveSelected;
	BButton*						fDontSave;
	BButton*						fCancel;
	std::vector<BCheckBox*>			fCheckboxes;
	std::vector<bool>				fAlertValue;
	sem_id							fAlertSem;

	void							_InitInterface();
};


#endif // QUITALERT_H
