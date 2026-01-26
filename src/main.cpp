/*
 * Copyright 2014-2019 Kacper Kasper <kacperkasper@gmail.com>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include "App.h"

#include <Roster.h>

#include <iostream>
#include <getopt.h>

#include "EditorWindow.h"
#include "Utils.h"


void
_PrintUsage()
{
	std::cerr
		<< "Usage: Koder [options] file..." << std::endl
		<< "Options:" << std::endl
		<< "  -h, --help\t\tPrints this message." << std::endl
		<< "  -w, --wait\t\tWait for the window to quit before returning." << std::endl
		<< "\t\t\tOpening in window stacks is not supported in this mode." << std::endl
		<< "\t\t\tCurrently accepts only one filename." << std::endl;
}


void
_RunApp(bool suppressWindow)
{
	App* app = new App();
	app->Init(suppressWindow);
	app->Run();
	delete app;
}


int
main(int argc, char** argv)
{
	// start the app immediately if we're being launched from the GUI in case we need to handle
	// an initial BMessage, otherwise the message will be lost when we Launch() in the background
	if(isatty(STDOUT_FILENO) == 0) {
		_RunApp(false);
		return 0;
	}

	int option_index = 0;
	struct option long_options[] = {
		{"help", no_argument, 0, 'h'},
		{"wait", no_argument, 0, 'w'},
		{"launchkoderapp", no_argument, 0, 0},
		{0, 0, 0, 0}};

	bool waitForExit = false;
	int c;
	while((c = getopt_long(argc, argv, "hw", long_options, &option_index)) != -1) {
		switch(c) {
			case 'w': {
				waitForExit = true;
			} break;
			case 'h': {
				_PrintUsage();
				return 1;
			} break;
			case 0: {
				if(strcmp(long_options[option_index].name, "launchkoderapp") == 0) {
					_RunApp(true);
					return 0;
				}
				return 1;
			} break;
			default:
				_PrintUsage();
				return 1;
		}
	}

	if(waitForExit == true && argc - optind > 1) {
		std::cerr << "Error: Only one filename allowed when using --wait." << std::endl;
		return 1;
	}

	BMessage windowMessage;
	if(waitForExit == true) {
		windowMessage.what = WINDOW_NEW_WITH_QUIT_REPLY;
	} else if(waitForExit == false && optind == argc) {
		// no file arguments, open a blank window
		windowMessage.what = WINDOW_NEW;
	} else {
		windowMessage.what = B_REFS_RECEIVED;
	}

	while(optind < argc) {
		int32 line, column;
		BPath filePath(ParseFileArgument(argv[optind++], &line, &column).c_str(), nullptr, true);
		if(filePath.InitCheck() != B_OK) {
			std::cerr << "Error: Invalid file path specified." << std::endl;
			return 1;
		}

		entry_ref ref;
		BEntry entry(filePath.Path(), true);
		if(entry.InitCheck() != B_OK || (entry.Exists() == true && entry.IsFile() == false)) {
			std::cerr << "Error: Specified path is not a regular file." << std::endl;
			return 1;
		}
		if(entry.GetRef(&ref) != B_OK) {
			std::cerr << "Error: Unable to get entry_ref for path." << std::endl;
			return 1;
		}
		// always add column and line so that the count is the same as the number of refs
		windowMessage.AddRef("refs", &ref);
		windowMessage.AddInt32("be:column", column);
		windowMessage.AddInt32("be:line", line);
	}

	entry_ref appRef;
	if(get_ref_for_path(argv[0], &appRef) != B_OK) {
		std::cerr << "Error: Unable to determine Koder application path." << std::endl;
		return 1;
	}

	BRoster roster;
	team_id team;
	const char* args[] = { "--launchkoderapp", nullptr };
	// Use the entry_ref version of Launch() to make sure B_SINGLE_LAUNCH works correctly
	status_t status = roster.Launch(&appRef, 1, args, &team);
	if(status != B_OK && status != B_ALREADY_RUNNING) {
		std::cerr << "Error: Unable to launch Koder." << std::endl;
		return 1;
	}

	BMessenger messenger(gAppMime, team);
	BMessage reply;
	messenger.SendMessage(&windowMessage, &reply);

	return 0;
}
